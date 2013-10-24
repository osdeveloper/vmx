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

/* stat.h - File status header */

#ifndef _SYS_STAT_H
#define _SYS_STAT_H

/* includes */

#include <sys/types.h>

/* defines */

#define S_IRUSR   0400
#define S_IWUSR   0200
#define S_IXUSR   0100

#define S_IRGRP   0040
#define S_IWGRP   0020
#define S_IXGRP   0010

#define S_IROTH   0004
#define S_IWOTH   0002
#define S_IXOTH   0001

#define S_IRWXU   (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG   (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO   (S_IROTH | S_IWOTH | S_IXOTH)

#define S_ISUID   04000
#define S_ISGID   02000
#define S_ISVTX   01000

#define S_IFMT    0xF000     /* Mask for type of file */

#define S_IFIFO   0x1000     /* FIFO special */
#define S_IFCHR   0x2000     /* Character special */
#define S_IFDIR   0x4000     /* Directory */
#define S_IFREG   0x8000     /* Regular file */

#define S_IFBLK   0x6000     /* Block special */
#define S_IFLNK   0xA000     /* Symbolic link */
#define S_IFSOCK  0xC000     /* Socket */

#define S_ISBLK(x)   (((x) & S_IFMT) == S_IFBLK)
#define S_ISCHR(x)   (((x) & S_IFMT) == S_IFCHR)
#define S_ISDIR(x)   (((x) & S_IFMT) == S_IFDIR)
#define S_ISFIFO(x)  (((x) & S_IFMT) == S_IFIFO)
#define S_ISLNK(x)   (((x) & S_IFMT) == S_IFLNK)
#define S_ISREG(x)   (((x) & S_IFMT) == S_IFREG)
#define S_ISSOCK(x)  (((x) & S_IFMT) == S_IFSOCK)

/* typedefs */

/* structs */

struct stat
{
    dev_t      st_dev;        /* ID of device that has the object */
    ino_t      st_ino;        /* object's inode # */
    mode_t     st_mode;       /* object's mode (permissions) */
    nlink_t    st_nlink;      /* # of hard links to the object */
    uid_t      st_uid;        /* User ID */
    gid_t      st_gid;        /* Group ID */
    dev_t      st_rdev;       /* Special device ID (if character or block) */
    off_t      st_size;       /* Size of file */
    time_t     st_atime;      /* Time object was last accessed. */
    time_t     st_mtime;      /* Time object data was last modified. */
    time_t     st_ctime;      /* Time object data/metadata last changed */
    blksize_t  st_blksize;    /* File system block size */
    blkcnt_t   st_blocks;     /* # of blocks assigned to this object */
};

/* externs */

#ifdef NOT_YET
extern int  chmod (const char *path, mode_t mode);
extern int  fchmod (int fd, mode_t mode);
extern int  fstat (int fd, struct stat *st);
extern int  lstat (const char * path, struct stat *st);
extern int  mkdir (const char *path, mode_t mode);
extern int  mkfifo (const char *path, mode_t);
extern int  stat (const char *path, struct stat *st);
extern int  umask (mode_t mode);
#endif

#endif /* _SYS_STAT_H */

