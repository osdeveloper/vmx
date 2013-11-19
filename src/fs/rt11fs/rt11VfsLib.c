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

/* rt11VfsLib.c - Rt11 compatible filesystem vfs operators */

#include <stdlib.h>
#include <vmx.h>
#include <os/erfLib.h>
#include <fs/bio.h>
#include <fs/buf.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/rt11fsLib.h>

LOCAL int  rt11VfsStart      (mount_t * pMount, int flags);
LOCAL int  rt11VfsUnmount    (mount_t * pMount, int flags);
LOCAL int  rt11VfsRoot       (mount_t * pMount, vnode_t ** ppVnode);
LOCAL int  rt11VfsStatVfs    (mount_t * pMount, struct statvfs *);
LOCAL int  rt11VfsVget       (mount_t * pMount, ino_t inode, vnode_t ** ppVnode);
LOCAL void rt11VfsInit       (void);
LOCAL void rt11VfsReInit     (void);
LOCAL void rt11VfsDone       (void);
LOCAL int  rt11VfsTransStart (mount_t * pMount, BOOL writeFlag);
LOCAL int  rt11VfsTransEnd   (mount_t * pMount, int error);

const vfsops_t rt11VfsOps =
    {
    "rt11FS",
    sizeof (RT11FS_DEV),
    sizeof (RT11FS_INODE),
    1,
    rt11VfsStart,
    rt11VfsUnmount,
    rt11VfsRoot,
    rt11VfsStatVfs,
    rt11VfsVget,
    rt11VfsInit,
    rt11VfsReInit,
    rt11VfsDone,
    rt11VfsTransStart,
    rt11VfsTransEnd
    };



/***************************************************************************
 *
 * rt11VfsStart -
 *
 * RETURNS: OK on success, error otherwise
 */

int rt11VfsStart
    (
    mount_t *  pMount,   /* pointer to mount */
    int        flags     /* not used */
    )
    {
    RT11FS_DEV *    pFsDev;
    int             error;

    pFsDev = (RT11FS_DEV *) pMount->mnt_data;

    error = rt11fsMount (pMount);
    if (error != OK)
        {
        /* TODO: some type of cleanup routine may need to be called */
        return (error);
        }

    error = mountBufAlloc (pMount, RT11FS_MIN_BUFFERS,
                           pFsDev->volDesc.vd_blkSize);
    if (error != OK)
        {
        /* TODO: some type of cleanup routine may need to be called */
        return (error);
        }

    return (error);
    }

/***************************************************************************
 *
 * rt11VfsUnmount -
 *
 * RETURNS: OK on success, error otherwise
 */

int rt11VfsUnmount
    (
    mount_t *  pMount,   /* pointer to mount */
    int        flags     /* not used */
    )
    {
    RT11FS_DEV *  pFsDev;

    pFsDev = (RT11FS_DEV *) pMount->mnt_data;

    /*
     * TODO: some type of cleanup routine must be called.
     * For simplicity, make it the same one as the others above.
     */

    pMount->mnt_data = NULL;

    return (OK);
    }

/***************************************************************************
 *
 * rt11VfsVget -
 *
 * RETURNS: OK on success, error otherwise
 */

int rt11VfsVget
    (
    mount_t *   pMount,   /* pointer to mount */
    ino_t       inode,    /* inode number */
    vnode_t **  ppVnode   /* return pointer to vnode here */
    )
    {
    return (vgetino (pMount, inode, (vnode_ops_t *) &rt11Vops, ppVnode));
    }

/***************************************************************************
 *
 * rt11VfsRoot -
 *
 * RETURNS: OK on success, error otherwise
 */

int rt11VfsRoot
    (
    mount_t *   pMount,   /* pointer to mount */
    vnode_t **  ppVnode   /* return pointer to vnode here */
    )
    {
    return (rt11VfsVget (pMount, RT11FS_ROOT_INODE, ppVnode));
    }

/***************************************************************************
 *
 * rt11VfsStatVfs -
 *
 * RETURNS: OK on success, error otherwise
 */

int rt11VfsStatVfs
    (
    mount_t *        pMount,   /* pointer to mount */
    struct statvfs * pStatVfs  /* pointer to VFS stat structure */
    )
    {
    return;
    }

/***************************************************************************
 *
 * rt11VfsInit -
 *
 * RETURNS: N/A
 */

void rt11VfsInit (void)
    {
    return;
    }

/***************************************************************************
 *
 * rt11VfsReInit -
 *
 * RETURNS: N/A
 */

void rt11VfsReInit (void)
    {
    return;
    }

/***************************************************************************
 *
 * rt11VfsDone -
 *
 * RETURNS: N/A
 */

void rt11VfsDone (void)
    {
    return;
    }

/***************************************************************************
 *
 * rt11VfsTransStart -
 *
 * RETURNS: N/A
 */

int rt11VfsTransStart
    (
    struct mount *  pMount,     /* pointer to mount */
    BOOL            writeFlag   /* TRUE if transaction will write */
    )
    {
    RT11FS_VOLUME_DESC *  pVolDesc;
    RT11FS_DEV *          pFsDev;
    int                   error;

    /*
     * Normally, if <writeFlag> were FALSE, then a shared lock could be used,
     * and if it were TRUE then an exclusive lock could be used.  However, as
     * the file system is currently single threaded (or at the time of writing
     * this, it is supposed to be/planned to be), the type of lock does not
     * make much of a difference; this makes <writeFlag> somewhat superfluous
     * for the time being.  For now, just take an exclusive lock (mutex) and
     * accept the hit on the read scenario.
     *
     * TODO: Actually take the lock.
     */

    error = vnodeLock (pMount->mnt_syncer);

    if (error == OK)
        {
        pFsDev   = (RT11FS_DEV *) pMount->mnt_data;
        pVolDesc = &pFsDev->volDesc;
        pVolDesc->vd_diskModified = FALSE;
        }

    return (error);
    }

/***************************************************************************
 *
 * rt11VfsTransEnd -
 *
 * RETURNS: N/A
 */

int rt11VfsTransEnd
    (
    struct mount *  pMount,    /* pointer to mount */
    int             error      /* previous error */
    )
    {
    if (error != OK)
        {
        vnodeUnlock (pMount->mnt_syncer);
        return (error);
        }

    /*
     * When delayed or asynchronous writes are implemented, there are two
     * options.
     * #1.  Flush any pended writes here and now.
     * #2.  Flush any pended writes when the FS is unmounted/ejected.
     * At the present time, the idea is to use option #2 (when it gets done).
     */

    vnodeUnlock (pMount->mnt_syncer);
    return (OK);
    }
