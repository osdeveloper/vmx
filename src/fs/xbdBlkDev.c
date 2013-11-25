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

/* xbdBlkDev.c - BLK_DEV to XBD interface converter */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/erfLib.h>
#include <os/logLib.h>

#include <fs/xbd.h>
#include <fs/bio.h>
#include <fs/xbdBlkDev.h>

typedef struct {
    XBD         xbd;
    BLK_DEV    *bd;
    SEMAPHORE   mutex;
} XBD_BLK_DEV;

LOCAL int xbdBlkDevIoctl (XBD *xbd, unsigned  command, void *data);
LOCAL int xbdBlkDevStrategy (XBD *xbd, struct bio *pBio);
LOCAL int xbdBlkDevDump (XBD *xbd, lblkno_t lblkno, void *data, size_t nBytes);

LOCAL XBD_FUNCS xbdBlkDevFuncs = {
    &xbdBlkDevIoctl,
    &xbdBlkDevStrategy,
    &xbdBlkDevDump,
};

/***************************************************************************
 *
 * xbdBlkDevCreate - create a XBD BLK interface wrapper device
 *
 * RETURNS: XBD's device ID on success, NULLDEV on error
 */

device_t xbdBlkDevCreate (
    BLK_DEV    *  bd,          /* block device */
    const char *  name         /* device name */
    ) {
    XBD_BLK_DEV *  xbdBlkDev;
    STATUS         status;
    device_t       device;
    devname_t      devname;

    /*
     * Fail if ...
     * 1. <name> is NULL or does not start with '/'.
     * 2. <name> has more than one '/'.
     */

    if ((name == NULL) || (name[0] != '/') ||
        (strchr (name + 1, '/') != NULL)) {
        errnoSet (EINVAL);
        return (NULLDEV);
    }

    /* Allocate memory for XBD_BLK_DEV structure */
    xbdBlkDev = (XBD_BLK_DEV *) malloc (sizeof (XBD_BLK_DEV));
    if (xbdBlkDev == NULL) {
        /* errno set by malloc() */
        return (NULLDEV);
    }

    /* Copy the name to the XBD_BLK_DEV. */
    strncpy (devname, name, XBD_MAX_NAMELEN);
#ifdef partition
    if (flag) {
        strncat (devname, ":0", XBD_MAX_NAMELEN);
    }
#endif

    semMInit (&xbdBlkDev->mutex, SEM_Q_PRIORITY);

    /* Set block device field */
    xbdBlkDev->bd = bd;

    status = xbdAttach (&xbdBlkDev->xbd, &xbdBlkDevFuncs, devname,
                        bd->bd_bytesPerBlk, bd->bd_nBlocks, &device);

    if (status != OK) {
        /* errno set by xbdAttach() */
        free (xbdBlkDev);
        return (NULLDEV);
    }

#ifdef partition
    /* If partitions should be supported */
    if (flag) {
        /* Raise primary insert event */
        status = erfEventRaise (xbdEventCategory, xbdEventPrimaryInsert,
                                ERF_ASYNC_PROCESS, (void *) device, NULL);
        if (status != OK) {
            xbdDetach ((XBD *) xbdBlkDev);
            free (xbdBlkDev);
            return (NULLDEV);
        }
    }
    else {
#endif
        /* Raise secondary insert event */
        status = erfEventRaise (xbdEventCategory, xbdEventSecondaryInsert,
                                ERF_ASYNC_PROCESS, (void *) device, NULL);
        if (status != OK) {
            xbdDetach ((XBD *) xbdBlkDev);
            free (xbdBlkDev);
            return (NULLDEV);
        }
#ifdef partition
    }
#endif

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
 * xbdBlkDevDelete - delete a XBD BLK interface wrapper device
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdBlkDevDelete (
    device_t    device,
    BLK_DEV **  ppbd
    ) {
    return (ERROR);
}

/***************************************************************************
 *
 * xbdBlkDevIoctl -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdBlkDevIoctl (
    XBD *     xbd,
    unsigned  command,
    void *    data
    ) {
    XBD_BLK_DEV  *  xbdBlkDev = (XBD_BLK_DEV *) xbd;
    XBD_GEOMETRY *  xbdGeometry;

    switch (command) {
        case XBD_GEOMETRY_GET:
            if (data == NULL) {
                return (EINVAL);
            }
            xbdGeometry = (XBD_GEOMETRY *) data;
            xbdGeometry->sectorsPerTrack = xbdBlkDev->bd->bd_blksPerTrack;
            xbdGeometry->numHeads = xbdBlkDev->bd->bd_nHeads;
            xbdGeometry->numCylinders = xbdBlkDev->bd->bd_nBlocks /
                                        xbdBlkDev->bd->bd_blksPerTrack;
            break;

        default:
            if (xbdBlkDev->bd->bd_ioctl(
                    xbdBlkDev->bd,
                    (int) command,
                    (int) data
                    ) != OK) {
                return (ERROR);
            }
            break;
    }

    return (OK);
}

/***************************************************************************
 *
 * xbdBlkDevStrategy -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdBlkDevStrategy (
    XBD *         xbd,
    struct bio *  pBio
    ) {
    XBD_BLK_DEV *  xbdBlkDev = (XBD_BLK_DEV *) xbd;
    lblkno_t       lblkno;
    struct bio     *bio;
    char *         pData;
    int            nBlks;

    /* LATER: Add BIO parameter to checks. */
    
    semTake (&xbdBlkDev->mutex, WAIT_FOREVER);

    if (pBio->bio_flags == BIO_READ) {
        for (bio = pBio; bio != NULL; bio = bio->bio_chain) {
            nBlks = bio->bio_bcount / xbdBlkDev->xbd.blkSize;
            if (bio->bio_bcount % xbdBlkDev->xbd.blkSize) {
                nBlks++;
            }

#ifdef DIAGNOSTIC
            logMsg("Rd:\t%d\t%d\t%x\n",
                   (ARG) (int) bio->bio_blkno,
                   (ARG) (int) bio->bio_bcount,
                   (ARG) nBlks,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0);
#endif

            xbdBlkDev->bd->bd_blkRd(
                xbdBlkDev->bd,
                (int) bio->bio_blkno,
                nBlks,
                bio->bio_data
                );
        }
    } else {
        for (bio = pBio; bio != NULL; bio = bio->bio_chain) {
            nBlks = bio->bio_bcount / xbdBlkDev->xbd.blkSize;
            if (bio->bio_bcount % xbdBlkDev->xbd.blkSize) {
                nBlks++;
            }

#ifdef DIAGNOSTIC
            logMsg("Rw:\t%d\t%d\t%x\n",
                   (ARG) (int) bio->bio_blkno,
                   (ARG) (int) bio->bio_bcount,
                   (ARG) nBlks,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0);
#endif

            xbdBlkDev->bd->bd_blkWrt(
                xbdBlkDev->bd,
                (int) bio->bio_blkno,
                nBlks,
                bio->bio_data
                );
        }
    }

    pBio->bio_resid = 0;
    bio_done (pBio, OK);

    semGive (&xbdBlkDev->mutex);

    return (OK);
}

/***************************************************************************
 *
 * xbdBlkDevDump - 
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdBlkDevDump (
    XBD *       xbd,
    lblkno_t    lblkno,
    void *      data,
    size_t      nBytes
    ) {
    XBD_BLK_DEV *  xbdBlkDev = (XBD_BLK_DEV *) xbd;
    int            nBlks;
    char *         pData;

    /*
     * xbdDump() may get called upon some sort of critical system error.
     * At such a time, the kernel may have suffered some unrecoverable
     * error, so it may not be safe to use kernel primitives such as
     * semTake() or semGive().
     */

    nBlks = nBytes / xbdBlkDev->xbd.blkSize;
    if (nBytes % xbdBlkDev->xbd.blkSize) {
        nBlks++;
    }

    xbdBlkDev->bd->bd_blkRd(xbdBlkDev->bd, (int) lblkno, nBlks, pData);

    return (OK);
}

