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

/* xbdRamDisk.c - Ram disk */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/erfLib.h>
#include <os/logLib.h>

#include <fs/xbd.h>
#include <fs/bio.h>
#include <fs/xbdRamDisk.h>

#define XBD_MIN_BLKSIZE   512
#define XBD_MAX_BLKSIZE   8096

typedef struct {
    XBD           xbd;
    XBD_GEOMETRY  geometry;
    SEMAPHORE     mutex;
    char *        data;
} XBD_RAMDISK;

LOCAL int xbdRamDiskIoctl (XBD *xbd, unsigned  command, void *data);
LOCAL int xbdRamDiskStrategy (XBD *xbd, struct bio *pBio);
LOCAL int xbdRamDiskDump (XBD *xbd, lblkno_t lblkno, void *data, size_t nBytes);

LOCAL XBD_FUNCS xbdRamDiskFuncs = {
    &xbdRamDiskIoctl,
    &xbdRamDiskStrategy,
    &xbdRamDiskDump,
};

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
    ) {
    XBD_RAMDISK *  xbdRamDisk;
    STATUS         status;
    device_t       device;
    devname_t      devname;
    unsigned       numBlocks;

    /*
     * Fail if ...
     * 1. <blockSize> is not a power of two or is out of range.
     * 2. <name> is NULL or does not start with '/'.
     * 3. <name> has more than one '/'.
     */

    if (((blockSize & (blockSize - 1)) != 0) || (diskSize < XBD_MIN_BLKSIZE) ||
        (blockSize < XBD_MIN_BLKSIZE) || (blockSize > XBD_MAX_BLKSIZE) ||
        (name == NULL) || (name[0] != '/') ||
        (strchr (name + 1, '/') != NULL)) {
        errnoSet (EINVAL);
        return (NULLDEV);
    }

    /* Allocate memory for XBD_RAMDISK structure, name and ramdisk data. */
    xbdRamDisk = (XBD_RAMDISK *) malloc (diskSize + sizeof (XBD_RAMDISK));
    if (xbdRamDisk == NULL) {
        /* errno set by malloc() */
        return (NULLDEV);
    }

    /* Copy the name to the XBD_RAMDISK. */
    strncpy (devname, name, XBD_MAX_NAMELEN);
    if (flag) {
        strncat (devname, ":0", XBD_MAX_NAMELEN);
    }

    semMInit (&xbdRamDisk->mutex, SEM_Q_PRIORITY);

    numBlocks = diskSize / blockSize;

    /*
     * The geometry information must be faked for dosfs (aka FAT file systems).
     * An arbitrary (power of two) number has been chosen for the number of
     * sectors per track.
     */

    xbdRamDisk->geometry.sectorsPerTrack = 32;
    xbdRamDisk->geometry.numHeads        = 1;
    xbdRamDisk->geometry.numCylinders    = numBlocks /
                                           xbdRamDisk->geometry.sectorsPerTrack;

    xbdRamDisk->data = (char *) &xbdRamDisk[1];
    memset (xbdRamDisk->data, 0, diskSize);

    status = xbdAttach (&xbdRamDisk->xbd, &xbdRamDiskFuncs, devname, blockSize,
                        numBlocks, &device);

    if (status != OK) {
        /* errno set by xbdAttach() */
        free (xbdRamDisk);
        return (NULLDEV);
    }

    /* If partitions should be supported */
    if (flag) {
        /* Raise primary insert event */
        status = erfEventRaise (xbdEventCategory, xbdEventPrimaryInsert,
                                ERF_ASYNC_PROCESS, (void *) device, NULL);
        if (status != OK) {
            xbdDetach ((XBD *) xbdRamDisk);
            free (xbdRamDisk);
            return (NULLDEV);
        }
    }
    else {
        /* Raise secondary insert event */
        status = erfEventRaise (xbdEventCategory, xbdEventSecondaryInsert,
                                ERF_ASYNC_PROCESS, (void *) device, NULL);
        if (status != OK) {
            xbdDetach ((XBD *) xbdRamDisk);
            free (xbdRamDisk);
            return (NULLDEV);
        }
    }

    /*
     * TODO:
     * Eventually some other steps will be required to wait for any devices
     * on top of this to initialize (such as partitions or file systems).
     * If the ramdisk was created (as opposed to initialized), then we can
     * expect it to be mounted with rawFS.
     */

    return (device);
}

/***************************************************************************
 *
 * xbdRamDiskDevDelete - delete an XBD ramdisk device
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdRamDiskDevDelete (
    device_t  device
    ) {
    return (ERROR);
}

/***************************************************************************
 *
 * xbdRamDiskIoctl -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdRamDiskIoctl (
    XBD *     xbd,
    unsigned  command,
    void *    data
    ) {
    XBD_RAMDISK *  xbdRamDisk = (XBD_RAMDISK *) xbd;
    XBD_GEOMETRY * xbdGeometry;

    switch (command) {
        case XBD_GEOMETRY_GET:
            if (data == NULL) {
                return (EINVAL);
            }
            xbdGeometry = (XBD_GEOMETRY *) data;
            memcpy (xbdGeometry, &xbdRamDisk->geometry, sizeof (XBD_GEOMETRY));
            break;

        default:
            return (ENOSYS);
    }

    return (OK);
}

/***************************************************************************
 *
 * xbdRamDiskStrategy -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdRamDiskStrategy (
    XBD *         xbd,
    struct bio *  pBio
    ) {
    XBD_RAMDISK *  xbdRamDisk = (XBD_RAMDISK *) xbd;
    lblkno_t       lblkno;
    struct bio     *bio;
    char *         pData;

    /* LATER: Add BIO parameter to checks. */
    
    semTake (&xbdRamDisk->mutex, WAIT_FOREVER);

    if (pBio->bio_flags == BIO_READ) {
        for (bio = pBio; bio != NULL; bio = bio->bio_chain) {
            pData = xbdRamDisk->data +
                    (bio->bio_blkno * xbdRamDisk->xbd.blkSize);
#ifdef DIAGNOSTIC
            logMsg("Rd:\t%d\t%d\t%x\n",
                   (ARG) (int) bio->bio_blkno,
                   (ARG) (int) bio->bio_bcount,
                   (ARG) pData,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0);
#endif
            memcpy  (bio->bio_data, pData, bio->bio_bcount);
        }
    } else {
        for (bio = pBio; bio != NULL; bio = bio->bio_chain) {
            pData = xbdRamDisk->data +
                    (bio->bio_blkno * xbdRamDisk->xbd.blkSize);
#ifdef DIAGNOSTIC
            logMsg("Wr:\t%d\t%d\t%x\n",
                   (ARG) (int) bio->bio_blkno,
                   (ARG) (int) bio->bio_bcount,
                   (ARG) pData,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0);
#endif
            memcpy  (pData, bio->bio_data, bio->bio_bcount);
        }
    }

    pBio->bio_resid = 0;
    bio_done (pBio, OK);

    semGive (&xbdRamDisk->mutex);

    return (OK);
}

/***************************************************************************
 *
 * xbdRamDiskDump - 
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdRamDiskDump (
    XBD *       xbd,
    lblkno_t    lblkno,
    void *      data,
    size_t      nBytes
    ) {
    XBD_RAMDISK *  xbdRamDisk = (XBD_RAMDISK *) xbd;
    char *         pData;

    /*
     * xbdDump() may get called upon some sort of critical system error.
     * At such a time, the kernel may have suffered some unrecoverable
     * error, so it may not be safe to use kernel primitives such as
     * semTake() or semGive().
     */

    pData = xbdRamDisk->data + (lblkno * xbdRamDisk->xbd.blkSize);
    memcpy (pData, data, nBytes);

    return (OK);
}

