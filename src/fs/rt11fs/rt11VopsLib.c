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
LOCAL int rt11VopLookup (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* retrieved vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rt11VopCreate (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
    );

LOCAL int rt11VopOpen (
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopClose (
    struct vnode *  vp,            /* file vnode pointer */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopAccess (
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopSetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopRead (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopWrite (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopIoctl (
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopFcntl (
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopFsync (
    struct vnode *  vp,            /* file vnode pointer */
    struct ucred *  ucp,           /* user credentials pointer */
    int             flags          /* flags */
    );

LOCAL int rt11VopSeek (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           off1,          /* old offset */
    off_t           off2,          /* new offset */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopRemove (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rt11VopLink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rt11VopRename (
    struct vnode *          fdvp,  /* from directory vnode pointer */
    struct vnode *          fvp,   /* from file vnode pointer */
    struct componentname *  fcnp,  /* from path name component pointer */
    struct vnode *          tdvp,  /* to directory vnode pointer */
    struct vnode *          tvp,   /* to file vnode pointer */
    struct componentname *  tcnp   /* to path name component pointer */
    );

LOCAL int rt11VopMkdir (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
    );

LOCAL int rt11VopRmdir (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rt11VopSymlink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap,   /* vnode attributes pointer */
    char *                  tgt    /* ptr to target path string */
    );

LOCAL int rt11VopReaddir (
    struct vnode *   vp,           /* directory vnode pointer */
    struct dirent *  dep,          /* directory entry pointer */
    struct ucred *   ucp,          /* user credentials pointer */
    int *            eof,          /* end of file status */
    int *            cookies       /* cookies */
    );

LOCAL int rt11VopReadlink (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopAbortop (
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    );

LOCAL int rt11VopActivate (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL int rt11VopInactive (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL void rt11VopStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    );

LOCAL void rt11VopPrint (
    struct vnode *  vp             /* file vnode pointer */
    );

LOCAL int rt11VopPathconf (
    struct vnode *  vp,            /* file vnode pointer */
    int             name,          /* type of info to return */
    long *          rv             /* return value */
    );

LOCAL int rt11VopAdvlock (
    struct vnode *  vp,            /* file vnode pointer */
    void *          id,            /* identifier */
    int             op,            /* operation */
    struct flock *  fl,            /* file loock */
    int             flags          /* flags */
    );

LOCAL int rt11VopTruncate (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           len,           /* new length of the file */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL int rt11VopDirGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    );

LOCAL void rt11VopDirStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    );

LOCAL void rt11VopSyncerStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    );

LOCAL void rt11VopInternalReadStrategy (
    struct vnode *pVnode,
    struct buf *pBuf
    );

LOCAL void rt11VopInternalWriteStrategy (
    struct vnode *pVnode,
    struct buf *pBuf
    );

/* globals */

const vnode_ops_t  rt11Vops =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopGetAttr,
    rt11VopSetAttr,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopFcntl,
    rt11VopFsync,
    rt11VopSeek,
    rt11VopRemove,
    rt11VopLink,
    rt11VopRename,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopSymlink,
    rt11VopReaddir,
    rt11VopReadlink,
    rt11VopAbortop,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopStrategy,
    rt11VopPrint,
    rt11VopPathconf,
    rt11VopAdvlock,
    rt11VopTruncate
};

const vnode_ops_t  rt11VopsDir =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopDirGetAttr,
    rt11VopSetAttr,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopFcntl,
    rt11VopFsync,
    rt11VopSeek,
    rt11VopRemove,
    rt11VopLink,
    rt11VopRename,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopSymlink,
    rt11VopReaddir,
    rt11VopReadlink,
    rt11VopAbortop,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopDirStrategy,
    rt11VopPrint,
    rt11VopPathconf,
    rt11VopAdvlock,
    rt11VopTruncate
};

const vnode_ops_t  rt11VopsSyncer =
{
    rt11VopLookup,
    rt11VopCreate,
    rt11VopOpen,
    rt11VopClose,
    rt11VopAccess,
    rt11VopGetAttr,
    rt11VopSetAttr,
    rt11VopRead,
    rt11VopWrite,
    rt11VopIoctl,
    rt11VopFcntl,
    rt11VopFsync,
    rt11VopSeek,
    rt11VopRemove,
    rt11VopLink,
    rt11VopRename,
    rt11VopMkdir,
    rt11VopRmdir,
    rt11VopSymlink,
    rt11VopReaddir,
    rt11VopReadlink,
    rt11VopAbortop,
    rt11VopActivate,
    rt11VopInactive,
    rt11VopSyncerStrategy,
    rt11VopPrint,
    rt11VopPathconf,
    rt11VopAdvlock,
    rt11VopTruncate
};

/***************************************************************************
 *
 * rt11VopLookup - directory lookup
 *
 * RETURNS: OK if found, error otherwise
 */

LOCAL int rt11VopLookup (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* retrieved vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    RT11FS_INODE  *     pDirInode;
    RT11FS_DIR_DESC  *  pDirDesc;
    vnode_t  *          pFileVnode;
    RT11FS_INODE  *     pFileInode;
    RT11FS_FILE_DESC  * pFileDesc;
    RT11FS_DIR_ENTRY    dirEntry;
    int                 entryNum, startBlock;
    int                 error;

    /* Check directory vnode */
    if (dvp == NULL) {
        return (EINVAL);
    }

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, dvp);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Get entry number */
    if ((entryNum = rt11fsFindDirEntry (pDirDesc->rdd_pDirSeg,
                                        &dirEntry,
                                        &startBlock,
                                        cnp->cn_nameptr)) == ERROR) {
        return (ENOENT);
    }

    /* Can not open until file is permanent */
    if (dirEntry.de_status == DES_TENTATIVE) {
        return (EINVAL);
    }

    /* Get file vnode */
    error = vgetino (dvp->v_mount,
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

    *vpp = pFileVnode;
    return (error);
}

/***************************************************************************
 *
 * rt11VopCreate -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopCreate (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
    ) {
    RT11FS_INODE *       pDirInode;
    RT11FS_DIR_DESC *    pDirDesc;
    RT11FS_DIR_ENTRY     dirEntry;
    vnode_t *            pFileVnode;
    RT11FS_INODE *       pFileInode;
    RT11FS_FILE_DESC *   pFileDesc;
    RT11FS_DEV  *        pFsDev;
    RT11FS_VOLUME_DESC * pVolDesc;
    int                  entryNum, startBlock;
    int                  error;

    /* Check directory vnode */
    if (dvp == NULL) {
        return (EINVAL);
    }

    /* Get volume descriptor */
    pFsDev = (RT11FS_DEV *) dvp->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, dvp);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Allocate a new directory entry */
    if ((entryNum = rt11fsAllocNewDirEntry (pDirDesc->rdd_pDirSeg,
                                            pDirDesc->rdd_maxEntries,
                                            &dirEntry,
                                            &startBlock,
                                            cnp->cn_nameptr)) == ERROR) {
        return (ENOENT);
    }

    /* Get file vnode */
    error = vgetino (dvp->v_mount,
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
    VOP_FSYNC (pFileVnode, NULL, 0);
    rt11fsVolFlush(pVolDesc);

    *vpp = pFileVnode;
    return (error);
}

/***************************************************************************
 *
 * rt11VopOpen -
 *
 * RETURNS: OK
 */

LOCAL int rt11VopOpen (
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
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
    struct vnode *  vp,            /* file vnode pointer */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    RT11FS_DIR_ENTRY  *   pDirEntry;
    int                   blksLeft, entryNum;

    /* Check vnode */
    if (vp == NULL) {
        return (EINVAL);
    }

    /* Get volume descriptor */
    pFsDev = (RT11FS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    /* Get inode */
    RT11FS_VTOI (pFileInode, vp);

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
        VOP_FSYNC (vp, NULL, 0);
        rt11fsVolFlush(pVolDesc);
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
    struct vnode *  vp,            /* file vnode pointer */
    int             mode,          /* mode */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11GetAttr -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_DEV *         pFsDev;
    RT11FS_VOLUME_DESC * pVolDesc;
    RT11FS_INODE  *      pFileInode;
    RT11FS_FILE_DESC  *  pFileDesc;

    /* Check file vnode */
    if (vp == NULL) {
        return (ENOENT);
    }

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get file inode */
    RT11FS_VTOI (pFileInode, vp);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    vap->va_type   = VREG;
    vap->va_mode   = 0666;
    vap->va_nlink  = 1;
    vap->va_uid    = 0;
    vap->va_gid    = 0;
    vap->va_fsid   = 0;           /* Ignored for now. */
    vap->va_fileid = pFileInode->in_inode;
    vap->va_size   = pFileDesc->rfd_nBlks * pVolDesc->vd_blkSize;
    vap->va_blksize = pVolDesc->vd_blkSize;
    vap->va_atime.tv_sec     = 0;  /* dummy value */
    vap->va_mtime.tv_sec     = 0;  /* dummy value */
    vap->va_ctime.tv_sec     = 0;  /* dummy value */
    vap->va_birthtime.tv_sec = 0;  /* dummy value */
    vap->va_flags = 0;
#if 0                /* remaining fields are not yet used */
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
 * rt11VopSetAttr -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopSetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopRead -
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rt11VopRead (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_VOLUME_DESC *  pVolDesc;
    RT11FS_DEV *          pFsDev;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToRead, bytesToEOF;

    if (uio->uio_resid == 0) {     /* If nothing to do, return early. */
        return (OK);
    }

    /* Check vnode */
    if (vp == NULL) {
        return (EINVAL);
    }

    pFsDev = (RT11FS_DEV *)  vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get inode */
    RT11FS_VTOI (pFileInode, vp);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    lbn = (uio->uio_offset >> pVolDesc->vd_blkSize2);
    off = (uio->uio_offset & (pVolDesc->vd_blkSize - 1));
    bytesToRead = pVolDesc->vd_blkSize - off;
    bytesToEOF  = (pFileDesc->rfd_nBlks << pVolDesc->vd_blkSize2) -
                  uio->uio_offset;

    while (uio->uio_resid > 0) {
        error = bread (vp, lbn, pVolDesc->vd_blkSize, NULL, &pBuf);
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
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    int             ioflag,        /* IO flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_VOLUME_DESC *  pVolDesc;
    RT11FS_DEV *          pFsDev;
    RT11FS_INODE  *       pFileInode;
    RT11FS_FILE_DESC  *   pFileDesc;
    buf_t *   pBuf;
    int       error;
    lblkno_t  lbn;
    voff_t    off, bytesToWrite, bytesToEOF;

    if (uio->uio_resid == 0) {     /* If nothing to do, return early. */
        return (OK);
    }

    /* Check vnode */
    if (vp == NULL) {
        return (EINVAL);
    }

    pFsDev = (RT11FS_DEV *)  vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* Get file inode */
    RT11FS_VTOI (pFileInode, vp);

    /* Get file descriptor */
    pFileDesc = (RT11FS_FILE_DESC *) pFileInode->in_data;

    lbn = (uio->uio_offset >> pVolDesc->vd_blkSize2);
    off = (uio->uio_offset & (pVolDesc->vd_blkSize - 1));
    bytesToEOF  = (pFileDesc->rfd_dirEntry.de_nblocks <<
                   pVolDesc->vd_blkSize2) - uio->uio_offset;

    while (uio->uio_resid > 0) {
        bytesToWrite = pVolDesc->vd_blkSize - off;
        if (bytesToWrite > uio->uio_resid) {
            bytesToWrite = uio->uio_resid;
        }

        if (bytesToEOF < bytesToWrite) {
            bytesToWrite = bytesToEOF;
        }

        if ((bytesToWrite != pVolDesc->vd_blkSize) && (bytesToWrite != 0)) {
            error = bread (vp, lbn, pVolDesc->vd_blkSize, NULL, &pBuf);
            if (error != OK) {
                return (error);
            }
        } else if (bytesToWrite != 0) {
            pBuf = buf_getblk (vp, lbn, pVolDesc->vd_blkSize);
        } else {
            return (EFBIG);
        }

        uiomove ((char *) pBuf->b_data + off, bytesToWrite, uio);

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
    struct vnode *  vp,            /* file vnode pointer */
    u_long          cmd,           /* device specific command */
    void *          data,          /* extra data */
    int             fflag,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_DEV * pFsDev;
    int  rv;

    pFsDev = (RT11FS_DEV *) vp->v_mount->mnt_data;

    rv = xbdIoctl (pFsDev->volDesc.vd_device, cmd, data);

    return (rv);
}

/***************************************************************************
 *
 * rt11VopFcntl -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopFcntl (
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
 * rt11VopFsync - fsync (has nothing to do as rt11FS is synchronous)
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopFsync (
    struct vnode *  vp,            /* file vnode pointer */
    struct ucred *  ucp,           /* user credentials pointer */
    int             flags          /* flags */
    ) {
    RT11FS_DEV *         pFsDev;
    RT11FS_VOLUME_DESC * pVolDesc;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    /* TODO:
     * Write directory segment to disk here
     */

    return (OK);
}

/***************************************************************************
 *
 * rt11VopSeek - seek (nothing to do as it is handled by VFS layer)
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopSeek (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           off1,          /* old offset */
    off_t           off2,          /* new offset */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (OK);    /* nothing to do for seek operation */
}

/***************************************************************************
 *
 * rt11VopRemove -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopRemove (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopLink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopLink (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopRename -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopRename (
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
 * rt11VopMkdir -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopMkdir (
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode **         vpp,   /* created vnode pointer */
    struct componentname *  cnp,   /* path name component pointer */
    struct vattr *          vap    /* vnode attributes pointer */
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
    struct vnode *          dvp,   /* directory vnode pointer */
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
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
 * rt11VopReaddir -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopReaddir (
    struct vnode *   vp,           /* directory vnode pointer */
    struct dirent *  dep,          /* directory entry pointer */
    struct ucred *   ucp,          /* user credentials pointer */
    int *            eof,          /* end of file status */
    int *            cookies       /* cookies */
    ) {
    RT11FS_INODE  *       pDirInode;
    RT11FS_DIR_DESC  *    pDirDesc;
    RT11FS_DIR_ENTRY  *   pDirEntry;
    size_t                len;
    int                   entryNum;
    char  *               pc;

    /* Check directory vnode */
    if (vp == NULL) {
        return (ENOENT);
    }

    /* Get directory inode */
    RT11FS_VTOI (pDirInode, vp);

    /* Get directory descriptor */
    pDirDesc = (RT11FS_DIR_DESC *) pDirInode->in_data;

    /* Get entry number */
        entryNum = (int) dep->d_ino;

        /* Do while dir status is empty */
        do {
        if (entryNum >= pDirDesc->rdd_maxEntries) {
            *eof = 1;
            return (OK);
        }

        /* Get entry */
        pDirEntry = &pDirDesc->rdd_pDirSeg->ds_entries[entryNum];
        if (pDirEntry->de_status == DES_END) {
            *eof = 1;
                return (OK);
        }

        /* Advance */
        entryNum++;
    } while (pDirEntry->de_status == DES_EMPTY);

    /* Copy name to dirent */
    rt11fsNameString (pDirEntry->de_name, dep->d_name);
        
    /* Cancel leading spaces and dots */
    len = strlen (dep->d_name);
    if (len > 0) {
        pc = dep->d_name + len - 1;
        while ( (*pc == ' ') || (*pc == '.') ) {
            *pc = EOS;
            pc++;
        }
    }

    /* Update entry number */
    dep->d_ino = entryNum;
 
    return (OK);
}

/***************************************************************************
 *
 * rt11VopReadlink -
 *
 * RETURNS: ENOSYS
 */

LOCAL int  rt11VopReadlink (
    struct vnode *  vp,            /* file vnode pointer */
    struct uio *    uio,           /* user IO pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (ENOSYS);
}

/***************************************************************************
 *
 * rt11VopAbortop - not yet used by VFS
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopAbortop (
    struct vnode *          vp,    /* file vnode pointer */
    struct componentname *  cnp    /* path name component pointer */
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopActivate - activate the vnode
 *
 * RETURNS: OK on success, errno otherwise
 */

LOCAL int  rt11VopActivate (
    struct vnode *  vp             /* file vnode pointer */
    ) {
    RT11FS_INODE  *       pInode;
    enum vtype            type;
    int                   error;

    /* Get inode */
    error = rt11fsInodeGet (vp);
    if (error != OK) {
        return (error);
    }

    /* Get inode */
    RT11FS_VTOI (pInode, vp);

    /* Get type */
    type = pInode->in_type;

    /* Store type in vnode */
    vp->v_type = type;

    /* Select operators for type */
    switch (type) {
        case VDIR:
            vp->v_ops  = (vnode_ops_t *) &rt11VopsDir;
            break;

        case VREG:
            vp->v_ops  = (vnode_ops_t *) &rt11Vops;
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
    struct vnode *  vp             /* file vnode pointer */
    ) {
    return (rt11fsInodeRelease (vp));
}

/***************************************************************************
 *
 * rt11VopStrategy -
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    ) {

    /* If read operation */
    if (bp->b_flags & B_READ) {
        rt11VopInternalReadStrategy (vp, bp);
    }
    else {
        /* Must be a write operation */
        rt11VopInternalWriteStrategy (vp, bp);
    }
}

/***************************************************************************
 *
 * rt11VopPrint - print a vnode for debugging (nothing to do)
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopPrint (
    struct vnode *  vp             /* file vnode pointer */
    ) {
    return;
}

/***************************************************************************
 *
 * rt11VopPathconf -
 *
 * RETURNS: EINVAL
 */

LOCAL int  rt11VopPathconf (
    struct vnode *  vp,            /* file vnode pointer */
    int             name,          /* type of info to return */
    long *          rv             /* return value */
    ) {
    return (EINVAL);
}

/***************************************************************************
 *
 * rt11VopAdvlock -
 *
 * RETURNS: ENOSYS
 */

LOCAL int rt11VopAdvlock (
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
 * rt11VopTruncate -
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int  rt11VopTruncate (
    struct vnode *  vp,            /* file vnode pointer */
    off_t           len,           /* new length of the file */
    int             flags,         /* flags */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    return (OK);
}

/***************************************************************************
 *
 * rt11VopDirGetAttr -
 *
 * RETURNS: OK
 */

LOCAL int  rt11VopDirGetAttr (
    struct vnode *  vp,            /* file vnode pointer */
    struct vattr *  vap,           /* vnode attributes pointer */
    struct ucred *  ucp            /* user credentials pointer */
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    vap->va_type   = VDIR;
    vap->va_mode   = 0666;
    vap->va_nlink  = 1;
    vap->va_uid    = 0;
    vap->va_gid    = 0;
    vap->va_fsid   = 0;            /* Ignored for now. */
    vap->va_fileid = RT11FS_ROOT_INODE;
    vap->va_size   = pVolDesc->vd_nSegBlks * pVolDesc->vd_blkSize;
    vap->va_blksize = pVolDesc->vd_blkSize;
    vap->va_atime.tv_sec     = 0;  /* dummy value */
    vap->va_mtime.tv_sec     = 0;  /* dummy value */
    vap->va_ctime.tv_sec     = 0;  /* dummy value */
    vap->va_birthtime.tv_sec = 0;  /* dummy value */
    vap->va_flags = 0;
#if 0                /* remaining fields are not yet used */
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
 * rt11VopDirStrategy -
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopDirStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    ) {
}

/***************************************************************************
 *
 * rt11VopSyncerStrategy -
 *
 * RETURNS: N/A
 */

LOCAL void rt11VopSyncerStrategy (
    struct vnode *  vp,            /* file vnode pointer */
    struct buf *    bp             /* buffer pointer */
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    struct bio *          pBio;

    /* Get volume descriptor */
    pFsDev   = (RT11FS_DEV *) vp->v_mount->mnt_data;
    pVolDesc = &pFsDev->volDesc;

    pBio = bp->b_bio;
    pBio->bio_blkno   = (bp->b_lblkno << pVolDesc->vd_secPerBlk2);
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
    physBlk = (pBuf->b_lblkno << pVolDesc->vd_secPerBlk2) +
               pFileDesc->rfd_startBlock;

    /* Read block from disk */
    error = bread (pSyncer, physBlk, pVolDesc->vd_blkSize, NULL, &pTmpBuf);
    if (error) {
        buf_done (pBuf, error);
        return;
    }

    buf_swapdata (pTmpBuf, pBuf);
    pTmpBuf->b_flags |= B_INVAL;
    brelse (pTmpBuf);

    buf_done (pBuf, OK);
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
    physBlk = (pBuf->b_lblkno << pVolDesc->vd_secPerBlk2) +
               pFileDesc->rfd_startBlock;

    /* Get a temporary buffer */
    pTmpBuf = buf_getblk (pSyncer, physBlk, pVolDesc->vd_blkSize);
    buf_swapdata (pBuf, pTmpBuf);
    buf_startwrite (pTmpBuf);
    error = buf_wait (pTmpBuf);
    buf_swapdata (pBuf, pTmpBuf);
    pTmpBuf->b_flags |= B_INVAL;
    brelse (pTmpBuf);

    buf_done (pBuf, error);
}

