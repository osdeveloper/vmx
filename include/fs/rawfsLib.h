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

/* rawfsLib.h - Raw filesystem library */

#ifndef _rawfsLib_h
#define _rawfsLib_h

#include <vmx.h>
#include <vmx/semLib.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/vnode.h>

/* Defines */
#define RAWFS_MIN_BUFFERS       16              /* Minimum buffers */
#define RAWFS_ROOT_INODE        2               /* Root inode number */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct raw_volume_desc {
  device_t                      device;         /* XBD device */
  mount_t                       *pMount;        /* Pointer to mount struct */
  int                           blkSize;        /* Blocksize */
  int                           blkSize2;       /* Blocksize */
  long long                     diskSize;       /* Total disksize */
  int                           secPerBlk;      /* Sectors per block */
  int                           secPerBlk2;     /* Sectors per block */
  BOOL                          diskModified;   /* True if modified */
} RAWFS_VOLUME_DESC;

typedef struct raw_dev {
  RAWFS_VOLUME_DESC             volDesc;        /* Volume descriptor */
  SEM_ID                        bioSem;         /* Block I/O semaphore */
} RAWFS_DEV;

typedef struct raw_inode {
  int                           inode;          /* Inode number */
} RAWFS_INODE;

/* Globals */
IMPORT const vfsops_t           rawVfsOps;      /* Vfs operators */
IMPORT const vnode_ops_t        rawVops;        /* Vnode operators */

/* Functions */

STATUS rawfsLibInit (
    int  maxBufs,
    int  maxFiles,
    int  reserved1,
    int  reserved2
    );

/***************************************************************************
 *
 * rawfsMount - mount rawfs filesystem
 *
 * This routine attempts to mount and inititalize a rawfs filesystem
 *
 * RETURNS: OK on success, errno otherwise
 */

int rawfsMount(
    struct mount *pMount
    );

/***************************************************************************
 *
 * rawfsDiskProbe - probes the media for the rawfs file system
 *
 * Unlike other file systems, rawfs does not have any superblocks or other
 * disk structures to test/access.  The entire device is treated as one
 * giant file.
 *
 * RETURNS: OK
 */

int  rawfsDiskProbe (
    device_t  device    /* identify device to probe */
    );

/***************************************************************************
 *
 * rawfsDevCreate2 - create a raw file system device
 *
 * This routine creates a raw file system device.  It is meant to be called
 * upon a successful rawfs probe.
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS rawfsDevCreate2 (
    device_t  device,    /* XBD device ID */
    char *    pDevName   /* device name */
    );

/*******************************************************************************
*
* usrRawfsInit -
*/

STATUS usrRawfsInit(
    int maxBufs,
    int maxFiles,
    int reserved1,
    int reserved2
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _rawfsLib_h */

