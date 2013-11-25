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

/* ramDrv.h - Ramdisk driver */

#ifndef _ramDrv_h
#define _ramDrv_h

#include <vmx.h>
#include <fs/blkIo.h>

#define RAMDRV_DEFAULT_DISK_SIZE        51200
#define RAMDRV_DEFAULT_SEC_SIZE         512

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef struct
{
    BLK_DEV        blkDev;
    char          *ramAddr;
    int            blkOffset;
} RAM_DEV;

/* Functions */

/******************************************************************************
 * ramDevCreate - Create ramdisk device
 *
 * RETURNS: Ramdisk block-device
 */

BLK_DEV* ramDevCreate(
    char *ramAddr,
    int   bytesPerBlk,
    int   blksPerTrack,
    int   nBlocks,
    int   blkOffset
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ramDrv_h */

