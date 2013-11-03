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

/* echoDrv.c - I/O Echo driver */

/* includes */

#include <vmx.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <os/iosLib.h>
#include <os/echoDrv.h>

/* locals */

LOCAL int echoNumber = 0;

/******************************************************************************
 * echoOpen - open an echo file
 *
 * RETURNS: Pointer to device
 */

LOCAL int echoOpen(
    ECHO_DEV *pEchoDev,         /* device pointer */
    const char *filename,       /* not used */
    int flags,                  /* test for O_CREAT */
    int mode                    /* not used */
    )
{
    return (int) pEchoDev;
}

/******************************************************************************
 * echoCreate - create an echo file
 *
 * RETURNS: ERROR
 */

LOCAL int echoCreate(
    ECHO_DEV *pEchoDev,         /* device pointer */
    const char *filename,       /* not used */
    int mode,                   /* not used */
    const char *symlink         /* not used */
    )
{
    return ERROR;
}

/******************************************************************************
 * echoRead - read from an echo file
 *
 * RETURNS: number of bytes read
 */

LOCAL int echoRead(
    ECHO_DEV *pEchoDev,         /* device pointer */
    void *buffer,               /* buffer to zero */
    int maxBytes                /* number of bytes to zero */
    )
{
    return tyRead(&pEchoDev->tyDev, buffer, maxBytes);
}
 
/******************************************************************************
 * echoWrite - write to an echo file
 *
 * RETURNS: number of bytes written
 */

LOCAL int echoWrite(
    ECHO_DEV *pEchoDev,         /* device pointer */
    void *buffer,               /* buffer to write */
    int maxBytes                /* number of bytes to write */
    )
{
    return tyWrite(&pEchoDev->tyDev, buffer, maxBytes);
}

/******************************************************************************
 * echoIoctl - Set/get status from an echo file
 *
 * RETURNS: Status from call
 */

LOCAL int echoIoctl(
    ECHO_DEV *pEchoDev,         /* device pointer */
    int command,                /* not used */
    int arg                     /* not used */
    )
{
    return tyIoctl(&pEchoDev->tyDev, command, arg);
}

/******************************************************************************
 * echoDelete - Delete an echo file
 *
 * RETURNS: ERROR
 */

LOCAL int echoDelete(
    ECHO_DEV *pEchoDev,         /* device pointer */
    const char *filename,       /* not used */
    mode_t mode                 /* not used */
    )
{
    /* No files on echo device to delete */
    return ERROR;
}

/******************************************************************************
 * echoClose - Close an echo file
 *
 * RETURNS: OK
 */

LOCAL int echoClose(
    ECHO_DEV *pEchoDev          /* device pointer */
    )
{
    return OK;
}

/******************************************************************************
 * echoTxStartup - Device startup
 *
 * RETURNS: 0
 */

int echoTxStartup(
    void
    )
{
    return 0;
}

/******************************************************************************
 * echoDrvInit - Initialize echo device
 *
 * RETURNS: OK or ERROR
 */

STATUS echoDrvInit(
    void
    )
{
    STATUS status;

    if (echoNumber > 0)
    {
        status = ERROR;
    }
    else
    {
        /* Install driver */
        echoNumber = iosDrvInstall(
                         (FUNCPTR) echoCreate,
                         (FUNCPTR) echoDelete,
                         (FUNCPTR) echoOpen,
                         (FUNCPTR) echoClose,
                         (FUNCPTR) echoRead,
                         (FUNCPTR) echoWrite,
                         (FUNCPTR) echoIoctl
                         );
        if (echoNumber == ERROR)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * echoDevCreate - Create echo device
 *
 * RETURNS: OK or ERROR
 */

STATUS echoDevCreate(
    char *name,
    int readBufferSize,
    int writeBufferSize
    )
{
    STATUS status;
    ECHO_DEV *pEchoDev;

    if (echoNumber <= 0)
    {
        status = ERROR;
    }
    else
    {
        pEchoDev = (ECHO_DEV *) malloc(sizeof(ECHO_DEV));
        if (pEchoDev == NULL)
        {
            status = ERROR;
        }
        else
        {
            if (tyDevInit(&pEchoDev->tyDev,
                           readBufferSize,
                           writeBufferSize,
                           (FUNCPTR) echoTxStartup
                           ) != OK)
            {
                free(pEchoDev);
                status = ERROR;
            }
            else
            {
                status = iosDevAdd(
                             &pEchoDev->tyDev.devHeader,
                             name,
                             echoNumber
                             );
                if (status != OK)
                {
                    free(pEchoDev);
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * echoDrvNumber - Get echo device number
 *
 * RETURNS: Driver number
 */

int echoDrvNumber(
    void
    )
{
    return echoNumber;
}

