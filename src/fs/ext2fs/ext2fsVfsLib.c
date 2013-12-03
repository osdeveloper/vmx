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

/* ext2fsVfsLib.c - ext2 filesystem VFS operators library */

#include <stdlib.h>
#include <vmx.h>
#include <fs/mount.h>
#include <fs/vnode.h>
#include <fs/ext2fsLib.h>

/* locals */

/* TODO */
LOCAL const struct vnode_ops ext2fsOps;

LOCAL int ext2fsVfsStart (
    struct mount * pMount,
    int            flags
    );

LOCAL int ext2fsVfsUnmount (
    struct mount * pMount,
    int            flags
    );

LOCAL int ext2fsVfsRoot (
    struct mount *  pMount,
    struct vnode ** ppVnode
    );

LOCAL int ext2fsVfsStatVfs (
    struct mount *   pMount,
    struct statvfs * pStatVfs
    );

LOCAL int ext2fsVfsVget (
    struct mount *  pMount,
    ino_t           inode,
    struct vnode ** ppVnode
    );

LOCAL void ext2fsVfsInit (
    void
    );

LOCAL void ext2fsVfsReInit (
    void
    );

LOCAL void ext2fsVfsDone (
    void
    );

LOCAL int ext2fsVfsTransStart (
    struct mount * pMount,
    BOOL           writeFlag
    );

LOCAL int ext2fsVfsTransEnd (
    struct mount * pMount,
    int            error
    );

/* globals */

const struct vfsops ext2fsVfsOps =
{
    "Ext2FS",
    sizeof (EXT2FS_DEV),
    sizeof (EXT2FS_INODE),
    1,
    ext2fsVfsStart,
    ext2fsVfsUnmount,
    ext2fsVfsRoot,
    ext2fsVfsStatVfs,
    ext2fsVfsVget,
    ext2fsVfsInit,
    ext2fsVfsReInit,
    ext2fsVfsDone,
    ext2fsVfsTransStart,
    ext2fsVfsTransEnd
};

/******************************************************************************
 *
 * ext2fsMountCleanup - free resources allocated during mount
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsMountCleanup (
    EXT2FS_DEV * pFsDev
    ) {

    if (pFsDev->bioSem != NULL) {
        semDelete (pFsDev->bioSem);
        pFsDev->bioSem = NULL;
    }

    if (pFsDev->pScratchBlk != NULL) {
        bio_free (pFsDev->pScratchBlk);
        pFsDev->pScratchBlk = NULL;
    }

    if (pFsDev->ext2fsVolDesc.pSuperBlk != NULL) {
        free (pFsDev->ext2fsVolDesc.pSuperBlk);
        pFsDev->ext2fsVolDesc.pSuperBlk = NULL;
    }
}

/******************************************************************************
 *
 * ext2fsVfsStart - start ext2 filesystem
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVfsStart (
    struct mount * pMount,
    int            flags
    ) {
    EXT2FS_DEV * pFsDev;
    int          error;

    pFsDev = (EXT2FS_DEV *) pMount->mnt_data;

    error = ext2fsMount (pMount);
    if (error != OK) {
        ext2fsMountCleanup (pFsDev);
        return (error);
    }

    error = mountBufAlloc (pMount, EXT2FS_MIN_BUFFERS,
                           pFsDev->ext2fsVolDesc.blkSize);
    if (error != OK) {
        ext2fsMountCleanup (pFsDev);
        return (error);
    }

    return (error);
}

/******************************************************************************
 *
 * ext2fsVfsUnmount - umount ext2 filesystem
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVfsUnmount (
    struct mount * pMount,
    int            flags
    ) {
    EXT2FS_DEV * pFsDev;

    pFsDev = (EXT2FS_DEV *) pMount->mnt_data;

    ext2fsMountCleanup (pFsDev);

    pMount->mnt_data = NULL;

    return (OK);
}

/******************************************************************************
 *
 * ext2fsVfsVget - get vnode for given inode number
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVfsVget (
    struct mount *  pMount,
    ino_t           inode,
    struct vnode ** ppVnode
    ) {

    return (vgetino (pMount, inode, &ext2fsOps, ppVnode));
}

/******************************************************************************
 *
 * ext2fsVfsRoot - get root vnode
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVfsRoot (
    struct mount *  pMount,
    struct vnode ** ppVnode
    ) {

    return (ext2fsVfsVget (pMount, EXT2FS_ROOT_INODE, ppVnode));
}

/******************************************************************************
 *
 * ext2fsVfsStatVfs - get filesystem status
 *
 * RETURNS: OK on success; errno otherwise
 */

LOCAL int ext2fsVfsStatVfs (
    struct mount *   pMount,
    struct statvfs * pStatVfs
    ) {

    return (ext2fsStat (pMount, pStatVfs));
}

/******************************************************************************
 *
 * ext2fsVfsInit - initialize
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVfsInit (
    void
    ) {

    return;
}

/******************************************************************************
 *
 * ext2fsVfsReInit - reinitialize
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVfsReInit (
    void
    ) {

    return;
}

/******************************************************************************
 *
 * ext2fsVfsDone - done
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVfsDone (
    void
    ) {

    return;
}

/******************************************************************************
 *
 * ext2fsVfsTransStart - start transaction for read or write
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVfsTransStart (
    struct mount * pMount,
    BOOL           writeFlag
    ) {
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_DEV *         pFsDev;
    int                  error;

    if (writeFlag == TRUE) {
        error = VN_EXLOCK (pMount->mnt_syncer);
    }
    else {
        error = VN_SHLOCK (pMount->mnt_syncer);
    }

    if (error == OK) {
        pFsDev   = (EXT2FS_DEV *) pMount->mnt_data;
        pVolDesc = &pFsDev->ext2fsVolDesc;
        pVolDesc->diskModified = FALSE;
    }

    return (error);
}

/******************************************************************************
 *
 * ext2fsVfsTransEnd - end transaction for read or write
 *
 *
 * RETURNS: OK or supplied error
 */

LOCAL int ext2fsVfsTransEnd (
    struct mount * pMount,
    int            error
    ) {

    if (error != OK) {
        VN_UNLOCK (pMount->mnt_syncer);
        return (error);
    }

    VN_UNLOCK (pMount->mnt_syncer);

    return (OK);
}

