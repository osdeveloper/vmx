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

#ifndef _DIRENT_H
#define _DIRENT_H

#include <stdio.h>              /* FILENAME_MAX defined here */

/* Directory is a file containing a sequence of dirent structures. */
#define DIRSIZ                  FILENAME_MAX

typedef struct dir_stream DIR;     /* Opaque type (see dirLib.c) */

struct dirent {
    ino_t  d_ino;
    char   d_name[FILENAME_MAX + 1];
};

/* functions */

/***************************************************************************
 *
 * closedir - close a directory
 *
 * RETURNS: OK on success, ERROR otherwise
 */

int closedir (
    DIR *  pDir   /* ptr directory to close */
    );

/***************************************************************************
 *
 * opendir - open a directory
 *
 * RETURNS: ptr to directory on success, NULL on ERROR
 */

DIR *  opendir (
    const char *  path  /* path to directory for which to open */
    );

/***************************************************************************
 *
 * readdir - read a directory entry
 *
 * RETURNS: ptr to directory entry on success, NULL on error
 */

struct dirent *  readdir (
    DIR * pDir    /* ptr to directory for which to read */
    );

/***************************************************************************
 *
 * rewinddir - rewind to the beginning of the directory
 *
 * RETURNS: OK on success, non-zero otherwise
 */

void rewinddir (
    DIR *  pDir  /* ptr to directory for which to reset */
    );

#endif /* _DIRENT_H */

