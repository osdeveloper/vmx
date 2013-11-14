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

/* rawfsLib.c - raw file system library */

/* includes */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <os/ffsLib.h>
#include <os/erfLib.h>

#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/rawfsLib.h>

/*
 * TODO:
 * As rawfs is the default file system and must be in the image if file systems
 * are to be used, it is unclear at this point if any special functionality
 * or cases must be considered when ejecting or deleting the raw file system
 * device.  More thought must be given to this.
 */

LOCAL BOOL rawfsLibInitialized = FALSE;
LOCAL int  rawfsMaxBuffers;
LOCAL int  rawfsMaxFiles;

/***************************************************************************
 *
 * rawfsEject - eject the raw file system
 *
 * RETURNS: N/A
 */

LOCAL void rawfsEject (
    int     category,
    int     type,
    void *  pEventData,   /* device to eject */
    void *  pUserData     /* ptr to device structure */
    ) {
    device_t     device;
    RAWFS_DEV *  pFsDev;

    if ((category != xbdEventCategory) ||
        ((type != xbdEventRemove) && (type != xbdEventMediaChanged))) {
        return;
    }

    device = (device_t) pEventData;
    pFsDev = (RAWFS_DEV *) pUserData;

    /* If this event is not for us, then return */
    if (pFsDev->volDesc.device != device) {
        return;
    }

    /* 
     * Unregister the registered events.  Then inform the vnode layer to
     * unmount the raw file system.
     */

    erfHandlerUnregister (xbdEventCategory, xbdEventRemove,
                          rawfsEject, pFsDev);
    erfHandlerUnregister (xbdEventCategory, xbdEventMediaChanged,
                          rawfsEject, pFsDev);

    mountEject (pFsDev->volDesc.pMount, type == xbdEventMediaChanged);
}

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
    ) {
    return (OK);        /* rawfs probe always succeeds */
}

/***************************************************************************
 *
 * rawfsLibInit - initialize the rawfs library
 *
 * RETURNS: OK
 */

STATUS rawfsLibInit (
    int  maxBufs,
    int  maxFiles,
    int  reserved1,
    int  reserved2
    ) {
    if (rawfsLibInitialized) {
        return (OK);
    }

    rawfsMaxBuffers = maxBufs;
    rawfsMaxFiles   = maxFiles;

    rawfsLibInitialized = TRUE;
    return (OK);
}

/***************************************************************************
 *
 * rawfsDevCreate - create a raw file system device
 *
 * This routine creates a raw file system device.
 *
 * RETURNS: ptr to the file system device on success, NULL otherwise
 */

RAWFS_DEV *  rawfsDevCreate (
    char *    pDevName,
    device_t  device,
    int       numBufs,
    int       maxFiles,
    int       reserved2,
    int       reserved1
    ) {
    mount_t *    pMount;
    RAWFS_DEV *  pFsDev;
    int          error;

    /*
     * If the library is not initialized or no device name has been supplied,
     * then return NULL and set the errno.
     */   

    if ((!rawfsLibInitialized) || (pDevName == NULL) ||
        (pDevName[0] == '\0')) {
        errnoSet (EINVAL);
        return (NULL);
    }

    if (numBufs < RAWFS_MIN_BUFFERS) {
        numBufs = RAWFS_MIN_BUFFERS;
    }

    error = mountCreate (&rawVfsOps, device, maxFiles, pDevName, &pMount);
    if (error != OK) {
        errnoSet (error);
        return (NULL);
    }

    pFsDev = (RAWFS_DEV *) pMount->mnt_data;
    mountBufFree(pMount);

    error = mountBufAlloc (pMount, numBufs, pFsDev->volDesc.blkSize);
    if (error != OK) {
        mountDelete (pMount, 0);
        errnoSet (error);
        return (NULL);
    }

    return (pFsDev);
}

/***************************************************************************
 *
 * rawfsDevDelete - delete a raw file system device
 *
 * This routine deletes a raw file system device.  It should only be called
 * if the underlying device is no longer in use.  This is typically used upon
 * the handling of an ejection event.
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS rawfsDevDelete (
    RAWFS_DEV *  pFsDev    /* ptr to the raw file system device */
    ) {
    if (INT_RESTRICT () != OK) {
        return (ERROR);
    }

    if (pFsDev == NULL) {
        return (ERROR);
    }

    mountDelete (pFsDev->volDesc.pMount, 0);

    return (OK);
}

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
    ) {
    RAWFS_DEV *pFsDev;
    RAWFS_VOLUME_DESC *pVolDesc;
    unsigned sectorSize;
    long long totalSize;

    pFsDev = (RAWFS_DEV *) pMount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    memset (pFsDev, 0, sizeof(RAWFS_DEV));

    /* Get setup from device */
    if ((xbdBlockSize (pMount->mnt_dev, &sectorSize) != OK) ||
        (xbdSize (pMount->mnt_dev, &totalSize) != OK)) {
        return (EINVAL);
    }

    /* Inititalize syncer vnode */
    pMount->mnt_syncer->v_ops = (struct vnode_ops *) &rawVops;

    /* Inititalize semaphore */
    pFsDev->bioSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
    if (pFsDev->bioSem == NULL) {
        return (ENOBUFS);
    }

    /* Inititalize volume desciptor */
    pVolDesc->pMount = pMount;
    pVolDesc->device = pMount->mnt_dev;
    pVolDesc->blkSize = sectorSize;
    pVolDesc->blkSize2 = ffsMsb(pVolDesc->blkSize) - 1;
    pVolDesc->secPerBlk2 = 1 + pVolDesc->blkSize2 - ffsMsb(sectorSize);
    pVolDesc->secPerBlk = (1 << pVolDesc->blkSize2);
    pVolDesc->diskSize = totalSize;

    return (OK);
}

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
    ) {
    RAWFS_DEV *  pFsDev;
    int          fd;

    pFsDev = rawfsDevCreate (pDevName, device, rawfsMaxBuffers,
                             rawfsMaxFiles, 0, 0);
    if (pFsDev == NULL) {
        /* errno set by rawfsDevCreate() */
        return (ERROR);
    }

    fd = open (pDevName, O_RDONLY, 0777);
    if (fd < 0) {
        goto errorReturn;
    }

    close (fd);

    if ((erfHandlerRegister (xbdEventCategory, xbdEventRemove,
                             rawfsEject, pFsDev, 0) != OK) ||
        (erfHandlerRegister (xbdEventCategory, xbdEventMediaChanged,
                             rawfsEject, pFsDev, 0) != OK)) {
        goto errorReturn;
    }

    xbdIoctl (device, XBD_STACK_COMPLETE, NULL);
    fsPathAddedEventRaise (pDevName);
    return (OK);

errorReturn:

    rawfsDevDelete (pFsDev);
    return (ERROR);
}

