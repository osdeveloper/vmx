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

/* vnode.c - Virtual filesystem inode */

#include <stdlib.h>
#include <vmx.h>
#include <util/listLib.h>
#include <fs/buf.h>
#include <fs/mount.h>
#include <fs/vnode.h>

/***************************************************************************
 *
 * vnodesCreate - create vnodes
 *
 * RETURNS: ptr to array of vnodes (syncer vnode is the 1st), NULL on error
 */

struct vnode *  vnodesCreate (
    struct mount *  pMount,    /* ptr to mount */
    int             isize,     /* size of the inode structure */
    int             nvnode     /* # of file/logical vnodes to create (> 0) */
    ) {
    char *          ptr;
    struct vnode *  pVnode;
    int             i;
    int             j;

    j = sizeof (vnode_t) + ((sizeof (vnode_t) + isize) * nvnode);
    ptr = (char *) malloc (j);
    if (ptr == NULL) {
        return (NULL);
    }

    memset (ptr, 0, j);
    pVnode = (struct vnode *) ptr;
    ptr += (sizeof (vnode_t) * (nvnode + 1));

    i = 0;                  /* Do not give the syncer vnode (first in array) */
    goto skip;              /* a private data section.  */
                              
    while (i <= nvnode) {
        pVnode[i].v_data = ptr;
        ptr += isize;
skip:
        pVnode[i].v_mount = pMount;

        listInit (&pVnode[i].v_buflist);
        i++;
    }

    return (pVnode);
}

/***************************************************************************
 *
 * vnodesDelete - delete vnodes
 *
 * RETURNS: N/A
 */

void vnodesDelete (
    struct vnode *  pVnode,
    int             nvnode
    ) {
    free (pVnode);
}

/***************************************************************************
 *
 * vnodeLock - establish a lock by incrementing the use count
 *
 * RETURNS: OK
 */

STATUS vnodeLock (
    vnode_t *  pVnode
    ) {
    /* No semaphore is needed as the mount is single-threaded. */
    pVnode->v_count++;    /* overflow is unlikely */

    return (OK);
}

/***************************************************************************
 *
 * vnodeUnlock - release the lock by decrementing the use count
 *
 * RETURNS: OK on success, non-zero if VOP_INACTIVE failed
 */

STATUS vnodeUnlock (
    vnode_t *  pVnode
    ) {
    int              error = OK;

    /*
     * Note that access to the mount has already been single threaded.
     * Thus, there is no current risk of concurrent access, and no
     * semaphore is required for protection.
     */

    if (--pVnode->v_count == 0) {
        error = VOP_INACTIVE (pVnode);

        /* Not concerned with <error> result yet. */
    }

    return (error);
}

/***************************************************************************
 *
 * vgetino - get the vnode for the given inode
 *
 * RETURNS: OK on success, error otherwise
 */

int vgetino (
    struct mount * pMount,     /* ptr to file system mount */
    ino_t          inode,      /* inode for which to get vnode */
    vnode_ops_t *  pVnodeOps,  /* ptr to vnode operators */
    vnode_t **     ppVnode     /* double ptr to vnode : output */
    ) {
    int  i;
    int  error;
    LIST_NODE *  pNode;
    vnode_t *    pVnode;

    mountLock (pMount);   /* Lock the mount structures' access */

    /* 1. Search the list of vnodes in use. */
    pNode = LIST_HEAD (&pMount->mnt_vused);
    while (pNode != NULL) {
        pVnode = (vnode_t *) pNode;     /* v_node is 1st field in vnode_t */
        if (pVnode->v_inode == inode) { /* Do inode #s match? */
            vnodeLock (pVnode);         /* Yes. Increment its use count */
            mountUnlock (pMount);       /* Unlock mount structures' access */
            *ppVnode = pVnode;
            return (OK);                /* Done. */
        }
        pNode = LIST_NEXT (pNode);      /* Advance to next node */
    }

    /* 2. If not currently in use, then look in LRU list. */
    pNode = LIST_TAIL (&pMount->mnt_vlist);    /* MRU @ tail.  LRU @ head */
    while (pNode != NULL) {
        pVnode = (vnode_t *) pNode;
        if (pVnode->v_inode == inode) {  /* Do inode #s match? */
            listRemove (&pMount->mnt_vlist, &pVnode->v_node);
            listAdd (&pMount->mnt_vused, &pVnode->v_node);
            vnodeLock (pVnode);          /* Yes.  Increment its use count */
            mountUnlock (pMount);        /* Unlock mount structures' access */
            *ppVnode = pVnode;
            return (OK);                 /* Done. */
        }
        pNode = LIST_PREV (pNode);       /* Rewind to previous node */
    }

    /* 3. If not in LRU list, recycle a vnode from the LRU list. */
    pVnode = (vnode_t *) LIST_HEAD (&pMount->mnt_vlist);
    if (pVnode == NULL) {    /* If LRU list is empty, all vnodes in use. */
        return (S_vfsLib_NOT_ENOUGH_VNODES);
    }

    pVnode->v_inode = inode;
    pVnode->v_ops = pVnodeOps;

    /*
     * Note that the mount structures' access is locked all the way through
     * VOP_ACTIVATE().  This is not ideal.  Improving this locking scheme is
     * something that can be done at a later time.
     *
     * VOP_ACTIVATE() is responsible for setting [v_type] in <pVnode>.
     */

    error = VOP_ACTIVATE (pVnode);    /* Activate the vnode */
    if (error != OK) {
        pVnode->v_inode = 0;          /* No FS inode # is ever 0. */
        mountUnlock (pMount);
        return (error);
    }

    /* Remove vnode from LRU and add to used list */
    listRemove (&pMount->mnt_vlist, &pVnode->v_node);
    listAdd (&pMount->mnt_vused, &pVnode->v_node);
    mountUnlock (pMount);

    *ppVnode = pVnode;
    return (OK);

}

/***************************************************************************
 *
 * vflushbuf - flush all dirty buffers to disk
 *
 * RETURNS: OK on success, error otherwise
 */
int vflushbuf (
    vnode_t  * vp,
    BOOL       sync
    ) {
    buf_t  * bp;
    int error;
    
    /* Write all buffers to disk */
    for (bp = (buf_t *) LIST_HEAD (&vp->v_buflist);
         bp != NULL;
         bp = (buf_t *) LIST_NEXT (&bp->b_node)) {
        buf_startwrite (bp);
        error = buf_wait (bp);
        if (error != OK) {
            return (error);
        }
        brelse (bp);
    }

    return (OK);
}

