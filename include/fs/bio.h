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

