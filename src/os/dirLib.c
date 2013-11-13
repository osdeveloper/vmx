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

/* dirLib.c - directory library */

/*
DESIGN NOTES:
The DIR (or dir_stream if you will) structure came close to becoming one of
the VFS structures and directly associated with the VFS file descriptor type.
However, it was felt that this could swell the structure size too much if
done statically.  In addition, there would likely be some concurrency issues
that would have to be resolved.  For the time being, it is restricted to this
module, and is allocated dynamically here (easiest and fastest to implement).
It is freed when the directory is closed using closedir().
*/

/* includes */

#include <vmx.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <os/private/ioLibP.h>

/* defines */

#define DS_MAGIC  0x4449522E  /* DIR/ */

/* typedefs */

/* structures */

struct dir_stream {
    unsigned       ds_magic;   /* Magic # for parameter checking */
    int            ds_fd;      /* Directory's file descriptor */
    struct dirent  ds_dirent;  /* Directory entry */
};

/* externs */

/* globals */

/***************************************************************************
 *
 * closedir - close a directory
 *
 * RETURNS: OK on success, ERROR otherwise
 */

int closedir (
    DIR *  pDir   /* ptr directory to close */
    ) {
    int rv;

    if ((pDir == NULL) || (pDir->ds_magic != DS_MAGIC)) {
        errnoSet (EINVAL);
        return (ERROR);
    }

    rv = close (pDir->ds_fd);

    if (rv == OK) {
        pDir->ds_magic = 0;
        free (pDir);
    }

    return (rv);
}

/***************************************************************************
 *
 * opendir - open a directory
 *
 * RETURNS: ptr to directory on success, NULL on ERROR
 */

DIR *  opendir (
    const char *  path  /* path to directory for which to open */
    ) {
    int    fd;
    DIR *  pDir;

    pDir = (DIR *) malloc (sizeof (struct dir_stream));
    if (pDir == NULL) {
        /* errno set by malloc() */
        return (NULL);
    }

    fd = openInternal(path, O_RDONLY, S_IFDIR);
    if (fd < 0) {
        free (pDir);
        /* errno set by openInternal() */
        return (NULL);
    }

    pDir->ds_magic = DS_MAGIC;
    pDir->ds_fd = fd;
    memset (&pDir->ds_dirent, 0, sizeof (struct dirent));

    return (pDir);
}

/***************************************************************************
 *
 * readdir - read a directory entry
 *
 * RETURNS: ptr to directory entry on success, NULL on error
 */

struct dirent *  readdir (
    DIR * pDir    /* ptr to directory for which to read */
    ) {
    struct dirent *  pDirEnt;

    if ((pDir == NULL) || (pDir->ds_magic != DS_MAGIC)) {
        errnoSet (EINVAL);
        return (NULL);
    }

    pDirEnt = &pDir->ds_dirent;
    if (ioctl (pDir->ds_fd, FIOREADDIR, (int) pDirEnt) != OK)
      return NULL;

    return (pDirEnt);
}

/***************************************************************************
 *
 * rewinddir - rewind to the beginning of the directory
 *
 * RETURNS: OK on success, non-zero otherwise
 */

void rewinddir (
    DIR *  pDir  /* ptr to directory for which to reset */
    ) {
    if ((pDir == NULL) || (pDir->ds_magic != DS_MAGIC)) {
        return;
    }

    lseek (pDir->ds_fd, 0, SEEK_SET);
}

