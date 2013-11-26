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

/* namei.h - component name identification header file */

#ifndef __NAMEI_H
#define __NAMEI_H

/* includes */

/* defines */

#define ISLASTCN    0x0001     /* set if component is last in path */
#define CNSLASH     0x0002     /* set if '/' follows component */
#define ISDOT       0x0004     /* "." path component */
#define ISDOTDOT    0x0008     /* ".." path component */
#define NOFOLLOW    0x0010     /* do not follow symlink on last component */
#define PDIRUNLOCK  0x0020     /* TODO */

#define ND_INIT(nidp, path, pathlen, dvp, cnop, cnflags)   \
               do {                                        \
               (nidp)->ni_path = (path);                   \
               (nidp)->ni_pathlen = (pathlen);             \
               (nidp)->ni_dvp = (dvp);                     \
               (nidp)->ni_cn.cn_nameiop = (cnop);          \
               (nidp)->ni_cn.cn_flags = (cnflags);         \
               } while (0)

/* structs */

struct vnode;

/* typedefs */

typedef enum {
    LOOKUP,
    DELETE,
    CREATE,
    RENAME
} cn_op_t;

typedef struct componentname {
    cn_op_t        cn_nameiop;  /* operation identifier */
    char *         cn_nameptr;  /* ptr to the component */
    int            cn_namelen;  /* length of the path name component */
    int            cn_flags;    /* flags */
    struct ucred * cn_cred;     /* credentials */
}  componentname_t;

typedef struct nameidata {
    char *           ni_path;     /* ptr to the full path */
    int              ni_pathlen;  /* length of the full path */
    struct vnode *   ni_dvp;      /* ptr to parent directory vnode */
    struct vnode *   ni_vp;       /* ptr to file/directory vnode */
    componentname_t  ni_cn;       /* component name */
} nameidata_t;

/* externs */

#endif

