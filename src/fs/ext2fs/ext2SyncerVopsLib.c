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

/* ext2SyncerVopsLib.c - Ext2fs syncer library */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <os/ffsLib.h>
#include <fs/vnode.h>
#include <fs/mount.h>
#include <fs/buf.h>
#include <fs/ext2fsLib.h>
#include <fs/xbd.h>
#include <fs/bio.h>

/* forward declarations */

LOCAL int ext2fsSyncerVopFsync (
    struct vnode * pSyncer,
    struct ucred * pCred,
    int            flags
    );

LOCAL int ext2fsSyncerVopActivate (
    struct vnode * pSyncer
    );

LOCAL int ext2fsSyncerVopInactive (
    struct vnode * pSyncer
    );

LOCAL void ext2fsSyncerVopStrategy (
    struct vnode * pSyncer,
    struct buf *   pBuf
    );

LOCAL void ext2fsSyncerVopPrint (
    struct vnode * pSyncer
    );

/* locals */

LOCAL struct vnode_ops ext2fsSyncerOps =
{
    vop_error_lookup,
    vop_error_create,
    vop_error_open,
    vop_error_close,
    vop_error_access,
    vop_error_getattr,
    vop_error_setattr,
    vop_error_read,
    vop_error_write,
    vop_error_ioctl,
    vop_error_fcntl,
    ext2fsSyncerVopFsync,
    vop_error_seek,
    vop_error_remove,
    vop_error_link,
    vop_error_rename,
    vop_error_mkdir,
    vop_error_rmdir,
    vop_error_symlink,
    vop_error_readdir,
    vop_error_readlink,
    vop_error_abortop,
    ext2fsSyncerVopActivate,
    ext2fsSyncerVopInactive,
    ext2fsSyncerVopStrategy,
    ext2fsSyncerVopPrint,
    vop_error_pathconf,
    vop_error_advlock,
    vop_error_truncate
};

LOCAL u_int32_t sparseSuperBlockList[46] =
{
    0U, 1U, 3U, 5U, 7U, 9U, 25U, 27U, 49U, 81U, 125U, 243U, 343U, 625U, 729U,
    2187U, 2401U, 3025U, 6561U, 15125U, 16807U, 19683U, 59049U, 75625U, 117649U,
    177147U, 390625U, 531441U, 823543U, 1594323U, 1953125U, 4782969U, 5764801U,
    9765625U, 14348907U, 40353607U, 43046721U, 48828125U, 129140163U,
    244140625U, 282475249U, 387420489U, 1162261467U, 1220703125U, 1977326743U,
    3486784401U
};

/******************************************************************************
 *
 * ext2fsSyncerVopFsync - flush unwritten data to syncer vnode
 *
 * RETURNS: OK on success, otherwise error
 */

LOCAL int ext2fsSyncerVopFsync (
    struct vnode * pSyncer,
    struct ucred * pCred,
    int            flags
    ) {
    int  ret;

    ret = vflushbuf (pSyncer, TRUE);

    return (ret);
}

/******************************************************************************
 *
 * ext2fsSyncerVopActivate - activate syncer vnode
 *
 * RETURNS: OK
 */

LOCAL int ext2fsSyncerVopActivate (
    struct vnode * pSyncer
    ) {

    return (OK);
}

/******************************************************************************
 *
 * ext2fsSyncerVopInactive - deactivate syncer vnode
 *
 * RETURNS: OK
 */

LOCAL int ext2fsSyncerVopInactive (
    struct vnode * pSyncer
    ) {

    return (OK);
}

/******************************************************************************
 *
 * ext2fsSyncerVopStrategy - strategy method for syncer vnode
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsSyncerVopStrategy (
    struct vnode * pSyncer,
    struct buf *   pBuf
    ) {
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_DEV *         pFsDev;
    struct bio *         pBio;

    pFsDev   = (EXT2FS_DEV *) pSyncer->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

#ifdef notyet
    if ((pBuf->b_flags & B_WRITE) != 0) {

        if (pVolDesc->diskModified != TRUE) {
            VN_UPLOCK (pSyncer);
        }

        pVolDesc->diskModified = TRUE;
    }
#endif

    pBio = pBuf->b_bio;
    pBio->bio_blkno  = (pBuf->b_lblkno << pVolDesc->secPerBlk2);
    pBio->bio_bcount = pVolDesc->blkSize;
    pBio->bio_error  = OK;
    xbdStrategy (pVolDesc->device, pBio);
}

/******************************************************************************
 *
 * ext2fsSyncerVopPrint - print the syncer vnode
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsSyncerVopPrint (
    struct vnode * pSyncer
    ) {

    printf ("\n");
}

/******************************************************************************
 *
 * ext2fsPhysFindCacheSearch - search the cached trie map for physical addrs
 *
 * RETURNS: ERROR -- mapping not possible
 *              0 -- obtained physical block number
 *              1 -- including single indirect block
 *              2 -- including double indirect block
 *              3 -- including triple indirect block
 */

LOCAL int ext2fsPhysFindCacheSearch (
    struct vnode *    pVnode,
    EXT2FS_TRIE_MAP * pTrieMap,
    u_int32_t         blkNum
    ) {
    EXT2FS_TRIE_MAP *    pCachedMap;
    EXT2FS_INODE *       pInode;
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_DEV *         pFsDev;
    u_int32_t            normalized;
    u_int32_t            mask;
    u_int32_t            shift;
    int                  level;
    int                  i;

    EXT2FS_VTOI (pInode, pVnode);
    pCachedMap = &(pInode->cachedTrieMap);

    /* obtained physical block number */
    if ((pCachedMap->index[0] == blkNum) && (pCachedMap->level >= 0)) {
        *pTrieMap = *pCachedMap;
        return (0);
    }

    pFsDev   = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    pTrieMap->index[0] = blkNum;
    normalized = blkNum;
    for (level = 0; level < 4; level++) {

        if (normalized < pVolDesc->nRefBlks[level]) {
            break;
        }

        normalized -= pVolDesc->nRefBlks[level];
    }

    /* block number cannot be accessed */
    if (level == 4) {
        return (ERROR);
    }

    pTrieMap->level = level;

    /* obtained physical block number */
    if (level == 0) {
        pTrieMap->physBlk[0] = pInode->host.i_block[blkNum];
        return (0);
    }

    shift = pVolDesc->blkSize2 - 2;
    mask  = (1 << shift) - 1;

    for (i = 1; i <= level; i++) {
        pTrieMap->index[i] = normalized & mask;
        normalized >>= shift;
    }

    /* get the highest indirect block number from the inode */
    pTrieMap->physBlk[level] =
        pInode->host.i_block[EXT2_N_DIRECT_BLKS + level - 1];

    while (level > 0) {

        if (pTrieMap->index[level] != pInode->cachedTrieMap.index[level]) {
            return (level);
        }

        level--;
        pTrieMap->physBlk[level] = pCachedMap->physBlk[level];
    }

    return (level);
}

/******************************************************************************
 *
 * ext2fsPhysFind - find the physical block for a logical block
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsPhysFind (
    struct vnode * pVnode,
    lblkno_t       blkNum
    ) {
    EXT2FS_TRIE_MAP      trieMap;
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_DEV *         pFsDev;
    EXT2FS_INODE *       pInode;
    struct vnode *       pSyncer;
    struct buf *         pBuf;
    u_int32_t *          pEntryList;
    int                  level;
    int                  error;

    EXT2FS_VTOI (pInode, pVnode);
    memset (&trieMap, 0, sizeof (EXT2FS_TRIE_MAP));

    level = ext2fsPhysFindCacheSearch (pVnode, &trieMap, blkNum);
    if (level == ERROR) {
        return (EACCES);
    }

    pSyncer  = pVnode->v_mount->mnt_syncer;
    pFsDev   = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    while (trieMap.physBlk[level] != EXT2FS_SPARSE_BLK_NUM) {

        if (level == 0) {
            break;
        }

        error = bread (pSyncer, trieMap.physBlk[level],
                       pVolDesc->blkSize, NULL, &pBuf);
        if(error != OK) {
            return (error);
        }

        pEntryList = (u_int32_t *) pBuf->b_data;
        trieMap.physBlk[level - 1] =
            ext2fsDiskToHost32 ((u_int8_t *)
                                &pEntryList[trieMap.index[level]]);
        level--;
        brelse (pBuf);
    }

    memcpy (pInode->cachedTrieMap.index, &trieMap.index,
            sizeof (trieMap.index));
    memcpy (pInode->cachedTrieMap.physBlk, &trieMap.physBlk,
            sizeof (trieMap.physBlk));
    pInode->cachedTrieMap.level = trieMap.level;

    return (OK);
}

/******************************************************************************
 *
 * ext2fsGroupDescriptorRead - read group descriptor from the disk
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsGroupDescriptorRead (
    EXT2FS_DEV *        pFsDev,
    struct vnode *      pSyncer,
    EXT2FS_GROUP_DESC * pGroupDesc,
    int                 gdNum
    ) {
    EXT2FS_VOLUME_DESC *     pVolDesc;
    EXT2FS_GROUP_DESC_DISK * pGroupDescDisk;
    lblkno_t                 blkNum;
    struct buf *             pBuf;
    int                      error;

    pVolDesc = &(pFsDev->ext2fsVolDesc);

    blkNum = EXT2FS_GROUP_DESC_BLKNUM +
             (gdNum / pVolDesc->bgPerBlk) +
             pVolDesc->pSuperBlk->s_first_data_block;

    error = bread (pSyncer, blkNum, pVolDesc->blkSize, NULL, &pBuf);
    if (error != OK) {
        return (error);
    }

    pGroupDescDisk = (EXT2FS_GROUP_DESC_DISK *)
                     ((char *) pBuf->b_data +
                     (gdNum * sizeof (EXT2FS_GROUP_DESC_DISK)));

    ext2fsGroupDescDiskToHost (pGroupDescDisk, pGroupDesc);

    brelse (pBuf);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsGroupDescriptorsRead - read block of group descriptors from the disk
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsGroupDescriptorsRead (
    EXT2FS_DEV *   pFsDev,
    struct vnode * pSyncer,
    int            bgNum
    ) {
    EXT2FS_VOLUME_DESC * pVolDesc;
    u_int32_t            nGroupDesc;
    int                  i;
    int                  error;

    pVolDesc = &(pFsDev->ext2fsVolDesc);

    nGroupDesc = pVolDesc->bgPerBlk;
    if ((bgNum + nGroupDesc) > pVolDesc->nBlkGroups) {
        nGroupDesc = (pVolDesc->nBlkGroups & (pVolDesc->bgPerBlk - 1));
    }

    pVolDesc->bgCached = bgNum;
    for (i = 0; i < nGroupDesc; i++) {
        error = ext2fsGroupDescriptorRead (pFsDev, pSyncer,
                                           &pVolDesc->pBlkGroupDesc[i],
                                           bgNum + i);
        if (error != OK) {
            return (error);
        }
    }

    return (OK);
}

/******************************************************************************
 *
 * ext2fsInodeGet - read inode from the disk
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsInodeGet (
    struct vnode * pVnode
    ) {
    struct vnode *       pSyncer;
    struct buf *         pBuf;
    EXT2FS_INODE *       pInode;
    EXT2FS_INODE_DISK *  pInodeDisk;
    EXT2FS_SUPERBLOCK *  pSuperBlk;
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;
    u_int32_t            blkGroup;
    u_int32_t            off;
    u_int32_t            inodeBlk;
    lblkno_t             blkNum;
    int                  error;

    EXT2FS_VTOI (pInode, pVnode);

    pSyncer   = pVnode->v_mount->mnt_syncer;
    pFsDev    = (EXT2FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc  = &(pFsDev->ext2fsVolDesc);
    pSuperBlk = pVolDesc->pSuperBlk;

    blkGroup = (pInode->inode - 1) / pSuperBlk->s_inodes_per_group;
    if (blkGroup >= pVolDesc->nBlkGroups) {
        return (EINVAL);
    }

    if ((blkGroup < pVolDesc->bgCached) ||
        (blkGroup >= (pVolDesc->bgCached + pVolDesc->bgPerBlk))) {

        off       = (blkGroup & (~(pVolDesc->bgPerBlk - 1)));
        blkGroup &= (pVolDesc->bgPerBlk - 1);
        error     = ext2fsGroupDescriptorsRead (pFsDev, pSyncer, off);
        if (error != OK) {
            return (error);
        }
    }

    blkNum   = pVolDesc->pBlkGroupDesc[blkGroup].bg_inode_table;
    inodeBlk = ((pInode->inode - 1) >> pVolDesc->inodesPerBlk2);

    /* read block that contains the inode */
    error = bread (pSyncer, blkNum + inodeBlk, pVolDesc->blkSize, NULL, &pBuf);
    if (error != OK) {
        return (error);
    }

    /* byte offset for inode */
    off = ((pInode->inode - 1) & (pVolDesc->inodesPerBlk - 1)) <<
          (pVolDesc->blkSize2 - pVolDesc->inodesPerBlk2);
    pInodeDisk = (EXT2FS_INODE_DISK *) ((char *) pBuf->b_data + off);

    /* offset into block */
    ext2fsInodeDiskToHost (pVolDesc, pInodeDisk, &pInode->host);

    brelse (pBuf);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsMount - mount ext2 filesystem
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsMount (
    struct mount * pMount
    ) {
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;
    EXT2FS_SUPERBLOCK *  pSuperBlk;
    struct vnode *       pSyncer;
    unsigned             sectorSize;
    long long            totalSize;
    int                  error;
    u_int32_t            blkSize;
    int                  savedError = errnoGet ();

    pFsDev   = (EXT2FS_DEV *) pMount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);
    
    memset (pFsDev, 0, sizeof (EXT2FS_DEV));
    if ((xbdBlockSize (pMount->mnt_dev, &sectorSize) != OK) ||
        (xbdSize (pMount->mnt_dev, &totalSize) != OK)) {
        error = errnoGet ();
        errnoSet (savedError);
        return (error);
    }

    if (((sectorSize & (sectorSize - 1)) != 0) ||
        (sectorSize < EXT2FS_MIN_SECTOR_SIZE) ||
        (sectorSize > EXT2FS_MAX_SECTOR_SIZE)) {
        return (EINVAL);
    }

    pVolDesc->pSuperBlk = malloc (sizeof (EXT2FS_SUPERBLOCK));
    if (pVolDesc->pSuperBlk == NULL) {
        error = errnoGet ();
        errnoSet (savedError);
        return (error);
    }

    pFsDev->pScratchBlk = bio_alloc (pMount->mnt_dev,
                                     (sectorSize + 1023) / sectorSize);
    if (pFsDev->pScratchBlk == NULL) {
        error = errnoGet ();
        errnoSet (savedError);
        return (error);
    }

    pFsDev->bioSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
    if (pFsDev->bioSem == NULL) {
        error = errnoGet ();
        errnoSet (savedError);
        return (error);
    }

    pVolDesc->pMount = pMount;
    pVolDesc->device = pMount->mnt_dev;

    error = ext2fsSuperBlkRead (pFsDev);
    if (error != OK) {
        return (error);
    }

    blkSize = (EXT2FS_MIN_BLK_SIZE << pVolDesc->pSuperBlk->s_log_block_size);
    if ((blkSize < EXT2FS_MIN_BLK_SIZE) || (blkSize > EXT2FS_MAX_BLK_SIZE) ||
        (blkSize & (blkSize - 1) != 0)) {
        return (EINVAL);
    }

    /* Setup syncer vnode */
    pSyncer        = pMount->mnt_syncer;
    pSyncer->v_ops = (struct vnode_ops *) &ext2fsSyncerOps;

    pSuperBlk = pVolDesc->pSuperBlk;

    pVolDesc->magic      = EXT2FS_VOLUME_MAGIC;
    pVolDesc->device     = pMount->mnt_dev;
    pVolDesc->blkSize    = (EXT2FS_MIN_BLK_SIZE <<
                            pVolDesc->pSuperBlk->s_log_block_size);
    pVolDesc->blkSize2   = ffsMsb (pVolDesc->blkSize) - 1;
    pVolDesc->secPerBlk2 = 1 + pVolDesc->blkSize2 - ffsMsb (sectorSize);
    pVolDesc->secPerBlk  = (1 << pVolDesc->blkSize2);

    if (pSuperBlk->s_rev_level < EXT2FS_DYNAMIC_REV) {
        pVolDesc->inodeSize = sizeof (EXT2FS_INODE_DISK);
    }
    else {
        pVolDesc->inodeSize = pSuperBlk->s_inode_size;
    }

    pVolDesc->inodeSize2    = ffsMsb (pVolDesc->inodeSize) - 1;
    pVolDesc->inodesPerBlk2 = pVolDesc->blkSize2 - pVolDesc->inodeSize2;
    pVolDesc->inodesPerBlk  = (1 << pVolDesc->inodesPerBlk2);

    pVolDesc->nBlkGroups =
        (pSuperBlk->s_blocks_count + pSuperBlk->s_blocks_per_group - 1) /
        pSuperBlk->s_blocks_per_group;

    pVolDesc->bgPerBlk = pVolDesc->blkSize / sizeof (EXT2FS_GROUP_DESC_DISK);

    pVolDesc->pBlkGroupDesc =
        malloc (pVolDesc->bgPerBlk * sizeof (EXT2FS_GROUP_DESC));
    if (pVolDesc->pBlkGroupDesc == NULL) {
        return (ENOMEM);
    }

    pVolDesc->nRefBlks[0] = EXT2_N_DIRECT_BLKS;
    pVolDesc->nRefBlks[1] = (pVolDesc->blkSize >> 2);
    pVolDesc->nRefBlks[2] = pVolDesc->nRefBlks[1] * pVolDesc->nRefBlks[1];
    pVolDesc->nRefBlks[3] = pVolDesc->nRefBlks[2] * pVolDesc->nRefBlks[1];
    pVolDesc->totalBlks   = (totalSize >> pVolDesc->blkSize2);
    pVolDesc->maxNameLen  = 255;
    pVolDesc->maxPathLen  = 1023;
    pVolDesc->maxFileSize = 0x7fffffff;

    if (pVolDesc->pSuperBlk->s_rev_level == 1) {
        pVolDesc->maxFileSize = ((pVolDesc->nRefBlks[0] +
                                  pVolDesc->nRefBlks[1] +
                                  pVolDesc->nRefBlks[2] +
                                  pVolDesc->nRefBlks[3]) <<
                                 pVolDesc->blkSize2) - 1;
    }

    error = mountBufAlloc(pMount, 1, pVolDesc->blkSize);
    if (error != OK) {
        return (error);
    }

    error = ext2fsGroupDescriptorsRead (pFsDev, pSyncer, 0);

    mountBufFree(pMount);

    if (error != OK) {
        return (error);
    }

    return (OK);
}

/******************************************************************************
 *
 * ext2fsStat - retrieve ext2 filesystem status
 *
 * RETURNS: OK
 */

int ext2fsStat (
    struct mount *   pMount,
    struct statvfs * pStatVfs
    ) {
    EXT2FS_DEV *         pFsDev;
    EXT2FS_VOLUME_DESC * pVolDesc;

    pFsDev   = (EXT2FS_DEV *) pMount->mnt_data;
    pVolDesc = &(pFsDev->ext2fsVolDesc);

    pStatVfs->f_bsize   = pVolDesc->blkSize;
    pStatVfs->f_frsize  = pVolDesc->blkSize;
    pStatVfs->f_blocks  = pVolDesc->totalBlks;
    pStatVfs->f_bfree   = pVolDesc->pSuperBlk->s_blocks_count;
    pStatVfs->f_bavail  = pVolDesc->freeDataBlks;
    pStatVfs->f_files   = pVolDesc->pSuperBlk->s_inodes_count;
    pStatVfs->f_ffree   = pVolDesc->pSuperBlk->s_free_inodes_count;
    pStatVfs->f_favail  = pVolDesc->pSuperBlk->s_free_inodes_count;
    pStatVfs->f_fsid    = 0;
    pStatVfs->f_flag    = 0;
    pStatVfs->f_namemax = pVolDesc->maxNameLen;

    return (OK);
}

/******************************************************************************
 *
 * ext2fsIsSparseBlockGroup - determine if block group is sparse
 *
 * RETURNS: FALSE if block group has super block group descriptors,
 *          otherwise TRUE
 */

BOOL ext2fsIsSparseBlockGroup (
    u_int32_t  bgNum
    ) {
    int  i = 0;

    /* perform linear search */
    while (bgNum < sparseSuperBlockList[i]) {
        i++;
    }

    return (bgNum != sparseSuperBlockList[i]) ? TRUE : FALSE;
}

