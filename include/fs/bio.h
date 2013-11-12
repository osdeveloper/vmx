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

/* bio.h - Buffer IO header */

#ifndef __BIO_H
#define __BIO_H

/* includes */

#include <vmx.h>
#include <vmx/semLib.h>
#include <fs/xbd.h>
#include <fs/vnode.h>

/* defines */

#define NBUF       10

#define BIO_READ   0x0001
#define BIO_WRITE  0x0002

/* structs */
struct bio;

/* typedefs */

/*
 * Notes: typically the [bio_done] function pointer will refer to a routine
 * that gives a semaphore ([bio_caller1]).  
 *
 * [bio_error] is the error code (if any) set by xbdStrategy().
 *
 * [bio_resid], indicates an IO error if set to non-zero by xbdStrategy().
 */

typedef struct bio {
    unsigned      bio_dev;                   /* device upon which FS runs */
    char *        bio_data;                  /* ptr to data area */
    unsigned      bio_flags;                 /* BIO_READ or BIO_WRITE */
    struct bio *  bio_chain;                 /* next bio in chain (not used) */
    int           bio_bcount;                /* # of bytes to read */
    lblkno_t      bio_blkno;                 /* logical block number */
    SEMAPHORE     bio_sem;                   /* I/O semaphore */

    void         (*bio_done) (struct bio *bio); /* called when done */
    void *        bio_caller1;               /* store argument for bio_done */

    int           bio_error;                 /* error code (0 = OK) */
    int           bio_resid;                 /* bytes left (expect 0) */
} bio_t;

/* structs */

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
    struct vnode *vp,
    lblkno_t  blkno,
    int size,
    struct ucred *cred,
    struct buf **bpp
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
    struct buf *bp
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
    struct buf *bp
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
    struct buf *bp
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
    struct vnode *vp,
    lblkno_t blkno
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
    struct vnode *vp,
    lblkno_t blkno
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
    struct vnode *vp,
    lblkno_t blkno,
    unsigned size
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
    struct buf *bp
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
    struct buf *bp,
    int error
    );

/***************************************************************************
 *
 * buf_swapdata - swap data between buffers
 *
 * RETURNS: N/A
 */

void buf_swapdata (
    struct buf *  b1,
    struct buf *  b2
    );

/***************************************************************************
 *
 * bio_alloc - allocate memory for block I/O
 *
 * RETURNS: Pointer to memory or NULL
 */

void  * bio_alloc (
    device_t  device,
    int       numBlocks
    );

/***************************************************************************
 *
 * bio_free - free memory allocated for block I/O
 *
 * RETURNS: N/A
 */

void bio_free (
    void  * pBioData
    );

/***************************************************************************
 *
 * bio_wait - wait for I/O to complete
 *
 * RETURNS: OK on success, otherwise error
 */

int bio_wait (
    struct bio * bio
    );

/***************************************************************************
 *
 * bio_done - called when block I/O has completed
 *
 * RETURNS: N/A
 */

void bio_done (
    struct bio  * bio,
    int error
    );

/***************************************************************************
 *
 * bio_new - allocate new bio buffer
 *
 * RETURNS: Pointer to bio on success, otherwise NULL
 */

LOCAL void bio_new (
    struct buf *bp
    );

#endif

