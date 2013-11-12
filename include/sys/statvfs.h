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

#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H

/* includes */

#include <sys/types.h>

/* defines */

/* typedefs */

#define ST_RDONLY   0x0001      /* Read-only file system */
#define ST_NOSUID   0x0002      /* ST_ISUID & ST_ISGID not supported */

/* structs */

struct statvfs {
    unsigned long  f_bsize;    /* file system block size */
    unsigned long  f_frsize;   /* fundamental file system block size */
    fsblkcnt_t     f_blocks;   /* total # of blocks on FS */
    fsblkcnt_t     f_bfree;    /* # of free blocks */
    fsblkcnt_t     f_bavail;   /* # of free blocks available to */
                               /*       non-privileged user */
    fsfilcnt_t     f_files;    /* total # of file identifiers (inodes) */
    fsfilcnt_t     f_ffree;    /* # of free inodes */
    fsfilcnt_t     f_favail;   /* # of free inodes available to */
                               /*      non-privileged user */
    unsigned long  f_fsid;     /* file system ID */
    unsigned long  f_flag;     /* bit mask of f_flag values */
    unsigned long  f_namemax;  /* max length of filename */
};

/* externs */

int  statvfs (const char *path, struct statvfs * st);
int  fstatvfs (int fd, struct statvfs * st);
#endif
