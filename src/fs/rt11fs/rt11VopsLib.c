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

/* rt11VopsLib.c - Rt11 compatible filesystem vops */

/* includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <vmx.h>
#include <os/erfLib.h>
#include <fs/bio.h>
#include <fs/buf.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/vnode.h>
#include <fs/rt11fsLib.h>

/* defines */

/* typedefs */

/* forward declarations */
LOCAL int rt11VopLookup (lookup_args_t args);
LOCAL int rt11VopCreate (create_args_t  args);
LOCAL int rt11VopOpen (open_args_t  args);
LOCAL int rt11VopClose (close_args_t args);
LOCAL int rt11VopAccess (access_args_t args);
LOCAL int rt11VopRead (read_args_t args);
LOCAL int rt11VopWrite (write_args_t args);
LOCAL int rt11VopIoctl (ioctl_args_t args);
LOCAL int rt11VopLink (link_args_t args);
LOCAL int rt11VopUnlink (unlink_args_t args);
LOCAL int rt11VopSymlink (symlink_args_t args);
LOCAL int rt11VopReadlink (readlink_args_t args);
LOCAL int rt11VopMkdir (mkdir_args_t args);
LOCAL int rt11VopRmdir (rmdir_args_t args);
LOCAL int rt11VopReaddir (readdir_args_t args);
LOCAL int rt11VopGetAttr (getattr_args_t args);
LOCAL int rt11VopSetAttr (setattr_args_t args);
LOCAL int rt11VopTruncate (truncate_args_t args);
LOCAL int rt11VopFsync (fsync_args_t args);
LOCAL int rt11VopActivate (activate_args_t args);
LOCAL int rt11VopInactive (inactive_args_t args);
LOCAL int rt11VopPathconf (pathconf_args_t args);
LOCAL int rt11VopSeek (seek_args_t args);
LOCAL int rt11VopRename (rename_args_t args);
LOCAL int rt11VopAbort (abort_args_t args);
LOCAL int rt11VopStrategy (strategy_args_t args);
LOCAL int rt11VopPrint (print_args_t args);

LOCAL int rt11VopDirGetAttr (getattr_args_t args);
LOCAL int rt11VopDirStrategy (strategy_args_t args);

LOCAL int rt11VopSyncerStrategy (strategy_args_t args);

LOCAL void rt11VopInternalReadStrategy (struct vnode *pVnode,
                                        struct buf *pBuf);
LOCAL void rt11VopInternalWriteStrategy (struct vnode *pVnode,
                                         struct buf *pBuf);

/* globals */

const vnode_ops_t  rt11Vops =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopLink,
    rt11VopUnlink,
    rt11VopSymlink,
    rt11VopReadlink,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopReaddir,
    rt11VopGetAttr,
    rt11VopSetAttr,
    rt11VopTruncate,
    rt11VopFsync,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopPathconf,
    rt11VopSeek,
    rt11VopRename,
    rt11VopAbort,
    rt11VopStrategy,
    rt11VopPrint
};

const vnode_ops_t  rt11VopsDir =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopLink,
    rt11VopUnlink,
    rt11VopSymlink,
    rt11VopReadlink,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopReaddir,
    rt11VopDirGetAttr,
    rt11VopSetAttr,
    rt11VopTruncate,
    rt11VopFsync,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopPathconf,
    rt11VopSeek,
    rt11VopRename,
    rt11VopAbort,
    rt11VopDirStrategy,
    rt11VopPrint
};

const vnode_ops_t  rt11VopsSyncer =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopLink,
    rt11VopUnlink,
    rt11VopSymlink,
    rt11VopReadlink,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopReaddir,
    rt11VopGetAttr,
    rt11VopSetAttr,
    rt11VopTruncate,
    rt11VopFsync,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopPathconf,
    rt11VopSeek,
    rt11VopRename,
    rt11VopAbort,
    rt11VopSyncerStrategy,
    rt11VopPrint
};

/***************************************************************************
 *
 * rt11VopLookup - directory lookup
 *
 * RETURNS: OK if found, error otherwise
 */

LOCAL int rt11VopLookup (
    lookup_args_t  args
    ) {
    vnode_t  *          pDirVnode;
    RT11FS_INODE  *     pDirInode;
    RT11FS_DIR_DESC  *  pDirDesc;
    vnode_t  *          pFileVnode;
    RT11FS_INODE  *     pFileInode;
    RT11FS_FILE_DESC  * pFileDesc;
    RT11FS_DIR_ENTRY    dirEntry;
    int                 entryNum, startBlock;
    int                 error;

    /* Get directory vnode */
    pDirVnode = args.dvp;
    if (pDirVnode == NULL) {
        return (EINVAL);
    }

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, pDirVnode);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Get entry number */
    if ((entryNum = rt11fsFindDirEntry (pDirDesc->rdd_pDirSeg,
                                        &dirEntry,
                                        &startBlock,
                                        args.cnp->cn_nameptr)) == ERROR) {
        return (ENOENT);
    }

    /* Can not open until file is permanent */
    if (dirEntry.de_status == DES_TENTATIVE) {
        return (EINVAL);
    }

    /* Get file vnode */
    error = vgetino (args.dvp->v_mount,
                     (ino_t) (entryNum + RT11FS_FILE_INODE_BASE),
                     (vnode_ops_t *) &rt11Vops, &pFileVnode);
    if (error != OK) {
        return (ENOENT);
    }

    /* Get file inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    /* Inititalize file descriptor */
    pFileDesc->rfd_startBlock = startBlock;
    pFileDesc->rfd_nBlks = dirEntry.de_nblocks;
    memcpy (&pFileDesc->rfd_dirEntry, &dirEntry, sizeof (RT11FS_DIR_ENTRY));

    *args.vpp = pFileVnode;
    return (error);
}

/***************************************************************************
 *
 * rt11VopCreate -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopCreate (
    create_args_t  args
    ) {
    vnode_t  *          pDirVnode;
    RT11FS_INODE  *     pDirInode;
    RT11FS_DIR_DESC  *  pDirDesc;
    RT11FS_DIR_ENTRY    dirEntry;
    vnode_t  *          pFileVnode;
    RT11FS_INODE  *     pFileInode;
    RT11FS_FILE_DESC  * pFileDesc;
    fsync_args_t        syncArgs;
    int                 entryNum, startBlock;
    int                 error;

    /* Get directory vnode */
    pDirVnode = args.dvp;
    if (pDirVnode == NULL) {
        return (EINVAL);
    }

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, pDirVnode);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Allocate a new directory entry */
    if ((entryNum = rt11fsAllocNewDirEntry (pDirDesc->rdd_pDirSeg,
                                            pDirDesc->rdd_maxEntries,
                                            &dirEntry,
                                            &startBlock,
                                            args.cnp->cn_nameptr)) == ERROR) {
        return (ENOENT);
    }

    /* Get file vnode */
    error = vgetino (args.dvp->v_mount,
                     (ino_t) (entryNum + RT11FS_FILE_INODE_BASE),
                     (vnode_ops_t *) &rt11Vops, &pFileVnode);
    if (error != OK) {
        return (ENOENT);
    }

    /* Get file inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    /* Inititalize file descriptor */
    pFileDesc->rfd_startBlock = startBlock;
    pFileDesc->rfd_nBlks = 0;
    memcpy (&pFileDesc->rfd_dirEntry, &dirEntry, sizeof (RT11FS_DIR_ENTRY));

    /* Put entry in directory */
    rt11fsPutDirEntry (pDirDesc->rdd_pDirSeg, entryNum, &dirEntry);

    /* Flush directory */
    syncArgs.vp = pFileVnode;
    VOP_FSYNC (pFileVnode, syncArgs);

    *args.vpp = pFileVnode;
    return (error);
}

/***************************************************************************
 *
 * rt11VopOpen -
 *
 * RETURNS: OK
 */

LOCAL int rt11VopOpen (
    open_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopClose -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopClose (
    close_args_t  args
    ) {
    vnode_t  *            pFileVnode;
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    RT11FS_DIR_ENTRY  *   pDirEntry;
    fsync_args_t          syncArgs;
    int                   blksLeft, entryNum;

    /* Get vnode */
    pFileVnode = args.vp;
    if (pFileVnode == NULL) {
        return (EINVAL);
    }

    /* Get volume descriptor */
    pFsDev = (RT11FS_DEV *) pFileVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    /* Get inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    /* Make file at lest one block */
    if (pFileDesc->rfd_nBlks < 1) {
        pFileDesc->rfd_nBlks = 1;
    }

    /* Get directory entry */
    pDirEntry = &pFileDesc->rfd_dirEntry;
    entryNum = pFileInode->in_inode - RT11FS_FILE_INODE_BASE;

    /* If new file update directory */
    if (pDirEntry->de_status == DES_TENTATIVE) {
        blksLeft = pDirEntry->de_nblocks - pFileDesc->rfd_nBlks;

        /* Mark file as permanent */
        pDirEntry->de_status = DES_PERMANENT;
        pDirEntry->de_nblocks = pFileDesc->rfd_nBlks;

        /* Put entry */
        rt11fsPutDirEntry (pVolDesc->vd_pDirSeg, entryNum, pDirEntry);

        /* If space left, return it pool */
        if (blksLeft != 0) {
            pDirEntry->de_status = DES_EMPTY;
            pDirEntry->de_nblocks = blksLeft;

            /* Insert a entry with space allocated to it */
            rt11fsInsertDirEntry (pVolDesc->vd_pDirSeg, entryNum + 1,
                                  pDirEntry);
            rt11fsDirMergeEmpty (pVolDesc->vd_pDirSeg, entryNum + 1);
        }

        /* Flush directory */
        syncArgs.vp = pFileVnode;
        VOP_FSYNC (pFileVnode, syncArgs);
    }

    return (OK);
}

/***************************************************************************
 *
 * rt11VopAccess - determine if this file can be accessed
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopAccess (
    access_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopRead -
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rt11VopRead (
    read_args_t  args
    ) {
    vnode_t  *            pFileVnode;
    RT11FS_VOLUME_DESC *  pVolDesc;
    RT11FS_DEV *          pFsDev;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToRead, bytesToEOF;

    if (args.uio->uio_resid == 0) {    /* If nothing to do, return early. */
        return (OK);
    }

    /* Get vnode */
    pFileVnode = args.vp;
    if (pFileVnode == NULL) {
        return (EINVAL);
    }

    pFsDev = (RT11FS_DEV *)  pFileVnode->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    lbn = (args.uio->uio_offset >> pVolDesc->vd_blkSize2);
    off = (args.uio->uio_offset & (pVolDesc->vd_blkSize - 1));
    bytesToRead = pVolDesc->vd_blkSize - off;
    bytesToEOF  = (pFileDesc->rfd_nBlks << pVolDesc->vd_blkSize2) -
                  args.uio->uio_offset;

    while (args.uio->uio_resid > 0) {
        error = bread (pFileVnode, lbn, pVolDesc->vd_blkSize, NULL, &pBuf);
        if (error != OK) {
            return (error);
        }

        if (bytesToRead > args.uio->uio_resid) {
            bytesToRead = args.uio->uio_resid;
        }

        if (bytesToEOF < bytesToRead) {
            bytesToRead = bytesToEOF;
        }

        uiomove ((char *) pBuf->b_data + off, bytesToRead, args.uio);

        brelse (pBuf);

        lbn++;

        if (bytesToRead == 0) {
            break;
        }

        /* Subsequent reads in this set will be block aligned */

        bytesToEOF -= bytesToRead;
        off = 0;
        bytesToRead = pVolDesc->vd_blkSize;
    }

    return (error);
}

/***************************************************************************
 *
 * rt11VopWrite -
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rt11VopWrite (
    write_args_t  args
    ) {
    vnode_t  *            pFileVnode;
    RT11FS_VOLUME_DESC *  pVolDesc;
    RT11FS_DEV *          pFsDev;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToWrite, bytesToEOF;

    if (args.uio->uio_resid == 0) {    /* If nothing to do, return early. */
        return (OK);
    }

    /* Get vnode */
    pFileVnode = args.vp;
    if (pFileVnode == NULL) {
        return (EINVAL);
    }

    pFsDev = (RT11FS_DEV *)  pFileVnode->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get file inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    lbn = (args.uio->uio_offset >> pVolDesc->vd_blkSize2);
    off = (args.uio->uio_offset & (pVolDesc->vd_blkSize - 1));
    bytesToEOF  = (pFileDesc->rfd_dirEntry.de_nblocks <<
                   pVolDesc->vd_blkSize2) -
                  args.uio->uio_offset;

    while (args.uio->uio_resid > 0) {
        bytesToWrite = pVolDesc->vd_blkSize - off;
        if (bytesToWrite > args.uio->uio_resid) {
            bytesToWrite = args.uio->uio_resid;
        }

        if (bytesToEOF < bytesToWrite) {
            bytesToWrite = bytesToEOF;
        }

        if ((bytesToWrite != pVolDesc->vd_blkSize) && (bytesToWrite != 0)) {
            error = bread (pFileVnode, lbn, pVolDesc->vd_blkSize, NULL, &pBuf);
            if (error != OK) {
                return (error);
            }
        } else if (bytesToWrite != 0) {
            pBuf = buf_getblk (pFileVnode, lbn, pVolDesc->vd_blkSize);
        } else {
            return (EFBIG);
        }

        uiomove ((char *) pBuf->b_data + off, bytesToWrite, args.uio);

        error = bwrite (pBuf);
        if (error != OK) {
            return (error);
        }

        lbn++;

        /* Grow file, file is not permanent yet */
        if (lbn > (lblkno_t) pFileDesc->rfd_nBlks) {
            pFileDesc->rfd_nBlks++;
        }

        if (bytesToWrite == 0) {
            break;
        }

        /* Subsequent writes in this set will be block aligned */

        bytesToEOF -= bytesToWrite;
        off = 0;
    }

    return (error);
}

/***************************************************************************
 *
 * rt11VopIoctl -
 *
 * RETURNS: value from XBD layer
 */

LOCAL int  rt11VopIoctl (
    ioctl_args_t  args
    ) {
    RT11FS_DEV * pFsDev;
    int  rv;

    pFsDev = (RT11FS_DEV *) args.vp->v_mount->mnt_data;

    rv = xbdIoctl (pFsDev->volDesc.vd_device, args.cmd, args.data);

    return (rv);
}

/***************************************************************************
 *
 * rt11VopLink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopLink (
    link_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopUnlink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopUnlink (
    unlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopSymlink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopSymlink (
    symlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopReadlink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopReadlink (
    readlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopMkdir -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopMkdir (
    mkdir_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopRmdir -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopRmdir (
    rmdir_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopReaddir -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopReaddir (
    readdir_args_t  args
    ) {
    vnode_t  *            pDirVnode;
    RT11FS_INODE  *       pDirInode;
    RT11FS_DIR_DESC  *    pDirDesc;
    RT11FS_DIR_ENTRY  *   pDirEntry;
    size_t                len;
    int                   entryNum;
    char  *               pc;

    /* Get directory vnode */
    pDirVnode = args.dvp;
    if (pDirVnode == NULL) {
        return (ENOENT);
    }

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, pDirVnode);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Get entry number */
        entryNum = (int) args.dep->d_ino;

        /* Do while dir status is empty */
        do {
        if (entryNum >= pDirDesc->rdd_maxEntries) {
            *args.eof = 1;
            return (OK);
        }

        /* Get entry */
        pDirEntry = &pDirDesc->rdd_pDirSeg->ds_entries[entryNum];
        if (pDirEntry->de_status == DES_END) {
            *args.eof = 1;
                return (OK);
        }

        /* Advance */
        entryNum++;
    } while (pDirEntry->de_status == DES_EMPTY);

    /* Copy name to dirent */
    rt11fsNameString (pDirEntry->de_name, args.dep->d_name);
        
    /* Cancel leading spaces and dots */
    len = strlen (args.dep->d_name);
    if (len > 0) {
        pc = args.dep->d_name + len - 1;
        while ( (*pc == ' ') || (*pc == '.') ) {
            *pc = EOS;
            pc++;
        }
    }

    /* Update entry number */
    args.dep->d_ino = entryNum;
 
    return (OK);
}

/***************************************************************************
 *
 * rt11GetAttr -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopGetAttr (
    getattr_args_t  args
    ) {
    vnode_t  *           pFileVnode;
    RT11FS_DEV *         pFsDev;
    RT11FS_VOLUME_DESC * pVolDesc;
    RT11FS_INODE  *      pFileInode;
    RT11FS_FILE_DESC  *  pFileDesc;

    /* Get file vnode */
    pFileVnode = args.vp;
    if (pFileVnode == NULL) {
        return (ENOENT);
    }

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) pFileVnode->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get file inode */
    RT11FS_VTOI (pFileInode, pFileVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    args.vap->va_type   = VREG;
    args.vap->va_mode   = 0666;
    args.vap->va_nlink  = 1;
    args.vap->va_uid    = 0;
    args.vap->va_gid    = 0;
    args.vap->va_fsid   = 0;           /* Ignored for now. */
    args.vap->va_fileid = pFileInode->in_inode;
    args.vap->va_size   = pFileDesc->rfd_nBlks * pVolDesc->vd_blkSize;
    args.vap->va_blksize = pVolDesc->vd_blkSize;
    args.vap->va_atime.tv_sec     = 0;  /* dummy value */
    args.vap->va_mtime.tv_sec     = 0;  /* dummy value */
    args.vap->va_ctime.tv_sec     = 0;  /* dummy value */
    args.vap->va_birthtime.tv_sec = 0;  /* dummy value */
    args.vap->va_flags = 0;
#if 0                /* remaining fields are not yet used */
    args.vap->va_gen = 0;
    args.vap->va_rdev = 0;
    args.vap->va_bytes = 0;
    args.vap->va_filerev = 0;
    args.vap->va_vaflags = 0;
#endif

    return (OK);
}

/***************************************************************************
 *
 * rt11VopSetAttr -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopSetAttr (
    setattr_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopTruncate -
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int  rt11VopTruncate (
    truncate_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopFsync - fsync (has nothing to do as rt11FS is synchronous)
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopFsync (
    fsync_args_t  args
    ) {
    RT11FS_DEV *         pFsDev;
    RT11FS_VOLUME_DESC * pVolDesc;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* TODO:
     * Write directory segment to disk here
     */

    return (OK);
}

/***************************************************************************
 *
 * rt11VopActivate - activate the vnode
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rt11VopActivate (
    activate_args_t  args
    ) {
    RT11FS_INODE  *       pInode;
    enum vtype            type;
    int                   error;

    /* Get inode */
    error = rt11fsInodeGet (args.vp);
    if (error != OK) {
        return (error);
    }

    /* Get inode */
    RT11FS_VTOI (pInode, args.vp);

    /* Get type */
    type = pInode->in_type;

    /* Store type in vnode */
    args.vp->v_type = type;

    /* Select operators for type */
    switch (type) {
        case VDIR:
            args.vp->v_ops  = (vnode_ops_t *) &rt11VopsDir;
            break;

        case VREG:
            args.vp->v_ops  = (vnode_ops_t *) &rt11Vops;
            break;

        default:
            return (EINVAL);
    }

    return (OK);
}

/***************************************************************************
 *
 * rt11VopInactive -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopInactive (
    inactive_args_t  args
    ) {

    return (rt11fsInodeRelease (args.vp));
}

/***************************************************************************
 *
 * rt11VopPathconf -
 *
 * RETURNS: EINVAL
 */

LOCAL int  rt11VopPathconf (
    pathconf_args_t  args
    ) {
    return (EINVAL);
}

/***************************************************************************
 *
 * rt11VopSeek - seek (nothing to do as it is handled by VFS layer)
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopSeek (
    seek_args_t  args
    ) {
    return (OK);    /* nothing to do for seek operation */
}

/***************************************************************************
 *
 * rt11VopRename -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopRename (
    rename_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopAbort - not yet used by VFS
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopAbort (
    abort_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopStrategy -
 *
 * RETURNS: N/A
 */

LOCAL int rt11VopStrategy (
    strategy_args_t  args
    ) {
    vnode_t  *            pVnode;
    struct buf *          pBuf;

    /* Get vnode */
    pVnode = args.vp;
    if (pVnode == NULL) {
        return (EINVAL);
    }

    /* Get buffer */
    pBuf = args.bp;

    /* If read operation */
    if (pBuf->b_flags & B_READ) {
        rt11VopInternalReadStrategy (pVnode, pBuf);
        return (OK);
    }

    /* Must be a write operation */
    rt11VopInternalWriteStrategy (pVnode, pBuf);
    return (OK);
}

/***************************************************************************
 *
 * rt11VopPrint - print a vnode for debugging (nothing to do)
 *
 * RETURNS: N/A
 */

LOCAL int rt11VopPrint (
    print_args_t  args
    ) {
    return;
}

/***************************************************************************
 *
 * rt11VopDirGetAttr -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopDirGetAttr (
    getattr_args_t  args
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    args.vap->va_type   = VDIR;
    args.vap->va_mode   = 0666;
    args.vap->va_nlink  = 1;
    args.vap->va_uid    = 0;
    args.vap->va_gid    = 0;
    args.vap->va_fsid   = 0;           /* Ignored for now. */
    args.vap->va_fileid = RT11FS_ROOT_INODE;
    args.vap->va_size   = pVolDesc->vd_nSegBlks * pVolDesc->vd_blkSize;
    args.vap->va_blksize = pVolDesc->vd_blkSize;
    args.vap->va_atime.tv_sec     = 0;  /* dummy value */
    args.vap->va_mtime.tv_sec     = 0;  /* dummy value */
    args.vap->va_ctime.tv_sec     = 0;  /* dummy value */
    args.vap->va_birthtime.tv_sec = 0;  /* dummy value */
    args.vap->va_flags = 0;
#if 0                /* remaining fields are not yet used */
    args.vap->va_gen = 0;
    args.vap->va_rdev = 0;
    args.vap->va_bytes = 0;
    args.vap->va_filerev = 0;
    args.vap->va_vaflags = 0;
#endif

    return (OK);
}

/***************************************************************************
 *
 * rt11VopDirStrategy -
 *
 * RETURNS: N/A
 */

LOCAL int rt11VopDirStrategy (
    strategy_args_t  args
    ) {
    return (EINVAL);
}

/***************************************************************************
 *
 * rt11VopSyncerStrategy -
 *
 * RETURNS: N/A
 */

LOCAL int rt11VopSyncerStrategy (
    strategy_args_t  args
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    struct bio *          pBio;
    struct buf *          pBuf;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get buffer */
    pBuf = args.bp;

    pBio = pBuf->b_bio;
    pBio->bio_blkno   = (pBuf->b_blkno << pVolDesc->vd_secPerBlk2);
    pBio->bio_bcount  = pVolDesc->vd_blkSize;
    pBio->bio_error   = OK;
    xbdStrategy (pVolDesc->vd_device, pBio);
}

/***************************************************************************
 *
 * rt11VopInternalReadStrategy -
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopInternalReadStrategy (
    struct vnode  * pVnode,
    struct buf  *   pBuf
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_INODE  *       pInode;
    RT11FS_VOLUME_DESC  * pVolDesc;
    RT11FS_FILE_DESC  *   pFileDesc;

    struct vnode *        pSyncer;
    struct buf *          pTmpBuf;
    lblkno_t              physBlk;
    int                   error;

    /* Get syncer */
    pSyncer = pVnode->v_mount->mnt_syncer;

    /* Get inode */
    RT11FS_VTOI (pInode, pVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pInode->in_data;

    /* Get volume descriptor */
    pFsDev = (RT11FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    /* Calculate physical block number */
    physBlk = (pBuf->b_blkno << pVolDesc->vd_secPerBlk2) +
               pFileDesc->rfd_startBlock;

    /* Read block from disk */
    error = bread (pSyncer, physBlk, pVolDesc->vd_blkSize, NULL, &pTmpBuf);
    if (error) {
        return;
    }

    buf_swapdata (pTmpBuf, pBuf);
    pTmpBuf->b_flags |= B_INVAL;
    brelse (pTmpBuf);

    buf_done (pBuf, error);
}

/***************************************************************************
 *
 * rt11VopInternalWriteStrategy -
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopInternalWriteStrategy (
    struct vnode  * pVnode,
    struct buf  *   pBuf
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_INODE  *       pInode;
    RT11FS_VOLUME_DESC  * pVolDesc;
    RT11FS_FILE_DESC  *   pFileDesc;

    struct vnode *        pSyncer;
    struct buf *          pTmpBuf;
    lblkno_t              physBlk;
    int                   error;

    /* Get syncer */
    pSyncer = pVnode->v_mount->mnt_syncer;

    /* Get inode */
    RT11FS_VTOI (pInode, pVnode);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pInode->in_data;

    /* Get volume descriptor */
    pFsDev = (RT11FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    /* Calculate physical block number */
    physBlk = (pBuf->b_blkno << pVolDesc->vd_secPerBlk2) +
               pFileDesc->rfd_startBlock;

    /* Get a temporary buffer */
    pTmpBuf = buf_getblk (pSyncer, physBlk, pVolDesc->vd_blkSize);

    buf_swapdata (pBuf, pTmpBuf);
    buf_startwrite (pTmpBuf);
    error = buf_wait (pTmpBuf);
    buf_swapdata (pBuf, pTmpBuf);
    pTmpBuf->b_flags |= B_INVAL;
    /*
     * Done by: buf_done()
     * Is this correct?
     */
    brelse (pTmpBuf);

    buf_done (pBuf, error);
}

