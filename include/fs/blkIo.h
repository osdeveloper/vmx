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

/* blkIo.h - Block device I/O */

#ifndef _blkIo_h
#define _blkIo_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    FUNCPTR       bd_blkRd;
    FUNCPTR       bd_blkWrt;
    FUNCPTR       bd_ioctl;
    FUNCPTR       bd_reset;
    FUNCPTR       bd_statusChk;
    BOOL          bd_removable;
    unsigned long bd_bytesPerBlk;
    unsigned long bd_nBlocks;
    unsigned long bd_blksPerTrack;
    unsigned long bd_nHeads;
    int           bd_retry;
    int           bd_mode;
    BOOL          bd_readyChanged;
} BLK_DEV;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _blkIo_h */

