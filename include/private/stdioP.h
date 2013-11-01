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

/* stdioP.h - Standard console input/output private header */

#ifndef _stdioP_h
#define _stdioP_h

#include <vmx.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct __siov
{
    void          *iov_base;
    size_t         iov_len;
};

struct __suio
{
    struct __siov *uio_iov;
    int            uio_iovcnt;
    int            uio_resid;
};

/* Macros */

#define CANTWRITE(fp)                                                         \
  ((((fp)->_flags & __SWR) == 0) || ((fp->_bf._base == NULL) && __swsetup(fp)))

#define HASUB(fp)       ( (fp)->_ub._base != NULL )
#define FREEUB(fp)                                                             \
{                                                                              \
  if ( (fp)->_ub._base != (fp)->_ubuf)                                         \
    free((fp)->_ub._base);                                                     \
  (fp)->_ub._base = NULL;                                                      \
}

#define HASLB(fp)       ( (fp)->_lb._base != NULL)
#define FREELB(fp)                                                             \
{                                                                              \
  free((fp)->_lb._base);                                                       \
  (fp)->_lb._base = NULL;                                                      \
}

/* Functions */

/******************************************************************************
 * stdioLibInit - Initialize stdio library
 *
 * RETURNS: OK or ERROR
 */

STATUS stdioLibInit(
    void
    );

/******************************************************************************
 * stdioFpCreate - Allocate file structure
 *
 * RETURNS: Pointer to FILE structure or NULL
 */

FILE* stdioFpCreate(
    void
    );

/******************************************************************************
 * stdioFpInit - Initialize file structure
 *
 * RETURNS: OK or ERROR
 */

STATUS stdioFpInit(
    FILE *fp
    );

/******************************************************************************
 * stdioFpDestroy - Free file structure
 *
 * RETURNS: OK or ERROR
 */

STATUS stdioFpDestroy(
    FILE *fp
    );

/******************************************************************************
 * stdioFp - Get file
 *
 * RETURNS: FILE pointer
 */

FILE* stdioFp(
    int stdFd
    );

/******************************************************************************
 * __stdin - Get stdin file
 *
 * RETURNS: stdin FILE pointer
 */

FILE** __stdin(
    void
    );

/******************************************************************************
 * __stdout - Get stdout file
 *
 * RETURNS: stdout FILE pointer
 */

FILE** __stdout(
    void
    );

/******************************************************************************
 * __stderr - Get stderr file
 *
 * RETURNS: stderr FILE pointer
 */

FILE** __stderr(
    void
    );

/******************************************************************************
 * __smakebuf - Allocate stdio buffer or turn buffering off
 *
 * RETURNS: N/A
 */

void __smakebuf(
    FILE *fp
    );

/******************************************************************************
 * __swsetup - Check or fix for write access
 *
 * RETURNS: N/A
 */

int __swsetup(
    FILE *fp
    );

/******************************************************************************
 * __sflags - Internal flags get for a given mode
 *
 * RETURNS: 0 or flags
 */

int __sflags(
    const char *mode,
    int *optr
    );

/******************************************************************************
 * __sread - Internal read for stdio
 *
 * RETURNS: Bytes read
 */

int __sread(
    FILE *fp,
    char *buf,
    int n
    );

/******************************************************************************
 * __swrite - Internal write for stdio
 *
 * RETURNS: Bytes written
 */

int __swrite(
    FILE *fp,
    char *buf,
    int n
    );

/******************************************************************************
 * __sflush - Flush stream fp
 *
 * RETURNS: 0 or EOF
 */

int __sflush(
    FILE *fp
    );

/******************************************************************************
 * __sseek - Internal seek in file
 *
 * RETURNS: Seek position or EOF
 */

fpos_t __sseek(
    FILE *fp,
    fpos_t offset,
    int whence
    );

/******************************************************************************
 * __sclose - Internal close
 *
 * RETURNS: 0 or EOF
 */

int __sclose(
    FILE *fp
    );

/******************************************************************************
 * __swbuf - Write to stdio buffer
 *
 * RETURNS: Last char written
 */

int __swbuf(
    int c,
    FILE *fp
    );

/******************************************************************************
 * __srefill - Refill stdio buffer
 *
 * RETURNS: 0 or EOF
 */

int __srefill(
    FILE *fp
    );

/******************************************************************************
 * __srget - Get data to stdio buffer
 *
 * RETURNS: 0 or EOF
 */

int __srget(
    FILE *fp
    );

/******************************************************************************
 * __sfvwrite - Internal write function
 *
 * RETURNS: 0 or EOF
 */

int __sfvwrite(
    FILE *fp,
    struct __suio *uio
    );

/******************************************************************************
 * __submore - Expand ungetc buffer
 *
 * RETURNS: 0 or EOF
 */

int __submore(
    FILE *fp
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _stdioP_h */

