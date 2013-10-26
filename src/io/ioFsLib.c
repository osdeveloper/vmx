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

/* ioFsLib.c - File System I/O support library */

#include <vmx.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/uio.h>
#include <io/ioLib.h>
#include <io/private/ioLibP.h>

/******************************************************************************
 * mkdir - create a directory
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS mkdir(
    const char *path,     /* path to directory to create */
    mode_t mode           /* permissions for directory */
    )
{
    STATUS status;
    int fd;

    if(mode & ~0777)      /* only allow permission bits to be set */
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        fd = ioConnect(creatConnect, path, S_IFDIR | mode, NULL);
        if (fd < 0)
        {
            status = ERROR;
        }

        close(fd);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * rmdir - remove a directory
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS rmdir(
    const char *path      /* path to directory to remove */
    )
{
    STATUS status;

    status = ioConnect(removeConnect, path, S_IFDIR);

    return status;
}

/******************************************************************************
 * symlink - create a symlink
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS symlink(
    const char *path,     /* path to symlink to create */
    const char *target,   /* path against which to link */
    mode_t mode           /* file permission bits */
    )
{
    STATUS status;
    int fd;

    if (mode & ~0777)     /* only allow permission bits */
    {
        errnoSet(EINVAL);
        status = ERROR;
    }
    else
    {
        fd = ioConnect(creatConnect, path, S_IFLNK | mode, target);
        if (fd < 0)
        {
            status = ERROR;
        }

        close(fd);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * readlink - read a symlink
 *
 * RETURNS: # of bytes in buffer on success, or ERROR otherwise
 */

ssize_t readlink(
    const char *path,
    char *buf,
    size_t bufsize
    )
{
    int fd;
    ssize_t size;
    struct iovec arg;

    fd = ioConnect(openConnect, path, O_RDONLY, S_IFLNK);
    if (fd < 0)
    {
        size = ERROR;
    }
    else
    {
        arg.iov_base = buf;
        arg.iov_len  = bufsize;

        size = ioctl(fd, FIOREADLINK, (int) &arg);

        close(fd);
    }

    return size;
}

/******************************************************************************
 * fstat - retrieve file status information
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS fstat(
    int fd,
    struct stat *buf
    )
{
    STATUS status;

    status = ioctl(fd, FIOSTATGET, (int) buf);

    return status;
}

/******************************************************************************
 * stat - retrieve file status information
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS stat(
    const char *path,
    struct stat *buf
    )
{
    int fd;
    STATUS status;

    fd = ioConnect(openConnect, path, O_RDONLY, 0);
    if (fd < 0)
    {
        status = ERROR;
    } 
    else
    {
        status = ioctl(fd, FIOSTATGET, (int) buf);

        close(fd);
    }

    return status;
}

/******************************************************************************
 * lstat - retrieve file status information
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS lstat(
    const char *path,
    struct stat *buf
    )
{
    int fd;
    STATUS status;

    /*
     * TODO: Check O_NOFOLLOW usage.  Documentation indicates that when
     * 'open()' uses NO_FOLLOW, if target is a symlink, then it is supposed
     * to fail.  This, routine however uses openConnect(), an internal
     * routine to VMX.  We could add some more checks to 'open()' for this
     * and allow the internal routines to open a symlink with this oflag set.
     * Anyways, food for thought.
     */

    fd = ioConnect(openConnect, path, O_NOFOLLOW | O_RDONLY, 0);
    if (fd < 0)
    {
        status = ERROR;
    }
    else
    {
        status = ioctl(fd, FIOSTATGET, (int) buf);

        close(fd);
    }

    return status;
}

/******************************************************************************
 * fpathconf - get configurable path variables
 *
 * RETURNS: current configurable value or limit on success, ERROR otherwise
 */

long fpathconf(
    int fd,
    int name
    )
{
    long rv;

    rv = ioctl(fd, FIOPATHCONF, name);

    return rv;
}

/******************************************************************************
 * pathconf - get configurable path variables
 *
 * RETURNS: current configurable value or limit on success, ERROR otherwise
 */

long pathconf(
    const char *path,
    int name
    )
{
    int fd;
    long rv;

    fd = ioConnect(openConnect, path, O_RDONLY, 0);
    if (fd < 0)
    {
        rv = ERROR;
    }
    else
    {
        rv = fpathconf(fd, name);

        close(fd);
    }

    return rv;
}

/******************************************************************************
 * ftruncate - truncate a file
 *
 * RETURNS: OK on success, ERROR otherwise
 */

int ftruncate(
    int fd,
    off_t length
    )
{
    STATUS status;

    status = ioctl(fd, FIOTRUNCATE, (int) &length);

    return status;
}

/***************************************************************************
 * lseek - seek to within a position in the file
 *
 * RETURNS: new position on success, ERROR otherwise
 */

off_t lseek(
    int fd,
    off_t offset,
    int whence
    )
{
    struct stat buf;
    STATUS status;
    off_t value;

    if (whence == SEEK_CUR)
    {
        /* Add <offset> to current position */
        if (ioctl(fd, FIOWHERE, (int) &value) != OK)
        {
            status = ERROR;
        }
        else
        {
            offset += value;
            status  = OK;
        }
    }
    else if (whence == SEEK_END) 
    {
        /* Add <offset> to size of the file */
        if (fstat (fd, &buf) != OK)
        {
            status = ERROR;
        }
        else
        {
            offset += buf.st_size;
            status  = OK;
        }
    }
    else if (whence != SEEK_SET)
    {
        errnoSet(EINVAL);
        status = ERROR;
    }

    if (offset < 0) {
        errnoSet(EINVAL);
        status = ERROR;
    }

    if (status == OK)
    {
        status = ioctl(fd, FIOSEEK, offset);
    }

    return status;
}

