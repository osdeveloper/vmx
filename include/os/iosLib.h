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

/* iosLib.h - I/O system driver library header */

#ifndef _iosLib_h
#define _iosLib_h

#include <tools/moduleNumber.h>

#define S_iosLib_DRIVER_GLUT                    (M_iosLib | 0x0001)
#define S_iosLib_DUPLICATE_DEVICE_NAME          (M_iosLib | 0x0002)
#define S_iosLib_DEVICE_NOT_FOUND               (M_iosLib | 0x0003)
#define S_iosLib_INVALID_FILE_DESCRIPTOR        (M_iosLib | 0x0004)
#define S_iosLib_TO_MANY_OPEN_FILES             (M_iosLib | 0x0005)
#define S_iosLib_NULL_DRIVER_METHOD             (M_iosLib | 0x0006)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <util/dllLib.h>

typedef struct
{
    DL_NODE  node;              /* node in linked list */
    int      drvNumber;         /* installed driver number index */
    char    *name;              /* ptr to device name */
} DEV_HEADER;

#include <os/private/iosLibP.h>

/******************************************************************************
 * iosLibInit - Initialize library
 *
 * RETURNS: OK or ERROR
 */

STATUS iosLibInit(
    int max_drv,
    int max_fd,
    char *nullDevName
    );

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
    );

/******************************************************************************
 * iosDrvRemove - Remove a driver
 *
 * RETURNS: OK or ERROR
 */

STATUS iosDrvRemove(
    int drvNumber,
    BOOL forceClose
    );

/******************************************************************************
 * iosDefaultPathSet - Set default path
 *
 * RETURNS: N/A
 */

void iosDefaultPathSet(
    char *name
    );

/******************************************************************************
 * iosDevAdd - Add a device entry
 *
 * RETURNS: OK or ERROR
 */

STATUS iosDevAdd(
    DEV_HEADER *pDevHeader,
    char *name,
    int drvNumber
    );

/******************************************************************************
 * iosDevDelete - Delete a device entry
 *
 * RETURNS: N/A
 */

void iosDevDelete(
    DEV_HEADER *pDevHeader
    );

/******************************************************************************
 * iosDevFind - Find a device by name
 *
 * RETURNS: Device header or NULL
 */

DEV_HEADER* iosDevFind(
    char *name,
    char **pNameTail
    );

/******************************************************************************
 * iosNextDevGet - Get next device in list
 *
 * RETURNS: DEV_HEADER pointer or NULL
 */

DEV_HEADER* iosNextDevGet(
    DEV_HEADER *pDevHeader
    );

/******************************************************************************
 * iosFdValue - Get driver specific value
 *
 * RETURNS: Driver value or ERROR
 */

ARG iosFdValue(
    int fd
    );

/******************************************************************************
 * iosFdDevFind - Get driver for file descriptor
 *
 * RETURNS: Deveice header pointer or ERROR
 */

DEV_HEADER* iosFdDevFind(
    int fd
    );

/******************************************************************************
 * iosFdFree - Free a file descriptor
 *
 * RETURNS: N/A
 */

void iosFdFree(
    int fd
    );

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
    );

/******************************************************************************
 * ioFdNew - Allocate and initialize new file descriptor
 *
 * RETURNS: File descriptor or ERROR
 */

int iosFdNew(
    DEV_HEADER *pDevHeader,
    char *name,
    ARG value
    );

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
    );

/******************************************************************************
 * iosDelete - Invoke driver function to delete file
 *
 * RETURNS: OK or ERROR
 */

int iosDelete(
    DEV_HEADER *pDevHeader,
    char *filename,
    mode_t mode
    );

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
    );

/******************************************************************************
 * iosClose - Invoke driver function to close file
 *
 * RETURNS: OK or ERROR
 */

int iosClose(
    int fd
    );

/******************************************************************************
 * iosRead - Invoke driver function to read file
 *
 * RETURNS: Bytes read or ERROR
 */

int iosRead(
    int fd,
    void *buffer,
    int maxBytes
    );

/******************************************************************************
 * iosWrite - Invoke driver function to write file
 *
 * RETURNS: Bytes written or ERROR
 */

int iosWrite(
    int fd,
    void *buffer,
    int maxBytes
    );

/******************************************************************************
 * iosIoctl - Invoke driver function ioctl
 *
 * RETURNS: OK or ERROR
 */

int iosIoctl(
    int fd,
    int function,
    ARG arg
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _iosLib_h */

