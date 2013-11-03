/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2008 Surplus Users Ham Society
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

/* nullDrvP.h - I/O Null Driver private header */

#ifndef _nullDrvP_h
#define _nullDrvP_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <os/iosLib.h>

/* Functions */

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
    );

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
    );

/******************************************************************************
 * nullRead - read from a null file
 *
 * RETURNS: number of bytes read
 */

int nullRead(
    DEV_HEADER *pDevHeader,     /* not used */
    void *buffer,               /* buffer to zero */
    int maxBytes                /* number of bytes to zero */
    );

/******************************************************************************
 * nullWrite - write to a null file
 *
 * RETURNS: number of bytes "written"
 */

int nullWrite(
    DEV_HEADER *pDevHeader,     /* not used */
    void *buffer,               /* buffer to write */
    int maxBytes                /* number of bytes to write */
    );

/******************************************************************************
 * nullIoctl - Set I/O options for null device
 *
 * RETURNS: ERROR
 */

int nullIoctl(
    DEV_HEADER *pDevHeader,     /* not used */
    int command,                /* not used */
    int arg                     /* not used */
    );

/******************************************************************************
 * nullDelete - Delete null device
 *
 * RETURNS: ERROR
 */

int nullDelete(
    DEV_HEADER *pDevHeader,     /* not used */
    const char *filename,       /* not used */
    mode_t mode                 /* not used */
    );

/******************************************************************************
 * nullClose - Close null device 
 *
 * RETURNS: OK
 */

int nullClose(
    DEV_HEADER *pDevHeader      /* not used */
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _nullDrvP_h */

