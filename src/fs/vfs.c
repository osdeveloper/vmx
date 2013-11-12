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


/* vfs.c - Virtual filesystem layer */

/*
For the present time, file system access will be single threaded.  This is
the simplest (and fastest) to implement.  Making it multi-threaded is a job
for another day.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <os/ioLib.h>
#include <fs/buf.h>
#include <fs/vnode.h>
#include <fs/mount.h>
#include <fs/namei.h>

/* forward declarations */

filedesc_t *  vfsFileDescGet (mount_t *pMount);
void vfsFileDescRelease (mount_t *pMount, filedesc_t *pFileDesc);

/* locals */

LOCAL int vfsDrvNumber;

/***************************************************************************
 *
 * vaccess -
 *
 * This routine is a stub.  It must be fleshed out later.  It is to be called
 * by the file system specific VOP_ACCESS() callout.
 *
 * RETURNS: OK on success, non-zero otherwise
 */

int vaccess (
    unsigned  vtype,         /* vnode type */
    int       permissions,   /* file - permission bits */
    uid_t     uid,           /* file - user ID */ 
    gid_t     gid,           /* file - group ID */
    int       mode,          /* requested mode */
    ucred_t * pCred          /* user credentials */
    ) {
    return (OK);
}

/***************************************************************************
 *
 * vnioReadlink -
 *
 * RETURNS: OK on success, error otherwise
 */

int  vnioReadlink (
    vnode_t *       vp,
    struct iovec *  iov,
    int *           bytesRead
    ) {
    struct vattr     va;
    getattr_args_t   getAttrArgs;
    readlink_args_t  readlinkArgs;
    struct uio       uio;
    int              error;

    getAttrArgs.vp  = vp;
    getAttrArgs.vap = &va;
    getAttrArgs.ucp = NULL;   /* Not yet supported. */

    error = VOP_GETATTR (vp, getAttrArgs);
    if (error != OK) {
        return (error);
    }

    uio.uio_iov    = iov;
    uio.uio_iovcnt = 1;
    uio.uio_offset = 0;
    uio.uio_resid  = va.va_size;
    uio.uio_rw     = UIO_READ;

    readlinkArgs.vp  = vp;
    readlinkArgs.uio = &uio;
    readlinkArgs.ucp = NULL;   /* Not yet supported. */

    error = VOP_READLINK (vp, readlinkArgs);
    if (error != OK) {
        return (error);
    }

    *bytesRead = va.va_size - uio.uio_resid;
}

/***************************************************************************
 *
 * lookup -
 *
 * RETURNS: OK on success, error otherwise
 */

int  lookup (
    nameidata_t *  nidp
    ) {
    readlink_args_t    readlinkArgs;
    lookup_args_t      args;
    componentname_t *  cnp;
    int     len;
    int     increment;
    int     error;
    char *  cp;
    char    symlink[PATH_MAX + 1];

    cnp = &nidp->ni_cn;
    increment = 1;

    if (cnp->cn_flags & CN_ISDOT) {
        if ((cnp->cn_flags & CN_ISLAST) && (cnp->cn_op == CN_CREATE)) {
            /*
             * Can not create current directory ... entry already exists.
             */
            return (EEXIST);
        }
        /*
         * The FS VOP_LOOKUP() callout is expected to increment the "."
         * vnode's use count.  The caller (namei) will decrement ni_dvp's
         * use count thereby resulting in an convoluted no-op.
         */
    } else if (cnp->cn_flags & CN_ISDOTDOT) {
        if ((cnp->cn_flags & CN_ISLAST) && (cnp->cn_op != CN_LOOKUP)) {
            /*
             * Deleting, creating, or renaming the parent directory is
             * not allowed.
             */
            return (EINVAL);
        }

        if (nidp->ni_dvp->v_flags & V_ROOT) {
            /*
             * Leaving this mount/FS via DOTDOT.
             * 1st, update the path pointers.
             */

            cp = cnp->cn_nameptr + cnp->cn_namelen;
            len = strlen (cp);

            memmove (nidp->ni_path, cp, strlen (cp));
            nidp->ni_pathlen = len;
            nidp->ni_cn.cn_nameptr = nidp->ni_path;
          
            return (FOLLOW_DOTDOT);
        }
    }

    args.dvp = nidp->ni_dvp;
    args.vpp = &nidp->ni_vp;
    args.cnp = cnp;
    error = VOP_LOOKUP (nidp->ni_dvp, args);

#ifdef optional
    if (cnp->cn_flags & CN_ISDOTDOT) {
        /*
         * Add code to remove ".." and current directory from the path.
         * 1.  This helps keep the path "clean" and could be used for a
         *     resolvepath() type call.
         * 2.  In the event a symlink is encountered, it helps to keep the
         *     total path shorter and cleaner.
         */
    }
#endif

    if (error != OK) {
        return (error);
    }

    /* FS callout VOP_LOOKUP() succeeded. */

#ifdef notyet
    if (nidp->ni_vp->v_type == VLNK) {
        /* Hmmm .... perhaps malloc() <symlink> to reduce stack footprint */
        memset (symlink, 0, PATH_MAX + 1);
        iov.iov_base = symlink;
        iov.iov_len = PATH_MAX;
        error = vnioReadlink (nidp->ni_vp, &iov, &bytesRead);
        if (error != OK) {
            return (error);
        }

        if (symlink[0] == '/') {
            strcpy (nidp->ni_path, symlink);    /* Absolute symlink */
            nidp->ni_pathlen = bytesRead;
            return (FOLLOW_LINK);
        }

        /* Relative symlink.  1. Try appending remaining path to <symlink>. */

        if (not enough space in <symlink>) {
            return (ENAMETOOLONG);
        }

        /* 2. Overwrite current path component onwards with <symlink>. */

        if (not enough space in path) {
            return (ENAMETOOLONG);
        }
    }
#endif

    return (OK);
}

/***************************************************************************
 *
 * namei -
 *
 * The root directory is locked before calling namei().  Heck, the entire
 * mount has been locked.
 *
 * RETURNS: OK on success, error otherwise
 */

int  namei (
    nameidata_t *  nidp,
    int            devnameLen    /* length of the device name */
    ) {
    componentname_t *  cnp;
    char *             cp;
    int                error;

    cp  = &nidp->ni_path[devnameLen];
    cnp = &nidp->ni_cn;

    while (*cp == '/') {    /* Determine start of the path component. */
        cp++;
    }

    if (*cp == '\0') {
        /* Trivial case.  Nothing to do.  Should not occur.*/
        /*
         * NOTE:
         * Added zero filename for rawfs device open
         * Just load root directory vnode? Which should be the root vnode
         */
        nidp->ni_vp = nidp->ni_dvp;
        return (OK);
    }

    for (;;) {
        cnp->cn_nameptr = cp;
        cnp->cn_flags &= ~(CN_SLASH | CN_ISLAST | CN_ISDOT | CN_ISDOTDOT);

        while ((*cp != '\0') && (*cp != '/')) {   /* Determine the length */
            cp++;                                 /* of the path component. */
        }
        cnp->cn_namelen = (int) (cp - cnp->cn_nameptr);

        while (*cp == '/') {                      /* skip all trailing '/' */
            cp++;
        }

        /* Did a '/' follow this path component? */
        if ((int) (cp - cnp->cn_nameptr) != cnp->cn_namelen) {
            cnp->cn_flags |= CN_SLASH;
        }

        /* Is this the last path compnonent? */
        if (*cp == '\0') {
            cnp->cn_flags |= CN_ISLAST;
        }

        /* Check for DOT and DOTDOT path components */
        if (*cnp->cn_nameptr == '.') {
            if (cnp->cn_namelen == 1) {
                cnp->cn_flags |= CN_ISDOT;
            } else if ((cnp->cn_nameptr[1] == '.') && (cnp->cn_namelen == 2)) {
                cnp->cn_flags |= CN_ISDOTDOT;
            }
        }

        error = lookup (nidp);

        if (error != OK) {
            /*
             * nidp->ni_dvp was locked; nidp->ni_vp was not locked.
             * Let the vnioXXX() caller decide what do.
             */
            return (error);
        }

        /* lookup() succeeded. */
        if (cnp->cn_flags & CN_ISLAST) {
            break;
        }

        vnodeUnlock (nidp->ni_dvp);    /* Unlock directory vnode */
        nidp->ni_dvp = nidp->ni_vp;    /* Found vnode becomes new dvp */
    }

    /* nidp->ni_dvp is locked; nidp->ni_vp is locked */
    return (OK);
}

/***************************************************************************
 *
 * vattrInit - initialize all vattr fields to VNOVAL
 *
 * RETURNS: N/A
 */

void vattrInit (
    vattr_t *  vap
    ) {
    vap->va_type      = VNOVAL;
    vap->va_mode      = VNOVAL;
    vap->va_nlink     = VNOVAL;
    vap->va_uid       = VNOVAL;
    vap->va_gid       = VNOVAL;
    vap->va_fileid    = VNOVAL;
    vap->va_size      = VNOVAL;
    vap->va_blksize   = VNOVAL;
    vap->va_atime.tv_sec      = VNOVAL;
    vap->va_atime.tv_nsec     = VNOVAL;
    vap->va_mtime.tv_sec      = VNOVAL;
    vap->va_mtime.tv_nsec     = VNOVAL;
    vap->va_ctime.tv_sec      = VNOVAL;
    vap->va_ctime.tv_nsec     = VNOVAL;
    vap->va_birthtime.tv_sec  = VNOVAL;
    vap->va_birthtime.tv_nsec = VNOVAL;
    vap->va_flags     = VNOVAL;

    /* The following fields are not yet used */
    vap->va_fsid      = VNOVAL;
    vap->va_gen       = VNOVAL;
    vap->va_rdev      = VNOVAL;
    vap->va_bytes     = VNOVAL;
    vap->va_filerev   = VNOVAL;
}

/***************************************************************************
 *
 * vnioConnect - common vnode open/create/mkdir/symlink code
 *
 * RETURNS:
 */

int  vnioConnect (
    DEV_HEADER *  pDevHeader,
    char *        filename,
    char *        tgt,
    int           oflags,
    int           mode,
    int           nidOp,
    int           nidFlags,
    int (*func)(nameidata_t *, filedesc_t *, char *, int, int, int)
    ) {
    nameidata_t   nid;
    filedesc_t *  pFileDesc;
    mount_t *     mp;
    vnode_t *     dvp;
    int           filenameLen;
    int           devnameLen;
    int           error;

    devnameLen  = strlen (pDevHeader->name);
    filenameLen = strlen (filename);

    /*
     * <devnameLen> >= <filenameLen> else we would not be here.
     * Ensure that a '/' follows the device name.
     *
     * NOTE: I/O system automatically removes device name from filename.
     */


    /*
     * NOTE: Don't require preceding slash
     */
#ifdef notused
    if (filename[0] != '/') {
        errnoSet (EINVAL);
        return (ERROR);
    }
#endif

    /*
     * The following typecast works as DEV_HEADER is the 1st
     * element in the {mount_t} structure.
     */

    mp = (mount_t *) pDevHeader;
    mountLock (mp);               /* Lock the mount (single thread it) */

    pFileDesc = vfsFileDescGet (mp);
    if (pFileDesc == NULL) {
        mountUnlock (mp);
        errnoSet (S_vfsLib_FILEDESC_EXHAUSTED);
        return (ERROR);
    }

    /* Inititalize fields to zero */
    pFileDesc->fd_off = 0;

    VFS_ROOT (mp, &dvp);      /* This will lock the root vnode */

    ND_INIT (&nid, filename, filenameLen, dvp, nidOp, nidFlags);
    error = namei (&nid, 0);

    /*
     * If told to follow an absolute symlink, or a ".." to a potentially
     * new device, unlock the parent vnode and return to ioConnect() for
     * further processing.
     */

    if ((error == FOLLOW_LINK) || (error == FOLLOW_DOTDOT)) {
        vnodeUnlock (nid.ni_dvp);
        /* <nid.ni_vp> has been unlocked by namei()/lookup() */
        vfsFileDescRelease (mp, pFileDesc);
        mountUnlock (mp);
        return (error);
    }

    return (func (&nid, pFileDesc, tgt, mode, oflags, error));
}

/***************************************************************************
 *
 * vnioMkdir - vnode create a directory
 *
 * RETURNS:
 */

int vnioMkdir (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,    /* not used */
    int            mode,   /* not used */
    int            oflags, /* not used */
    int            error
    ) {
    mkdir_args_t  args;
    vattr_t       vattr;
    mount_t *     mp;

    mp = nidp->ni_dvp->v_mount;

    if (error == OK) {
        error = EEXIST;            /* Entry already exists. */
        vnodeUnlock (nidp->ni_vp);
    }

    if ((error != OK) && (error != ENOENT)) {
        goto errorReturn;
    }

    vattrInit (&vattr);
    vattr.va_type = VDIR;
    vattr.va_mode = mode;

    args.dvp = nidp->ni_dvp;
    args.vpp = &fdp->fd_vnode;
    args.cnp = &nidp->ni_cn;
    args.vap = &vattr;

    error = VOP_MKDIR (nidp->ni_dvp, args);

    if (error != OK) {
        goto errorReturn;
    }

    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */

    mountUnlock (mp);

    return ((int) fdp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);
    mountUnlock (mp);
    errnoSet (error);
    return (ERROR);
}

/***************************************************************************
 *
 * vnioSymlink - vnode create a symlink
 *
 * RETURNS:
 */

int vnioSymlink (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,
    int            mode,   /* not used */
    int            oflags, /* not used */
    int            error
    ) {
    symlink_args_t  args;
    vattr_t         vattr;
    mount_t *       mp;

    mp = nidp->ni_dvp->v_mount;

    if (error == OK) {
        vnodeUnlock (nidp->ni_vp);
        error = EEXIST;
    }

    if ((error != OK) && (error != ENOENT)) {
        goto errorReturn;
    }

    vattrInit (&vattr);
    vattr.va_type = VLNK;
    vattr.va_mode = mode;

    args.dvp = nidp->ni_dvp;
    args.vpp = &fdp->fd_vnode;
    args.cnp = &nidp->ni_cn;
    args.vap = &vattr;
    args.tgt = tgt;

    error = VOP_SYMLINK (nidp->ni_dvp, args);

    if (error != OK) {
        goto errorReturn;
    }

    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */

    mountUnlock (mp);

    return ((int) fdp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */
    mountUnlock (mp);
    errnoSet (error);
    return (ERROR);
}

/***************************************************************************
 *
 * vnioCreate - vnode create a regular file
 *
 * RETURNS:
 */

int vnioCreate (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,    /* not used */
    int            mode,   /* not used */
    int            oflags, /* not used */
    int            error
    ) {
    create_args_t    createArgs;
    open_args_t      openArgs;
    truncate_args_t  truncateArgs;
    vattr_t          vattr;
    BOOL             doesNotExist;
    mount_t *        mp;

    mp = nidp->ni_dvp->v_mount;

    doesNotExist = (error != OK);
    if ((error == OK) &&
        ((nidp->ni_vp->v_type != VREG) || (oflags & O_EXCL))) {
        vnodeUnlock (nidp->ni_vp);
        error == EEXIST;
    }

    if ((error != OK) && (error != ENOENT)) {
        goto errorReturn;
    }

    vattrInit (&vattr);
    vattr.va_type = VREG;
    vattr.va_mode = mode & 0777;

    if (doesNotExist) {
        createArgs.dvp = nidp->ni_dvp;
        createArgs.vpp = &fdp->fd_vnode;
        createArgs.cnp = &nidp->ni_cn;
        createArgs.vap = &vattr;

        error = VOP_CREATE (nidp->ni_dvp, createArgs);
        if (error != OK) {
            goto errorReturn;
        }

        /* Store created file vnode */
        nidp->ni_vp = *createArgs.vpp;
    } else {
        fdp->fd_vnode = nidp->ni_vp;
        openArgs.vp   = nidp->ni_vp;
        openArgs.mode = mode;
        openArgs.ucp  = NULL;

        error = VOP_OPEN (nidp->ni_vp, openArgs);
    }

    if ((error == OK) && (oflags & O_TRUNC)) {
        truncateArgs.vp    = nidp->ni_vp;
        truncateArgs.len   = 0;
        truncateArgs.flags = 0;     /* XXX: change later */
        truncateArgs.ucp   = NULL;
            
        error = VOP_TRUNCATE (nidp->ni_vp, truncateArgs);
    }

    if (error != OK) {
        vnodeUnlock (nidp->ni_vp);
        goto errorReturn;
    }

    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */

    mountUnlock (mp);

    /* Setup file descriptor fields */
    fdp->fd_mode = oflags & O_ACCMODE;

    return ((int) fdp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */
    mountUnlock (mp);
    errnoSet (error);
    return (ERROR);
}

/***************************************************************************
 *
 * vnioOpen -
 *
 */

int vnioOpen (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,    /* not used */
    int            mode,
    int            oflags, /* not used */
    int            error
    ) {
    mount_t *      mp;
    open_args_t    openArgs;

    mp = nidp->ni_dvp->v_mount;

    if (error != OK) {
        goto errorReturn;
    }

    /* <error> is OK. */

    if ((mode & S_IFMT) != 0) {
        if ((S_ISREG (mode) && (nidp->ni_vp->v_type != VREG)) ||
            (S_ISDIR (mode) && (nidp->ni_vp->v_type != VDIR)) ||
            (S_ISLNK (mode) && (nidp->ni_vp->v_type != VLNK))) {
            vnodeUnlock (nidp->ni_vp);     /* Unlock vnode */
            error = EPERM;
            goto errorReturn;
        }
    } else {
        if ((nidp->ni_vp->v_type == VDIR) &&
            ((oflags & O_ACCMODE) != O_RDONLY)) {
            vnodeUnlock (nidp->ni_vp);     /* Unlock vnode */
            error = EISDIR;
            goto errorReturn;
        }
        if ((nidp->ni_vp->v_type != VREG) && (nidp->ni_vp->v_type != VDIR) &&
            (nidp->ni_vp->v_type != VLNK)) {
            vnodeUnlock (nidp->ni_vp);     /* Unlock vnode */
            error = EPERM;
            goto errorReturn;
        }
    }

    fdp->fd_vnode = nidp->ni_vp;
    openArgs.vp   = nidp->ni_vp;
    openArgs.mode = mode;
    openArgs.ucp  = NULL;

    error = VOP_OPEN (nidp->ni_vp, openArgs);

    if (error != OK) {
        goto errorReturn;
    }

    vnodeUnlock (nidp->ni_dvp);

    mountUnlock (mp);

    /* Setup file descriptor fields */
    fdp->fd_mode = oflags & O_ACCMODE;

    return ((int) fdp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */
    mountUnlock (mp);
    errnoSet (error);
    return (ERROR);
}

/***************************************************************************
 *
 * vnioRmdir - remove a directory
 *
 * RETURNS:  OK or ERROR
 */

int vnioRmdir (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,    /* not used */
    int            mode,   /* not used */
    int            oflags, /* not used */
    int            error
    ) {
    mount_t *     mp;
    rmdir_args_t  args;

    mp = nidp->ni_dvp->v_mount;

    if (error != OK) {
        goto errorReturn;
    }

    if (nidp->ni_vp->v_type != VDIR) {
        vnodeUnlock (nidp->ni_vp);
        error = ENOTDIR;
        goto errorReturn;
    }

    fdp->fd_vnode = nidp->ni_vp;
    args.dvp      = nidp->ni_dvp;
    args.vp       = nidp->ni_vp;
    args.cnp      = &nidp->ni_cn;

    error = VOP_RMDIR (nidp->ni_vp, args);

    vnodeUnlock (nidp->ni_vp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);    /* Unlock parent directory */
    mountUnlock (mp);
    return ((error != OK) ? (ERROR, errnoSet (error)) : OK);
}

/***************************************************************************
 *
 * vnioRemove - remove a file or a symlink or a directory
 *
 * RETURNS:
 */

int vnioRemove (
    nameidata_t *  nidp,
    filedesc_t *   fdp,
    char *         tgt,    /* not used */
    int            mode,   /* not used */
    int            oflags, /* not used */
    int            error
    ) {
    mount_t *      mp;
    rmdir_args_t   rmdirArgs;
    unlink_args_t  unlinkArgs;

    mp = nidp->ni_dvp->v_mount;

    if (error != OK) {
        goto errorReturn;
    }

    fdp->fd_vnode = nidp->ni_vp;

    switch (nidp->ni_vp->v_type) {
    case VDIR:
        rmdirArgs.dvp = nidp->ni_dvp;
        rmdirArgs.vp  = nidp->ni_vp;
        rmdirArgs.cnp = &nidp->ni_cn;
        error = VOP_RMDIR (nidp->ni_vp, rmdirArgs);
        break;
    case VREG:
    case VLNK:
        unlinkArgs.dvp = nidp->ni_dvp;
        unlinkArgs.vp  = nidp->ni_vp;
        unlinkArgs.cnp = &nidp->ni_cn;
        error = VOP_UNLINK (nidp->ni_vp, unlinkArgs);
        if (error == OK) {
            nidp->ni_vp->v_flags |= V_DELETED;
            /*
             * Truncation/freeing of data & blocks will occur when the
             * vnode gets marked as inactive (aka as when there are no
             * more references to the vnode).
             */
        }
        break;
    default:
        error = EPERM;
        break;
    }

    vnodeUnlock (nidp->ni_vp);

errorReturn:
    vfsFileDescRelease (mp, fdp);
    vnodeUnlock (nidp->ni_dvp);
    mountUnlock (mp);

    return ((error != OK) ? (ERROR, errnoSet (error)) : OK);
}

/***************************************************************************
 *
 * vncioOpen - vnode core IO open
 *
 * This routine is called by iosOpen().
 * It plugs into the IO driver framework.
 *
 * RETURNS: fd on success, -1 on error
 */

int  vncioOpen (
    DEV_HEADER *  pDevHeader,  /* ptr to device header */
    char *        filename,    /* ptr to filename to open */
    int           oflags,      /* O_xxx flags */
    int           mode         /* permission bits and the like */
    ) {
    BOOL         follow;
    vnode_t *    dvp;
    cn_op_t      op;
    nameidata_t  nid;
    int          devnameLen;
    int          filenameLen;
    int          error;

    /* Neither O_TRUNC nor O_APPEND can not be used with O_RDONLY */
    if (((oflags & O_ACCMODE) == O_RDONLY) &&
        (oflags & (O_TRUNC | O_APPEND))) {
        errnoSet (EINVAL);
        return (ERROR);
    }

    /*
     * The VFS may open regular files, directories (read only), and
     * symlinks.  If the type specified by mode is not supported, then
     * return an error.
     */

    if (oflags & O_CREAT) {
        /* Create a regular file. */
        if (!S_ISREG (mode)) {
            errnoSet (EPERM);       /* Oops. This is not a regular file. */
            return (ERROR);
        }
        return (vnioConnect (pDevHeader, filename, NULL, oflags, mode,
                             CN_CREATE, CN_NOFOLLOW, vnioCreate));
    }

    /* Open a regular file, directory or symlink. */
    if (((mode & S_IFMT) != 0) &&
        !S_ISREG (mode) && !S_ISDIR (mode) && !S_ISLNK (mode)) {
        errnoSet (EPERM);        /* Unknown file type. */
        return (ERROR);
    }

    error = vnioConnect (pDevHeader, filename, NULL, oflags, mode,
                         CN_LOOKUP, CN_NOFOLLOW, vnioOpen);

    return (error);
}

/***************************************************************************
 *
 * vncioClose - vnode core IO close
 *
 * This routine is called by iosClose().
 * It plugs into the IO driver framework.
 *
 * RETURNS: OK or ERROR
 */

int  vncioClose (
    filedesc_t *  pFileDesc
    ) {
    mount_t *    mp;
    vnode_t *    vp;
    close_args_t args;
    int          error;

    mp = pFileDesc->fd_vnode->v_mount;

    mountLock (mp);

    vp = pFileDesc->fd_vnode;
    pFileDesc->fd_inuse = FALSE;
    pFileDesc->fd_vnode = NULL;

    /*
     * Currently, only synchronous operations are supported.  When this
     * eventually changes, then any dirty buffers associated with vnode
     * <vp> will have to be written to the backing media.
     */

    args.vp = vp;
    args.flags = 0;
    args.ucp = NULL;
    error = VOP_CLOSE (args.vp, args);

    vnodeUnlock (vp);     /* unlock (and mark as inactive if necessary) */

    mountUnlock (mp);

    return (error);
}

/***************************************************************************
 *
 * vncioCreate - vnode core IO create
 *
 * This routine is called by iosCreate().
 * It plugs into the IO driver framework.
 *
 * RETURNS: OK or ERROR
 */

int  vncioCreate (
    DEV_HEADER *  pDevHeader,  /* ptr to device header */
    char *        filename,    /* ptr to filename to create */
    int           flags,       /* permission bits and the like */
    char *        symlink      /* ptr to symlink target */
    ) {
    int  mode;
    int  error;

    /* Set mode to create file */
    mode = flags | O_TRUNC | O_CREAT;

    if (S_ISREG (mode)) {
        error = vnioConnect (pDevHeader, filename, NULL, mode,
                             flags & S_IFMT, CN_CREATE, CN_NOFOLLOW,
                             vnioCreate);
    } else if (S_ISDIR (mode)) {
        error = vnioConnect (pDevHeader, filename, NULL, mode,
                             flags & S_IFMT, CN_CREATE, CN_NOFOLLOW,
                             vnioMkdir);
    } else if (S_ISLNK (mode)) {
        error = vnioConnect (pDevHeader, filename, symlink, mode,
                             flags & S_IFMT, CN_CREATE, CN_NOFOLLOW,
                             vnioSymlink);
    } else {
        errnoSet (EINVAL);        /* Don't know what to create. */
        return (ERROR);
    }
    return (error);
}

/***************************************************************************
 *
 * vncioDelete - vnode core IO delete
 *
 * This routine is called by iosDelete().
 * It plugs into the IO driver framework.
 *
 * RETURNS: OK or ERROR
 */

int  vncioDelete (
    DEV_HEADER *  pDevHeader,
    char *        filename,
    int           mode
    ) {
    int  error;

    if (S_ISDIR (mode)) {
        error = vnioConnect (pDevHeader, filename, NULL, O_RDWR, mode,
                             CN_DELETE, CN_NOFOLLOW, vnioRmdir);
    } else if (mode == 0) {
        error = vnioConnect (pDevHeader, filename, NULL, O_RDWR, mode,
                             CN_DELETE, CN_NOFOLLOW, vnioRemove);
    } else {
        error = EINVAL;    /* Don't know what to do. */
    }

    if (error != OK) {
        errnoSet (error);
    }

    return (error == OK ? OK : ERROR);
}

/***************************************************************************
 *
 * vncioRead - vnode core IO read
 *
 * This routine is called by iosRead().
 * It plugs into the IO driver framework.
 *
 * RETURNS: # of bytes read, or ERROR
 */

int vncioRead (
    filedesc_t * pFileDesc,     /* VFS filedesc_t is {FD_ENTRY} [value] */
    void *       buffer,        /* buffer into which to read data */
    int          maxBytes       /* maximum # of bytes to read */
    ) {
    read_args_t   args;
    struct uio    uio;
    struct iovec  iovec;
    mount_t *     mp;
    int           error;

    mp = pFileDesc->fd_vnode->v_mount;

    if ((pFileDesc->fd_mode & O_ACCMODE) == O_WRONLY) {
        errnoSet (EBADF);
        return (ERROR);
    }

    iovec.iov_base = buffer;
    iovec.iov_len = maxBytes;

    uio.uio_iov    = &iovec;
    uio.uio_iovcnt = 1;
    uio.uio_offset = pFileDesc->fd_off;
    uio.uio_resid  = maxBytes;
    uio.uio_rw     = UIO_READ;

    args.vp = pFileDesc->fd_vnode;
    args.uio = &uio;
    args.ioflag = 0;                /* Not used (yet) */
    args.ucp = NULL;                /* Not used (yet) */

    mountLock (mp);

    error = VOP_READ (args.vp, args);

    mountUnlock (mp);

    if (error != OK) {
        errnoSet (error);
        return (ERROR);
    }

    pFileDesc->fd_off += (maxBytes - uio.uio_resid);

    return (maxBytes - uio.uio_resid);
}

/***************************************************************************
 *
 * vncioWrite - vnode core IO write
 *
 * This routine is called by iosWrite().
 * It plugs into the IO driver framework.
 *
 * RETURNS: # of bytes written, or ERROR
 */

int vncioWrite (
    filedesc_t * pFileDesc,    /* VFS filedesc_t is {FD_ENTRY} [value] */
    void *       buffer,       /* buffer containing data to be written */
    int          maxBytes      /* maximum # of bytes to write */
    ) {
    write_args_t  args;
    struct uio    uio;
    struct iovec  iovec;
    mount_t *  mp;
    int        error;

    mp = pFileDesc->fd_vnode->v_mount;

    if ((pFileDesc->fd_mode & O_ACCMODE) == O_RDONLY) {
        errnoSet (EBADF);
        return (ERROR);
    }

    iovec.iov_base = buffer;
    iovec.iov_len = maxBytes;

    uio.uio_iov    = &iovec;
    uio.uio_iovcnt = 1;
    uio.uio_offset = pFileDesc->fd_off;
    uio.uio_resid  = maxBytes;
    uio.uio_rw     = UIO_WRITE;

    args.vp = pFileDesc->fd_vnode;
    args.uio = &uio;
    args.ioflag = 0;                /* Not used (yet) */
    args.ucp = NULL;                /* Not used (yet) */

    mountLock (mp);

    error = VOP_WRITE (args.vp, args);

    mountUnlock (mp);

    if (error != OK) {
        errnoSet (error);
        return (ERROR);
    }

    pFileDesc->fd_off += (maxBytes - uio.uio_resid);

    return (maxBytes - uio.uio_resid);
}

/***************************************************************************
 *
 * vncioIoctl - vnode core IO ioctl
 *
 * This routine is called by iosIoctl().
 * It plugs into the IO driver framework.
 *
 * RETURNS: value specific to <command>
 */

int vncioIoctl (
    filedesc_t * pFileDesc,    /* VFS filedesc_t is {FD_ENTRY} [value] */
    int          command,      /* ioctl command */
    void *       arg           /* additional parameter */
    ) {
    int              error;
    int              rv;
    int              eof;
    int              bytesRead;
    getattr_args_t   getAttrArgs;
    fsync_args_t     fsyncArgs;
    readlink_args_t  readlinkArgs;
    pathconf_args_t  pathconfArgs;
    truncate_args_t  truncateArgs;
    readdir_args_t   readdirArgs;
    ioctl_args_t     ioctlArgs;
    vattr_t          vattr;
    off_t            offset;
    struct stat *    pStat;
    struct uio       uio;
    mount_t *        mp;

    mp = pFileDesc->fd_vnode->v_mount;

    switch (command) {
        case FIONREAD:
            if (arg == NULL) {
                error = EINVAL;
                break;
            }

            /*
             * XXX TODO: eventually add VFS_TRANS_START().
             * Get the current file size from the file attributes.
             */

            getAttrArgs.vp  = pFileDesc->fd_vnode;
            getAttrArgs.vap = &vattr;
            getAttrArgs.ucp = NULL;   /* Not yet supported. */
            mountLock (mp);
            error = VOP_GETATTR (pFileDesc->fd_vnode, getAttrArgs);
            mountUnlock (mp);
            if (error != OK) {
                break;
            }

            offset = (vattr.va_size > pFileDesc->fd_off) ?
                     (vattr.va_size - pFileDesc->fd_off) : 0;
            *((off_t *) arg) = offset;
            return (OK);

        case FIOFLUSH:
            /*
             * XXX TODO: eventually add VFS_TRANS_START().
             * Flush any unwritten data to the backing media.
             */

             fsyncArgs.vp    = pFileDesc->fd_vnode;
             fsyncArgs.ucp   = NULL;  /* Not yet supported. */
             fsyncArgs.flags = 0;
             mountLock (mp);
             error = VOP_FSYNC (pFileDesc->fd_vnode, fsyncArgs);
             mountUnlock (mp);
             break;

        case FIOSEEK:
            /*
             * VFS_TRANS_START() not needed here as only the VFS
             * file descriptor is used.
             */

            /* Get off_t offset */
            memset (&offset, 0, sizeof(off_t));
            memcpy (&offset, &arg, sizeof(arg));

            if (offset < 0) {
                error = EINVAL;
                break;
            }
            pFileDesc->fd_off = offset;
            break;

        case FIOWHERE:
            /*
             * VFS_TRANS_START() not needed here as only the VFS
             * file descriptor is used.
             */

            if (arg == NULL) {
                error = EINVAL;
                break;
            }
            *((off_t *) arg) = pFileDesc->fd_off;
            return (OK);

        case FIOSTATGET:
            if (arg == NULL) {
                error = EINVAL;
                break;
            }

            getAttrArgs.vp  = pFileDesc->fd_vnode;
            getAttrArgs.vap = &vattr;
            getAttrArgs.ucp = NULL;   /* Not yet supported. */

            mountLock (mp);
            error = VOP_GETATTR (pFileDesc->fd_vnode, getAttrArgs);
            mountUnlock (mp);
            if (error != OK) {
                break;
            }

            pStat = (struct stat *) arg;
            pStat->st_dev     = (dev_t) &mp->mnt_devhdr;
            pStat->st_ino     = vattr.va_fileid;
            pStat->st_mode    = vattr.va_mode;
            pStat->st_nlink   = vattr.va_nlink;
            pStat->st_uid     = vattr.va_uid;
            pStat->st_gid     = vattr.va_gid;
            pStat->st_rdev    = vattr.va_rdev;
            pStat->st_size    = vattr.va_size;
            pStat->st_atime   = vattr.va_atime.tv_sec;
            pStat->st_mtime   = vattr.va_mtime.tv_sec;
            pStat->st_ctime   = vattr.va_ctime.tv_sec;
            pStat->st_blksize = vattr.va_blksize;
            pStat->st_blocks  = vattr.va_size / vattr.va_blksize;
            return (OK);

        case FIOREADLINK:
            if (arg == NULL) {
                error = EINVAL;
                break;
            }

            /*
             * XXX TODO: eventually add VFS_TRANS_START().
             * Before we can obtain the target from the symlink, we
             * must ascertain the length of the symlink.
             */

            mountLock (mp);
            error = vnioReadlink (pFileDesc->fd_vnode, (struct iovec *) arg,
                                  &bytesRead);
            if (error != OK) {
                errnoSet (error);
                bytesRead = ERROR;
            }
            mountUnlock (mp);

            return (bytesRead);

        case FIOPATHCONF:
            /*
             * Although pathconf() makes a VOP_xxx() call, it should not
             * be necessary to use VFS_TRANS_START().
             */

             pathconfArgs.vp   = pFileDesc->fd_vnode;
             pathconfArgs.name = (int) arg;
             pathconfArgs.rv   = &rv;
 
             error = VOP_PATHCONF (pFileDesc->fd_vnode, pathconfArgs);
             break;

        case FIOTRUNCATE:
            /*
             * XXX TODO: eventually add VFS_TRANS_START().
             * Grow or shrink the file.
             * XXX TODO: There ought to be a check that the file is writable.
             */

            /* Get off_t offset */
            memset (&offset, 0, sizeof(off_t));
            memcpy (&offset, &arg, sizeof(arg));

            if (offset < 0) {
                error = EINVAL;
                break;
            }

            truncateArgs.vp    = pFileDesc->fd_vnode;
            truncateArgs.len   = offset;
            truncateArgs.flags = 0;   /* Change when writable check added. */
            truncateArgs.ucp   = NULL;

            mountLock (mp);
            error = VOP_TRUNCATE (pFileDesc->fd_vnode, truncateArgs);
            mountUnlock (mp);
            break;

        case FIOREADDIR:
            /*
             * As another task may have changed the directory contents it is
             * possible that the current directory position no longer points
             * to the start of a directory entry.  This is not something can
             * be tested at this level--it must be off-loaded to the specific
             * FS, as it is a condition that does not affect all file systems.
             * Should it be invalid, the VOP_READDIR() callout will return
             * ENOENT.
             */

            eof = 0;
            readdirArgs.dvp = pFileDesc->fd_vnode;
            readdirArgs.dep = (struct dirent *) arg;
            readdirArgs.ucp = NULL;
            readdirArgs.eof = &eof;
            readdirArgs.nCookies = &pFileDesc->fd_num_cookies;
            readdirArgs.cookies = pFileDesc->fd_cookie_data;

            error = VOP_READDIR (pFileDesc->fd_vnode, readdirArgs);

            if (error != OK) {
                errnoSet (error);
                return (ERROR);
            }

            return ((eof) ? ERROR : OK);

        default:
            mountLock (mp);
            VFS_TRANS_START (mp, FALSE);   /* transaction does not write */
            ioctlArgs.vp    = pFileDesc->fd_vnode;
            ioctlArgs.cmd   = command;
            ioctlArgs.data  = arg;
            ioctlArgs.fflag = 0;     /* Eventually will need file flags */
            ioctlArgs.ucp   = NULL;
            error = VOP_IOCTL (pFileDesc->fd_vnode, ioctlArgs);
            VFS_TRANS_END (mp, error);
            mountUnlock (mp);
            break;
    }

    if (error != OK) {
        errnoSet (error);
        return (ERROR);
    }

    return (OK);
}

/***************************************************************************
 *
 * vfsFileDescGet - get a mount specific file descriptor
 *
 * This routine only gets the file descriptor.  It does not initialize any
 * of its fields other than to mark it as "in use".
 *
 * RETURNS: pointer to mount specific file descriptor, NULL on error
 */

filedesc_t *  vfsFileDescGet (
    mount_t *  pMount
    ) {
    int  i;
    filedesc_t *  pFileDesc;

    pFileDesc = pMount->mnt_fdlist;
    for (i = 0; i < pMount->mnt_maxfiles; i++, pFileDesc++) {
        if (!pFileDesc->fd_inuse) {
            pFileDesc->fd_inuse = TRUE;
            return (pFileDesc);
        }
    }

    return (NULL);
}

/***************************************************************************
 *
 * vfsFileDescRelease - release a mount specific file descriptor
 *
 * This routine releases the file descriptor.  It does not clear any of its
 * fields other than to mark it as "not in use".
 *
 * RETURNS: N/A
 */

void vfsFileDescRelease (
    mount_t *     pMount,
    filedesc_t *  pFileDesc
    ) {
    /*
     * We must still lock the mount structures even though the assignment
     * ought to be atomic.  This is done so that vfsFileDescGet() does not
     * "miss" a newly released file descriptor.
     */

    mountLock (pMount);
    pFileDesc->fd_inuse = FALSE;
    mountUnlock (pMount);
}

/***************************************************************************
 * 
 * vfsInit - initialize the VFS
 * 
 * RETURNS: OK or ERROR
 */

STATUS vfsInit (void) {
    vfsDrvNumber = iosDrvInstall (vncioCreate, vncioDelete,
                                  vncioOpen, vncioClose,
                                  vncioRead, vncioWrite, vncioIoctl);

    return ((vfsDrvNumber != ERROR) ? OK : ERROR);
}

/***************************************************************************
 * 
 * vfsMountInsert - Add a vfs device for a mounted filesystem
 * 
 * RETURNS: OK or ERROR
 */

STATUS vfsMountInsert (mount_t *pMount, char *pDevName) {

    if (vfsDrvNumber != ERROR) {
        return iosDevAdd (&pMount->mnt_devhdr, pDevName, vfsDrvNumber);
    }

    return ERROR;
}

/***************************************************************************
 * 
 * vfsMountEject - Remove a vfs device for a mounted filesystem
 * 
 * RETURNS: OK or ERROR
 */

STATUS vfsMountEject (mount_t *pMount) {

    if (vfsDrvNumber != ERROR) {
        iosDevDelete (&pMount->mnt_devhdr);
        return (OK);
    }

    return ERROR;
}

