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

/* rawVopsLib.c - Raw fs vfs operators */

/*
DESCRIPTION:
RawFS treats the entire device as a single file.  It is useful for formatting
the device as well as for reading the contents so as to determine what type of
(if any) file system is installed on it.
*/

/* includes */
#include <stdlib.h>
#include <vmx.h>
#include <os/erfLib.h>

#include <fs/bio.h>
#include <fs/buf.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/vnode.h>
#include <fs/rawfsLib.h>

/* forward declarations */
LOCAL int rawVopLookup (lookup_args_t args);
LOCAL int rawVopCreate (create_args_t  args);
LOCAL int rawVopOpen (open_args_t  args);
LOCAL int rawVopClose (close_args_t args);
LOCAL int rawVopAccess (access_args_t args);
LOCAL int rawVopRead (read_args_t args);
LOCAL int rawVopWrite (write_args_t args);
LOCAL int rawVopIoctl (ioctl_args_t args);
LOCAL int rawVopLink (link_args_t args);
LOCAL int rawVopUnlink (unlink_args_t args);
LOCAL int rawVopSymlink (symlink_args_t args);
LOCAL int rawVopReadlink (readlink_args_t args);
LOCAL int rawVopMkdir (mkdir_args_t args);
LOCAL int rawVopRmdir (rmdir_args_t args);
LOCAL int rawVopReaddir (readdir_args_t args);
LOCAL int rawVopGetAttr (getattr_args_t args);
LOCAL int rawVopSetAttr (setattr_args_t args);
LOCAL int rawVopTruncate (truncate_args_t args);
LOCAL int rawVopFsync (fsync_args_t args);
LOCAL int rawVopActivate (activate_args_t args);
LOCAL int rawVopInactive (inactive_args_t args);
LOCAL int rawVopPathconf (pathconf_args_t args);
LOCAL int rawVopSeek (seek_args_t args);
LOCAL int rawVopRename (rename_args_t args);
LOCAL int rawVopAbort (abort_args_t args);
LOCAL int rawVopStrategy (strategy_args_t args);
LOCAL int rawVopPrint (print_args_t args);

LOCAL void rawBioDone (struct bio *pBio);

/* globals */

const vnode_ops_t  rawVops =
{
    rawVopLookup,
    rawVopCreate,
    rawVopOpen,
    rawVopClose,
    rawVopAccess,
    rawVopRead,
    rawVopWrite,
    rawVopIoctl,
    rawVopLink,
    rawVopUnlink,
    rawVopSymlink,
    rawVopReadlink,
    rawVopMkdir,
    rawVopRmdir,
    rawVopReaddir,
    rawVopGetAttr,
    rawVopSetAttr,
    rawVopTruncate,
    rawVopFsync,
    rawVopActivate,
    rawVopInactive,
    rawVopPathconf,
    rawVopSeek,
    rawVopRename,
    rawVopAbort,
    rawVopStrategy,
    rawVopPrint
};

/***************************************************************************
 *
 * rawVopLookup - directory lookup
 *
 * On rawFS, the entire device is treated as a single file, so there is no
 * lookup like on other file systems.
 *
 * RETURNS: OK if found, error otherwise
 */

LOCAL int rawVopLookup (
    lookup_args_t  args
    ) {
    /* 
     * TODO:
     * For now, only return ENOENT.  Later, figure out what is needed to
     * only open the device file.  That may require some changes to the VFS.
     */
    return (ENOENT);
}

/***************************************************************************
 *
 * rawVopCreate - creating a file is not supported for rawFS
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopCreate (
    create_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopOpen - open a file (has nothing special to do)
 *
 * RETURNS: OK
 */

LOCAL int rawVopOpen (
    open_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rawVopClose - close a file (has nothing special to do)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopClose (
    close_args_t  args
    ) {
    /* rawFS does not have any timestamps to update */
    return (OK);
}

/***************************************************************************
 *
 * rawVopAccess - determine if this file can be accessed
 *
 * RETURNS: OK
 */

LOCAL int  rawVopAccess (
    access_args_t  args
    ) {
    /*
     * rawFS does not have any permission restrictions.  If it did, it would
     * need to call vaccess().
     */

    return (OK);
}

/***************************************************************************
 *
 * rawVopRead - read from rawFS
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rawVopRead (
    read_args_t  args
    ) {
    RAWFS_VOLUME_DESC *  pVolDesc;
    RAWFS_DEV *          pFsDev;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToRead, bytesToEOF;

    if (args.uio->uio_resid == 0) {    /* If nothing to do, return early. */
        return (OK);
    }

    pFsDev = (RAWFS_DEV *)  args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    lbn = (args.uio->uio_offset >> pVolDesc->blkSize2);
    off = (args.uio->uio_offset & (pVolDesc->blkSize - 1));
    bytesToRead = pVolDesc->blkSize - off;
    bytesToEOF  = pVolDesc->diskSize - args.uio->uio_offset;

    while (args.uio->uio_resid > 0) {
        error = bread (args.vp, lbn, pVolDesc->blkSize, NULL, &pBuf);
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
        bytesToRead = pVolDesc->blkSize;
    }

    return (error);
}

/***************************************************************************
 *
 * rawVopWrite - write to rawFS
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rawVopWrite (
    write_args_t  args
    ) {
    RAWFS_VOLUME_DESC *  pVolDesc;
    RAWFS_DEV *          pFsDev;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToWrite, bytesToEOF;

    if (args.uio->uio_resid == 0) {    /* If nothing to do, return early. */
        return (OK);
    }

    pFsDev = (RAWFS_DEV *)  args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    lbn = (args.uio->uio_offset >> pVolDesc->blkSize2);
    off = (args.uio->uio_offset & (pVolDesc->blkSize - 1));
    bytesToEOF  = pVolDesc->diskSize - args.uio->uio_offset;

    while (args.uio->uio_resid > 0) {
        bytesToWrite = pVolDesc->blkSize - off;
        if (bytesToWrite > args.uio->uio_resid) {
            bytesToWrite = args.uio->uio_resid;
        }

        if (bytesToEOF < bytesToWrite) {
            bytesToWrite = bytesToEOF;
        }

        if ((bytesToWrite != pVolDesc->blkSize) && (bytesToWrite != 0)) {
            error = bread (args.vp, lbn, pVolDesc->blkSize, NULL, &pBuf);
            if (error != OK) {
                return (error);
            }
        } else if (bytesToWrite != 0) {
            pBuf = buf_getblk (args.vp, lbn, pVolDesc->blkSize);
        } else {
            return (EFBIG);
        }

        uiomove ((char *) pBuf->b_data + off, bytesToWrite, args.uio);

        error = bwrite (pBuf);
        if (error != OK) {
            return (error);
        }

        lbn++;

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
 * rawVopIoctl - rawFS specific ioctl() commands
 *
 * RETURNS: value from XBD layer
 */

LOCAL int  rawVopIoctl (
    ioctl_args_t  args
    ) {
    RAWFS_DEV *  pFsDev;
    int  rv;

    pFsDev = (RAWFS_DEV *) args.vp->v_mount->mnt_data;

    rv = xbdIoctl (pFsDev->volDesc.device, args.cmd, args.data);

    return (rv);
}

/***************************************************************************
 *
 * rawVopLink - create a hard link (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopLink (
    link_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopUnlink - unlink a file (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopUnlink (
    unlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopSymlink - create a symlink (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopSymlink (
    symlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopReadlink - read a symlink (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopReadlink (
    readlink_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopMkdir - create a directory (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopMkdir (
    mkdir_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopRmdir - remove a directory (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopRmdir (
    rmdir_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopReaddir - read a directory entry (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopReaddir (
    readdir_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawGetAttr - get file attributes
 *
 * RETURNS: OK
 */

LOCAL int  rawVopGetAttr (
    getattr_args_t  args
    ) {
    RAWFS_DEV *  pFsDev;
    RAWFS_VOLUME_DESC *  pVolDesc;

    pFsDev   = (RAWFS_DEV *) args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    args.vap->va_type   = VREG;
    args.vap->va_mode   = 0666;
    args.vap->va_nlink  = 1;
    args.vap->va_uid    = 0;
    args.vap->va_gid    = 0;
    args.vap->va_fsid   = 0;           /* Ignored for now. */
    args.vap->va_fileid = RAWFS_ROOT_INODE;
    args.vap->va_size   = pVolDesc->diskSize;
    args.vap->va_blksize = pVolDesc->blkSize;
    args.vap->va_atime.tv_sec     = 0;  /* dummy value */
    args.vap->va_mtime.tv_sec     = 0;  /* dummy value */
    args.vap->va_ctime.tv_sec     = 0;  /* dummy value */
    args.vap->va_birthtime.tv_sec = 0;  /* dummy value */
    args.vap->va_flags = 0;
#ifdef notyet                /* remaining fields are not yet used */
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
 * rawVopSetAttr - set file attributes (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopSetAttr (
    setattr_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopTruncate - change file size (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopTruncate (
    truncate_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopFsync - fsync (has nothing to do as rawFS is synchronous)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopFsync (
    fsync_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rawVopActivate - activate the vnode
 *
 * RETURNS: OK
 */

LOCAL int  rawVopActivate (
    activate_args_t  args
    ) {
    args.vp->v_type = VREG;
    args.vp->v_ops  = (vnode_ops_t *) &rawVops;

    return (OK);
}

/***************************************************************************
 *
 * rawVopInactive - deactivate the vnode (has nothing to do)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopInactive (
    inactive_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rawVopPathconf - pathconf() not applicable to rawFS
 *
 * RETURNS: EINVAL
 */

LOCAL int  rawVopPathconf (
    pathconf_args_t  args
    ) {
    return (EINVAL);
}

/***************************************************************************
 *
 * rawVopSeek - seek (nothing to do as it is handled by VFS layer)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopSeek (
    seek_args_t  args
    ) {
    return (OK);    /* nothing to do for seek operation */
}

/***************************************************************************
 *
 * rawVopRename - rename file (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopRename (
    rename_args_t  args
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopAbort - not yet used by VFS
 *
 * RETURNS: OK
 */

LOCAL int  rawVopAbort (
    abort_args_t  args
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rawVopStrategy - strategy routine for rawFS
 *
 * RETURNS: N/A
 */

LOCAL int rawVopStrategy (
    strategy_args_t  args
    ) {
    RAWFS_DEV *          pFsDev;
    RAWFS_VOLUME_DESC *  pVolDesc;
    struct bio *         pBio;
    struct buf *         pBuf;

    pFsDev   = (RAWFS_DEV *) args.vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;
    pBuf = args.bp;

    /*
     * The logical block number is the same as the physical block number
     * in rawFS.  Thus no translation is required between the two.
     */

    pBio = pBuf->b_bio;

    if (pBuf->b_flags & B_READ) {
        pBio->bio_flags = BIO_READ;
    }
    else {
        pBio->bio_flags = BIO_WRITE;
    }

    pBio->bio_blkno   = (pBuf->b_blkno << pVolDesc->secPerBlk2);
    pBio->bio_bcount  = pVolDesc->blkSize;
    pBio->bio_error   = OK;
#ifdef fssync
    pBio->bio_done    = rawBioDone;
    pBio->bio_caller1 = pFsDev->bioSem;
#endif

    xbdStrategy (pVolDesc->device, pBio);

#ifdef fssync
    semTake (pFsDev->bioSem, WAIT_FOREVER);

    buf_done (pBuf, OK);
#endif
}

/***************************************************************************
 *
 * rawVopPrint - print a vnode for debugging (nothing to do)
 *
 * RETURNS: N/A
 */

LOCAL int rawVopPrint (
    print_args_t  args
    ) {
    return;
}

/***************************************************************************
 *
 * rawBioDone - called when I/O is complete
 *
 * RETURNS: N/A
 */

LOCAL void rawBioDone (
    struct bio *pBio
    ) {
    semGive ((SEM_ID) pBio->bio_caller1);
}

