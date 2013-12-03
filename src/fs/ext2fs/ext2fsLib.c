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

/* ext2fsLib.c - Ext2 filesystem library */

#include <stdlib.h>
#include <fcntl.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/semLib.h>
#include <os/erfLib.h>
#include <fs/mount.h>
#include <fs/fsEventUtilLib.h>
#include <fs/ext2fsLib.h>
#include <fs/bio.h>
#include <fs/xbd.h>

/* globals */

BOOL ext2fsLibInstalled = FALSE;

/* locals */

LOCAL int  ext2fsMaxBuffers;
LOCAL int  ext2fsMaxFiles;

LOCAL void ext2fsDiskProbeDone (
    struct bio * pBio
    );

LOCAL void ext2fsEject (
    int    category,
    int    type,
    void * eventData,
    void * userData
    );

/******************************************************************************
 *
 * ext2fsDiskProbe - probe media for ext2 filesystem
 *
 * RETURNS: OK if ext2 filesystem found, otherwise error
 */

int ext2fsDiskProbe (
    device_t  device
    ) {
    lblkno_t                 nSectors;
    unsigned                 sectorSize;
    int                      error;
    void *                   pData;
    SEM_ID                   semId;
    struct bio               bio;
    EXT2FS_SUPERBLOCK_DISK * pSuperBlkDisk;

    error = xbdNBlocks (device, &nSectors);
    if (error != OK) {
        return (error);
    }

    error = xbdBlockSize (device, &sectorSize);
    if (error != OK) {
        return (error);
    }

    pData = bio_alloc (device, (EXT2FS_SUPERBLK_SIZE + sectorSize - 1) /
                               sectorSize);
    if (pData == NULL) {
        return (ENOMEM);
    }

    semId = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
    if (semId == NULL) {
        error = errno;
        bio_free (pData);
        return (error);
    }

    /* Read superblock */
    bio.bio_dev     = device;
    bio.bio_blkno   = EXT2FS_SUPERBLK_SIZE / sectorSize;
    bio.bio_bcount  = (sectorSize > EXT2FS_SUPERBLK_SIZE)
                          ? sectorSize
                          : EXT2FS_SUPERBLK_SIZE;
    bio.bio_error   = OK;
    bio.bio_chain   = NULL;
    bio.bio_flags   = BIO_READ;
    bio.bio_data    = pData;
    bio.bio_caller1 = semId;
    bio.bio_done    = ext2fsDiskProbeDone;
    bio.bio_resid   = 0;

    xbdStrategy (device, &bio);
    semTake (semId, WAIT_FOREVER);

    error = bio.bio_error;
    if ((error == OK) && (bio.bio_resid != 0)) {
        semDelete (semId);
        bio_free (pData);
        return (EIO);
    }

    /* check the magic number and rev. level in superblock */
    pSuperBlkDisk = (EXT2FS_SUPERBLOCK_DISK *) pData;
    if (sectorSize > EXT2FS_SUPERBLK_SIZE) {
        pSuperBlkDisk = (EXT2FS_SUPERBLOCK_DISK *) (((u_int8_t *) pData) +
                                                    EXT2FS_SUPERBLK_SIZE);
    }

    if (ext2fsDiskToHost16 (pSuperBlkDisk->s_magic) != EXT2FS_SUPERBLK_MAGIC) {
        semDelete (semId);
        bio_free (pData);
        return (ENOEXEC);
    }

    if ((ext2fsDiskToHost32 (pSuperBlkDisk->s_rev_level) != 0) &&
        (ext2fsDiskToHost32 (pSuperBlkDisk->s_rev_level) != 1)) {
        semDelete (semId);
        bio_free (pData);
        return (ENOEXEC);
    }

    semDelete (semId);
    bio_free (pData);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsLibInit - initialize ext2 filesystem library
 *
 * RETURNS: OK
 */

STATUS ext2fsLibInit (
    int  maxBufs,
    int  maxFiles,
    int  reserved2,
    int  reserved1
    ) {

    if (ext2fsLibInstalled == TRUE) {
        return (OK);
    }

    ext2fsMaxBuffers = maxBufs;
    ext2fsMaxFiles   = maxFiles;

    ext2fsLibInstalled = TRUE;

    return (OK);
}

/******************************************************************************
 *
 * ext2fsDevCreate - create an ext2 filesystem device
 *
 * RETURNS: pointer to the device on success, otherwise NULL
 */

EXT2FS_DEV *  ext2fsDevCreate (
    char *    pDevName,
    device_t  device,
    int       numBufs,
    int       maxFiles,
    int       reserved2,
    int       reserved1
    ) {
    struct mount * pMount;
    EXT2FS_DEV *   pExt2fsDev;
    int            error;

    if ((ext2fsLibInstalled != TRUE) || (pDevName == NULL)) {
        errnoSet (EINVAL);
        return (NULL);
    }

    /* check that numBufs is not less than the minimum */
    if (numBufs < EXT2FS_MIN_BUFFERS) {
        numBufs = EXT2FS_MIN_BUFFERS;
    }

    error = mountCreate (&ext2fsVfsOps, device, maxFiles, pDevName, &pMount);
    if (error != OK) {
        errnoSet (error);
        return (NULL);
    }
    pExt2fsDev = (EXT2FS_DEV *) pMount->mnt_data;
    mountBufFree (pMount);

    error = mountBufAlloc (pMount, numBufs, pExt2fsDev->ext2fsVolDesc.blkSize);
    if (error != OK) {
        mountDelete (pMount, 0);
        errnoSet (error);
        return (NULL);
    }

    return (pExt2fsDev);
}

/******************************************************************************
 *
 * ext2fsDevDelete - delete an Ext2 file system device
 *
 * RETURNS: OK on success, otherwise ERROR
 */

STATUS ext2fsDevDelete (
    EXT2FS_DEV * pExt2fsDev
    ) {

    if (INT_RESTRICT () != OK) {
        return (ERROR);
    }

    if (pExt2fsDev == NULL) {
        return (ERROR);
    }

    mountDelete (pExt2fsDev->ext2fsVolDesc.pMount, 0);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsDevCreate2 - create ext2 filesystem device
 *
 * RETURNS: OK on success, otherwise ERROR
 */

STATUS ext2fsDevCreate2 (
    device_t  device,
    char *    pDevName
    ) {
    EXT2FS_DEV * pExt2fsDev;
    int          fd;

    pExt2fsDev = ext2fsDevCreate (pDevName, device, ext2fsMaxBuffers,
                                  ext2fsMaxFiles, 0, 0);
    if(pExt2fsDev == NULL) {
        return (NULL);
    }

    /* check access rights */
    fd = open (pDevName, O_RDONLY, 0777);
    if (fd < 0) {
        ext2fsDevDelete (pExt2fsDev);
        return (ERROR);
    }

    close (fd);

    if ((erfHandlerRegister (xbdEventCategory, xbdEventRemove,
                             ext2fsEject, pExt2fsDev, 0) != OK) ||
        (erfHandlerRegister (xbdEventCategory, xbdEventMediaChanged,
                             ext2fsEject, pExt2fsDev, 0) != OK)) {
        ext2fsDevDelete (pExt2fsDev);
        return (ERROR);
    }

    xbdIoctl (device, XBD_STACK_COMPLETE, NULL);
    fsPathAddedEventRaise (pDevName);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsEject - eject ext2 filesystem
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsEject (
    int    category,
    int    type,
    void * eventData,
    void * userData
    ) {
    device_t     device;
    EXT2FS_DEV * pExt2fsDev;

    if ((category == xbdEventCategory) &&
        ((type == xbdEventRemove) || (type == xbdEventMediaChanged))) {
        device     = (device_t) eventData;
        pExt2fsDev = (EXT2FS_DEV *) userData;

        /* If not releated event return early */
        if (pExt2fsDev->ext2fsVolDesc.device != device) {
            return;
        }

        /* Unregister handlers and eject */
        erfHandlerUnregister (xbdEventCategory, xbdEventRemove,
                              ext2fsEject, pExt2fsDev);
        erfHandlerUnregister (xbdEventCategory, xbdEventMediaChanged,
                              ext2fsEject, pExt2fsDev);
        mountEject (pExt2fsDev->ext2fsVolDesc.pMount,
                    type == xbdEventMediaChanged);
    }
}

/******************************************************************************
 *
 * ext2fsDiskProbeDone - bio operations for ext2 filesystem probe done
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsDiskProbeDone (
    struct bio * pBio
    ) {

    semGive ((SEM_ID) pBio->bio_caller1);
}

