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

/* xbdRamDisk.h - Ramdisk XBD device */

#ifndef _xbdRamDisk_h
#define _xbdRamDisk_h

#include <vmx.h>
#include <fs/xbd.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* functions */

/***************************************************************************
 *
 * xbdRamDiskDevCreate - create an XBD ramdisk device
 *
 * RETURNS: XBD's device ID on success, NULLDEV on error
 */

device_t xbdRamDiskDevCreate (
    unsigned      blockSize,   /* block size (bytes) */
    unsigned      diskSize,    /* disk size (bytes) */
    BOOL          flag,        /* TRUE if partitions supported, else FALSE */
    const char *  name         /* ramdisk name */
    );

/***************************************************************************
 *
 * xbdRamDiskDevDelete - delete an XBD ramdisk device
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdRamDiskDevDelete (
    device_t  device
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _xbdRamDisk_h */

