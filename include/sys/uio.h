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

/* uio.h - header file for uio (used by scatter-gather I/O)  */

#ifndef _SYS_UIO_H
#define _SYS_UIO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE

#include <sys/types.h>

struct iovec
{
    void   *iov_base;    /* Base address. */
    size_t  iov_len;     /* # of bytes to copy to/from base address. */
};

enum uio_rw
{
    UIO_READ = 0,        /* Read from I/O vector(s) into buffer. */
    UIO_WRITE            /* Write from buffer into I/O vector(s). */
};

/*
 * Incomplete!
 * Ultimately, the uio structure will require some fields for copying between
 * user-space and kernel-space.  At the current time, it is assumed that all
 * transactions will take place in the same memory-space.
 */

struct uio
{
    struct iovec *uio_iov;     /* ptr to one or more I/O vectors */
    int           uio_iovcnt;  /* # of remaining I/O vectors */
    off_t         uio_offset;  /* offset within src/dest buffer */
    int           uio_resid;   /* # of bytes left to copy */
    enum uio_rw   uio_rw;      /* direction of copy: UIO_READ or UIO_WRITE */
};

/***************************************************************************
 * uiomove - move data between buffer and I/O vectors
 *
 * This routine moves data between a buffer and one or more I/O vectors.  This
 * is particularly useful when dealing with scatter-gather I/O.  Data is written
 * from the I/O vector(s) to the buffer.  Data is read from the buffer to the
 * I/O vector(s).
 *
 * RETURNS: OK, or EINVAL
 */

int uiomove(
    void *buffer,           /* source or destination buffer */
    int nBytes,             /* number of bytes to move */
    struct uio *pUio        /* ptr to uio structure */
    );

#endif  /* !_ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif  /* _SYS_UIO_H */

