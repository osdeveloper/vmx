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
LOCAL int rawVopLookup (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* retrieved vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rawVopCreate (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
    );

LOCAL int rawVopOpen (
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopClose (
    struct vnode *  vp,            /* file vnode pointer */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopAccess (
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopSetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopRead (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopWrite (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopIoctl (
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopFcntl (
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopFsync (
    struct vnode *  vp,            /* file vnode pointer */
    struct ucred *  ucp,           /* user credentials pointer */
    int             flags          /* flags */
    );

LOCAL int rawVopSeek (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           off1,          /* old offset */
    off_t           off2,          /* new offset */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopRemove (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rawVopLink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rawVopRename (
    struct vnode *          fdvp,  /* from directory vnode pointer */
    struct vnode *          fvp,   /* from file vnode pointer */
    struct componentname *  fcnp,  /* from path name component pointer */
    struct vnode *          tdvp,  /* to directory vnode pointer */
    struct vnode *          tvp,   /* to file vnode pointer */
    struct componentname *  tcnp   /* to path name component pointer */
    );

LOCAL int rawVopMkdir (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
    );

LOCAL int rawVopRmdir (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rawVopSymlink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap,   /* vnode attributes pointer */
    char *                  tgt    /* ptr to target path string */
    );

LOCAL int rawVopReaddir (
    struct vnode *   vp,           /* directory vnode pointer */
    struct dirent *  dep,          /* directory entry pointer */
    struct ucred *   ucp,          /* user credentials pointer */
    int *            eof,          /* end of file status */
    u_long *         cookies       /* cookies */
    );

LOCAL int rawVopReadlink (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rawVopAbortop (
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rawVopActivate (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL int rawVopInactive (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL int rawVopStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    );

LOCAL int rawVopPrint (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL int rawVopPathconf (
    struct vnode *  vp,            /* file vnode pointer */
    int             name,          /* type of info to return */
    int *           rv             /* return value */
    );

LOCAL int rawVopAdvlock(
    struct vnode *  vp,            /* file vnode pointer */
    void *          id,            /* identifier */
    int             op,            /* operation */
    struct flock *  fl,            /* file loock */
    int             flags          /* flags */
    );

LOCAL int rawVopTruncate (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           len,           /* new length of the file */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL void rawBioDone (struct bio *pBio);

/* globals */

const vnode_ops_t  rawVops =
{
    rawVopLookup,
    rawVopCreate,
    rawVopOpen,
    rawVopClose,
    rawVopAccess,
    rawVopGetAttr,
    rawVopSetAttr,
    rawVopRead,
    rawVopWrite,
    rawVopIoctl,
    rawVopFcntl,
    rawVopFsync,
    rawVopSeek,
    rawVopRemove,
    rawVopLink,
    rawVopRename,
    rawVopMkdir,
    rawVopRmdir,
    rawVopSymlink,
    rawVopReaddir,
    rawVopReadlink,
    rawVopAbortop,
    rawVopActivate,
    rawVopInactive,
    rawVopStrategy,
    rawVopPrint,
    rawVopPathconf,
    rawVopAdvlock,
    rawVopTruncate
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* retrieved vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    /*
     * rawFS does not have any permission restrictions.  If it did, it would
     * need to call vaccess().
     */

    return (OK);
}

/***************************************************************************
 *
 * rawGetAttr - get file attributes
 *
 * RETURNS: OK
 */

LOCAL int  rawVopGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RAWFS_DEV *  pFsDev;
    RAWFS_VOLUME_DESC *  pVolDesc;

    pFsDev   = (RAWFS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    vap->va_type   = VREG;
    vap->va_mode   = 0666;
    vap->va_nlink  = 1;
    vap->va_uid    = 0;
    vap->va_gid    = 0;
    vap->va_fsid   = 0;            /* Ignored for now. */
    vap->va_fileid = RAWFS_ROOT_INODE;
    vap->va_size   = pVolDesc->diskSize;
    vap->va_blksize = pVolDesc->blkSize;
    vap->va_atime.tv_sec     = 0;  /* dummy value */
    vap->va_mtime.tv_sec     = 0;  /* dummy value */
    vap->va_ctime.tv_sec     = 0;  /* dummy value */
    vap->va_birthtime.tv_sec = 0;  /* dummy value */
    vap->va_flags = 0;
#ifdef notyet                /* remaining fields are not yet used */
    vap->va_gen = 0;
    vap->va_rdev = 0;
    vap->va_bytes = 0;
    vap->va_filerev = 0;
    vap->va_vaflags = 0;
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
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopRead - read from rawFS
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rawVopRead (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RAWFS_VOLUME_DESC *  pVolDesc;
    RAWFS_DEV *          pFsDev;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToRead, bytesToEOF;

    if (uio->uio_resid == 0) {     /* If nothing to do, return early. */
        return (OK);
    }

    pFsDev = (RAWFS_DEV *)  vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    lbn = (uio->uio_offset >> pVolDesc->blkSize2);
    off = (uio->uio_offset & (pVolDesc->blkSize - 1));
    bytesToRead = pVolDesc->blkSize - off;
    bytesToEOF  = pVolDesc->diskSize - uio->uio_offset;

    while (uio->uio_resid > 0) {
        error = bread (vp, lbn, pVolDesc->blkSize, NULL, &pBuf);
        if (error != OK) {
            return (error);
        }

        if (bytesToRead > uio->uio_resid) {
            bytesToRead = uio->uio_resid;
        }

        if (bytesToEOF < bytesToRead) {
            bytesToRead = bytesToEOF;
        }

        uiomove ((char *) pBuf->b_data + off, bytesToRead, uio);

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
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RAWFS_VOLUME_DESC *  pVolDesc;
    RAWFS_DEV *          pFsDev;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToWrite, bytesToEOF;

    if (uio->uio_resid == 0) {     /* If nothing to do, return early. */
        return (OK);
    }

    pFsDev = (RAWFS_DEV *)  vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    lbn = (uio->uio_offset >> pVolDesc->blkSize2);
    off = (uio->uio_offset & (pVolDesc->blkSize - 1));
    bytesToEOF  = pVolDesc->diskSize - uio->uio_offset;

    while (uio->uio_resid > 0) {
        bytesToWrite = pVolDesc->blkSize - off;
        if (bytesToWrite > uio->uio_resid) {
            bytesToWrite = uio->uio_resid;
        }

        if (bytesToEOF < bytesToWrite) {
            bytesToWrite = bytesToEOF;
        }

        if ((bytesToWrite != pVolDesc->blkSize) && (bytesToWrite != 0)) {
            error = bread (vp, lbn, pVolDesc->blkSize, NULL, &pBuf);
            if (error != OK) {
                return (error);
            }
        } else if (bytesToWrite != 0) {
            pBuf = buf_getblk (vp, lbn, pVolDesc->blkSize);
        } else {
            return (EFBIG);
        }

        uiomove ((char *) pBuf->b_data + off, bytesToWrite, uio);

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
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RAWFS_DEV *  pFsDev;
    int  rv;

    pFsDev = (RAWFS_DEV *) vp->v_mount->mnt_data;

    rv = xbdIoctl (pFsDev->volDesc.device, cmd, data);

    return (rv);
}

/***************************************************************************
 *
 * rawVopFcntl - rawFS specific ioctl() commands
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopFcntl (
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    struct ucred *  ucp,           /* user credentials pointer */
    int             flags          /* flags */
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rawVopSeek - seek (nothing to do as it is handled by VFS layer)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopSeek (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           off1,          /* old offset */
    off_t           off2,          /* new offset */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (OK);    /* nothing to do for seek operation */
}

/***************************************************************************
 *
 * rawVopRemove - remove a file (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopRemove (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopLink - create a hard link (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopLink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopRename - rename file (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rawVopRename (
    struct vnode *          fdvp,  /* from directory vnode pointer */
    struct vnode *          fvp,   /* from file vnode pointer */
    struct componentname *  fcnp,  /* from path name component pointer */
    struct vnode *          tdvp,  /* to directory vnode pointer */
    struct vnode *          tvp,   /* to file vnode pointer */
    struct componentname *  tcnp   /* to path name component pointer */
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap,   /* vnode attributes pointer */
    char *                  tgt    /* ptr to target path string */
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
    struct vnode *   vp,           /* directory vnode pointer */
    struct dirent *  dep,          /* directory entry pointer */
    struct ucred *   ucp,          /* user credentials pointer */
    int *            eof,          /* end of file status */
    u_long *         cookies       /* cookies */
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
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rawVopAbortop - not yet used by VFS
 *
 * RETURNS: OK
 */

LOCAL int  rawVopAbortop (
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
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
    struct vnode *  vp             /* file vnode pointer */
    ) {
    vp->v_type = VREG;
    vp->v_ops  = (vnode_ops_t *) &rawVops;

    return (OK);
}

/***************************************************************************
 *
 * rawVopInactive - deactivate the vnode (has nothing to do)
 *
 * RETURNS: OK
 */

LOCAL int  rawVopInactive (
    struct vnode *  vp             /* file vnode pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    ) {
    RAWFS_DEV *          pFsDev;
    RAWFS_VOLUME_DESC *  pVolDesc;
    struct bio *         pBio;
    struct buf *         pBuf;

    pFsDev   = (RAWFS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /*
     * The logical block number is the same as the physical block number
     * in rawFS.  Thus no translation is required between the two.
     */

    if (bp->b_flags & B_READ) {
    }

    pBio = bp->b_bio;
    pBio->bio_blkno   = (bp->b_blkno << pVolDesc->secPerBlk2);
    pBio->bio_bcount  = pVolDesc->blkSize;
    pBio->bio_error   = OK;
#ifdef fssync
    pBio->bio_done    = rawBioDone;
    pBio->bio_caller1 = pFsDev->bioSem;
#endif

    xbdStrategy (pVolDesc->device, pBio);
#ifdef fssync
    semTake (pFsDev->bioSem, WAIT_FOREVER);

    buf_done (bp, OK);
#endif
}

/***************************************************************************
 *
 * rawVopPrint - print a vnode for debugging (nothing to do)
 *
 * RETURNS: N/A
 */

LOCAL int rawVopPrint (
    struct vnode *  vp             /* file vnode pointer */
    ) {
    return;
}

/***************************************************************************
 *
 * rawVopPathconf - pathconf() not applicable to rawFS
 *
 * RETURNS: EINVAL
 */

LOCAL int  rawVopPathconf (
    struct vnode *  vp,            /* file vnode pointer */
    int             name,          /* type of info to return */
    int *           rv             /* return value */
    ) {
    return (EINVAL);
}

/***************************************************************************
 *
 * rawVopAdvlock - advisory record locking (not supported in rawFS)
 *
 * RETURNS: ENOSYS
 */

LOCAL int rawVopAdvlock(
    struct vnode *  vp,            /* file vnode pointer */
    void *          id,            /* identifier */
    int             op,            /* operation */
    struct flock *  fl,            /* file loock */
    int             flags          /* flags */
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
    struct vnode *  vp,            /* file vnode pointer */
    off_t           len,           /* new length of the file */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (ENOSYS);
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

