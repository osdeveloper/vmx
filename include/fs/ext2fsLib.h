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

/* ext2fsLib.h - Ext2 filesystem header */

#ifndef _ext2fsLib_h
#define _ext2fsLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>
#include <sys/types.h>
#include <fs/vnode.h>
#include <fs/buf.h>
#include <fs/mount.h>

/* defines */

#define EXT2FS_VOLUME_MAGIC                             0x45585432
#define EXT2FS_DYNAMIC_REV                              1
#define EXT2FS_GROUP_DESC_BLKNUM                        1

#define EXT2FS_MIN_BLK_SIZE                             1024
#define EXT2FS_MAX_BLK_SIZE                             8192
#define EXT2FS_MIN_SECTOR_SIZE                          512
#define EXT2FS_MAX_SECTOR_SIZE                          8192
#define EXT2FS_MAX_HARD_LINKS                           0xffff

#define EXT2_N_BLKS                                     15
#define EXT2_N_DIRECT_BLKS                              12
#define EXT2_SINGLE_INDIRECT_BLK                        12
#define EXT2_DOUBLE_INDIRECT_BLK                        13
#define EXT2_TRIPLE_INDIRECT_BLK                        14

#define EXT2_DIRENT_EOF                                 0x01
#define EXT2_DIRENT_MINIMUM                             8

#define EXT2FS_MIN_BUFFERS                              16

#define EXT2_DIR_ENTRY_HEADER_SIZE                      8

#define EXT2FS_ROOT_INODE                               2
#define EXT2FS_SUPERBLK_MAGIC                           0xef53

#define EXT2FS_BGD_CACHE_DIRTY                          0x00000001

#define EXT2FS_SPARSE_BLK_NUM                           0

#define EXT2FS_COMPATIBLE_DIRECTORY_PREALLOC            0x00000001
#define EXT2FS_COMPATIBLE_IMAGIC_INODES                 0x00000002
#define EXT2FS_COMPATIBLE_HAS_JOURNAL                   0x00000004
#define EXT2FS_COMPATIBLE_EXTENDED_ATTRIBUTES           0x00000008
#define EXT2FS_COMPATIBLE_RESIZED_INODE                 0x00000010
#define EXT2FS_COMPATIBLE_DIRECTORY_INDEX               0x00000020
#define EXT2FS_COMPATIBLE_IMPLEMENTED                   0x00000000

#define EXT2FS_INCOMPATIBLE_COMPRESSION                 0x00000001
#define EXT2FS_INCOMPATIBLE_FILETYPE                    0x00000002
#define EXT2FS_INCOMPATIBLE_RECOVER                     0x00000004
#define EXT2FS_INCOMPATIBLE_JOURNAL_DEV                 0x00000008
#define EXT2FS_INCOMPATIBLE_META_BG                     0x00000010
#define EXT2FS_INCOMPATIBLE_IMPLEMENTED                 0x00000002

#define EXT2FS_READ_ONLY_COMPATIBLE_SPARSE_SUPERBLOCKS  0x00000001
#define EXT2FS_READ_ONLY_COMPATIBLE_LARGE_FILE_SUPPORT  0x00000002
#define EXT2FS_READ_ONLY_COMPATIBLE_BTREE_DIRECTORIES   0x00000004
#define EXT2FS_READ_ONLY_COMPATIBLE_IMPLEMENTED         0x00000003

/* types */

/* Group descriptor on-disk structure */
typedef struct ext2_group_desc_disk {
    u_int8_t  bg_block_bitmap[4];
    u_int8_t  bg_inode_bitmap[4];
    u_int8_t  bg_inode_table[4];
    u_int8_t  bg_free_blocks_count[2];
    u_int8_t  bg_free_inodes_count[2];
    u_int8_t  bg_used_dirs_count[2];
    u_int8_t  bg_pad[2];
    u_int8_t  bg_reserved[3][4];
} EXT2FS_GROUP_DESC_DISK;

/* Inode on-disk structure */
typedef struct ext2_inode_disk {
    u_int8_t  i_mode[2];
    u_int8_t  i_uid[2];
    u_int8_t  i_size[4];
    u_int8_t  i_atime[4];
    u_int8_t  i_ctime[4];
    u_int8_t  i_mtime[4];
    u_int8_t  i_dtime[4];
    u_int8_t  i_gid[2];
    u_int8_t  i_links_count[2];
    u_int8_t  i_blocks[4];
    u_int8_t  i_flags[4];
    u_int8_t  i_osd1[4];
    u_int8_t  i_block[EXT2_N_BLKS][4];
    u_int8_t  i_version[4];
    u_int8_t  i_file_acl[4];
    u_int8_t  i_dir_acl[4];
    u_int8_t  i_faddr[4];
    u_int8_t  i_osd2[12];
} EXT2FS_INODE_DISK;

/* Directory entry on-disk structure */
typedef struct ext2_dir_entry_disk {
    u_int8_t  inode[4];
    u_int8_t  rec_len[2];
    u_int8_t  name_len;
    u_int8_t  file_type;
    u_int8_t  name[1];
} EXT2FS_DIR_ENTRY_DISK;

/* Superblock on-disk structure */
typedef struct ext2_superblock_disk {
    u_int8_t  s_inodes_count[4];
    u_int8_t  s_blocks_count[4];
    u_int8_t  s_r_blocks_count[4];
    u_int8_t  s_free_blocks_count[4];
    u_int8_t  s_free_inodes_count[4];
    u_int8_t  s_first_data_block[4];
    u_int8_t  s_log_block_size[4];
    u_int8_t  s_log_frag_size[4];
    u_int8_t  s_blocks_per_group[4];
    u_int8_t  s_frags_per_group[4];
    u_int8_t  s_inodes_per_group[4];
    u_int8_t  s_mtime[4];
    u_int8_t  s_wtime[4];
    u_int8_t  s_mnt_count[2];
    u_int8_t  s_max_mnt_count[2];
    u_int8_t  s_magic[2];
    u_int8_t  s_state[2];
    u_int8_t  s_errors[2];
    u_int8_t  s_minor_rev_level[2];
    u_int8_t  s_lastcheck[4];
    u_int8_t  s_checkinterval[4];
    u_int8_t  s_creator_os[4];
    u_int8_t  s_rev_level[4];
    u_int8_t  s_def_resuid[2];
    u_int8_t  s_def_resgid[2];
    u_int8_t  s_first_ino[4];
    u_int8_t  s_inode_size[2];
    u_int8_t  s_block_group_nr[2];
    u_int8_t  s_feature_compat[4];
    u_int8_t  s_feature_incompat[4];
    u_int8_t  s_feature_ro_compat[4];
    u_int8_t  s_uuid[16];
    u_int8_t  s_volume_name[16];
    u_int8_t  s_last_mounted[64];
    u_int8_t  s_algo_bitmap[4];
    u_int8_t  s_prealloc_blocks;
    u_int8_t  s_prealloc_dir_blocks;
    u_int8_t  s_pad2[2];
    u_int8_t  s_journal_uuid[16];
    u_int8_t  s_journal_inum[4];
    u_int8_t  s_journal_dev[4];
    u_int8_t  s_last_orphan[4];
    u_int8_t  s_reserved[788];
} EXT2FS_SUPERBLOCK_DISK;

/* Group descriptor in-memory structure */
typedef struct ext2_group_desc {
    u_int32_t  bg_block_bitmap;
    u_int32_t  bg_inode_bitmap;
    u_int32_t  bg_inode_table;
    u_int16_t  bg_free_blocks_count;
    u_int16_t  bg_free_inodes_count;
    u_int16_t  bg_used_dirs_count;
    u_int16_t  bg_pad;
    u_int32_t  bg_reserved[3];
} EXT2FS_GROUP_DESC;

typedef struct ext2_trie_map {
    u_int32_t    index[4];
    u_int32_t    physBlk[4];
    u_int32_t    runBlk[4];
    struct buf * runBufs[4];
    u_int32_t    runLength;
    u_int32_t    runIndex;
    int          level;
} EXT2FS_TRIE_MAP;

typedef struct ext2_inode_host {
    u_int16_t  i_mode;
    u_int16_t  i_uid;
    u_int64_t  i_size;
    u_int32_t  i_atime;
    u_int32_t  i_ctime;
    u_int32_t  i_mtime;
    u_int32_t  i_dtime;
    u_int16_t  i_gid;
    u_int16_t  i_links_count;
    u_int32_t  i_blocks;
    u_int32_t  i_flags;
    u_int8_t   i_osd1[4];
    u_int32_t  i_block[EXT2_N_BLKS];
    u_int32_t  i_version;
    u_int32_t  i_file_acl;
    u_int32_t  i_dir_acl;
    u_int32_t  i_faddr;
    u_int8_t   i_osd2[12];
} EXT2FS_INODE_HOST;
    
typedef struct ext2_lookup_info {
    int  curIndex;
    int  prevIndex;
    int  nextIndex;
} EXT2FS_LOOKUP_INFO;
    
typedef struct ext2_inode {
    EXT2FS_INODE_HOST   host;
    EXT2FS_LOOKUP_INFO  info;
    BOOL                deleted;
    u_int32_t           inode;
    u_int32_t           blkGroupHint;
    EXT2FS_TRIE_MAP     cachedTrieMap;
} EXT2FS_INODE;

typedef struct ext2_dir_entry {
    u_int32_t  inode;
    u_int32_t  rec_len;
    u_int32_t  name_len;
    u_int32_t  file_type;
} EXT2FS_DIR_ENTRY;

/* Superblock in-memory structure */
typedef struct ext2_superblock {
    u_int32_t  s_inodes_count;
    u_int32_t  s_blocks_count;
    u_int32_t  s_r_blocks_count;
    u_int32_t  s_free_blocks_count;
    u_int32_t  s_free_inodes_count;
    u_int32_t  s_first_data_block;
    u_int32_t  s_log_block_size;
    u_int32_t  s_log_frag_size;
    u_int32_t  s_blocks_per_group;
    u_int32_t  s_frags_per_group;
    u_int32_t  s_inodes_per_group;
    u_int32_t  s_mtime;
    u_int32_t  s_wtime;
    u_int16_t  s_mnt_count;
    u_int16_t  s_max_mnt_count;
    u_int16_t  s_magic;
    u_int16_t  s_state;
    u_int16_t  s_errors;
    u_int16_t  s_minor_rev_level;
    u_int32_t  s_lastcheck;
    u_int32_t  s_checkinterval;
    u_int32_t  s_creator_os;
    u_int32_t  s_rev_level;
    u_int16_t  s_def_resuid;
    u_int16_t  s_def_resgid;
    u_int32_t  s_first_ino;
    u_int16_t  s_inode_size;
    u_int16_t  s_block_group_nr;
    u_int32_t  s_feature_compat;
    u_int32_t  s_feature_incompat;
    u_int32_t  s_feature_ro_compat;
    u_int8_t   s_uuid[16];
    u_int8_t   s_volume_name[16];
    u_int8_t   s_last_mounted[64];
    u_int32_t  s_algo_bitmap;
    u_int8_t   s_prealloc_blocks;
    u_int8_t   s_prealloc_dir_blocks;
    u_int16_t  s_pad2;
    u_int8_t   s_journal_uuid[16];
    u_int32_t  s_journal_inum;
    u_int32_t  s_journal_dev;
    u_int32_t  s_last_orphan;
    u_int8_t   s_reserved[788];
} EXT2FS_SUPERBLOCK;

typedef struct ext2_volume_desc {
    u_int               magic;
    device_t            device;
    struct mount      * pMount;
    u_int32_t           secPerBlk;
    u_int32_t           secPerBlk2;
    u_int32_t           blkSize;
    u_int32_t           blkSize2;
    u_int32_t           inodeSize;
    u_int32_t           inodeSize2;
    u_int32_t           inodesPerBlk;
    u_int32_t           inodesPerBlk2;
    u_int32_t           totalBlks;
    u_int32_t           freeDataBlks;
    u_int32_t           nBlkGroups;
    u_int32_t           bgPerBlk;
    u_int32_t           bgCached;
    u_int32_t           bgdCacheFlags;
    EXT2FS_GROUP_DESC * pBlkGroupDesc;
    BOOL                diskModified;
    int                 maxNameLen;
    int                 maxPathLen;
    voff_t              maxFileSize;
    EXT2FS_SUPERBLOCK * pSuperBlk;
    u_int64_t           nRefBlks[4];
} EXT2FS_VOLUME_DESC;
    
typedef struct ext2fs_dev {
    EXT2FS_VOLUME_DESC   ext2fsVolDesc;
    u_int8_t           * pScratchBlk;
    SEM_ID               bioSem;
} EXT2FS_DEV;

/* macros */

/******************************************************************************
 *
 * EXT2FS_VTOI - Vnode to inode
 *
 * RETURNS: N/A
 */

#define EXT2FS_VTOI(pInode, pVnode)                                           \
    (pInode) = VTODATA(EXT2FS_INODE, (pVnode));                               \
    (pInode)->inode = (pVnode)->v_inode

/******************************************************************************
 *
 * EXT2FS_DIR_ENTRY_RED_LEN -
 *
 * RETURNS: Dir entry record length
 */

#define EXT2FS_DIR_ENTRY_REC_LEN(namelen)                                     \
    (((OFFSET(EXT2FS_DIR_ENTRY_DISK, name) + (namelen)) + 3) & (~3))

/* functions */

/******************************************************************************
 *
 * ext2fsDiskToHost16 - convert 16-bit integer from disk to host format
 *
 * RETURNS: host format integer value
 */

u_int32_t  ext2fsDiskToHost16 (
    u_int8_t * pDisk
    );

/******************************************************************************
 *
 * ext2fsDiskToHost32 - convert 32-bit integer from disk to host format
 *
 * RETURNS: host format integer value
 */

u_int32_t  ext2fsDiskToHost32 (
    u_int8_t * pDisk
    );

/******************************************************************************
 *
 * ext2fsHostToDisk16 - convert 16-bit integer from host to disk format
 *
 * RETURNS: N/A
 */

void  ext2fsHostToDisk16 (
    u_int32_t  host,
    u_int8_t * pDisk
    );

/******************************************************************************
 *
 * ext2fsHostToDisk32 - convert 32-bit integer from host to disk format
 *
 * RETURNS: N/A
 */

void  ext2fsHostToDisk32 (
    u_int32_t  host,
    u_int8_t * pDisk
    );

/******************************************************************************
 *
 * ext2fsDirEntryDiskToHost - convert directory entry from disk to host format
 *
 * RETURNS: N/A
 */

void ext2fsDirEntryDiskToHost (
    EXT2FS_DIR_ENTRY_DISK * pDisk,
    EXT2FS_DIR_ENTRY *      pHost
    );

/******************************************************************************
 *
 * ext2fsDirEntryHostToDisk - convert directory entry from host to disk format
 *
 * RETURNS: N/A
 */

void ext2fsDirEntryHostToDisk (
    EXT2FS_DIR_ENTRY *      pHost,
    EXT2FS_DIR_ENTRY_DISK * pDisk
    );

/******************************************************************************
 *
 * ext2fsInodeDiskToHost - convert inode structure from disk to host format
 *
 * RETURNS: N/A
 */

void ext2fsInodeDiskToHost (
    EXT2FS_VOLUME_DESC * pVolDesc,
    EXT2FS_INODE_DISK *  pDisk,
    EXT2FS_INODE_HOST *  pHost
    );

/******************************************************************************
 *
 * ext2fsInodeHostToDisk - convert inode structure from host to disk format
 *
 * RETURNS: N/A
 */

void ext2fsInodeHostToDisk (
    EXT2FS_VOLUME_DESC * pVolDesc,
    EXT2FS_INODE_HOST *  pHost,
    EXT2FS_INODE_DISK *  pDisk
    );

/******************************************************************************
 *
 * ext2fsGroupDescDiskToHost - convert group descriptor from disk to host form
 *
 * RETURNS: N/A
 */

void ext2fsGroupDescDiskToHost (
    EXT2FS_GROUP_DESC_DISK * pDisk,
    EXT2FS_GROUP_DESC      * pHost
    );

/******************************************************************************
 *
 * ext2fsGroupDescHostToDisk - convert group descriptor from host to disk form
 *
 * RETURNS: N/A
 */

void ext2fsGroupDescHostToDisk (
    EXT2FS_GROUP_DESC      * pHost,
    EXT2FS_GROUP_DESC_DISK * pDisk
    );

/******************************************************************************
 *
 * ext2fsSuperBlockDiskToHost - convert superblock from disk to host format
 *
 * RETURNS: N/A
 */

void ext2fsSuperBlockDiskToHost (
    EXT2FS_SUPERBLOCK_DISK * pDisk,
    EXT2FS_SUPERBLOCK *      pHost
    );

/******************************************************************************
 *
 * ext2fsSuperBlockHostToDisk - convert superblock from host to disk format
 *
 * RETURNS: N/A
 */

void ext2fsSuperBlockHostToDisk (
    EXT2FS_SUPERBLOCK *      pHost,
    EXT2FS_SUPERBLOCK_DISK * pDisk
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ext2fsLib_h */

