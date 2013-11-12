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

/* buf.h - I/O buffers header */

#ifndef _buf_h
#define _buf_h

#include <vmx.h>
#include <vmx/semLib.h>
#include <fs/vnode.h>
#include <fs/bio.h>
#include <util/listLib.h>

/* defines */

/* The following macros are the buffer flags */

#define B_BUSY    0x0001  /* buffer is locked by some process */
#define B_DONE    0x0002  /* Buffer I/O done */
#define B_INVAL   0x0004  /* buffer contents invalid */
#define B_READ    0x0008  /* buffer read operation if set, else write */
#define B_ASYNC   0x0010  /* Start I/O without wait */
#define B_ERROR   0x0020  /* I/O error */
#define B_CACHE   0x0040  /* Buffer in cache */
#define B_WANTED  0x0080  /* Buffer wanted */

/* typedefs */

typedef struct buf {
    LIST_NODE        b_node;   /* linked list node */
    lblkno_t         b_blkno;  /* block number associated with buffer */
    int              b_flags;  /* buffer flags */
    int              b_count;  /* # of bytes to transfer */
    int              b_size;   /* size of the data area in buffer */
    unsigned         b_dev;    /* Device */
    struct vnode *   b_vp;     /* back ptr to vnode */
    bio_t *          b_bio;    /* ptr to block io */
    unsigned char *  b_data;   /* data area (matches [b_bio->bio_data]) */
    SEMAPHORE        b_sem;    /* buffer synchronization */
    int              b_error;  /* Error code set by buf_done */
    int              b_resid;  /* Bytes left (except 0) */
} buf_t;

#endif /* _buf_h */

