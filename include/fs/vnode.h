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

/* vnode.h - vnode header file */

#ifndef __VNODE_H
#define __VNODE_H

/* includes */

#include <time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <vmx.h>
#include <util/listLib.h>
#include <fs/namei.h>
#include <fs/mount.h>

/* defines */

#define V_ROOT          0x0001
#define V_DELETED       0x0002

/* Flags for ioflag */

#define VNIO_UNIT       0x01
#define VNIO_APPEND     0x02
#define VNIO_SYNC       0x04

/* Flags for vaflags */

#define VA_UTIMES_NULL  0x01
#define VA_EXCLUSIVE    0x02

/* Modes for vaccess */

#define VNOVAL          -1
#define VREAD           0004
#define VWRITE          0002
#define VEXEC           0001

/* enums */

enum vtype {
    VNON,    /* non-type */
    VREG,    /* regular file type */
    VDIR,    /* directory type */
    VLNK,    /* symbolic link type */
    VBLK,    /* block device type */
    VSOCK,   /* socket type */
    VFIFO,   /* FIFO type */
    VCHR,    /* character type */
};

/* structs */

struct mount;
struct dirent;
struct buf;
struct flock;

/* typedefs */

typedef off_t        voff_t;

typedef struct ucred {
    uid_t  cr_uid;
    gid_t  cr_gid;
} ucred_t;

typedef struct vattr {
    unsigned         va_type;         /* vnode type */
    mode_t           va_mode;         /* permission bits */
    nlink_t          va_nlink;        /* # of hard links */
    uid_t            va_uid;          /* user ID */
    gid_t            va_gid;          /* group ID */
    ino_t            va_fileid;       /* inode number */
    off_t            va_size;         /* file size in bytes */
    blksize_t        va_blocksize;    /* device block size */
    struct timespec  va_atime;        /* time of last access */
    struct timespec  va_mtime;        /* time of last data modification */
    struct timespec  va_ctime;        /* time of last modification */
    struct timespec  va_birthtime;    /* time of file creation */
    unsigned         va_flags;        /* special inode flags */
    unsigned         va_vaflags;      /* operations flags */

    /* The following fields are not yet used */
    unsigned         va_fsid;         /* file system identifier */
    unsigned         va_gen;          /* file generation # */
    unsigned         va_rdev;         /* special device identifier */
    unsigned         va_bytes;        /* # of bytes used by file */
    unsigned         va_filerev;      /* file revision # */
} vattr_t;

typedef struct vnode_ops {
    int (*vop_lookup) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode **         vpp,   /* retrieved vnode pointer */
        struct componentname *  cnp    /* path name component pointer */
        );

    int (*vop_create) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode **         vpp,   /* created vnode pointer */
        struct componentname *  cnp,   /* path name component pointer */
        struct vattr *          vap    /* vnode attributes pointer */
        );

    int (*vop_open) (
        struct vnode *  vp,            /* file vnode pointer */
        int             mode,          /* mode */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_close) (
        struct vnode *  vp,            /* file vnode pointer */
        int             flags,         /* flags */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_access) (
        struct vnode *  vp,            /* file vnode pointer */
        int             mode,          /* mode */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_getattr) (
        struct vnode *  vp,            /* file vnode pointer */
        struct vattr *  vap,           /* vnode attributes pointer */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_setattr) (
        struct vnode *  vp,            /* file vnode pointer */
        struct vattr *  vap,           /* vnode attributes pointer */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_read) (
        struct vnode *  vp,            /* file vnode pointer */
        struct uio *    uio,           /* user IO pointer */
        int             ioflag,        /* IO flags */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_write) (
        struct vnode *  vp,            /* file vnode pointer */
        struct uio *    uio,           /* user IO pointer */
        int             ioflag,        /* IO flags */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_ioctl) (
        struct vnode *  vp,            /* file vnode pointer */
        u_long          cmd,           /* device specific command */
        void *          data,          /* extra data */
        int             fflag,         /* flags */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_fcntl) (
        struct vnode *  vp,            /* file vnode pointer */
        u_long          cmd,           /* device specific command */
        void *          data,          /* extra data */
        int             fflag,         /* flags */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_fsync) (
        struct vnode *  vp,            /* file vnode pointer */
        struct ucred *  ucp,           /* user credentials pointer */
        int             flags          /* flags */
        );

    int (*vop_seek) (
        struct vnode *  vp,            /* file vnode pointer */
        off_t           off1,          /* old offset */
        off_t           off2,          /* new offset */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_remove) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode *          vp,    /* file vnode pointer */
        struct componentname *  cnp    /* path name component pointer */
        );

    int (*vop_link) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode *          vp,    /* file vnode pointer */
        struct componentname *  cnp    /* path name component pointer */
        );

    int (*vop_rename) (
        struct vnode *          fdvp,  /* from directory vnode pointer */
        struct vnode *          fvp,   /* from file vnode pointer */
        struct componentname *  fcnp,  /* from path name component pointer */
        struct vnode *          tdvp,  /* to directory vnode pointer */
        struct vnode *          tvp,   /* to file vnode pointer */
        struct componentname *  tcnp   /* to path name component pointer */
        );

    int (*vop_mkdir) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode **         vpp,   /* created vnode pointer */
        struct componentname *  cnp,   /* path name component pointer */
        struct vattr *          vap    /* vnode attributes pointer */
        );

    int (*vop_rmdir) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode *          vp,    /* file vnode pointer */
        struct componentname *  cnp    /* path name component pointer */
        );

    int (*vop_symlink) (
        struct vnode *          dvp,   /* directory vnode pointer */
        struct vnode **         vpp,   /* created vnode pointer */
        struct componentname *  cnp,   /* path name component pointer */
        struct vattr *          vap,   /* vnode attributes pointer */
        char *                  tgt    /* ptr to target path string */
        );

    int (*vop_readdir) (
        struct vnode *   vp,           /* directory vnode pointer */
        struct dirent *  dep,          /* directory entry pointer */
        struct ucred *   ucp,          /* user credentials pointer */
        int *            eof,          /* end of file status */
        int *            cookies       /* cookies */
        );

    int (*vop_readlink) (
        struct vnode *  vp,            /* file vnode pointer */
        struct uio *    uio,           /* user IO pointer */
        struct ucred *  ucp            /* user credentials pointer */
        );

    int (*vop_abortop) (
        struct vnode *          vp,    /* file vnode pointer */
        struct componentname *  cnp    /* path name component pointer */
        );

    int (*vop_activate) (
        struct vnode *  vp             /* file vnode pointer */
        );

    int (*vop_inactive) (
        struct vnode *  vp             /* file vnode pointer */
        );

    void (*vop_strategy) (
        struct vnode *  vp,            /* file vnode pointer */
        struct buf *    bp             /* buffer pointer */
        );

    void (*vop_print) (
        struct vnode *  vp             /* file vnode pointer */
        );

    int (*vop_pathconf) (
        struct vnode *  vp,            /* file vnode pointer */
        int             name,          /* type of info to return */
        long *          rv             /* return value */
        );

    int (*vop_advlock) (
        struct vnode *  vp,            /* file vnode pointer */
        void *          id,            /* identifier */
        int             op,            /* operation */
        struct flock *  fl,            /* file loock */
        int             flags          /* flags */
        );

    int (*vop_truncate) (
        struct vnode *  vp,            /* file vnode pointer */
        off_t           len,           /* new length of the file */
        int             flags,         /* flags */
        struct ucred *  ucp            /* user credentials pointer */
        );
} vnode_ops_t;

typedef struct vnode {
    LIST_NODE                 v_node;     /* linked list node */
    const struct vnode_ops *  v_ops;      /* ptr to vnode operators */
    struct mount *            v_mount;    /* ptr to mount structure */
    int                       v_count;    /* use count (# of FDs using it) */
    int                       v_flags;    /* various flags such as V_ROOT */
    unsigned                  v_type;     /* vnode type */
    LIST                      v_buflist;  /* buffers in use by this vnode */
    ino_t                     v_inode;    /* inode number */
    char *                    v_data;     /* FS specific data */
} vnode_t;

/* macros */

/* Get shared or exclusive lock on vnode */
#define VN_SHLOCK(vp)   (0)
#define VN_EXLOCK(vp)   (0)
#define VN_UNLOCK(vp)   (0)

#define VOP_LOOKUP(dvp, vpp, cnp) \
    (dvp)->v_ops->vop_lookup ((dvp), (vpp), (cnp))

#define VOP_CREATE(dvp, vpp, cnp, vap) \
    (dvp)->v_ops->vop_create ((dvp), (vpp), (cnp), (vap))

#define VOP_OPEN(vp, mode, ucp) \
    (vp)->v_ops->vop_open ((vp), (mode), (ucp))

#define VOP_CLOSE(vp, flags, ucp) \
    (vp)->v_ops->vop_close ((vp), (flags), (ucp))

#define VOP_ACCESS(vp, mode, ucp) \
    (vp)->v_ops->vop_access ((vp), (mode), (ucp))

#define VOP_GETATTR(vp, vap, ucp) \
    (vp)->v_ops->vop_getattr ((vp), (vap), (ucp))

#define VOP_SETATTR(vp, vap, ucp) \
    (vp)->v_ops->setattr ((vp), (vap), (ucp))

#define VOP_READ(vp, uio, ioflag, ucp) \
    (vp)->v_ops->vop_read ((vp), (uio), (ioflag), (ucp))

#define VOP_WRITE(vp, uio, ioflag, ucp) \
    (vp)->v_ops->vop_write ((vp), (uio), (ioflag), (ucp))

#define VOP_IOCTL(vp, cmd, data, fflag, ucp) \
    (vp)->v_ops->vop_ioctl ((vp), (cmd), (data), (fflag), (ucp))

#define VOP_FCNTL(vp, cmd, data, fflag, ucp) \
    (vp)->v_ops->vop_ioctl ((vp), (cmd), (data), (fflag), (ucp))

#define VOP_FSYNC(vp, ucp, flags) \
    (vp)->v_ops->vop_fsync ((vp), (ucp), (flags))

#define VOP_SEEK(vp, off1, off2, ucp) \
    (vp)->v_ops->vop_seek ((vp), (off1), (off2), (ucp))

#define VOP_REMOVE(dvp, vp, cnp) \
    (dvp)->v_ops->vop_remove ((dvp), (vp), (cnp))

#define VOP_LINK(dvp, vp, cnp) \
    (dvp)->v_ops->vop_link ((dvp), (vp), (cnp))

#define VOP_RENAME(fdvp, fvp, fcnp, tdvp, tvp, tcnp) \
    (fdvp)->v_ops->vop_rename ((fdvp), (fvp), (fcnp), (tdvp), (tvp), (tcnp))

#define VOP_MKDIR(dvp, vpp, cnp, vap) \
    (dvp)->v_ops->vop_mkdir ((dvp), (vpp), (cnp), (vap))

#define VOP_RMDIR(dvp, vp, cnp) \
    (dvp)->v_ops->vop_rmdir ((dvp), (vp), (cnp))

#define VOP_SYMLINK(dvp, vpp, cnp, vap, tgt) \
    (dvp)->v_ops->vop_symlink((dvp), (vpp), (cnp), (vap), (tgt))

#define VOP_READDIR(vp, dep, ucp, eof, cookies) \
    (vp)->v_ops->vop_readdir ((vp), (dep), (ucp), (eof), (cookies))

#define VOP_READLINK(vp, uio, ucp) \
    (vp)->v_ops->vop_readlink ((vp), (uio), (ucp))

#define VOP_ABORTOP(vp, cnp) \
    (vp)->v_ops->vop_abortop ((vp), (cnp))

#define VOP_ACTIVATE(vp) \
    (vp)->v_ops->vop_activate ((vp))

#define VOP_INACTIVE(vp) \
    (vp)->v_ops->vop_inactive ((vp))

#define VOP_STRATEGY(vp, bp) \
    (vp)->v_ops->vop_strategy ((vp), (bp))

#define VOP_PRINT(vp) \
    (vp)->v_ops->vop_print ((vp))

#define VOP_PATHCONF(vp, name, rv) \
    (vp)->v_ops->vop_pathconf ((vp), (name), (rv))

#define VOP_ADVLOCK(vp, id, op, fl, flags) \
    (vp)->v_ops->vop_advlock ((vp), (id), (op), (fl), (flags))

#define VOP_TRUNCATE(vp, len, flags, ucp) \
    (vp)->v_ops->vop_truncate ((vp), (len), (flags), (ucp))

#define VTODATA(type, vp) \
    ((type *) (vp)->v_data)

#define vop_error_lookup                                                      \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode **,                                                      \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_lookup_enotdir                                              \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode **,                                                      \
        struct componentname *                                                \
        )) vopErrorENOTDIR)

#define vop_error_create                                                      \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode **,                                                      \
        struct componentname *,                                               \
        struct vattr *                                                        \
        )) vopErrorENOSYS)

#define vop_error_open                                                        \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_close                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_access                                                      \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_getattr                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vattr *,                                                       \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_setattr                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vattr *,                                                       \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_read                                                        \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct uio *,                                                         \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_write                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct uio *,                                                         \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_ioctl                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        u_long, void *,                                                       \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_fcntl                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        u_long,                                                               \
        void *,                                                               \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_fsync                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct ucred *,                                                       \
        int                                                                   \
        )) vopErrorENOSYS)

#define vop_error_seek                                                        \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        voff_t,                                                               \
        voff_t,                                                               \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_remove                                                      \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_link                                                        \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_rename                                                      \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *,                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_mkdir                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode **,                                                      \
        struct componentname *,                                               \
        struct vattr *                                                        \
        )) vopErrorENOSYS)

#define vop_error_rmdir                                                       \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_rmdir_enotdir                                               \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOTDIR)

#define vop_error_symlink                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct vnode **,                                                      \
        struct componentname *,                                               \
        struct vattr *,                                                       \
        char *                                                                \
        )) vopErrorENOSYS)

#define vop_error_readdir                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct dirent *,                                                      \
        struct ucred *,                                                       \
        int *,                                                                \
        int *                                                                 \
        )) vopErrorENOSYS)

#define vop_error_readdir_enotdir                                             \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct dirent *,                                                      \
        struct ucred *,                                                       \
        int *,                                                                \
        int *                                                                 \
        )) vopErrorENOTDIR)

#define vop_error_readlink                                                    \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct uio *,                                                         \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

#define vop_error_abortop                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        struct componentname *                                                \
        )) vopErrorENOSYS)

#define vop_error_strategy                                                    \
    ((void (*) (                                                              \
        struct vnode *,                                                       \
        struct buf *                                                          \
        )) vopErrorVOID)

#define vop_error_pathconf                                                    \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        int,                                                                  \
        long *                                                                \
        )) vopErrorENOSYS)

#define vop_error_advlock                                                     \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        void *,                                                               \
        int,                                                                  \
        struct flock *,                                                       \
        int                                                                   \
        )) vopErrorENOSYS)

#define vop_error_truncate                                                    \
    ((int (*) (                                                               \
        struct vnode *,                                                       \
        voff_t,                                                               \
        int,                                                                  \
        struct ucred *                                                        \
        )) vopErrorENOSYS)

/* functions */

/***************************************************************************
 *
 * vnodesCreate - create vnodes
 *
 * RETURNS: ptr to array of vnodes (syncer vnode is the 1st), NULL on error
 */

struct vnode *  vnodesCreate (
    struct mount *  pMount,    /* ptr to mount */
    int             isize,     /* size of the inode structure */
    int             nvnode     /* # of file/logical vnodes to create (> 0) */
    );

/***************************************************************************
 *
 * vnodesDelete - delete vnodes
 *
 * RETURNS: N/A
 */

void vnodesDelete (
    struct vnode *  pVnode,
    int             nvnode
    );

/***************************************************************************
 *
 * vnodeLock - establish a lock by incrementing the use count
 *
 * RETURNS: OK
 */

STATUS vnodeLock (
    vnode_t *  pVnode
    );

/***************************************************************************
 *
 * vnodeUnlock - release the lock by decrementing the use count
 *
 * RETURNS: OK on success, non-zero if VOP_INACTIVE failed
 */

STATUS vnodeUnlock (
    vnode_t *  pVnode
    );

/***************************************************************************
 *
 * vgetino - get the vnode for the given inode
 *
 * RETURNS: OK on success, error otherwise
 */

int vgetino (
    struct mount *       pMount,     /* ptr to file system mount */
    ino_t                inode,      /* inode for which to get vnode */
    const vnode_ops_t *  pVnodeOps,  /* ptr to vnode operators */
    vnode_t **           ppVnode     /* double ptr to vnode : output */
    );

/***************************************************************************
 *
 * vflushbuf - flush all dirty buffers to disk
 *
 * RETURNS: OK on success, error otherwise
 */

int vflushbuf (
    vnode_t  * vp,
    BOOL       sync
    );

/***************************************************************************
 *
 * vopErrorENOSYS - error default operator for ENOSYS
 *
 * RETURNS: ENOSYS
 */


int vopErrorENOSYS(
    void
    );

/***************************************************************************
 *
 * vopErrorENOTDIR - error default operator
 *
 * RETURNS: ENOTDIR
 */

int vopErrorENOTDIR(
    void
    );

/***************************************************************************
 *
 * vopErrorVOID - error default operator
 *
 * RETURNS: N/A
 */

void vopErrorVOID(
    void
    );

#endif

