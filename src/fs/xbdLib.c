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

/* xbdLib.c - Extended Block Device Library */

/*
DESCRIPTION
This library implements the extended block device module; other modules will
implement specific extended block devices.  This module is dependent upon the
event reporting framework (ERF), and so if used, it must be initialized AFTER
the ERF.

At present, it supports two ERF types--xbdEventRemove and xbdEventMediaChanged.
Each one is to be used when appropriate.  For example, some devices (such as
floppy drives or CD-ROM drives) allow have removable media.  Should the media
be removed or replaced from such a device, it should use xbdEventMediaChanged.
Other devices (such as RAM disks) when removed (or deleted) should use the
xbdEventRemove ERF type.
*/

/* includes */

#include <vmx.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arch/intArchLib.h>
#include <os/erfLib.h>

#include <fs/bio.h>
#include <fs/xbd.h>

/* defines */

/* typedefs */

/* globals */

int xbdEventCategory        = -1;
int xbdEventRemove          = -1;
int xbdEventMediaChanged    = -1;
int xbdEventPrimaryInsert   = -1;
int xbdEventSecondaryInsert = -1;
int xbdEventSoftInsert      = -1;
int xbdEventInstantiated    = -1;

/* locals */

LOCAL XBD ** xbdDeviceList = NULL;    /* list index is device_t value */
LOCAL int    xbdMaxDevices = 0;

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
    ) {
    if (xbdDeviceList != NULL) {
        return (OK);
    }

    if (maxXbds <= 0) {
        errnoSet (S_xbdLib_INVALID_PARAM);
        return (ERROR);
    }

    xbdDeviceList = (XBD **) malloc (maxXbds * sizeof (XBD *));
    if (xbdDeviceList == NULL) {
        /* errno set by malloc() */
        return (ERROR);
    }

    xbdMaxDevices = maxXbds;
    memset (xbdDeviceList, 0, maxXbds * sizeof (XBD *));

    if ((erfCategoryAllocate (&xbdEventCategory) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventRemove) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventMediaChanged) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventPrimaryInsert) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventSecondaryInsert) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventSoftInsert) != OK) ||
        (erfTypeAllocate (xbdEventCategory, &xbdEventInstantiated) != OK)) {
        /* errno set by ERF routines */
        free (xbdDeviceList);
        xbdDeviceList = NULL;
        return (ERROR);
    }

    return (OK);
}

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
    ) {
        device_t device;
    int      i;
    int      level;

    if (strlen (xbdName) + 1 >= XBD_MAX_NAMELEN) {
        errnoSet (S_xbdLib_NAME_TOO_LONG);
        return (ERROR);
    }

    /* Check if a device with that name already exists */
    if (xbdDevFind (&device, xbdName) == OK) {
        errnoSet (S_xbdLib_NAME_DUPLICATE);
        return (ERROR);
    }

    xbd->blkSize = blkSize;
    xbd->nBlks = nBlks;
    xbd->xbdFuncs.xbdIoctl    = xbdFuncs->xbdIoctl;
    xbd->xbdFuncs.xbdStrategy = xbdFuncs->xbdStrategy;
    xbd->xbdFuncs.xbdDump     = xbdFuncs->xbdDump;
    strcpy (xbd->name, xbdName);

    INT_LOCK (level);
    for (i = 0; i < xbdMaxDevices; i++) {
        if (xbdDeviceList[i] == NULL) {
            xbdDeviceList[i] = xbd;
            break;
        }
    }
    INT_UNLOCK (level);

    if (i == xbdMaxDevices) {
        errnoSet (S_xbdLib_DEVICE_TABLE_FULL);
        return  (ERROR);
    }

    *pDevice = i;

    return (OK);
}

/***************************************************************************
 *
 * xbdDetach - detach an XBD
 *
 * RETURNS: OK on success, ERROR otherwise
 */

int xbdDetach (
    XBD  * xbd
    ) {
    int  i;
    int  level;

    INT_LOCK (level);

    for (i = 0; i < xbdMaxDevices; i++) {
        if (xbdDeviceList[i] == xbd) {
            xbdDeviceList[i] = NULL;
            INT_UNLOCK (level);
            return (OK);
        }
    }

    INT_UNLOCK (level);

    errnoSet (S_xbdLib_DEVICE_NOT_FOUND);
    return (ENODEV);
}

/***************************************************************************
 *
 * xbdNBlocks - get the number of blocks in the device
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdNBlocks (
    device_t    device,
    lblkno_t *  pNumBlks
    ) {
    XBD *  xbd;
    int    level;

    INT_LOCK (level);

    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    *pNumBlks = xbd->nBlks;

    INT_UNLOCK (level);

    return (OK);
}

/***************************************************************************
 *
 * xbdBlockSize - get the device block size
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdBlockSize (
    device_t    device,
    unsigned *  pBlkSize
    ) {
    XBD *  xbd;
    int    level;

    INT_LOCK (level);

    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    *pBlkSize = xbd->blkSize;

    INT_UNLOCK (level);

    return (OK);
}

/***************************************************************************
 *
 * xbdSize - get the device total size
 *
 * RETURNS: OK on success, ENODEV on error
 */

int xbdSize (
    device_t device,
    long long *pSize
    ) {
    XBD *  xbd;
    int level;
    unsigned blkSize;
    lblkno_t numBlks;

    INT_LOCK (level);

    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    blkSize = xbd->blkSize;
    numBlks = xbd->nBlks;

    *pSize = (long long) blkSize * (long long) numBlks;

    INT_UNLOCK (level);

    return (OK);
}

/***************************************************************************
 *
 * xbdStrategy - Call strategy function
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdStrategy (
    device_t      device,
    struct bio *  pBio
    ) {
    XBD *  xbd;
    int    (*strategy)(XBD *, struct bio *);
    int    level;

    INT_LOCK (level);

    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    strategy = xbd->xbdFuncs.xbdStrategy;

    INT_UNLOCK (level);

    return (strategy (xbd, pBio));
}

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
    ) {
    XBD *  xbd;
    int    (*func)(XBD *, unsigned, void *);
    int    level;
    STATUS status;

    INT_LOCK (level);

    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    func = xbd->xbdFuncs.xbdIoctl;

    INT_UNLOCK (level);

    /* Process general commands */
    switch (command) {

      case XBD_STACK_COMPLETE:
          status = erfEventRaise (xbdEventCategory, xbdEventInstantiated,
                                  ERF_ASYNC_PROCESS, (void *) xbd, NULL);
      break;

      case XBD_HARD_EJECT:
          status = erfEventRaise (xbdEventCategory, xbdEventMediaChanged,
                                  ERF_ASYNC_PROCESS, (void *) device, NULL);

      break;

      case XBD_SOFT_EJECT:
          status = erfEventRaise (xbdEventCategory, xbdEventMediaChanged,
                                  ERF_ASYNC_PROCESS, (void *) device, NULL);
          if (status != OK) {
              break;
          }

          status = erfEventRaise (xbdEventCategory, xbdEventSoftInsert,
                                  ERF_ASYNC_PROCESS, (void *) device, NULL);
      break;

      default:
          status = func (xbd, command, data);
      break;

    }

    return (status);
}

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
    ) {
    XBD *  xbd;
    int    (*func)(XBD *, lblkno_t, void *, size_t);
    int    level;

    INT_LOCK (level);
    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }

    func = xbd->xbdFuncs.xbdDump;

    INT_UNLOCK (level);

    return (func (xbd, lblkno, data, nBytes));
}

/***************************************************************************
 *
 * xbdDevName - Get device name
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdDevName (
    device_t   device,
    devname_t  devname
    ) {
    XBD *   xbd;
    int     level;

    INT_LOCK (level);
    if ((device >= xbdMaxDevices) || ((xbd = xbdDeviceList[device]) == NULL)) {
        INT_UNLOCK (level);
        return (ENODEV);
    }
    INT_UNLOCK (level);

    strcpy (devname, xbd->name);
    return (OK);
}

/***************************************************************************
 *
 * xbdDevFind - Find device with name
 *
 * RETURNS: OK on success, error otherwise
 */

int xbdDevFind (
    device_t  *     pDevice,
    const devname_t devname
    ) {
    int i;

    for (i = 0; i < xbdMaxDevices; i++) {
        if (xbdDeviceList[i] != NULL) {
            if (strcmp (xbdDeviceList[i]->name, devname) == 0) {
                *pDevice = i;
                return (OK);
            }
        }
    }

    errnoSet (S_xbdLib_DEVICE_NOT_FOUND);
    return (ENODEV);
}

