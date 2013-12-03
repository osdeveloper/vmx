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

/* ext2VopsLib.c -- This library deals with the Ext2FS vnode operators */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <os/logLib.h>
#include <fs/vnode.h>
#include <fs/mount.h>
#include <fs/buf.h>
#include <fs/namei.h>
#include <fs/ext2fsLib.h>
#include <fs/xbd.h>

/* locals */

LOCAL int ext2fsVopErrorActivate (
    struct vnode * pVnode
    );

LOCAL int ext2fsVopErrorInactive (
    struct vnode * pVnode
    );

void ext2fsVopErrorPrint (
    struct vnode * pVnode
    );

LOCAL void ext2fsVopsDebug (
    char * str
    );

LOCAL int ext2fsVopLookup (
    struct vnode *         pDirVnode,
    struct vnode **        pFileVnode,
    struct componentname * pComponentName
    );

LOCAL void ext2fsVopStrategy (
    struct vnode * pVnode,
    struct buf *   pBuf
    );

LOCAL int ext2fsVopRead (
    struct vnode * pVnode,
    struct uio *   pUio,
    int            ioflag,
    struct ucred * pCred
    );

LOCAL int ext2fsVopOpen (
    struct vnode * pVnode,
    int            mode,
    struct ucred * pCred
    );

LOCAL int ext2fsVopClose (
    struct vnode * pVnode,
    int            flags,
    struct ucred * pCred
    );

LOCAL int ext2fsVopAccess (
    struct vnode * pVnode,
    int            mode,
    struct ucred * pCred
    );

LOCAL int ext2fsVopGetAttr (
    struct vnode * pVnode,
    struct vattr * pVattr,
    struct ucred * pCred
    );

LOCAL int ext2fsVopIoctl (
    struct vnode * pVnode,
    u_long         command,
    void *         pData,
    int            fflag,
    struct ucred * pCred
    );

LOCAL int ext2fsVopSeek (
    struct vnode * pVnode,
    voff_t         oldOffset,
    voff_t         newOffset,
    struct ucred * pCred
    );

LOCAL int ext2fsVopAbort (
    struct vnode *         pVnode,
    struct componentname * pComponentName
    );

LOCAL int ext2fsVopActivate (
    struct vnode * pVnode
    );

LOCAL int ext2fsVopInactive (
    struct vnode * pVnode
    );

LOCAL void ext2fsVopPrint (
    struct vnode * pVnode
    );

LOCAL int ext2fsVopPathconf (
    struct vnode * pVnode,
    int            name,
    long *         retval
    );

LOCAL int ext2fsVopReaddir (
    struct vnode *  pDirVnode,
    struct dirent * pDirEnt,
    struct ucred *  pCred,
    int *           pIsEof,
    int *           pIndex
    );

/* globals */

const struct vnode_ops ext2fsOps =
{
    ext2fsVopLookup,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    ext2fsVopRead,
    vop_error_write,
    ext2fsVopIoctl,
    vop_error_fcntl,
    vop_error_fsync,
    ext2fsVopSeek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    ext2fsVopAbort,
    ext2fsVopActivate,
    ext2fsVopInactive,
    ext2fsVopStrategy,
    ext2fsVopPrint,
    ext2fsVopPathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsNoTypeOps =
{
    vop_error_lookup,
    vop_error_create,
    vop_error_open,
    vop_error_close,
    vop_error_access,
    vop_error_getattr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopErrorActivate,
    ext2fsVopErrorInactive,
    vop_error_strategy,
    ext2fsVopErrorPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsFifoOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopActivate,
    ext2fsVopInactive,
    vop_error_strategy,
    ext2fsVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsCharOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopActivate,
    ext2fsVopInactive,
    vop_error_strategy,
    ext2fsVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsBlockOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopActivate,
    ext2fsVopInactive,
    vop_error_strategy,
    ext2fsVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsSymlinkOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopActivate,
    ext2fsVopInactive,
    vop_error_strategy,
    ext2fsVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsSocketOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    vop_error_fsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsVopActivate,
    ext2fsVopInactive,
    vop_error_strategy,
    ext2fsVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsDirOps =
{
    ext2fsVopLookup,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    ext2fsVopRead,
    vop_error_write,
    ext2fsVopIoctl,
    vop_error_fcntl,
    vop_error_fsync,
    ext2fsVopSeek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    ext2fsVopReaddir,
    vop_error_readlink,
    ext2fsVopAbort,
    ext2fsVopActivate,
    ext2fsVopInactive,
    ext2fsVopStrategy,
    ext2fsVopPrint,
    ext2fsVopPathconf,
    vop_error_advlock,
    vop_error_truncate
};

const struct vnode_ops ext2fsFileOps =
{
    vop_error_lookup_enotdir,
    vop_error_create,
    ext2fsVopOpen,
    ext2fsVopClose,
    ext2fsVopAccess,
    ext2fsVopGetAttr,
    vop_error_setattr,
    ext2fsVopRead,
    vop_error_write,
    ext2fsVopIoctl,
    vop_error_fcntl,
    vop_error_fsync,
    ext2fsVopSeek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir_enotdir,
    vop_error_symlink,
    vop_error_readdir_enotdir,
    vop_error_readlink,
    ext2fsVopAbort,
    ext2fsVopActivate,
    ext2fsVopInactive,
    ext2fsVopStrategy,
    ext2fsVopPrint,
    ext2fsVopPathconf,
    vop_error_advlock,
    vop_error_truncate
};

/******************************************************************************
 *
 * ext2fsVopErrorActivate - activate vnode error
 *
 * RETURNS: ENOSYS
 */

LOCAL int ext2fsVopErrorActivate (
    struct vnode * pVnode
    ) {

    return (ENOSYS);
}

/******************************************************************************
 *
 * ext2fsVopErrorInactive - inactive vnode error
 *
 * RETURNS: ENOSYS
 */

LOCAL int ext2fsVopErrorInactive (
    struct vnode * pVnode
    ) {

    return (ENOSYS);
}

/******************************************************************************
 *
 * ext2fsVopErrorPrint - print vnode error
 *
 * RETURNS: N/A
 */

void ext2fsVopErrorPrint (
    struct vnode * pVnode
    ) {

    printf ("ENOSYS\n");
}

/******************************************************************************
 *
 * ext2fsVopsDebug - log debug message
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVopsDebug (
    char * str
    ) {

    logMsg ("ext2fsVopsDebug: %s\n", str, 0, 0, 0, 0, 0);
}

/******************************************************************************
 *
 * ext2fsDirEntryRead - read directory entry
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsDirEntryRead (
    struct vnode *     pDirVnode,
    voff_t             offset,
    EXT2FS_DIR_ENTRY * pDirEntry,
    char *             name,
    int *              pInfo
    ) {
    EXT2FS_DEV *            pFsDev;
    EXT2FS_VOLUME_DESC *    pVolDesc;
    EXT2FS_INODE *          pDirInode;
    EXT2FS_DIR_ENTRY_DISK * pDirEntryDisk;
    struct buf *            pBuf;
    u_int32_t               lbn;
    u_int32_t               off;
    int                     error;

    pFsDev   = (EXT2FS_DEV *) pDirVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    EXT2FS_VTOI (pDirInode, pDirVnode);

    lbn = (offset >> pVolDesc->blkSize2);
    if (offset >= pDirInode->host.i_size) {
        *pInfo = EXT2_DIRENT_EOF;
    return (OK);
    }

    off = (offset & (pVolDesc->blkSize - 1));

    /* Check that direntries does not cross block boundaries */
    if ((pVolDesc->blkSize - off) < EXT2_DIRENT_MINIMUM) {
        return (EINVAL);
    }

    error = bread (pDirVnode, lbn, pVolDesc->blkSize, NULL, &pBuf);
    if (error != OK) {
        return (error);
    }

    pDirEntryDisk = (EXT2FS_DIR_ENTRY_DISK *) ((char *) pBuf->b_data + off);
    ext2fsDirEntryDiskToHost (pDirEntryDisk, pDirEntry);

    /* perform sanity checks and store entry name */
    if ((off + pDirEntry->rec_len) > pVolDesc->blkSize) {
        error = EINVAL;
    }
    else if (pDirEntry->inode != 0) {
        if (pDirEntry->name_len == 0) {
            error = EINVAL;
        }
        else if (pDirEntry->name_len >
                 (pDirEntry->rec_len - EXT2_DIRENT_MINIMUM)) {
            error = EINVAL;
        }
        else if (pDirEntry->name_len > pVolDesc->maxNameLen) {
            error = ENAMETOOLONG;
        }

        strncpy (name, (char *) pDirEntryDisk->name, pDirEntry->name_len);
        name[pDirEntry->name_len] = EOS;
    }

    brelse (pBuf);
    *pInfo = !EXT2_DIRENT_EOF;

    return (error);
}

/******************************************************************************
 *
 * ext2fsEmptyEntryTest - tests if there space enough for entry
 *
 * RETURNS: TRUE if enough space, otherwise FALSE
 */

LOCAL BOOL ext2fsEmptyEntryTest (
    EXT2FS_DIR_ENTRY * pDirEntry,
    int                bytesNeeded
    ) {
    int  nameLength;

    if (pDirEntry->inode == 0) {
        return (pDirEntry->rec_len >= bytesNeeded);
    }

    /* Round nameLength to a 4 byte boundary */
    nameLength = ((pDirEntry->name_len + 3) & (~3));

    return ((pDirEntry->rec_len - nameLength) >= bytesNeeded) ? TRUE : FALSE;
}

/******************************************************************************
 *
 * ext2fsVopLookup - find file in the directory
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopLookup (
    struct vnode *         pDirVnode,
    struct vnode **        pFileVnode,
    struct componentname * pComponentName
    ) {
    EXT2FS_LOOKUP_INFO  info;
    EXT2FS_INODE *      pDirInode;
    EXT2FS_DIR_ENTRY    dirEntry;
    int                 error;
    int                 bytesNeeded;
    int                 isEof;
    voff_t              offset;
    char                name[FILENAME_MAX];
    BOOL                wantEmptyEntry = FALSE;
    BOOL                wantFoundEntry = FALSE;

    /* check access rights */
    error = VOP_ACCESS (pDirVnode, VEXEC, pComponentName->cn_cred);
    if (error != OK) {
        return (error);
    }

    EXT2FS_VTOI (pDirInode, pDirVnode);

    if ((pComponentName->cn_namelen == 1) &&
        (pComponentName->cn_nameptr[0] == '.')) {
        vnodeLock (pDirVnode);
        *pFileVnode = pDirVnode;
        return (OK);
     }

    if (pComponentName->cn_flags & ISLASTCN) {

        if (pComponentName->cn_nameiop == CREATE) {
            wantEmptyEntry = TRUE;
        }
        else if (pComponentName->cn_nameiop == DELETE) {
            wantFoundEntry = TRUE;
        }
        else if (pComponentName->cn_nameiop == RENAME) {
            wantEmptyEntry = TRUE;
            wantFoundEntry = TRUE;
        }

        info.curIndex  = -1;
        info.prevIndex = -1;
        info.nextIndex = -1;

        pDirInode->info = info;
    }

    bytesNeeded = pComponentName->cn_namelen + EXT2_DIR_ENTRY_HEADER_SIZE;
    offset      = 0;

    while (1) {
        error = ext2fsDirEntryRead (pDirVnode, (voff_t) offset,
                                    &dirEntry, name, &isEof);
        if (error != OK) {
            return (error);
        }

        if (isEof == EXT2_DIRENT_EOF) {
            break;
        }

        info.nextIndex = offset + dirEntry.rec_len;
            
        if ((wantEmptyEntry == TRUE) &&
             ext2fsEmptyEntryTest (&dirEntry, bytesNeeded)) {
            pDirInode->info = info;
            wantEmptyEntry = FALSE;
        }

        if ((dirEntry.inode != 0) &&
            (dirEntry.name_len == pComponentName->cn_namelen) &&
            (strncmp (name, pComponentName->cn_nameptr,
                      pComponentName->cn_namelen) == 0)) {

            /* store found entry */
            if (wantFoundEntry == TRUE) {
                pDirInode->info = info;
            }

            if (dirEntry.inode == pDirInode->inode) {
                vnodeLock (pDirVnode);
                *pFileVnode = pDirVnode;
                return (OK);
            }

            if (pComponentName->cn_flags & ISDOTDOT) {
                pComponentName->cn_flags |= PDIRUNLOCK;
                VN_UNLOCK (pDirVnode);
            }

            error = vgetino (pDirVnode->v_mount, dirEntry.inode,
                             &ext2fsOps, pFileVnode);

            return (error);
        }

        offset         += dirEntry.rec_len;
        info.prevIndex  = info.curIndex;
        info.curIndex   = info.nextIndex;
    }

    return (ENOENT);
}

/******************************************************************************
 *
 * ext2fsVopInternalReadStrategy - read-only portion of a file's "strategy"
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVopInternalReadStrategy (
    struct vnode * pVnode,
    struct buf *   pBuf
    ) {
    EXT2FS_DEV *         pFsDev;
    EXT2FS_INODE *       pInode;
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_TRIE_MAP *    pTrieMap;
    struct vnode *       pSyncer;
    struct buf *         pTmpBuf;
    int                  error;

    pSyncer  = pVnode->v_mount->mnt_syncer;
    pFsDev   = (EXT2FS_DEV *) pSyncer->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    EXT2FS_VTOI (pInode, pVnode);

    pTrieMap = &(pInode->cachedTrieMap);

    /* if this is a sparse block */
    if (pTrieMap->physBlk[0] == EXT2FS_SPARSE_BLK_NUM) {
        memset (pBuf->b_data, 0, pVolDesc->blkSize);
        buf_done (pBuf, OK);
        return;
    }

    error = bread (pSyncer, pTrieMap->physBlk[0], pVolDesc->blkSize,
                   NULL, &pTmpBuf);
    if (error != OK) {
        buf_done (pBuf, error);
        return;
    }

    /* swap out data from temporary buffer to target buffer */
    buf_swapdata (pTmpBuf, pBuf);
    pTmpBuf->b_flags |= B_INVAL;
    brelse (pTmpBuf);
    buf_done (pBuf, OK);
}

/******************************************************************************
 *
 * ext2fsVopStrategy - strategy routine for vnode
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVopStrategy (
    struct vnode * pVnode,
    struct buf *   pBuf
    ) {
    int  error;

    /* get private i-node from the v-node */
    error = ext2fsPhysFind (pVnode, pBuf->b_lblkno);
    if (error != OK) {
        buf_done(pBuf, error);
        return;
    }

    if ((pBuf->b_flags & B_READ) != 0) {
        ext2fsVopInternalReadStrategy (pVnode, pBuf);
        return;
    }

    /* TODO: Implement write operation */

    buf_done (pBuf, EROFS);
}

/******************************************************************************
 *
 * ext2fsVopRead - read from file
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopRead (
    struct vnode * pVnode,
    struct uio *   pUio,
    int            ioflag,
    struct ucred * pCred
    ) {
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_INODE *       pInode;
    struct buf *         pBuf;
    u_int32_t            lbn;
    u_int32_t            off;
    u_int32_t            bytesToRead;
    int                  error;
    voff_t               bytesToEOF;

    /* nothing to do */
    if (pUio->uio_resid == 0) {
        return (OK);
    }

    EXT2FS_VTOI (pInode, pVnode);

    pFsDev   = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    lbn         = (pUio->uio_offset >> pVolDesc->blkSize2);
    off         = (pUio->uio_offset & (pVolDesc->blkSize - 1));
    bytesToRead = pVolDesc->blkSize - off;
    bytesToEOF  = pInode->host.i_size - pUio->uio_offset;
    error       = OK;

    while (pUio->uio_resid > 0) {

        error = bread (pVnode, lbn, pVolDesc->blkSize, NULL, &pBuf);
        if (error != OK) {
            return (error);
        }

        if (bytesToRead > pUio->uio_resid) {
            bytesToRead = pUio->uio_resid;
        }

        if (bytesToEOF < bytesToRead) {
            bytesToRead = bytesToEOF;
        }

        uiomove ((char *) pBuf->b_data + off, bytesToRead, pUio);

        brelse (pBuf);
        lbn++;

        if (bytesToRead == 0) {
            break;
        }

        /* subsequent reads in this set will be block aligned */
        bytesToEOF -= bytesToRead;
        off         = 0;
        bytesToRead = pVolDesc->blkSize;
    }

    return (error);
}

/******************************************************************************
 *
 * ext2fsVopOpen - open file
 *
 * RETURNS: OK
 */

LOCAL int ext2fsVopOpen (
    struct vnode * pVnode,
    int            mode,
    struct ucred * pCred
    ) {

    /* nothing to do */
    return (OK);
}

/******************************************************************************
 *
 * ext2fsVopClose - close file
 *
 * RETURNS: OK
 */

LOCAL int ext2fsVopClose (
    struct vnode * pVnode,
    int            flags,
    struct ucred * pCred
    ) {

    /* TODO: Check if anything to do for writable filesystem */
    return (OK);
}

/******************************************************************************
 *
 * ext2fsVopAccess - initiate access to file
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopAccess (
    struct vnode * pVnode,
    int            mode,
    struct ucred * pCred
    ) {
    EXT2FS_INODE * pInode;
    int            fileMode;
    int            ret;

    EXT2FS_VTOI (pInode, pVnode);
    fileMode = pInode->host.i_mode & 0777;

    ret = vaccess (pVnode->v_type, fileMode, pInode->host.i_uid,
                   pInode->host.i_gid, mode, pCred);
    return (ret);
}

/******************************************************************************
 *
 * ext2fsVopGetAttr - get file attributes
 *
 * RETURNS: OK
 */

LOCAL int ext2fsVopGetAttr (
    struct vnode * pVnode,
    struct vattr * pVattr,
    struct ucred * pCred
    ) {
    EXT2FS_INODE *       pInode;
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;

    EXT2FS_VTOI (pInode, pVnode);

    pFsDev   = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    memset (pVattr, 0, sizeof (struct vattr));

    /* set fields in attribute structure */
    pVattr->va_type             = pVnode->v_type;
    pVattr->va_mode             = pInode->host.i_mode;
    pVattr->va_nlink            = pInode->host.i_links_count;
    pVattr->va_uid              = pInode->host.i_uid;
    pVattr->va_gid              = pInode->host.i_gid;
    pVattr->va_fsid             = 0;           /* TODO: Set to filesystem id */
    pVattr->va_fileid           = pInode->inode;
    pVattr->va_size             = pInode->host.i_size;
    pVattr->va_blocksize        = pVolDesc->blkSize;
    pVattr->va_atime.tv_sec     = pInode->host.i_atime;
    pVattr->va_mtime.tv_sec     = pInode->host.i_mtime;
    pVattr->va_ctime.tv_sec     = pInode->host.i_ctime;
    pVattr->va_birthtime.tv_sec = pInode->host.i_ctime;
    pVattr->va_flags            = pInode->host.i_flags;
#ifdef notyet
    pVattr->va_gen              = 0;
    pVattr->va_rdev             = 0;
    pVattr->va_bytes            = 0;
    pVattr->va_filerev          = 0;
    pVattr->va_vaflags          = 0;
#endif

    return (OK);
}

/******************************************************************************
 *
 * ext2fsVopIoctl - perform an I/O control on file
 *
 * RETURNS: OK on success, or error from driver
 */

LOCAL int ext2fsVopIoctl (
    struct vnode * pVnode,
    u_long         command,
    void *         pData,
    int            fflag,
    struct ucred * pCred
    ) {
    EXT2FS_DEV * pExt2Dev;
    int          ret;

    pExt2Dev = (EXT2FS_DEV *)(pVnode->v_mount->mnt_data);

    ret = xbdIoctl (pExt2Dev->ext2fsVolDesc.device, command, pData);

    return (ret);
}

/******************************************************************************
 *
 * ext2fsVopSeek - seek to a file position
 *
 * RETURNS: OK
 */

LOCAL int ext2fsVopSeek (
    struct vnode * pVnode,
    voff_t         oldOffset,
    voff_t         newOffset,
    struct ucred * pCred
    ) {

    /* handled by the I/O system */
    return (OK);
}

/******************************************************************************
 *
 * ext2fsVopAbort - abort
 *
 * RETURNS: OK
 */

LOCAL int ext2fsVopAbort (
    struct vnode *         pVnode,
    struct componentname * pComponentName
    ) {

    /* nothing to do */
    return (OK);
}

/******************************************************************************
 *
 * extfsVopActivate - activate a vnode
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopActivate (
    struct vnode * pVnode
    ) {
    static enum vtype  vtypes[16] =
    {
        VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
        VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VNON
    };

    EXT2FS_INODE *    pInode;
    int               error;
    int               type;

    /* get i-node */
    error = ext2fsInodeGet (pVnode);
    if (error != OK) {
        return (error);
    }

    EXT2FS_VTOI (pInode, pVnode);

    memset (&pInode->cachedTrieMap, 0, sizeof (EXT2FS_TRIE_MAP));

    pInode->cachedTrieMap.level = -1;
    pInode->deleted             = FALSE;
    pInode->blkGroupHint        = 0;    /* TODO: Check how to use */

    memset (&pInode->info, 0, sizeof (EXT2FS_LOOKUP_INFO));

    type = ((pInode->host.i_mode & S_IFMT) >> 12);

    pVnode->v_type = vtypes[type];

    /* select vnode operators */
    switch (pVnode->v_type) {
        case VNON:
            pVnode->v_ops = &ext2fsNoTypeOps;
            break;

        case VFIFO:
            pVnode->v_ops = &ext2fsFifoOps;
            break;

        case VCHR:
            pVnode->v_ops = &ext2fsCharOps;
            break;

        case VDIR:
            pVnode->v_ops = &ext2fsDirOps;
            break;

        case VBLK:
            pVnode->v_ops = &ext2fsBlockOps;
            break;

        case VREG:
            pVnode->v_ops = &ext2fsFileOps;
            break;

        case VLNK:
            pVnode->v_ops = &ext2fsSymlinkOps;
            break;

        case VSOCK:
            pVnode->v_ops = &ext2fsSocketOps;
            break;

        default:
            return (EINVAL);
    }

    return (OK);
}

/******************************************************************************
 *
 * ext2fsVopInactive - inactivate vnode
 *
 * RETURNS: EBADF
 */

LOCAL int ext2fsVopInactive (
    struct vnode * pVnode
    ) {

    /* TODO: Check what to do for write filesystem */
    return (EBADF);
}

/******************************************************************************
 *
 * ext2fsVopPrint - print a file's vnode
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsVopPrint (
    struct vnode * pVnode
    ) {

    printf ("Ext2fs vnode\n");
}

/******************************************************************************
 *
 * ext2fsVopPathconf - get path configuration
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopPathconf (
    struct vnode * pVnode,
    int            name,
    long *         retval
    ) {
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;
    int                  error;

    pFsDev   = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    /* select parameter */
    switch (name) {
        case _PC_LINK_MAX:
            *retval = (long) EXT2FS_MAX_HARD_LINKS;
            error = OK;
            break;

        case _PC_MAX_CANON:
            *retval = _POSIX_MAX_CANON;
            error = OK;
            break;

        case _PC_MAX_INPUT:
            *retval = _POSIX_MAX_INPUT;
            error = OK;
            break;

        case _PC_FILESIZEBITS:
            *retval = (pVolDesc->maxFileSize > 0x1FFFFFFF) ? 64 : 32;
            error = OK;
            break;

        case _PC_NAME_MAX:
            *retval = pVolDesc->maxNameLen;
            error = OK;
            break;

        case _PC_PATH_MAX:
            *retval = pVolDesc->maxPathLen;
            error = OK;
            break;

        case _PC_PIPE_BUF:
            *retval = _POSIX_PIPE_BUF;
            error = OK;
            break;

        case _PC_CHOWN_RESTRICTED:
            *retval = 1;
            error = OK;
            break;

        case _PC_NO_TRUNC:
            *retval = 1;
            error = OK;
            break;

        case _PC_PRIO_IO:
            *retval = 0;
            error = OK;
            break;

        case _PC_ASYNC_IO:
            *retval = 1;
            error = OK;
            break;

        case _PC_SYNC_IO:
            *retval = 1;
            error = OK;
            break;

        case _PC_VDISABLE:
            *retval = 0;
            error = OK;
            break;

        case _PC_2_SYMLINKS:
            *retval = 1;
            error = OK;
            break;

        case _PC_SYMLINK_MAX:
            *retval = pVolDesc->maxPathLen;
            error = OK;
            break;

        default:
            error = EINVAL;
            break;
    }

    return (error);
}

/******************************************************************************
 *
 * ext2fsVopReaddir - read directory
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsVopReaddir (
    struct vnode *  pDirVnode,
    struct dirent * pDirEnt,
    struct ucred *  pCred,
    int *           pIsEof,
    int *           pIndex
    ) {
    EXT2FS_INODE *    pDirInode;
    EXT2FS_DIR_ENTRY  dirEntry;
    voff_t            offset;
    int               info;
    int               error;
    char              name[FILENAME_MAX];

    EXT2FS_VTOI (pDirInode, pDirVnode);

    offset  = *pIndex;
    *pIsEof = 0;

    while (1) {
        error = ext2fsDirEntryRead (pDirVnode, offset,
                                    &dirEntry, name, &info);
        if ((error != OK) || (info == EXT2_DIRENT_EOF)) {
            break;
        }

        offset += dirEntry.rec_len;

        if (dirEntry.inode != 0) {
            pDirEnt->d_ino = dirEntry.inode;
            strncpy (pDirEnt->d_name, name, dirEntry.name_len);
            pDirEnt->d_name[dirEntry.name_len] = '\0';
            *pIndex = offset;
            return (OK);
        }
    }

    *pIsEof = 1;

    return (error);
}

