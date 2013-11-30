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

#define B_BUSY          0x0001  /* buffer is locked by some process */
#define B_DONE          0x0002  /* Buffer I/O done */
#define B_INVAL         0x0004  /* buffer contents invalid */
#define B_READ          0x0008  /* buffer read operation if set, else write */
#define B_ASYNC         0x0010  /* Start I/O without wait */
#define B_ERROR         0x0020  /* I/O error */
#define B_CACHE         0x0040  /* Buffer in cache */
#define B_WANTED        0x0080  /* Buffer wanted */

/* Values for cache priority */

#define BUF_ORDINARY    0

/* typedefs */

typedef struct buf {
    LIST_NODE        b_node;       /* linked list node */
    lblkno_t         b_lblkno;     /* block number associated with buffer */
    int              b_flags;      /* buffer flags */
    int              b_count;      /* # of bytes to transfer */
    int              b_size;       /* size of the data area in buffer */
    unsigned         b_dev;        /* Device */
    struct vnode *   b_vp;         /* back ptr to vnode */
    bio_t *          b_bio;        /* ptr to block io */
    unsigned char *  b_data;       /* data area (matches [b_bio->bio_data]) */
    SEMAPHORE        b_sem;        /* buffer synchronization */
    int              b_error;      /* Error code set by buf_done */
    int              b_resid;      /* Bytes left (except 0) */
    int              b_priority;   /* cache priority */
} buf_t;

/* functions */

/***************************************************************************
 *
 * bread - read data from backing media into a buffer
 *
 * Get a buffer with the specified data.  Look in the cache first.
 *
 * RETURNS: OK on success, non-zero otherwise
 */

int bread (
    struct vnode * vp,
    lblkno_t       blkno,
    int            size,
    struct ucred * cred,
    struct buf **  bpp
    );

/***************************************************************************
 *
 * bwrite - write the buf's contents to backing media
 *
 * This routine writes the buf's contents to the backing media.
 * Note that the buf must be locked before calling this routine.
 *
 * RETURNS: OK on success, non-zero otherwise
 */

int bwrite (
    struct buf * bp
    );

/***************************************************************************
 *
 * buf_startwrite - start writing buffer to disk
 *
 * Asynchronous write.
 * Start output on a buffer, but do not wait for it to complete.
 * The buffer is released when the output completes.
 *
 * RETURNS: N/A
 */

void buf_startwrite (
    struct buf * bp
    );

/***************************************************************************
 *
 * brelse - release the buffer
 *
 * This routine releases the buffer.  It is removed from its current position
 * on the list, and then added to the head of the list as it has become the
 * most recently used buffer.
 *
 * RETURNS: N/A
 */

void brelse (
    struct buf * bp
    );

/***************************************************************************
 *
 * buf_new - get usable a buffer
 *
 * Find a buffer header which is available for use.
 *
 * RETURNS: Pointer to buffer on success, otherwise NULL
 */

struct buf* buf_new (
    struct vnode * vp,
    lblkno_t       blkno
    );

/***************************************************************************
 *
 * buf_incore - search for a valid buffer
 *
 * Check to see if a block is currently memory resident.
 *
 * RETURNS: Pointer to buffer on success, otherwise NULL
 */

struct buf* buf_incore (
    struct vnode * vp,
    lblkno_t       blkno
    );

/***************************************************************************
 *
 * buf_getblk - get a buffer
 *
 * This routine attempts to get a buffer of size <nBytes> for a vnode/blkNum
 * pairing.  First, it checks for an existing match.  If not found, it will
 * then try to allocate a new pairing.  In either case, it is supposed to
 * return a locked buffer.
 *
 * RETURNS: Pointer to buffer on success, otherwise NULL
 */

struct buf* buf_getblk (
    struct vnode * vp,
    lblkno_t       blkno,
    unsigned       size
    );

/***************************************************************************
 *
 * buf_wait - wait for buffer I/O to complete
 *
 * Wait for buffer I/O completion, returning error status.
 *
 * RETURNS: OK on success, error otherwise
 */

int buf_wait (
    struct buf * bp
    );

/***************************************************************************
 *
 * buf_done - called when buffer modification has completed
 *
 * Finish I/O on a buffer, calling an optional function.
 * This is usually called from interrupt level, so process blocking
 * is not *a good idea*.
 *
 * RETURNS: N/A
 */

void buf_done (
    struct buf * bp,
    int          error
    );

/***************************************************************************
 *
 * buf_swapdata - swap data between buffers
 *
 * RETURNS: N/A
 */

void buf_swapdata (
    struct buf * b1,
    struct buf * b2
    );

/***************************************************************************
 *
 * buf_setpriority - set buffer cache priority
 *
 * RETURNS: N/A
 */

void buf_setpriority (
    struct buf * bp,
    int          pri
    );

#endif /* _buf_h */

