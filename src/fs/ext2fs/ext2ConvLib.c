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

/* ext2ConvLib.c - Convert between host and disk for ext2 filesystem */

#include <string.h>
#include <vmx.h>
#include <fs/ext2fsLib.h>

/******************************************************************************
 *
 * ext2fsDiskToHost16 - convert 16-bit integer from disk to host format
 *
 * RETURNS: host format integer value
 */

u_int32_t  ext2fsDiskToHost16 (
    u_int8_t * pDisk
    ) {
    u_int32_t  ret;

    ret = ((u_int32_t) pDisk[0]) + (((u_int32_t) pDisk[1]) << 8);

    return (ret);
}

/******************************************************************************
 *
 * ext2fsDiskToHost32 - convert 32-bit integer from disk to host format
 *
 * RETURNS: host format integer value
 */

u_int32_t  ext2fsDiskToHost32 (
    u_int8_t * pDisk
    ) {
    u_int32_t  ret;

    ret = ((u_int32_t) pDisk[0]) + (((u_int32_t) pDisk[1]) << 8) +
          (((u_int32_t) pDisk[2]) << 16) + (((u_int32_t) pDisk[3]) << 24);

    return (ret);
}

/******************************************************************************
 *
 * ext2fsHostToDisk16 - convert 16-bit integer from host to disk format
 *
 * RETURNS: N/A
 */

void  ext2fsHostToDisk16 (
    u_int32_t  host,
    u_int8_t * pDisk
    ) {

    pDisk[0] = (u_int8_t) host;
    pDisk[1] = (u_int8_t) (host >> 8);
}

/******************************************************************************
 *
 * ext2fsHostToDisk32 - convert 32-bit integer from host to disk format
 *
 * RETURNS: N/A
 */

void  ext2fsHostToDisk32 (
    u_int32_t  host,
    u_int8_t * pDisk
    ) {

    pDisk[0] = (u_int8_t) host;
    pDisk[1] = (u_int8_t) (host >> 8);
    pDisk[2] = (u_int8_t) (host >> 16);
    pDisk[3] = (u_int8_t) (host >> 24);
}

/******************************************************************************
 *
 * ext2fsDirEntryDiskToHost - convert directory entry from disk to host format
 *
 * RETURNS: N/A
 */

void ext2fsDirEntryDiskToHost (
    EXT2FS_DIR_ENTRY_DISK * pDisk,
    EXT2FS_DIR_ENTRY *      pHost
    ) {

    pHost->inode   = ext2fsDiskToHost32 (pDisk->inode);
    pHost->rec_len = ext2fsDiskToHost16 (pDisk->rec_len);
    pHost->name_len  = pDisk->name_len + ((pDisk->file_type & 0xf0) << 4);
    pHost->file_type = (pDisk->file_type & 0x0F);
}

/******************************************************************************
 *
 * ext2fsDirEntryHostToDisk - convert directory entry from host to disk format
 *
 * RETURNS: N/A
 */

void ext2fsDirEntryHostToDisk (
    EXT2FS_DIR_ENTRY *      pHost,
    EXT2FS_DIR_ENTRY_DISK * pDisk
    ) {

    ext2fsHostToDisk32 (pHost->inode, pDisk->inode);
    ext2fsHostToDisk16 (pHost->rec_len, pDisk->rec_len);
    pDisk->name_len  = (u_int8_t) pHost->name_len;
    pDisk->file_type = (pHost->file_type & 0x0F) |
                       ((pHost->name_len >> 4) & 0xF0);
}

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
    ) {
    int        i;
    u_int8_t * ptr;

    pHost->i_mode = ext2fsDiskToHost16 (pDisk->i_mode);
    pHost->i_uid  = ext2fsDiskToHost16 (pDisk->i_uid);

    if (pVolDesc->pSuperBlk->s_rev_level == 1) {
        pHost->i_size = ext2fsDiskToHost32 (pDisk->i_dir_acl);
        pHost->i_size <<= 32;
        pHost->i_size += ext2fsDiskToHost32 (pDisk->i_size);
        pHost->i_dir_acl = 0;
    }
    else {
        pHost->i_size = ext2fsDiskToHost32 (pDisk->i_size);
        pHost->i_dir_acl = ext2fsDiskToHost32 (pDisk->i_dir_acl);
    }

    pHost->i_atime       = ext2fsDiskToHost32 (pDisk->i_atime);
    pHost->i_ctime       = ext2fsDiskToHost32 (pDisk->i_ctime);
    pHost->i_mtime       = ext2fsDiskToHost32 (pDisk->i_mtime);
    pHost->i_dtime       = ext2fsDiskToHost32 (pDisk->i_dtime);
    pHost->i_gid         = ext2fsDiskToHost16 (pDisk->i_gid);
    pHost->i_links_count = ext2fsDiskToHost16 (pDisk->i_links_count);
    pHost->i_blocks      = ext2fsDiskToHost32 (pDisk->i_blocks);
    pHost->i_flags       = ext2fsDiskToHost32 (pDisk->i_flags);

    memcpy ((char *) pHost->i_osd1, pDisk->i_osd1, 4);

    ptr = (u_int8_t *) pDisk->i_block;
    for (i = 0; i < EXT2_N_BLKS; i++, ptr+= 4) {
        pHost->i_block[i] = ext2fsDiskToHost32 (ptr);
    }

    pHost->i_version  = ext2fsDiskToHost32 (pDisk->i_version);
    pHost->i_file_acl = ext2fsDiskToHost32 (pDisk->i_file_acl);
    pHost->i_faddr    = ext2fsDiskToHost32 (pDisk->i_faddr);

    memcpy (pHost->i_osd2, pDisk->i_osd2, 12);
}

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
    ) {
    int        i;
    u_int8_t * ptr;

    ext2fsHostToDisk16 (pHost->i_mode, pDisk->i_mode);
    ext2fsHostToDisk16 (pHost->i_uid, pDisk->i_uid);
    ext2fsHostToDisk32 ((u_int32_t) pHost->i_size, pDisk->i_size);
    ext2fsHostToDisk32 (pHost->i_atime, pDisk->i_atime);
    ext2fsHostToDisk32 (pHost->i_ctime, pDisk->i_ctime);
    ext2fsHostToDisk32 (pHost->i_mtime, pDisk->i_mtime);
    ext2fsHostToDisk32 (pHost->i_dtime, pDisk->i_dtime);
    ext2fsHostToDisk16 (pHost->i_gid, pDisk->i_gid);
    ext2fsHostToDisk16 (pHost->i_links_count, pDisk->i_links_count);
    ext2fsHostToDisk32 (pHost->i_blocks, pDisk->i_blocks);
    ext2fsHostToDisk32 (pHost->i_flags, pDisk->i_flags);

    memcpy (pDisk->i_osd1, pHost->i_osd1, 4);

    ptr = (u_int8_t *) pDisk->i_block;
    for (i = 0; i < EXT2_N_BLKS; i++, ptr += 4) {
        ext2fsHostToDisk32(pHost->i_block[i], ptr);
    }

    ext2fsHostToDisk32 (pHost->i_version, pDisk->i_version);
    ext2fsHostToDisk32 (pHost->i_file_acl, pDisk->i_file_acl);

    if (pVolDesc->pSuperBlk->s_rev_level == 1) {
        ext2fsHostToDisk32 ((u_int32_t) (pHost->i_size >> 32),
                            pDisk->i_dir_acl);
    }
    else {
        ext2fsHostToDisk32 (pHost->i_dir_acl, pDisk->i_dir_acl);
    }

    ext2fsHostToDisk32 (pHost->i_faddr, pDisk->i_faddr);

    memcpy (pDisk->i_osd2, pHost->i_osd2, 12);
}

/******************************************************************************
 *
 * ext2fsGroupDescDiskToHost - convert group descriptor from disk to host form
 *
 * RETURNS: N/A
 */

void ext2fsGroupDescDiskToHost (
    EXT2FS_GROUP_DESC_DISK * pDisk,
    EXT2FS_GROUP_DESC      * pHost
    ) {
    int  i;

    pHost->bg_block_bitmap = ext2fsDiskToHost32 (pDisk->bg_block_bitmap);
    pHost->bg_inode_bitmap = ext2fsDiskToHost32 (pDisk->bg_inode_bitmap);
    pHost->bg_inode_table  = ext2fsDiskToHost32 (pDisk->bg_inode_table);

    pHost->bg_free_blocks_count =
        ext2fsDiskToHost16 (pDisk->bg_free_blocks_count);
    pHost->bg_free_inodes_count =
        ext2fsDiskToHost16 (pDisk->bg_free_inodes_count);

    pHost->bg_used_dirs_count = ext2fsDiskToHost16 (pDisk->bg_used_dirs_count);
    pHost->bg_pad             = ext2fsDiskToHost16 (pDisk->bg_pad);

    for (i = 0; i < 3; i++) {
        pHost->bg_reserved[i] = ext2fsDiskToHost32 (&pDisk->bg_reserved[i][0]);
    }
}

/******************************************************************************
 *
 * ext2fsGroupDescHostToDisk - convert group descriptor from host to disk form
 *
 * RETURNS: N/A
 */

void ext2fsGroupDescHostToDisk (
    EXT2FS_GROUP_DESC      * pHost,
    EXT2FS_GROUP_DESC_DISK * pDisk
    ) {
    int  i;

    ext2fsHostToDisk32 (pHost->bg_block_bitmap, pDisk->bg_block_bitmap);
    ext2fsHostToDisk32 (pHost->bg_inode_bitmap, pDisk->bg_inode_bitmap);
    ext2fsHostToDisk32 (pHost->bg_inode_table, pDisk->bg_inode_table);
    ext2fsHostToDisk16 (pHost->bg_free_blocks_count,
                        pDisk->bg_free_blocks_count);
    ext2fsHostToDisk16 (pHost->bg_free_inodes_count,
                        pDisk->bg_free_inodes_count);
    ext2fsHostToDisk16 (pHost->bg_used_dirs_count, pDisk->bg_used_dirs_count);
    ext2fsHostToDisk16 (pHost->bg_pad, pDisk->bg_pad);

    for (i = 0; i < 3; i++) {
        ext2fsHostToDisk32 (pHost->bg_reserved[i], &pDisk->bg_reserved[i][0]);
    }
}

/******************************************************************************
 *
 * ext2fsSuperBlockDiskToHost - convert superblock from disk to host format
 *
 * RETURNS: N/A
 */

void ext2fsSuperBlockDiskToHost (
    EXT2FS_SUPERBLOCK_DISK * pDisk,
    EXT2FS_SUPERBLOCK *      pHost
    ) {

    pHost->s_inodes_count   = ext2fsDiskToHost32 (pDisk->s_inodes_count);
    pHost->s_blocks_count   = ext2fsDiskToHost32 (pDisk->s_blocks_count);
    pHost->s_r_blocks_count = ext2fsDiskToHost32 (pDisk->s_r_blocks_count);

    pHost->s_free_blocks_count =
        ext2fsDiskToHost32 (pDisk->s_free_blocks_count);
    pHost->s_free_inodes_count =
        ext2fsDiskToHost32 (pDisk->s_free_inodes_count);

    pHost->s_first_data_block = ext2fsDiskToHost32 (pDisk->s_first_data_block);
    pHost->s_log_block_size   = ext2fsDiskToHost32 (pDisk->s_log_block_size);
    pHost->s_log_frag_size    = ext2fsDiskToHost32 (pDisk->s_log_frag_size);
    pHost->s_blocks_per_group = ext2fsDiskToHost32 (pDisk->s_blocks_per_group);
    pHost->s_frags_per_group  = ext2fsDiskToHost32 (pDisk->s_frags_per_group);
    pHost->s_inodes_per_group = ext2fsDiskToHost32 (pDisk->s_inodes_per_group);

    pHost->s_mtime           = ext2fsDiskToHost32 (pDisk->s_mtime);
    pHost->s_wtime           = ext2fsDiskToHost32 (pDisk->s_wtime);
    pHost->s_mnt_count       = ext2fsDiskToHost16 (pDisk->s_mnt_count);
    pHost->s_max_mnt_count   = ext2fsDiskToHost16 (pDisk->s_max_mnt_count);
    pHost->s_magic           = ext2fsDiskToHost16 (pDisk->s_magic);
    pHost->s_state           = ext2fsDiskToHost16 (pDisk->s_state);
    pHost->s_errors          = ext2fsDiskToHost16 (pDisk->s_errors);
    pHost->s_minor_rev_level = ext2fsDiskToHost16 (pDisk->s_minor_rev_level);
    pHost->s_lastcheck       = ext2fsDiskToHost32 (pDisk->s_lastcheck);
    pHost->s_checkinterval   = ext2fsDiskToHost32 (pDisk->s_checkinterval);
    pHost->s_creator_os      = ext2fsDiskToHost32 (pDisk->s_creator_os);
    pHost->s_rev_level       = ext2fsDiskToHost32 (pDisk->s_rev_level);
    pHost->s_def_resuid      = ext2fsDiskToHost16 (pDisk->s_def_resuid);
    pHost->s_def_resgid      = ext2fsDiskToHost16 (pDisk->s_def_resgid);

    pHost->s_first_ino        = ext2fsDiskToHost32 (pDisk->s_first_ino);
    pHost->s_inode_size       = ext2fsDiskToHost16 (pDisk->s_inode_size);
    pHost->s_block_group_nr   = ext2fsDiskToHost16 (pDisk->s_block_group_nr);
    pHost->s_feature_compat   = ext2fsDiskToHost32 (pDisk->s_feature_compat);
    pHost->s_feature_incompat = ext2fsDiskToHost32 (pDisk->s_feature_incompat);

    pHost->s_feature_ro_compat =
        ext2fsDiskToHost32 (pDisk->s_feature_ro_compat);

    memcpy (pHost->s_uuid, pDisk->s_uuid, 16);
    memcpy (pHost->s_volume_name, pDisk->s_volume_name, 16);
    memcpy (pHost->s_last_mounted, pDisk->s_last_mounted, 64);

    pHost->s_algo_bitmap = ext2fsDiskToHost32 (pDisk->s_algo_bitmap);

    pHost->s_prealloc_blocks     = pDisk->s_prealloc_blocks;
    pHost->s_prealloc_dir_blocks = pDisk->s_prealloc_dir_blocks;
    pHost->s_pad2                = ext2fsDiskToHost16 (pDisk->s_pad2);

    memcpy (pHost->s_journal_uuid, pDisk->s_journal_uuid, 16);

    pHost->s_journal_inum = ext2fsDiskToHost32 (pDisk->s_journal_inum);
    pHost->s_journal_dev  = ext2fsDiskToHost32 (pDisk->s_journal_dev);
    pHost->s_last_orphan  = ext2fsDiskToHost32 (pDisk->s_last_orphan);

    memcpy (pHost->s_reserved, pDisk->s_reserved, 788);
}

/******************************************************************************
 *
 * ext2fsSuperBlockHostToDisk - convert superblock from host to disk format
 *
 * RETURNS: N/A
 */

void ext2fsSuperBlockHostToDisk (
    EXT2FS_SUPERBLOCK *      pHost,
    EXT2FS_SUPERBLOCK_DISK * pDisk
    ) {

    ext2fsHostToDisk32 (pHost->s_inodes_count, pDisk->s_inodes_count);
    ext2fsHostToDisk32 (pHost->s_blocks_count, pDisk->s_blocks_count);
    ext2fsHostToDisk32 (pHost->s_r_blocks_count, pDisk->s_r_blocks_count);
    ext2fsHostToDisk32 (pHost->s_free_blocks_count, pDisk->s_free_blocks_count);
    ext2fsHostToDisk32 (pHost->s_free_inodes_count, pDisk->s_free_inodes_count);
    ext2fsHostToDisk32 (pHost->s_first_data_block, pDisk->s_first_data_block);
    ext2fsHostToDisk32 (pHost->s_log_block_size, pDisk->s_log_block_size);
    ext2fsHostToDisk32 (pHost->s_log_frag_size, pDisk->s_log_frag_size);
    ext2fsHostToDisk32 (pHost->s_blocks_per_group, pDisk->s_blocks_per_group);
    ext2fsHostToDisk32 (pHost->s_frags_per_group, pDisk->s_frags_per_group);
    ext2fsHostToDisk32 (pHost->s_inodes_per_group, pDisk->s_inodes_per_group);
    ext2fsHostToDisk32 (pHost->s_mtime, pDisk->s_mtime);
    ext2fsHostToDisk32 (pHost->s_wtime, pDisk->s_wtime);
    ext2fsHostToDisk16 (pHost->s_mnt_count, pDisk->s_mnt_count);
    ext2fsHostToDisk16 (pHost->s_max_mnt_count, pDisk->s_max_mnt_count);
    ext2fsHostToDisk16 (pHost->s_magic, pDisk->s_magic);
    ext2fsHostToDisk16 (pHost->s_state, pDisk->s_state);
    ext2fsHostToDisk16 (pHost->s_errors, pDisk->s_errors);
    ext2fsHostToDisk16 (pHost->s_minor_rev_level, pDisk->s_minor_rev_level);
    ext2fsHostToDisk32 (pHost->s_lastcheck, pDisk->s_lastcheck);
    ext2fsHostToDisk32 (pHost->s_checkinterval, pDisk->s_checkinterval);
    ext2fsHostToDisk32 (pHost->s_creator_os, pDisk->s_creator_os);
    ext2fsHostToDisk32 (pHost->s_rev_level, pDisk->s_rev_level);
    ext2fsHostToDisk16 (pHost->s_def_resuid, pDisk->s_def_resuid);
    ext2fsHostToDisk16 (pHost->s_def_resgid, pDisk->s_def_resgid);

    ext2fsHostToDisk32 (pHost->s_first_ino, pDisk->s_first_ino);
    ext2fsHostToDisk16 (pHost->s_inode_size, pDisk->s_inode_size);
    ext2fsHostToDisk16 (pHost->s_block_group_nr, pDisk->s_block_group_nr);
    ext2fsHostToDisk32 (pHost->s_feature_compat, pDisk->s_feature_compat);
    ext2fsHostToDisk32 (pHost->s_feature_incompat, pDisk->s_feature_incompat);
    ext2fsHostToDisk32 (pHost->s_feature_ro_compat, pDisk->s_feature_ro_compat);

    memcpy (pDisk->s_uuid, pHost->s_uuid, 16);
    memcpy (pDisk->s_volume_name, pHost->s_volume_name, 16);
    memcpy (pDisk->s_last_mounted, pHost->s_last_mounted, 64);

    ext2fsHostToDisk32 (pHost->s_algo_bitmap, pDisk->s_algo_bitmap);

    pDisk->s_prealloc_blocks     = pHost->s_prealloc_blocks;
    pDisk->s_prealloc_dir_blocks = pHost->s_prealloc_dir_blocks;
    ext2fsHostToDisk16 (pHost->s_pad2, pDisk->s_pad2);

    memcpy (pDisk->s_journal_uuid, pHost->s_journal_uuid, 16);

    ext2fsHostToDisk32 (pHost->s_journal_inum, pDisk->s_journal_inum);
    ext2fsHostToDisk32 (pHost->s_journal_dev, pDisk->s_journal_dev);
    ext2fsHostToDisk32 (pHost->s_last_orphan, pDisk->s_last_orphan);

    memcpy (pDisk->s_reserved, pHost->s_reserved, 788);
}

