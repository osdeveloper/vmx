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

/* bio.c -  Buffer cache. */

/*
  The buffer cache is a linked list of buf structures
  holding cached copies of disk block contents.
  Each buf has two state bits B_BUSY and B_VALID.
  If B_BUSY is set, it means that some code is currently
  editing buf, so other code is not allowed to look at it.
  To wait for a buffer that is B_BUSY, sleep on buf.
  (See buf_getblk() below.)
 
  If B_VALID is set, it means that the memory contents
  have been initialized by reading them off the disk.
  (Conversely, if B_VALID is not set, the memory contents
  of buf must be initialized, often by calling bread,
  before being used.)
  
  After making changes to a buf's memory, call bwrite to flush
  the changes out to disk, to keep the disk and memory copies
  in sync.
 
  When finished with a buffer, call brelse to release the buffer
  (i.e., clear B_BUSY), so that others can access it.

  Bufs that are not B_BUSY are fair game for reuse for other
  disk blocks.  It is not allowed to use a buf after calling brelse.

  For the present time, the number bytes to be read from/written to
  a buffer will match the buffer size.  This could conceivably change
  in the future.

  Furthermore, one or both of these fields may be necessary should
  a file system require different buffer sizes.  (For example, the
  DOS FAT file system could use this as the FAT is accessed by
  sector, but the file and directory data are accessed by cluster.)

  Mount buffers:
  pBufHead->head is most recently used.
  pBufHead->tail is least recently used.
*/

/*
 * Copyright (c) 1994 John S. Dyson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Absolutely no warranty of function or purpose is made by the author
 *    John S. Dyson.
 * 4. Modifications may be freely made to this file if the above conditions
 *    are met.
 */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/errnoLib.h>
#include <os/logLib.h>
#include <fs/vnode.h>
#include <fs/xbd.h>
#include <fs/buf.h>
#include <fs/bio.h>

/* forward declarations */
LOCAL void bio_new (struct buf *bp);
    
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
    ) {
    struct buf *bp;
    strategy_args_t args;

    bp = buf_getblk (vp, blkno, size);
    *bpp = bp;

    /* if not found in cache, do some I/O */
    if ((bp->b_flags & B_CACHE) == 0) {
        bp->b_flags |= B_READ;
        bp->b_flags &= ~(B_DONE | B_ERROR | B_INVAL);

        args.vp = vp;
        args.bp = bp;
        VOP_STRATEGY (vp, args);
        
        return (buf_wait (bp));
    }
    
    return (OK);
}

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
    ) {
    strategy_args_t args;
    int rtval;
    int oldflags = bp->b_flags;

    if(bp->b_flags & B_INVAL) {
        brelse (bp);
        return (OK);
    }

    if((bp->b_flags & B_BUSY) == 0) {
#ifdef DIAGNOSTIC
        logMsg ("bwrite: buffer is not busy",
                0, 0, 0, 0, 0, 0);
#endif
        return;
    }

    bp->b_flags &= ~(B_READ | B_DONE | B_ERROR);

    args.vp = bp->b_vp;
    args.bp = bp;
    VOP_STRATEGY (bp->b_vp, args);

    if ((oldflags & B_ASYNC) == 0) {
        rtval = buf_wait (bp);
        brelse (bp);
        return (rtval);
    } 

    return(OK);
}

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
    ) {
    bp->b_flags |= B_ASYNC;
    bwrite (bp);
}

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
    ) {
    LIST *pBufHead = &bp->b_vp->v_mount->mnt_buflist;

    if ((bp->b_flags & B_BUSY) == 0) {
#ifdef DIAGNOSTIC
        logMsg ("brelse: buffer is not busy",
                0, 0, 0, 0, 0, 0);
#endif
        return;
    }
    
    /* anyone need this very block? */
    if (bp->b_flags & B_WANTED) {
        bp->b_flags &= ~B_WANTED;
        semGive (&bp->b_sem);
    }

    /* Put buffer at head */
    listRemove (pBufHead, &bp->b_node);        /* Remove from list */
    listInsert (pBufHead, NULL, &bp->b_node);  /* Insert at list head */

    /* unlock */
    bp->b_flags &= ~(B_WANTED | B_BUSY | B_ASYNC);
}

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
    ) {
    struct buf *bp;
    LIST *pBufHead = &vp->v_mount->mnt_buflist;

    for (bp = (struct buf *) LIST_TAIL (pBufHead);
         bp != NULL;
         bp = (struct buf *) LIST_PREV (&bp->b_node)) {
        if ((bp->b_flags & B_BUSY) == 0) {
            bp->b_flags = B_BUSY;
            bp->b_blkno = 0;
            bp->b_count = 0;
            bp->b_dev   = -1;
            bp->b_vp    = NULL;
            bp->b_error = OK;
            bp->b_resid = 0;

            return (bp);
        }
    }

    return (NULL);
}

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
    ) {
    struct buf *bp;
    LIST *pBufHead = &vp->v_mount->mnt_buflist;

    /* Search buffer list */
    for (bp = (struct buf *) LIST_HEAD (pBufHead);
         bp != NULL;
         bp = (struct buf *) LIST_NEXT (&bp->b_node)) {
        /* hit */
        if ((bp->b_blkno == blkno) && (bp->b_vp == vp) &&
            ((bp->b_flags & B_INVAL) == 0)) {
            return (bp);
        }
    }

    return(NULL);
}

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
    ) {
    struct buf *bp;
    LIST *pBufHead = &vp->v_mount->mnt_buflist;

loop:
    if ((bp = buf_incore (vp, blkno)) != NULL) {
        if (bp->b_flags & B_BUSY) {
            bp->b_flags |= B_WANTED;
            semTake (&bp->b_sem, WAIT_FOREVER);
        }
        bp->b_flags |= (B_BUSY | B_CACHE);
        /*
         * check for size inconsistancies
         */
        if (bp->b_size != size) {
            logMsg ("getblk: invalid buffer size: %d\n",
                    (ARG) bp->b_size,
                    0, 0, 0, 0, 0);
            bp->b_flags |= B_INVAL;
            bwrite (bp);
            goto loop;
        }
    } else {
        if ((bp = buf_new (vp, blkno)) == NULL) {
            logMsg ("buf_getblk: no buffers",
                    0, 0, 0, 0, 0, 0);
            goto loop;
        }
        bp->b_dev = vp->v_mount->mnt_dev;
        bp->b_blkno = blkno;
        bp->b_vp = vp;
        bio_new (bp);
        
        /* Put buffer at head */
        listRemove (pBufHead, &bp->b_node);
        listInsert (pBufHead, NULL, &bp->b_node);
    }

    return (bp);
}

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
    ) {
    LIST *pBufHead = &bp->b_vp->v_mount->mnt_buflist;

    /* Make sure I/O is complete */
    if ((bp->b_flags & B_DONE) == 0) {
        if (bio_wait (bp->b_bio)) {
            bp->b_flags |= B_ERROR;
        }
        buf_done (bp, OK);
    }

    if ((bp->b_flags & B_ERROR) || bp->b_error) {
        if ((bp->b_flags & B_INVAL) == 0) {
            bp->b_flags |= B_INVAL;
            listRemove (pBufHead, &bp->b_node);        /* Remove from list */
            listInsert (pBufHead, NULL, &bp->b_node);  /* Insert at list head */
        }
        if (!bp->b_error) {
            bp->b_error = EIO;
        } else {
            bp->b_flags |= B_ERROR;
        }
        return (bp->b_error);
    } else {
        return (OK);
    }
}

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
    ) {
    bp->b_error = error;
    bp->b_flags |= B_DONE;

    /*
     * For asynchronous completions, release the buffer now. The brelse
     * checks for B_WANTED and will do the wakeup there if necessary -
     * so no need to do a wakeup here in the async case.
     */
    if (bp->b_flags & B_ASYNC) {
        brelse (bp);
    } else {
        bp->b_flags &= ~B_WANTED;
        semGive (&bp->b_sem);
    }
}

/***************************************************************************
 *
 * buf_swapdata - swap data between buffers
 *
 * RETURNS: N/A
 */

void buf_swapdata (
    struct buf *  b1,
    struct buf *  b2
    ) {
    int b_size;
    unsigned char  * b_data;

    /* Swap data buffers */
    b_data = b2->b_data;
    b2->b_data = b1->b_data;
    b1->b_data = b_data;

    /* Swap sizes */
    b_size = b2->b_size;
    b2->b_size = b1->b_size;
    b1->b_size = b_size;
}

/***************************************************************************
 *
 * bio_alloc - allocate memory for block I/O
 *
 * RETURNS: Pointer to memory or NULL
 */

void  * bio_alloc (
    device_t  device,
    int       numBlocks
    ) {
    unsigned blkSize;
    char  *  buf;
    int      error;

    /* Get blocksize from device */
    if ((error = xbdBlockSize (device, &blkSize)) != OK) {
        errnoSet (error);
        return (NULL);
    }

    /* Allocate buffer */
    buf = (char *) malloc (numBlocks * blkSize);
    if (buf == NULL) {
        return (NULL);
    }

    return (buf);
}

/***************************************************************************
 *
 * bio_free - free memory allocated for block I/O
 *
 * RETURNS: N/A
 */

void bio_free (
    void  * pBioData
    ) {
    free (pBioData);
}

/***************************************************************************
 *
 * bio_wait - wait for I/O to complete
 *
 * RETURNS: OK on success, otherwise error
 */

int bio_wait (
    struct bio * bio
    ) {
    semTake (&bio->bio_sem, WAIT_FOREVER);

    if (bio->bio_error) {
        return (bio->bio_error);
    }

    return (OK);
}

/***************************************************************************
 *
 * bio_done - called when block I/O has completed
 *
 * RETURNS: N/A
 */

void bio_done (
    struct bio  * bio,
    int error
    ) {
    bio->bio_error = error;

    semGive (&bio->bio_sem);

    /* call optional completion function if requested */
    if (bio->bio_done != NULL) {
        bio->bio_done (bio);
        bio->bio_done = NULL;
    }
}

/***************************************************************************
 *
 * bio_new - allocate new bio buffer
 *
 * RETURNS: Pointer to bio on success, otherwise NULL
 */

LOCAL void bio_new (
    struct buf *bp
    ) {
    struct bio *pBio;

    /* Allocate struct */
    pBio = (struct bio *) malloc (sizeof (struct bio));
    if (pBio == NULL) {
        panic ("bio: out of memory");
    }

    /* Clear struct */
    memset (pBio, 0, sizeof(struct bio));

    /* Fill the vital parts of the bio struct */
    pBio->bio_dev    = bp->b_dev;
    pBio->bio_data   = bp->b_data;
    pBio->bio_flags  = (bp->b_flags & B_READ) ? BIO_READ : BIO_WRITE;

    semBInit (&pBio->bio_sem, SEM_Q_PRIORITY, SEM_EMPTY);

    /* Fill in pointer to struct in buffer */
    bp->b_bio = pBio;
}

