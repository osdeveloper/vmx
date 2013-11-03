/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2013 Surplus Users Ham Society
 *
 *   Real VMX is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Real VMX is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
 */

/* iosLib.c - I/O system driver library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <vmx/errnoLib.h>
#include <os/private/nullDrvP.h>
#include <os/ioLib.h>
#include <os/iosLib.h>

/* GLOBALS */
char             iosDefaultPath[PATH_MAX + 1];
int              iosMaxDrv;
int              iosMaxFd;
DRV_ENTRY       *iosDrvTable;
FD_ENTRY        *iosFdTable;
DL_LIST          iosDevList;
VOIDFUNCPTR      iosFdNewHook = NULL;
VOIDFUNCPTR      iosFdFreeHook = NULL;

/* LOCALS */
LOCAL BOOL       iosLibInstalled = FALSE;
LOCAL SEMAPHORE  iosSem;
LOCAL DEV_HEADER iosNullDevHeader;

LOCAL void iosLock(
    void
    );

LOCAL void iosUnlock(
    void
    );

LOCAL DEV_HEADER* iosDevMatch(
    char *name
    );

/******************************************************************************
 * iosLibInit - Initialize library
 *
 * RETURNS: OK or ERROR
 */

STATUS iosLibInit(
    int max_drv,
    int max_fd,
    char *nullDevName
    )
{
    STATUS status;
    int i, size;

    if (iosLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Setup max variables */
        iosMaxDrv = max_drv;
        iosMaxFd = max_fd;

        /* Setup empty path */
        strcpy(iosDefaultPath, "");

        /* Init file table */
        size = iosMaxFd * sizeof(FD_ENTRY);

        iosFdTable = malloc(size);
        if (iosFdTable == NULL)
        {
            status = ERROR;
        }
        else
        {
            memset(iosFdTable, 0, size);
            for (i = 0; i < iosMaxFd; i++)
            {
                iosFdFree(STD_FIX(i));
            }

            /* Init driver table */
            size = iosMaxDrv * sizeof(DRV_ENTRY);

            iosDrvTable = (DRV_ENTRY *) malloc(size);
            if (iosDrvTable == NULL)
            {
                free(iosFdTable);
                status = ERROR;
            }
            else
            {
                memset(iosDrvTable, 0, size);
                for (i = 0; i < iosMaxDrv; i++)
                {
                    iosDrvTable[i].dev_used = FALSE;
                }

                /* Initialize semahpore */
                if (semBInit(&iosSem, SEM_Q_PRIORITY, SEM_FULL) != OK)
                {
                    free(iosFdTable);
                    free(iosDrvTable);
                    status = ERROR;
                }
                else
                {
                    /* Initialize linked list */
                    if (dllInit(&iosDevList) != OK)
                    {
                        free(iosFdTable);
                        free(iosDrvTable);
                        semDestroy(&iosSem, TRUE);
                        status = ERROR;
                    }
                    else
                    {
                        /* Add null device */
                        iosDrvTable[0].dev_create = (FUNCPTR) nullCreate;
                        iosDrvTable[0].dev_delete = (FUNCPTR) nullDelete;
                        iosDrvTable[0].dev_open   = (FUNCPTR) nullOpen;
                        iosDrvTable[0].dev_close  = (FUNCPTR) nullClose;
                        iosDrvTable[0].dev_read   = (FUNCPTR) nullRead;
                        iosDrvTable[0].dev_write  = (FUNCPTR) nullWrite;
                        iosDrvTable[0].dev_ioctl  = (FUNCPTR) nullIoctl;
                        iosDevAdd(&iosNullDevHeader, nullDevName, 0);

                        iosLibInstalled = TRUE;
                        status = OK;
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * iosDrvInstall - Install a driver
 *
 * RETURNS: Driver number or ERROR
 */

int iosDrvInstall(
    FUNCPTR createMethod,
    FUNCPTR deleteMethod,
    FUNCPTR openMethod,
    FUNCPTR closeMethod,
    FUNCPTR readMethod,
    FUNCPTR writeMethod,
    FUNCPTR ioctlMethod
    )
{
    int drvNumber;
    DRV_ENTRY *pDrvEntry;

    if ((createMethod == NULL) || (deleteMethod == NULL) ||
        (openMethod == NULL)   || (closeMethod == NULL)  ||
        (readMethod == NULL)   || (writeMethod == NULL)  ||
        (ioctlMethod == NULL))
    {
        errnoSet(S_iosLib_NULL_DRIVER_METHOD);
        drvNumber = ERROR;
    }
    else
    {
        iosLock();

        /* Find a free place in table */
        for (drvNumber = 1; drvNumber < iosMaxDrv; drvNumber++)
        {
            if (iosDrvTable[drvNumber].dev_used == FALSE)
            {
                pDrvEntry = &iosDrvTable[drvNumber];
                break;
            }
        }

        if (pDrvEntry == NULL)
        {
            errnoSet(S_iosLib_DRIVER_GLUT);
            iosUnlock();
            drvNumber = ERROR;
        }
        else
        {
            /* Setup struct */
            pDrvEntry->dev_used   = TRUE;
            pDrvEntry->dev_create = createMethod;
            pDrvEntry->dev_delete = deleteMethod;
            pDrvEntry->dev_open   = openMethod;
            pDrvEntry->dev_close  = closeMethod;
            pDrvEntry->dev_read   = readMethod;
            pDrvEntry->dev_write  = writeMethod;
            pDrvEntry->dev_ioctl  = ioctlMethod;

            iosUnlock();
        }
    }

    return drvNumber;
}

/******************************************************************************
 * iosDrvRemove - Remove a driver
 *
 * RETURNS: OK or ERROR
 */

STATUS iosDrvRemove(
    int drvNumber,
    BOOL forceClose
    )
{
    STATUS status;
    DEV_HEADER *pDevHeader;
    int fd;
    FD_ENTRY *pFdEntry;
    DRV_ENTRY *pDrvEntry;
    FUNCPTR drvClose;

    pDrvEntry = &iosDrvTable[drvNumber];
    drvClose = pDrvEntry->dev_close;

    iosLock();

    /* Check if any open files */
    for (fd = 0; fd < iosMaxFd; fd++)
    {
        pFdEntry = &iosFdTable[fd];
        if ((pFdEntry->used == TRUE) &&
            (pFdEntry->pDevHeader->drvNumber == drvNumber))
        {
            if (forceClose != TRUE)
            {
                /* Don't close, ERROR */
                iosUnlock();
                status = ERROR;
                break;
            }
            else
            {
                /* Close file */
                if (drvClose != NULL)
                {
                    (*drvClose)(pFdEntry->value);
                }
                iosFdFree(STD_FIX(fd));
                status = OK;
            }
        }
    }

    if (status == OK)
    {
        /* Remove devices for driver */
        for (pDevHeader = (DEV_HEADER *) DLL_HEAD(&iosDevList);
             pDevHeader != NULL;
             pDevHeader = (DEV_HEADER *) DLL_NEXT(&pDevHeader->node))
        {
            if (pDevHeader->drvNumber == drvNumber)
            {
                free(pDevHeader->name);
                dllRemove(&iosDevList, &pDevHeader->node);
            }
        }

        /* Clear struct */
        pDrvEntry->dev_used   = FALSE;
        pDrvEntry->dev_create = NULL;
        pDrvEntry->dev_delete = NULL;
        pDrvEntry->dev_open   = NULL;
        pDrvEntry->dev_close  = NULL;
        pDrvEntry->dev_read   = NULL;
        pDrvEntry->dev_write  = NULL;
        pDrvEntry->dev_ioctl  = NULL;

        iosUnlock();
    }

    return status;
}

/******************************************************************************
 * iosDefaultPathSet - Set default path
 *
 * RETURNS: N/A
 */

void iosDefaultPathSet(
    char *name
    )
{
    strcpy(iosDefaultPath, name);
}

/******************************************************************************
 * iosDevAdd - Add a device entry
 *
 * RETURNS: OK or ERROR
 */

STATUS iosDevAdd(
    DEV_HEADER *pDevHeader,
    char *name,
    int drvNumber
    )
{
    STATUS status;
    DEV_HEADER *pMatch;

    /* Find best matching device */
    pMatch = iosDevMatch(name);

    /* Check if no full matching device exists */
    if ((pMatch != NULL) && (strcmp(pMatch->name, name) == 0))
    {
        errnoSet(S_iosLib_DUPLICATE_DEVICE_NAME);
        status = ERROR;
    }
    else
    {
        /* Store name */
        pDevHeader->name = (char *) malloc(strlen(name) + 1);
        if (pDevHeader->name == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Store driver number */
            pDevHeader->drvNumber = drvNumber;

            /* Copy device name */
            strcpy(pDevHeader->name, name);

            iosLock();

            /* Add to list */
            dllAdd(&iosDevList, &pDevHeader->node);

            iosUnlock();
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * iosDevDelete - Delete a device entry
 *
 * RETURNS: N/A
 */

void iosDevDelete(
    DEV_HEADER *pDevHeader
    )
{
    iosLock();

    free(pDevHeader->name);
    dllRemove(&iosDevList, &pDevHeader->node);

    iosUnlock();
}

/******************************************************************************
 * iosDevFind - Find a device by name
 *
 * RETURNS: Device header or NULL
 */

DEV_HEADER* iosDevFind(
    char *name,
    char **pNameTail
    )
{
    DEV_HEADER *pDevHeader;

    /* Find best match */
    pDevHeader = iosDevMatch(name);
    if (pDevHeader != NULL)
    {
        /* Find match */
        *pNameTail = name + strlen(pDevHeader->name);
    }
    else
    {
        /* Get default match */
        pDevHeader = iosDevMatch(iosDefaultPath);
        *pNameTail = name;
    }

    if (pDevHeader == NULL)
    {
        errnoSet(S_iosLib_DEVICE_NOT_FOUND);
    }

    return pDevHeader;
}

/******************************************************************************
 * iosDevMatch - Find device with matching name
 *
 * RETURNS: DEV_HEADER pointer or NULL
 */

LOCAL DEV_HEADER* iosDevMatch(
    char *name
    )
{
    DEV_HEADER *pDevHeader, *pBestDevHeader = NULL;
    int len, maxLen = 0;

    iosLock();

    for (pDevHeader = (DEV_HEADER *) DLL_HEAD(&iosDevList);
         pDevHeader != NULL;
         pDevHeader = (DEV_HEADER *) DLL_NEXT(&pDevHeader->node))
    {
        /* Get length */
        len = strlen(pDevHeader->name);

        if (strncmp(pDevHeader->name, name, len) == 0)
        {
            /* Name is within device name */
            if (len > maxLen)
            {
                pBestDevHeader = pDevHeader;
                maxLen = len;
            }
        }
    }

    iosUnlock();

    return pBestDevHeader;
}

/******************************************************************************
 * iosNextDevGet - Get next device in list
 *
 * RETURNS: DEV_HEADER pointer or NULL
 */

DEV_HEADER* iosNextDevGet(
    DEV_HEADER *pDevHeader
    )
{
    DEV_HEADER *pNext;

    if (pDevHeader == NULL)
    {
        pNext = (DEV_HEADER *) DLL_HEAD(&iosDevList);
    }
    else
    {
        pNext = (DEV_HEADER *) DLL_NEXT(&pDevHeader->node);
    }

    return pNext;
}

/******************************************************************************
 * iosFdValue - Get driver specific value
 *
 * RETURNS: Driver value or ERROR
 */

ARG iosFdValue(
    int fd
    )
{
    int f;
    ARG value;

    /* Validate and get fd */
    f = STD_MAP(fd);

    if (((f >= 0) && (f < iosMaxFd)) && 
        (iosFdTable[f].used != FALSE))
    {
        value = iosFdTable[f].value;
    }
    else
    {
        errnoSet(S_iosLib_INVALID_FILE_DESCRIPTOR);
        value = (ARG) 0;
    }

    return value;
}

/******************************************************************************
 * iosFdDevFind - Get driver for file descriptor
 *
 * RETURNS: Deveice header pointer or ERROR
 */

DEV_HEADER* iosFdDevFind(
    int fd
    )
{
    DEV_HEADER *pDevHeader;
    int f;

    /* Validate and get fd */
    f = STD_MAP(fd);

    if (((f >= 0) && (f < iosMaxFd)) && 
        (iosFdTable[f].used != FALSE))
    {
        pDevHeader = iosFdTable[f].pDevHeader;
    }
    else
    {
        errnoSet(S_iosLib_INVALID_FILE_DESCRIPTOR);
        pDevHeader = NULL;
    }

    return pDevHeader;
}

/******************************************************************************
 * iosFdFree - Free a file descriptor
 *
 * RETURNS: N/A
 */

void iosFdFree(
    int fd
    )
{
    FD_ENTRY *pFdEntry;
    int f;

    /* Get and validate fd */
    f = STD_MAP(fd);

    /* Get entry */
    pFdEntry = FD_CHECK(f);

    if (pFdEntry != NULL)
    {
        if (pFdEntry->name != NULL)
        {
            if (pFdEntry->name != pFdEntry->pDevHeader->name)
            {
                free(pFdEntry->name);
            }
            pFdEntry->name = NULL;
        }

        /* Call free hook */
        if (iosFdFreeHook != NULL)
        {
            (*iosFdFreeHook)(fd);
        }

        pFdEntry->used = FALSE;
    }
}

/******************************************************************************
 * iosFdSet - Set file descriptor
 *
 * RETURNS: N/A
 */

STATUS iosFdSet(
    int fd,
    DEV_HEADER *pDevHeader,
    char *name,
    ARG value
    )
{
    STATUS status;
    FD_ENTRY *pFdEntry;
    char *ptr;

    if (fd >= iosMaxFd)       /* Bounds check */
    {
        status = ERROR;
    }
    else
    {
        /* Initialize locals */
        pFdEntry = &iosFdTable[STD_UNFIX(fd)];
        if (pFdEntry->used != TRUE)       /* Do nothing if fd not in use. */
        {
            status = ERROR;
        }
        else
        {
            if (pFdEntry->name != NULL)
            {
                free(pFdEntry->name);
            }

            if (name != NULL)
            {
                ptr = (char *) malloc(strlen(name) + 1);
                if (ptr == NULL)
                {
                    status = ERROR;
                }
                else
                {
                    strcpy(ptr, name);
                    status = OK;
                }
            }
            else
            {
                ptr = NULL;
                status = OK;
            }

            if (status == OK)
            {
                pFdEntry->name       = ptr;
                pFdEntry->pDevHeader = pDevHeader;
                pFdEntry->value      = value;
            }
        }
    }

    return status;
}

/******************************************************************************
 * ioFdNew - Allocate and initialize new file descriptor
 *
 * RETURNS: File descriptor or ERROR
 */

int iosFdNew(
    DEV_HEADER *pDevHeader,
    char *name,
    ARG value
    )
{
    int fd;
    FD_ENTRY *pFdEntry;

    /* Initailize locals */
    pFdEntry = NULL;

    iosLock();

    /* Loop for free fd */
    for (fd  = 0; fd < iosMaxFd; fd++)
    {
        if (iosFdTable[fd].used == FALSE)
        {
            /* Fond one */
            pFdEntry           = &iosFdTable[fd];
            pFdEntry->used     = TRUE;
            pFdEntry->obsolete = FALSE;
            pFdEntry->name     = NULL;
            break;
        }
    }

    iosUnlock();

    fd = STD_FIX(fd);
    if (fd >= iosMaxFd)
    {
        errnoSet(S_iosLib_TO_MANY_OPEN_FILES);
        fd = ERROR;
    }
    else
    {
        if (iosFdSet(fd, pDevHeader, name, value) != OK)
        {
            fd = ERROR;
        }
        else
        {
            /* Call hook */
            if (iosFdNewHook != NULL)
            {
                (*iosFdNewHook)(fd);
            }
        }
    }

    return fd;
}

/******************************************************************************
 * iosCreate - Invoke driver function to create file
 *
 * RETURNS: OK or ERROR
 */

int iosCreate(
    DEV_HEADER *pDevHeader,
    char *filename,
    int mode,
    const char *symlink
    )
{
    int status;
    FUNCPTR func;

    func = iosDrvTable[pDevHeader->drvNumber].dev_create;
    status = (*func)(pDevHeader, filename, mode, symlink);

    return status;
}

/******************************************************************************
 * iosDelete - Invoke driver function to delete file
 *
 * RETURNS: OK or ERROR
 */

int iosDelete(
    DEV_HEADER *pDevHeader,
    char *filename,
    mode_t mode
    )
{
    int status;
    FUNCPTR func;

    func = iosDrvTable[pDevHeader->drvNumber].dev_delete;
    status = (*func)(pDevHeader, filename, mode);

    return status;
}

/******************************************************************************
 * iosOpen - Invoke driver function to open file
 *
 * RETURNS: File descriptor or ERROR
 */

int iosOpen(
    DEV_HEADER *pDevHeader,
    char *filename,
    int flags,
    int mode
    )
{
    int status;
    FUNCPTR func;

    func = iosDrvTable[pDevHeader->drvNumber].dev_open;
    status = (*func)(pDevHeader, filename, flags, mode);

    return status;
}

/******************************************************************************
 * iosClose - Invoke driver function to close file
 *
 * RETURNS: OK or ERROR
 */

int iosClose(
    int fd
    )
{
    int status;
    FD_ENTRY *pFdEntry;
    FUNCPTR func;
    int fDesc;

    fDesc = STD_MAP(fd);
    pFdEntry = FD_CHECK(fDesc);
    if (pFdEntry == NULL)
    {
        status = ERROR;
    }
    else
    {
        func = iosDrvTable[pFdEntry->pDevHeader->drvNumber].dev_close;
        status = (*func)(pFdEntry->value);

        /* Remove fd */
        iosFdFree(STD_FIX(fDesc));
    }

    return status;
}

/******************************************************************************
 * iosRead - Invoke driver function to read file
 *
 * RETURNS: Bytes read or ERROR
 */

int iosRead(
    int fd,
    void *buffer,
    int maxBytes
    )
{
    int status;
    FD_ENTRY *pFdEntry;
    FUNCPTR func;
    int fDesc;

    fDesc = STD_MAP(fd);
    pFdEntry = FD_CHECK(fDesc);
    if (pFdEntry == NULL)
    {
        status = ERROR;
    }
    else
    {
        func = iosDrvTable[pFdEntry->pDevHeader->drvNumber].dev_read;
        status = (*func)(pFdEntry->value, buffer, maxBytes);
    }

    return status;
}

/******************************************************************************
 * iosWrite - Invoke driver function to write file
 *
 * RETURNS: Bytes written or ERROR
 */

int iosWrite(
    int fd,
    void *buffer,
    int maxBytes
    )
{
    int status;
    FD_ENTRY *pFdEntry;
    FUNCPTR func;
    int fDesc;

    fDesc = STD_MAP(fd);
    pFdEntry = FD_CHECK(fDesc);
    if (pFdEntry == NULL)
    {
        status = ERROR;
    }
    else
    {
        func = iosDrvTable[pFdEntry->pDevHeader->drvNumber].dev_write;
        status = (*func)(pFdEntry->value, buffer, maxBytes);
    }

    return status;
}

/******************************************************************************
 * iosIoctl - Invoke driver function ioctl
 *
 * RETURNS: OK or ERROR
 */

int iosIoctl(
    int fd,
    int function,
    ARG arg
    )
{
    int status;
    FD_ENTRY *pFdEntry;
    FUNCPTR func;
    int fDesc;

    fDesc = STD_MAP(fd);
    pFdEntry = FD_CHECK(fDesc);
    if (pFdEntry == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Check if name get */
        if (function == FIOGETNAME)
        {
            strcpy((char *) arg, pFdEntry->name);
            status = OK;
        }
        else
        {
            func = iosDrvTable[pFdEntry->pDevHeader->drvNumber].dev_ioctl;
            status = (*func)(pFdEntry->value, function, arg);
        }
    }

    return status;
}

/******************************************************************************
 * iosLock - Get exclusive access to I/O struct
 *
 * RETURNS: N/A
 */

LOCAL void iosLock(
    void
    )
{
    semTake(&iosSem, WAIT_FOREVER);
}

/******************************************************************************
 * iosUnlock - Release exclusive access to I/O struct
 *
 * RETURNS: N/A
 */

LOCAL void iosUnlock(
    void
    )
{
    semGive(&iosSem);
}

/******************************************************************************
 * iosNullWrite - Null write
 *
 * RETURNS: Number parameter
 */

LOCAL int iosNullWrite(
    int dummy,
    void *buf,
    int n
    )
{
    return n;
}

