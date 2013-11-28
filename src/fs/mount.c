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

/* mount.c - Mount filesytem utitilies */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <fs/mount.h>
#include <fs/vnode.h>
#include <fs/buf.h>

/* Imports */
IMPORT STATUS vfsMountInsert    (mount_t *pMount, char *pDevName);
IMPORT STATUS vfsMountEject     (mount_t *pMount);

/***************************************************************************
 *
 * mountCreate -
 *
 * RETURNS: OK on success, non-zero otherwise
 */

int mountCreate (
    const vfsops_t *  vfsops,
    unsigned    device,
    int         maxFiles,
    char *      devName,
    mount_t **  ppMount
    ) {
    SEM_ID     semId;
    mount_t *  mp;
    vnode_t *  vp;
    filedesc_t *  fdList;
    int        size;
    int        i;
    char *     data;
    int *      cookieData;

    if ((vfsops == NULL) || (devName == NULL) || (ppMount == NULL) ||
        (vfsops->devSize <= 0) || (vfsops->inodeSize <= 0) ||
        (maxFiles <= 0)) {
        return (EINVAL);
    }

    /* Allocate memory for the size of the device */
    mp = (mount_t *) malloc (sizeof (mount_t) + vfsops->devSize);
    if (mp == NULL) {
        return (ENOMEM);
    }

    /* Create syncer and file/logical vnodes.  <vp> will be syncer. */
    vp = vnodesCreate (mp, vfsops->inodeSize, maxFiles);
    if (vp == NULL) {
        free (mp);
        return (ENOMEM);
    }

    semId = semMCreate (SEM_Q_PRIORITY);
    if (semId == NULL) {
        vnodesDelete (vp, maxFiles);
        free (mp);
        return (ENOMEM);
    }

    fdList = malloc ((sizeof (filedesc_t) +
                      sizeof (cookieData) * vfsops->maxCookies) *
                     maxFiles);
    if (fdList == NULL) {
        vnodesDelete (vp, maxFiles);
        semDelete (semId);
        free (mp);
        return (ENOMEM);
    }

    cookieData = (int *) &fdList[maxFiles];
    memset(cookieData, sizeof (cookieData) * vfsops->maxCookies * maxFiles);
    for (i = 0; i < maxFiles; i++) {
        fdList[i].fd_vnode = NULL;             /* not using any vnode */
        fdList[i].fd_mode  = 0;                /* no mode (yet) */
        fdList[i].fd_off   = 0;                /* current file position */
        fdList[i].fd_inuse = FALSE;            /* filedesc not in use */
        fdList[i].fd_cookies = cookieData;     /* cookie data area */

        cookieData += vfsops->maxCookies;
    }
    
    mp->mnt_ops = (vfsops_t *) vfsops;
    mp->mnt_syncer = vp++;

    listInit (&mp->mnt_vlist);    /* lru list of available vnodes */
    listInit (&mp->mnt_vused);    /* list of used vnodes */
    listInit (&mp->mnt_buflist);  /* lru list of available buffers */

    for (i = 0; i < maxFiles; i++, vp++) {
        listAdd (&mp->mnt_vlist, &vp->v_node);
    }

    mp->mnt_bufsize = 0;    /* No buffers allocated yet */
    mp->mnt_maxfiles = maxFiles;
    mp->mnt_fdlist = fdList;
    mp->mnt_lock = semId;
    mp->mnt_dev = device;
                                        
    data = ((char *) mp) + sizeof (mount_t);
    mp->mnt_data = data;

    /* Inititalize filesystem */
    VFS_START (mp, 0);

    /* Add mount to vfs layer */
    if (vfsMountInsert (mp, devName) != OK) {
        return (ERROR);
    }

    *ppMount = mp;

    return (OK);
}

/***************************************************************************
 *
 * mountBufAlloc - create buffers for the FS
 *
 * This routine allcoates buffers for the file system to use.  It should
 * be called at some time after mountCreate().
 *
 * RETURNS: OK on success, non-zero otherwise
 */

int mountBufAlloc (
    mount_t *  pMount,   /* ptr to mount */
    int        nBufs,    /* # of buffers to create (> 0) */
    int        bufSize   /* size of each buffer (power of two) */
    ) {
    buf_t *  bp;
    char *   data;
    int      i;

    if ((pMount == NULL) || (nBufs <= 0) || (bufSize <= 0) ||
        ((bufSize & (bufSize - 1)) != 0)) {
        return (EINVAL);
    }

    bp = malloc ((sizeof (buf_t) + bufSize) * nBufs);
    if (bp == NULL) {
        return (ENOMEM);
    }

    memset (bp, 0, (sizeof (buf_t) + bufSize) * nBufs);

    pMount->mnt_bufs = bp;
    pMount->mnt_bufsize = bufSize;
    data = (char *) &bp[nBufs];

    for (i = 0; i < nBufs; i++) {
        listAdd (&pMount->mnt_buflist, &bp->b_node);

        bp->b_size = bufSize;
        bp->b_data = data;
        semBInit (&bp->b_sem, SEM_Q_PRIORITY, SEM_EMPTY);

/*
 *      b_blkno: leave as 0    (see memset above)
 *      b_flags: leave as 0    (see memset above)
 *      b_count: leave as 0    (see memset above)
 *      b_vp:    leave as NULL (see memset above)
 */

        data += bufSize;
        bp++;
    }

    return (OK);
}

/***************************************************************************
 *
 * mountBufFree - free mount buffer
 *
 *
 * RETURNS: N/A
 */

void mountBufFree(
    mount_t *pMount
    ) {
    free (pMount->mnt_bufs);

    pMount->mnt_bufs = NULL;
    pMount->mnt_bufsize = 0;

    listInit (&pMount->mnt_buflist);
}

/***************************************************************************
 *
 * mountEject - eject mount
 *
 *
 * RETURNS: N/A
 */

void mountEject(
    mount_t *  pMount,   /* ptr to mount */
    BOOL       changed
    ) {

    vfsMountEject (pMount);
}

/***************************************************************************
 *
 * mountDelete - delete mount
 *
 *
 * RETURNS: N/A
 */

void mountDelete(
    mount_t *  pMount,   /* ptr to mount */
    int        reserved
    ) {
}

/***************************************************************************
 *
 * mountUnlock- unlock the mount data structures (such as buffer lists)
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS mountUnlock (
    mount_t *  pMount
    ) {
    STATUS  status;

    status = semGive (pMount->mnt_lock);

    return (status);
}

/***************************************************************************
 *
 * mountLock - lock the mount data structures (such as buffer lists)
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS mountLock (
    mount_t *  pMount
    ) {
    STATUS  status;

    status = semTake (pMount->mnt_lock, WAIT_FOREVER);

    return (status);
}

