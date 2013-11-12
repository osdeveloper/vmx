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

/*
 * XXX
 * This file is definitely incomplete.  It is a work in progress.
 * XXX
 */

#ifndef __VNODE_H
#define __VNODE_H

/* includes */

#include <time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <vmx.h>
#include <util/listLib.h>
#include <vmx/semLib.h>
#include <fs/namei.h>
#include <fs/mount.h>

/* defines */

#define V_ROOT      0x0001
#define V_DELETED   0x0002

#define VNOVAL      -1

/* macros */
#define VOP_LOOKUP(vp, args)    (vp)->v_ops->lookup  ((args))
#define VOP_CREATE(vp, args)    (vp)->v_ops->create   ((args))
#define VOP_OPEN(vp, args)      (vp)->v_ops->open     ((args))
#define VOP_CLOSE(vp, args)     (vp)->v_ops->close    ((args))
#define VOP_ACCESS(vp, args)    (vp)->v_ops->access   ((args))
#define VOP_READ(vp, args)      (vp)->v_ops->read     ((args))
#define VOP_WRITE(vp, args)     (vp)->v_ops->write    ((args))
#define VOP_IOCTL(vp, args)     (vp)->v_ops->ioctl    ((args))
#define VOP_LINK(vp, args)      (vp)->v_ops->link     ((args))
#define VOP_UNLINK(vp, args)    (vp)->v_ops->unlink   ((args))
#define VOP_SYMLINK(vp, args)   (vp)->v_ops->symlink  ((args))
#define VOP_READLINK(vp, args)  (vp)->v_ops->readlink ((args))
#define VOP_MKDIR(vp, args)     (vp)->v_ops->mkdir    ((args))
#define VOP_RMDIR(vp, args)     (vp)->v_ops->rmdir    ((args))
#define VOP_READDIR(vp, args)   (vp)->v_ops->readdir  ((args))
#define VOP_GETATTR(vp, args)   (vp)->v_ops->getattr  ((args))
#define VOP_SETATTR(vp, args)   (vp)->v_ops->setattr  ((args))
#define VOP_TRUNCATE(vp, args)  (vp)->v_ops->truncate ((args))
#define VOP_FSYNC(vp, args)     (vp)->v_ops->fsync    ((args))
#define VOP_ACTIVATE(vp, args)  (vp)->v_ops->activate ((args))
#define VOP_INACTIVE(vp, args)  (vp)->v_ops->inactive ((args))
#define VOP_PATHCONF(vp, args)  (vp)->v_ops->pathconf ((args))
#define VOP_SEEK(vp, args)      (vp)->v_ops->seek     ((args))
#define VOP_RENAME(vp, args)    (vp)->v_ops->rename   ((args))
#define VOP_ABORT(vp, args)     (vp)->v_ops->abort    ((args))
#define VOP_STRATEGY(vp, args)  (vp)->v_ops->strategy ((args))
#define VOP_PRINT(vp, args)     (vp)->v_ops->print    ((args))

#define VTODATA(type, vp)       ((type *) (vp)->v_data)

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

/* typedefs */

typedef off_t        voff_t;

typedef struct ucred {
    /* Not yet filled */
} ucred_t;

typedef struct vattr {
    unsigned         va_type;         /* vnode type */
    mode_t           va_mode;         /* permission bits */
    nlink_t          va_nlink;        /* # of hard links */
    uid_t            va_uid;          /* user ID */
    gid_t            va_gid;          /* group ID */
    ino_t            va_fileid;       /* inode number */
    off_t            va_size;         /* file size in bytes */
    blksize_t        va_blksize;      /* device block size */
    struct timespec  va_atime;        /* time of last access */
    struct timespec  va_mtime;        /* time of last data modification */
    struct timespec  va_ctime;        /* time of last modification */
    struct timespec  va_birthtime;    /* time of file creation */
    unsigned         va_flags;        /* special inode flags */

    /* The following fields are not yet used */
    unsigned         va_fsid;         /* file system identifier */
    unsigned         va_gen;          /* file generation # */
    unsigned         va_rdev;         /* special device identifier */
    unsigned         va_bytes;        /* # of bytes used by file */
    unsigned         va_filerev;      /* file revision # */
} vattr_t;

/*
 * The arguments to the various vnode operator methods have their own types.
 * Should new parameters need to be added to a particular type in the future,
 * this should hopefully minimize the amount of retro-fitting.
 */

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode **         vpp;   /* retrieved vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
} lookup_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode **         vpp;   /* created vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
    struct vattr *          vap;   /* vnode attributes pointer */
} create_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    int             mode;          /* mode */
    struct ucred *  ucp;           /* user credentials pointer */
} open_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    int             flags;         /* flags */
    struct ucred *  ucp;           /* user credentials pointer */
} close_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    int             mode;          /* mode */
    struct ucred *  ucp;           /* user credentials pointer */
} access_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct uio *    uio;           /* user IO pointer */
    int             ioflag;        /* IO flags */
    struct ucred *  ucp;           /* user credentials pointer */
} read_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct uio *    uio;           /* user IO pointer */
    int             ioflag;        /* IO flags */
    struct ucred *  ucp;           /* user credentials pointer */
} write_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    u_long          cmd;           /* device specific command */
    void *          data;          /* extra data */
    int             fflag;         /* flags */
    struct ucred *  ucp;           /* user credentials pointer */
} ioctl_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode *          vp;    /* file vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
} link_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode *          vp;    /* file vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
} unlink_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode **         vpp;   /* created vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
    struct vattr *          vap;   /* vnode attributes pointer */
    char *                  tgt;   /* ptr to target path string */
} symlink_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct uio *    uio;           /* user IO pointer */
    struct ucred *  ucp;           /* user credentials pointer */
} readlink_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode **         vpp;   /* created vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
    struct vattr *          vap;   /* vnode attributes pointer */
} mkdir_args_t;

typedef struct {
    struct vnode *          dvp;   /* directory vnode pointer */
    struct vnode *          vp;    /* file vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
} rmdir_args_t;

typedef struct {
    struct vnode *   dvp;          /* directory vnode pointer */
    struct dirent *  dep;          /* directory entry pointer */
    struct ucred *   ucp;          /* user credentials pointer */
    int *            eof;          /* end of file status */
    int *            nCookies;     /* number of cookies */
    u_long *         cookies;      /* cookies */
} readdir_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct vattr *  vap;           /* vnode attributes pointer */
    struct ucred *  ucp;           /* user credentials pointer */
} getattr_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct vattr *  vap;           /* vnode attributes pointer */
    struct ucred *  ucp;           /* user credentials pointer */
} setattr_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    off_t           len;           /* new length of the file */
    int             flags;         /* flags */
    struct ucred *  ucp;           /* user credentials pointer */
} truncate_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct ucred *  ucp;           /* user credentials pointer */
    int             flags;         /* flags */
} fsync_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
} activate_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
} inactive_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    int             name;          /* type of info to return */
    int *           rv;            /* return value */
} pathconf_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    off_t           off1;          /* old offset */
    off_t           off2;          /* new offset */
    struct ucred *  ucp;           /* user credentials pointer */
} seek_args_t;

typedef struct {
    struct vnode *          fdvp;  /* from directory vnode pointer */
    struct vnode *          fvp;   /* from file vnode pointer */
    struct componentname *  fcnp;  /* from path name component pointer */
    struct vnode *          tdvp;  /* to directory vnode pointer */
    struct vnode *          tvp;   /* to file vnode pointer */
    struct componentname *  tcnp;  /* to path name component pointer */
} rename_args_t;

typedef struct {
    struct vnode *          vp;    /* file vnode pointer */
    struct componentname *  cnp;   /* path name component pointer */
} abort_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
    struct buf *    bp;            /* buffer pointer */
} strategy_args_t;

typedef struct {
    struct vnode *  vp;            /* file vnode pointer */
} print_args_t;

typedef struct vnode_ops {
    int  (*lookup)   (lookup_args_t);
    int  (*create)   (create_args_t);
    int  (*open)     (open_args_t);
    int  (*close)    (close_args_t);
    int  (*access)   (access_args_t);
    int  (*read)     (read_args_t);
    int  (*write)    (write_args_t);
    int  (*ioctl)    (ioctl_args_t);
    int  (*link)     (link_args_t);
    int  (*unlink)   (unlink_args_t);
    int  (*symlink)  (symlink_args_t);
    int  (*readlink) (readlink_args_t);
    int  (*mkdir)    (mkdir_args_t);
    int  (*rmdir)    (rmdir_args_t);
    int  (*readdir)  (readdir_args_t);
    int  (*getattr)  (getattr_args_t);
    int  (*setattr)  (setattr_args_t);
    int  (*truncate) (truncate_args_t);
    int  (*fsync)    (fsync_args_t);
    int  (*activate) (activate_args_t);
    int  (*inactive) (inactive_args_t);
    int  (*pathconf) (pathconf_args_t);
    int  (*seek)     (seek_args_t);
    int  (*rename)   (rename_args_t);
    int  (*abort)    (abort_args_t);
    int  (*strategy) (strategy_args_t);
    int  (*print)    (print_args_t);
} vnode_ops_t;

typedef struct vnode {
    LIST_NODE           v_node;     /* linked list node */
    struct vnode_ops *  v_ops;      /* ptr to vnode operators */
    struct mount *      v_mount;    /* ptr to mount structure */
    int                 v_count;    /* use count (# of FDs using it) */
    int                 v_flags;    /* various flags such as V_ROOT */
    unsigned            v_type;     /* vnode type */
    LIST                v_buflist;  /* buffers in use by this vnode */
    ino_t               v_inode;    /* inode number */
    char *              v_data;     /* FS specific data */
} vnode_t;

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
    struct mount * pMount,     /* ptr to file system mount */
    ino_t          inode,      /* inode for which to get vnode */
    vnode_ops_t *  pVnodeOps,  /* ptr to vnode operators */
    vnode_t **     ppVnode     /* double ptr to vnode : output */
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

#endif

