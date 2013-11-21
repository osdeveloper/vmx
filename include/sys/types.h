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

/* sys/types.h - Posix types */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <arch/types.h>

/* Filesystem size */
typedef long fsize_t;

/* BSD types */
typedef unsigned char           u_char;
typedef unsigned short          u_short;
typedef unsigned int            u_int;
typedef unsigned long           u_long;
typedef long long int           quad_t;
typedef unsigned long long int  u_quad_t;
typedef int                     fsid_t;
typedef long long int           off_t;      /* for file sizes */
typedef char                    *caddr_t;
typedef unsigned long           n_time;
typedef unsigned long           n_long;

typedef unsigned                dev_t;      /* for device IDs */
typedef unsigned long long      ino_t;      /* for inode numbers */
typedef unsigned short          mode_t;     /* for file attributes */
typedef unsigned short          nlink_t;    /* for counting hardlinks */
typedef unsigned short          uid_t;      /* for user IDs */
typedef unsigned short          gid_t;      /* for group IDs */
typedef unsigned                blksize_t;  /* for block sizes */
typedef unsigned                blkcnt_t;   /* for file block counts */
typedef unsigned long long      time_t;     /* for time in seconds */
typedef unsigned long long      fsblkcnt_t; /* for # of blocks in file system */
typedef unsigned long long      fsfilcnt_t; /* for # of inodes in file system */

typedef unsigned long long      lblkno_t;    /* logical block number */
typedef long                    clock_t;
typedef int                     clockid_t;

/* ANSI types */
typedef unsigned long           fpos_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif

