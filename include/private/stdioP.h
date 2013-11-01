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
extern STATUS stdioLibInit(void);
extern FILE* stdioFpCreate(void);
extern STATUS stdioFpInit(FILE *fp);
extern STATUS stdioFpDestroy(FILE *fp);
extern FILE* stdioFp(int stdFd);

extern FILE** __stdin(void);
extern FILE** __stdout(void);
extern FILE** __stderr(void);
extern void __smakebuf(FILE *fp);
extern int __swsetup(FILE *fp);
extern int __sflags(const char *mode, int *optr);
extern int __sread(FILE *fp, char *buf, int n);
extern int __swrite(FILE *fp, char *buf, int n);
extern int __sflush(FILE *fp);
extern fpos_t __sseek(FILE *fp, fpos_t offset, int whence);
extern int __sclose(FILE *fp);
extern int __swbuf(int c, FILE *fp);
extern int __srefill(FILE *fp);
extern int __srget(FILE *fp);
extern int __sfvwrite(FILE *fp, struct __suio *uio);
extern int __submore(FILE *fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _stdioP_h */

