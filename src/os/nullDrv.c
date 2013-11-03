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

/* nullDrvLib.c - I/O Null Driver */

/*
 * DESCRIPTION
 * The null driver should always exist in a VMX system that uses IO.  Of the
 * seven IO dwarves (open, close, creat, remove, ioctl, read, write) none of
 * the driver function pointers should be NULL.  This means that the code to
 * invoke the driver functions can be simplified and NOT need to check for NULL.
*/

/* includes */

#include <vmx.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <os/iosLib.h>

/******************************************************************************
 * nullOpen - open a null file
 *
 * RETURNS: NULL if opening a file, ERROR if creating a file
 */

int nullOpen(
    DEV_HEADER *pDevHeader,     /* not used */
    const char *filename,       /* not used */
    int flags,                  /* test for O_CREAT */
    int mode                    /* not used */
    )
{
    int ret;

    if (flags & O_CREAT)
    {
        ret = ERROR;
    }
    else
    {
        ret = (int) NULL;
    }

    return ret;
}

/******************************************************************************
 * nullCreate - create a null file
 *
 * RETURNS: ERROR
 */

int nullCreate(
    DEV_HEADER *pDevHeader,     /* not used */
    const char *filename,       /* not used */
    int mode,                   /* not used */
    const char *symlink         /* not used */
    )
{
    return ERROR;
}

/******************************************************************************
 * nullRead - read from a null file
 *
 * RETURNS: number of bytes read
 */

int nullRead(
    DEV_HEADER *pDevHeader,     /* not used */
    void *buffer,               /* buffer to zero */
    int maxBytes                /* number of bytes to zero */
    )
{
    memset(buffer, 0, maxBytes);

    return maxBytes;
}
 
/******************************************************************************
 * nullWrite - write to a null file
 *
 * RETURNS: number of bytes "written"
 */

int nullWrite(
    DEV_HEADER *pDevHeader,     /* not used */
    void *buffer,               /* buffer to write */
    int maxBytes                /* number of bytes to write */
    )
{
    return maxBytes;
}

/******************************************************************************
 * nullIoctl - Set I/O options for null device
 *
 * RETURNS: ERROR
 */

int nullIoctl(
    DEV_HEADER *pDevHeader,     /* not used */
    int command,                /* not used */
    int arg                     /* not used */
    )
{
    /*
     * Hmmm ... should we process any commands here?
     * Probably.  To be done later.
     */

    return ERROR;
}

/******************************************************************************
 * nullDelete - Delete null device
 *
 * RETURNS: ERROR
 */

int nullDelete(
    DEV_HEADER *pDevHeader,     /* not used */
    const char *filename,       /* not used */
    mode_t mode                 /* not used */
    )
{
    /* No files on null device to delete */
    return ERROR;
}

/******************************************************************************
 * nullClose - Close null device
 *
 * RETURNS: OK
 */

int nullClose(
    DEV_HEADER *pDevHeader      /* not used */
    )
{
    return OK;
}

