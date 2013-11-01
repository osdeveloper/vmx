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

/* stdio.h - Standard console input/output */

#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <vmx.h>
#include <sys/types.h>
#include <vmx/classLib.h>
#include <vmx/objLib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stdio buffer */
struct __sbuf
{
    unsigned char *_base;       /* Base address of buffer */
    int            _size;       /* Size of buffer */
};

/* File struct */
typedef struct __sFILE
{
    OBJ_CORE       objCore;     /* Class identifier */
    unsigned char *_p;          /* Position in buffer */
    int            _r;          /* Read space left */
    int            _w;          /* Write space left */
    short          _flags;      /* Flags */
    short          _file;       /* Fileno */
    struct __sbuf  _bf;         /* Stdio buffer */
    int            _lbfsize;    /* Size of buffer or 0 */
    struct __sbuf  _ub;         /* Ungetc buffer */
    unsigned char *_up;         /* old _p in ungetc */
    int            _ur;         /* old _r in ungetc */
    unsigned char  _ubuf[3];    /* Safety ungetc buffer */
    unsigned char  _nbuf[1];    /* Safety buffer */
    struct __sbuf  _lb;         /* Fgetline buffer */
    int            _blksize;    /* Stat blksize */
    int            _offset;     /* Current lseek offset */
    int            taskId;      /* Owner task */
} FILE;

extern CLASS_ID         fpClassId;

#define __SLBF          0x0001          /* Line buffered */
#define __SNBF          0x0002          /* Unbuffered */
#define __SRD           0x0004          /* Read */
#define __SWR           0x0008          /* Write */
#define __SWRNBF        (__SWR|__SNBF)  /* Write unbuffered */
#define __SRW           0x0010          /* Read and write */
#define __SEOF          0x0020          /* EOF */
#define __SERR          0x0040          /* ERROR */
#define __SMBF          0x0080          /* Buffer malloced */
#define __SAPP          0x0100          /* Append mode */
#define __SSTR          0x0200          /* Used for sprintf */
#define __SOPT          0x0400          /* Fseek optimization */
#define __SNPT          0x0800          /* No fseek optimization */
#define __SOFF          0x1000          /* See if _offset is correct */
#define __SMOD          0x2000          /* fgetline modified text */

/* Possible values for third argument to setvbuf */
#define _IOFBF               0          /* Full buffering */
#define _IOLBF               1          /* Line buffering */
#define _IONBF               2          /* No buffering */

/*
 * FOPEN_MAX is a minimum maximum, and is the number of streams that
 * stdio can provide without attempting to allocate further resources
 * (which could fail).  Do not use this for anything.
 */
                                /* must be == _POSIX_STREAM_MAX <limits.h> */
#define FOPEN_MAX       20      /* must be <= OPEN_MAX <sys/syslimits.h> */
#define FILENAME_MAX    1024    /* must be <= PATH_MAX <sys/syslimits.h> */
#define BUFSIZ          1024

#include <private/stdioP.h>

/* Macros */
#define stdin   (*__stdin())
#define stdout  (*__stdout())
#define stderr  (*__stderr())

#define __sgetc(p)      ((OBJ_VERIFY(p, fpClassId) != OK) ? (EOF) :            \
                         ((--(p)->_r < 0) ? (__srget(p)) : ((int)(*(p)->_p++))))

#define __sputc(c, p)   ((OBJ_VERIFY(p, fpClassId) != OK) ? (EOF) :            \
                         (--(p)->_w < 0 ?                                      \
                             (p)->_w >= (p)->_lbfsize ?                        \
                                 (*(p)->_p = (c)), *(p)->_p != '\n' ?          \
                                     (int)*(p)->_p++ :                         \
                                     __swbuf('\n', p) :                        \
                                 __swbuf((int)(c), p) :                        \
                             (*(p)->_p = (c), (int)*(p)->_p++)))

#define __sfeof(p)      ((OBJ_VERIFY(p, fpClassId) != OK) ?                    \
                         (FALSE) : (((p)->_flags & __SEOF) != 0))

#define __sferror(p)    ((OBJ_VERIFY(p, fpClassId) != OK) ?                    \
                         (FALSE) : (((p)->_flags & __SERR) != 0))

/* Functions */

/******************************************************************************
 * puts - Print string on screen
 *
 * RETURNS: N/A
 */

void puts(
    const char *str
    );

extern int ferror(FILE *fp);
extern int feof(FILE *fp);

extern int fflush(FILE *fp);

extern FILE* fopen(const char *file, const char *mode);
extern FILE* fdopen(int fd, const char *mode);

extern int fclose(FILE *fp);

extern int setvbuf(FILE *fp, char *buf, int mode, size_t size);

extern size_t fread(void *buf, size_t size, size_t count, FILE *fp);
extern size_t fwrite(const void *buf, size_t size, size_t count, FILE *fp);

extern int getc(FILE *fp);
extern int fgetc(FILE *fp);
extern int getchar(void);

extern int putc(int c, FILE *fp);
extern int fputc(int c, FILE *fp);
extern int putchar(int c);

extern int ungetc(int c, FILE *fp);

extern char* gets(char *buf);
extern char* fgets(char *buf, size_t num, FILE *fp);

extern int printf(const char *fmt, ...);
extern int asprintf(char **app, const char *fmt, ...);
extern int sprintf(char *sp, const char *fmt, ...);
extern int snprintf(char *sp, size_t size, const char *fmt, ...);
extern int fprintf(FILE * fp, const char *fmt, ...);
extern int fnprintf(FILE * fp, size_t size, const char *fmt, ...);
extern int vprintf(const char *fmt, va_list ap);
extern int vfprintf(FILE * op, const char *fmt, va_list ap);
extern int vsprintf(char *sp, const char *fmt, va_list ap);
extern int vsnprintf(char *sp, size_t size, const char *fmt, va_list ap);
extern int fdprintf(int fd, const char *fmt, ...);
extern int vdprintf(int fd, const char *fmt, va_list ap);
extern int vfnprintf(FILE * op, size_t max_size, const char *fmt, va_list ap);

extern int scanf(const char *fmt, ...);
extern int sscanf(const char *s, const char *fmt, ...);
extern int fscanf(FILE *fp, const char *fmt, ...);
extern int vfscanf(FILE *fp, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STDIO_H */

