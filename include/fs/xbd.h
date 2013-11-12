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

/* xbd.h - extended block device header */

#ifndef __XBD_H
#define __XBD_H

/* includes */

#include <vmx.h>
#include <ostool/moduleNumber.h>

/* defines */

#define S_xbdLib_NAME_TOO_LONG      (M_xbdLib | 0x0001)
#define S_xbdLib_DEVICE_TABLE_FULL  (M_xbdLib | 0x0002)
#define S_xbdLib_INVALID_PARAM      (M_xbdLib | 0x0003)
#define S_xbdLib_NAME_DUPLICATE     (M_xbdLib | 0x0004)
#define S_xbdLib_DEVICE_NOT_FOUND   (M_xbdLib | 0x0005)

#define XBD_GEOMETRY_GET            0xebd00001
#define XBD_STACK_COMPLETE          0xebd00002
#define XBD_HARD_EJECT              0xebd00003
#define XBD_SOFT_EJECT              0xebd00004

#define XBD_MAX_NAMELEN   32

#define NULLDEV  (-1)

/* typedefs */

typedef int   device_t;        /* ??? at least for now ??? */

typedef struct xbd_geometry {
    unsigned  sectorsPerTrack;
    unsigned  numHeads;
    unsigned  numCylinders;
} XBD_GEOMETRY;

typedef char  devname_t[XBD_MAX_NAMELEN];

/* structs */
struct bio;
struct xbd;

typedef struct xbd_funcs {
    int  (*xbdIoctl) (struct xbd *pXbd, unsigned command, void *data);
    int  (*xbdStrategy) (struct xbd *pXbd, struct bio *bio);
    int  (*xbdDump) (struct xbd *pXbd, lblkno_t blk, void *data, size_t nBytes);
} XBD_FUNCS;

/* When creating a new XBD device structure, XBD shall be its first field. */
typedef struct xbd {
    unsigned      blkSize;
    lblkno_t      nBlks;
    XBD_FUNCS     xbdFuncs;
    devname_t     name;
} XBD;

/* externs */

IMPORT int  xbdEventCategory;
IMPORT int  xbdEventRemove;
IMPORT int  xbdEventMediaChanged;
IMPORT int  xbdEventPrimaryInsert;
IMPORT int  xbdEventSecondaryInsert;
IMPORT int  xbdEventSoftInsert;
IMPORT int  xbdEventInstantiated;

/* functions */

/***************************************************************************
 *
 * xbdLibInit - initialize the XBD library
 *
 * This routine initializes the XBD library.  If used, it must be called
 * after erfLibInit().
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdLibInit (
    int  maxXbds
    );

/***************************************************************************
 *
 * xbdAttach - attach an XBD
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdAttach (
    XBD *            xbd,        /* ptr to XBD */
    XBD_FUNCS *      xbdFuncs,   /* ptr to XBD functions */
    const devname_t  xbdName,    /* name to give XBD device */
    unsigned         blkSize,    /* block size */
    lblkno_t         nBlks,      /* number of blocks on device */
    device_t *       pDevice     /* resulting device_t ID */
    );

/***************************************************************************
 *
 * xbdDetach - detach an XBD
 *
 * RETURNS: OK on success, ERROR otherwise
 */

int xbdDetach (
    XBD  * xbd
    );

/***************************************************************************
 *
 * xbdNBlocks - get the number of blocks in the device
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdNBlocks (
    device_t    device,
    lblkno_t *  pNumBlks
    );

/***************************************************************************
 *
 * xbdBlockSize - get the device block size
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdBlockSize (
    device_t    device,
    unsigned *  pBlkSize
    );

/***************************************************************************
 *
 * xbdSize - get the device total size
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdSize (
    device_t device,
    long long *pSize
    );

/***************************************************************************
 *
 * xbdStrategy - Call strategy function
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdStrategy (
    device_t      device,
    struct bio *  pBio
    );

/***************************************************************************
 *
 * xbdIoctl - I/O control function
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdIoctl (
    device_t  device,
    unsigned  command,
    void *    data
    );

/***************************************************************************
 *
 * xbdDump - Dump device
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdDump (
    device_t  device,
    lblkno_t  lblkno,
    void *    data,
    size_t    nBytes
    );

/***************************************************************************
 *
 * xbdDevName - Get device name
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdDevName (
    device_t   device,
    devname_t  devname
    );

/***************************************************************************
 *
 * xbdDevFind - Find device with name
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdDevFind (
    device_t  *     pDevice,
    const devname_t devname
    );

#endif

