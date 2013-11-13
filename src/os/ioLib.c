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

/* ioLib.c - I/O library */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <os/errnoLib.h>
#include <os/iosLib.h>
#include <os/ioLib.h>

/* LOCALS */
LOCAL int ioStdFd[3];

/******************************************************************************
 * ioGlobalStdSet - Set global standard file descriptor
 *
 * RETURNS: N/A 
 */

void ioGlobalStdSet(
    int stdFd,
    int newFd
    )
{
    if (STD_VALID(stdFd) == TRUE)
    {
        ioStdFd[stdFd] = newFd;
    }
}

/******************************************************************************
 * ioGlobalStdGet - Get global standard file descriptor
 *
 * RETURNS: File descriptor or ERROR
 */

int ioGlobalStdGet(
    int stdFd
    )
{
    int f;

    if (STD_VALID(stdFd) == TRUE)
    {
        f = ioStdFd[stdFd];
    }
    else
    {
        f = ERROR;
    }

    return f;
}

/******************************************************************************
 * ioTaskStdSet - Set local standard file descriptor
 *
 * RETURNS: N/A 
 */

void ioTaskStdSet(
    int taskId,
    int stdFd,
    int newFd
    )
{
    TCB_ID tcbId;

    /* Get task or default task if zero */
    tcbId = taskTcb(taskId);
    if ((STD_VALID(stdFd) == TRUE) && (tcbId != NULL))
    {
        tcbId->taskStd[stdFd] = newFd;
    }
}

/******************************************************************************
 * ioTaskStdGet - Get local standard file descriptors
 *
 * RETURNS: File descriptor or ERROR
 */

int ioTaskStdGet(
    int taskId,
    int stdFd
    )
{
    TCB_ID tcbId;
    int fd, f;

    /* Get task or default task if zero */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        f = ERROR;
    }
    else
    {
        fd = tcbId->taskStd[stdFd];
        if (STD_VALID(fd) == TRUE)
        {
            f = ioStdFd[fd];
        }
        else
        {
            f = fd;
        }
    }

    return f;
}

/******************************************************************************
 * openConnect - Generic open connection function
 *
 * RETURNS: Status from I/O open function
 */

LOCAL int openConnect(
    DEV_HEADER *pDevHeader,
    char       *filename,
    va_list     args
    )
{
    int    oflags;
    mode_t mode;

    oflags = va_arg(args, int);
    mode   = va_arg(args, mode_t);

    return iosOpen(pDevHeader, filename, oflags, mode);
}

/******************************************************************************
 * creatConnect - Generic create function
 *
 * RETURNS: Status from I/O creat function
 */

LOCAL int creatConnect(
    DEV_HEADER *pDevHeader,
    char       *filename,
    va_list     args
    )
{
    mode_t mode;
    char  *symlink;

    mode    = va_arg(args, mode_t);
    symlink = va_arg(args, char *);

    return iosCreate(pDevHeader, filename, mode, symlink);
}

/******************************************************************************
 * removeConnect - Generic remove function
 *
 * RETURNS: Status from I/O delete function
 */

LOCAL int removeConnect(
    DEV_HEADER *pDevHeader,
    char       *filename,
    va_list     args
    )
{
    mode_t mode;     /* type of file to remove */

    mode = va_arg(args, mode_t);

    return iosDelete(pDevHeader, filename, mode);
}

/******************************************************************************
 * ioConnect - Generic I/O connect function
 *
 * RETURNS: ERROR on failure, other on success
 */

LOCAL int ioConnect(
    FUNCPTR     funcInternal,
    const char *filename,
    ...
    )
{
    int         rv;
    int         len;
    int         fd;
    int         cwdLen;
    char        path[PATH_MAX + 1];
    char       *tail;
    DEV_HEADER *pDevHeader;
    va_list     args;

    if ((filename == NULL) || (filename[0] == '\0'))
    {
        errnoSet(ENOENT);
        rv = ERROR;
    }
    else
    {
        /*
         * Prepend current working directory.
         * Also ensure that total path length does not exceed PATH_MAX.
         */

        /* Check length */
        cwdLen = pathCwdLen();
        len = strlen (filename);
        if (cwdLen + len > PATH_MAX)
        {
            errnoSet(ENAMETOOLONG);
            rv = ERROR;
        }
        else
        {
            /* Prepend default path name */
            if (pathPrependCwd((char *) filename, path) != OK)
            {
                errnoSet(EINVAL);
                rv = ERROR;
            }
            else
            {
                /* Allocate a file descriptor */
                fd = iosFdNew(NULL, NULL, NULL);

                args = va_start (args, filename);

                while (1)
                {
                    /*
                     * Strip ...
                     *   1.  Any superfluous '/' characters.
                     *   2.  All "." path components.
                     *   3.  Any ".." path components at the start of <path>.
                     */
                    pathCondense(path);

                    pDevHeader = iosDevFind(path, &tail);
                    if (pDevHeader == NULL)
                    {
                        errnoSet(ENODEV);
                        rv = ERROR;
                        break;
                    }

                    rv = (*funcInternal)(pDevHeader, tail, args);
                    if ((rv != FOLLOW_LINK) && (rv != FOLLOW_DOTDOT))
                    {
                        break;
                    }
                }

                va_end (args);

                /*
                 * If the driver's open/create routine failed, or if the
                 * allocated file descriptor could not be updated with the
                 * correct information, then free the file descriptor and
                 * return ERROR.
                 */

                if ((rv == ERROR) ||
                    (iosFdSet(fd, pDevHeader, path, (ARG) rv) != OK))
                {
                    iosFdFree(fd);
                    rv = ERROR;
                }
                else
                {
                    rv = fd;
                }
            }
        }
    }

    return rv;
}

/******************************************************************************
 * openInternal - open a file bypassing mode check
 *
 * RETURNS: Status from openConnect
 */

int openInternal(
    const char *filename,
    int         oflags,
    ...
    )
{
    int      mode;
    va_list  args;

    va_start(args, oflags);
    mode = va_arg (args, int);
    va_end(args);

    return ioConnect(openConnect, filename, oflags, mode);
}

/******************************************************************************
 * open - open a file
 *
 * RETURNS: File descriptor or ERROR
 */

int open(
    const char *filename,
    int         oflags,
    ...
    )
{
    int     fd;
    int     mode;
    va_list args;

    mode = 0;
    if (oflags & O_CREAT)
    {
        va_start(args, oflags);
        mode = va_arg(args, int);
        va_end(args);

        if ((mode & ~0777) != 0)      /* Only permission bits may be set */
        {
            errnoSet(EINVAL);
            fd = ERROR;
        }
        else
        {
            mode |= S_IFREG;    /* May only create regular files this way */
            fd = OK;
        }
    }
    else
    {
        fd = OK;
    }

    if (fd == OK)
    {
        fd = ioConnect(openConnect, filename, oflags, mode);
    }

    return fd;
}

/******************************************************************************
 * creat - create a file
 *
 * RETURNS: File descriptor or ERROR
 */

int creat(
    const char *filename,
    mode_t      mode
    )
{
    int fd;

    if ((mode & ~0777) != 0)      /* Only permission bits may be set */
    {
        errnoSet(EINVAL);
        fd = ERROR;
    }

    fd = ioConnect(creatConnect, filename, S_IFREG | mode, NULL);

    return fd;
}

/******************************************************************************
 * close - Close a file
 *
 * RETURNS: OK or ERROR
 */

int close(
    int fd
    )
{
    return iosClose (fd);
}

/***************************************************************************
 * remove - remove a file or a symlink (but not a directory)
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS remove(
    const char *path
    )
{
    return ioConnect(removeConnect, path, 0);
}

/******************************************************************************
 * read - Read from a file or device
 *
 * RETURNS: Bytes read or ERROR
 */

int read(
    int    fd,
    void  *buffer,
    size_t nBytes
    )
{
    return iosRead(fd, buffer, (int) nBytes);
}

/******************************************************************************
 * write - Write a file or device
 *
 * RETURNS: Bytes written or ERROR
 */

int write(
    int    fd,
    void  *buffer,
    size_t nBytes
    )
{
    return iosWrite(fd, buffer, (int) nBytes);
}

/******************************************************************************
 * ioctl - I/O control function
 *
 * RETURNS: Driver specific or ERROR
 */

int ioctl(
    int fd,
    int func,
    int arg
    )
{
    return iosIoctl(fd, func, (ARG) arg);
}

/******************************************************************************
 * isatty - Check if device is a terminal
 *
 * RETURNS: TRUE or FALSE
 */

int isatty(
    int fd
    )
{
    return (ioctl(fd, FIOISATTY, 0) == TRUE);
}

/******************************************************************************
 * mkdir - create a directory
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS mkdir(
    const char *path,     /* path to directory to create */
    mode_t      mode      /* permissions for directory */
    )
{
    STATUS status;
    int    fd;

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
        else
        {
            close(fd);
            status = OK;
        }
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
    mode_t      mode      /* file permission bits */
    )
{
    STATUS status;
    int    fd;

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
        else
        {
            close(fd);
            status = OK;
        }
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
    int     fd;
    ssize_t size;
    struct  iovec arg;

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
    int          fd,
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
    const char  *path,
    struct stat *buf
    )
{
    int    fd;
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
    const char  *path,
    struct stat *buf
    )
{
    int    fd;
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
    int         name
    )
{
    int  fd;
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
    int   fd,
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
    int   fd,
    off_t offset,
    int   whence
    )
{
    STATUS status;
    struct stat buf;
    off_t  value;
    off_t  ret;

    if (whence == SEEK_SET)
    {
        status = OK;
    }
    else if (whence == SEEK_CUR)
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
        if (fstat(fd, &buf) != OK)
        {
            status = ERROR;
        }
        else
        {
            offset += buf.st_size;
            status  = OK;
        }
    }
    else
    {
        errnoSet(EINVAL);
        status = ERROR;
    }

    if (status != OK)
    {
        ret = ERROR;
    }
    else
    {
        if (offset < 0)
        {
            errnoSet(EINVAL);
            status = ERROR;
        }
        else
        {
            if (ioctl(fd, FIOSEEK, offset) != OK)
            {
                ret = ERROR;
            }
            else
            {
                ret = offset;
            }
        }
    }

    return ret;
}

