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

/* rt11fsLib.h - Rt11 compatible filesystem header */

#ifndef _rt11fsLib_h
#define _rt11fsLib_h

#include <vmx.h>
#include <vmx/semLib.h>
#include <os/iosLib.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/vnode.h>

#define RT11FS_DIR_BLOCK                6
#define RT11FS_FILES_FOR_2_BLOCK_SEG    72
#define RT11FS_MAX_BLOCKS_PER_FILE      0xffff
#define RT11FS_MIN_BUFFERS              16
#define RT11FS_ROOT_INODE               2
#define RT11FS_FILE_INODE_BASE          (RT11FS_ROOT_INODE + 1)

#define DES_BOGUS                       0x0000        /* Not real dir entry */
#define DES_TENTATIVE                   0x0100        /* Tentative file */
#define DES_EMPTY                       0x0200        /* Empty space */
#define DES_PERMANENT                   0x0400        /* Permanent file */
#define DES_END                         0x0800        /* End of directory */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#define RT11FS_VTOI(pInode, pVnode) \
    (pInode) = VTODATA (RT11FS_INODE, (pVnode)); \
    (pInode)->in_inode = (pVnode)->v_inode

typedef struct {
    unsigned short     nm_name1;                     /* Filename 1-3 chars */
    unsigned short     nm_name2;                     /* Filename 4-6 chars */
    unsigned short     nm_type;                      /* Filetype 1-3 chars */
} RT11FS_NAME;

typedef struct {
    short              de_status;                    /* File status */
    RT11FS_NAME        de_name;                      /* Filename */
    unsigned short     de_nblocks;                   /* Number of blocks */
    char               de_jobnum;                    /* Job with file */
    char               de_channel;                   /* Channel with file */
    short              de_date;                      /* File date */
} RT11FS_DIR_ENTRY;

typedef struct {
    short              ds_nsegs;                     /* Number of segments */
    short              ds_next_seg;                  /* Next segment */
    short              ds_last_seg;                  /* Last segment */
    short              ds_extra;                     /* Extra date */
    short              ds_start;                     /* Start block */
    RT11FS_DIR_ENTRY   ds_entries[1];                /* Directory entries */
} RT11FS_DIR_SEG;

typedef struct {
    int                rdd_maxEntries;               /* Maximum entries */
    RT11FS_DIR_SEG  *  rdd_pDirSeg;                  /* Directory segment */
} RT11FS_DIR_DESC;

typedef struct {
    int                rfd_startBlock;               /* First block of file */
    int                rfd_nBlks;                    /* Current blocks */
    RT11FS_DIR_ENTRY   rfd_dirEntry;                 /* Directory entry */
} RT11FS_FILE_DESC;

typedef struct {
    device_t           vd_device;                    /* XBD device */
    mount_t  *         vd_pMount;                    /* Pointer to mount */
    int                vd_blkSize;                   /* Blocksize */
    int                vd_blkSize2;                  /* Blocksize */
    lblkno_t           vd_nBlks;                     /* Number of blocks */
    long long          vd_diskSize;                  /* Total disksize */
    int                vd_secPerBlk;                 /* Sectors per block */
    int                vd_secPerBlk2;                /* Sectors per block */
    BOOL               vd_rtFmt;                     /* Skev track offset */
    BOOL               vd_diskModified;              /* True if modified */
    int                vd_maxEntries;                /* Number of entries */
    int                vd_nSegBlks;                  /* Block per dir seg */
    RT11FS_DIR_SEG  *  vd_pDirSeg;                   /* Directory segment */
} RT11FS_VOLUME_DESC;

typedef struct {
    RT11FS_VOLUME_DESC volDesc;                      /* Volume descriptor */
    SEM_ID             bioSem;                       /* I/O semaphore */
} RT11FS_DEV;

typedef struct {
    int                in_inode;                     /* Inode */
    enum vtype         in_type;                      /* Inode type */
    void  *            in_data;                      /* Pointer to inode data */
} RT11FS_INODE;

IMPORT const vfsops_t           rt11VfsOps;            /* Vfs operators */
IMPORT const vnode_ops_t        rt11Vops;              /* Vnode operators */
IMPORT const vnode_ops_t        rt11VopsDir;           /* Vnode operators */
IMPORT const vnode_ops_t        rt11VopsSyncer;        /* Vnode operators */

/*******************************************************************************
 *
 * usrRt11fsInit -
 *
 * RETURNS: OK or ERROR
 */

STATUS usrRt11fsInit(
    int maxBufs,
    int maxFiles,
    int maxEntries,
    int reserved1
    );

/******************************************************************************
 *
 * rt11fsLibInit - Initialize library
 *
 * RETURNS: OK or ERROR
 */

STATUS rt11fsLibInit(
    int maxBufs,
    int maxFiles,
    int maxEntries,
    int reserved1
    );

/***************************************************************************
 *
 * rt11fsDiskProbe - check if device is formated for rt11fs
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS rt11fsDiskProbe (
    device_t device
    );

/***************************************************************************
 *
 * rt11fsDevCreate - create rt11fs device
 *
 * RETURNS: Pointer to device, NULL otherwise
 */

RT11FS_DEV *  rt11fsDevCreate (
    char  *   pDevName,
    device_t  device,
    int       numBufs,
    int       maxFiles,
    int       maxEntries
    );

/***************************************************************************
 *
 * rt11fsDevCreate2 - create rt11fs device callback
 *
 * RETURNS: Pointer to device, NULL otherwise
 */

STATUS rt11fsDevCreate2 (
    device_t  device,
    char  *   pDevName
    );

/***************************************************************************
 *
 * rt11fsMount - mount rt11fs filesystem
 *
 * This routine attempts to mount and inititalize a rt11fs filesystem
 *
 * RETURNS: OK on success, errno otherwise
 */

int rt11fsMount (
    struct mount *pMount
    );

/*******************************************************************************
 *
 * rt11fsVolFormat - format volume
 *
 * RETURNS: OK or ERROR
 */

STATUS rt11fsVolFormat (
    char  * pathName,
    int     maxEntries
    );

/*******************************************************************************
 *
 * rt11fsInodeGet - get disk inode
 *
 * RETURNS: OK on success, errno otherwise
 */

int rt11fsInodeGet (
    struct vnode *pVnode
    );

/*******************************************************************************
 *
 * rt11fsInodeRelease - release disk inode
 *
 * RETURNS: OK on success, errno otherwise
 */

int rt11fsInodeRelease (
    struct vnode  * pVnode
    );

/***************************************************************************
 *
 * rt11fsGetDirEntry - get directory entry
 *
 * RETURNS: N/A
 */

void rt11fsGetDirEntry (
    RT11FS_DIR_SEG  *     pDirSeg,
    int                   entryNum,
    RT11FS_DIR_ENTRY  *   pDirEntry
    );

/***************************************************************************
 *
 * rt11fsPutDirEntry - store directory entry
 *
 * RETURNS: N/A
 */

void rt11fsPutDirEntry (
    RT11FS_DIR_SEG  *     pDirSeg,
    int                   entryNum,
    RT11FS_DIR_ENTRY  *   pDirEntry
    );

/*******************************************************************************
 *
 * rt11fsNameR50 - Convert ASCII string to radix-50 name
 *
 *
 * RETURNS: N/A
 */

void rt11fsNameR50 (
    char         * string,
    RT11FS_NAME  * pName
    );

/*******************************************************************************
 *
 * rt11fsNameString - Convert radix-50 name string to ASCII string
 *
 *
 * RETURNS: N/A
 */

void rt11fsNameString (
    RT11FS_NAME name,
    char  *     string
    );

/*******************************************************************************
 *
 * rt11fsAllocDirEntry - allocate a new directory entry
 *
 * RETURNS: entry number on success, otherwise ERROR
 */

int rt11fsAllocNewDirEntry (
    RT11FS_DIR_SEG  *   pDirSeg,
    int                 maxEntries,
    RT11FS_DIR_ENTRY  * pNewDirEntry,
    int               * pStartBlock,
    char  *             name
    );

/*******************************************************************************
 *
 * rt11fsInsertDirEntry - insert directory entry
 *
 * RETURNS: N/A
 */

void rt11fsInsertDirEntry (
    RT11FS_DIR_SEG  *   pDirSeg,
    int                 entryNum,
    RT11FS_DIR_ENTRY  * pDirEntry
    );

/*******************************************************************************
 *
 * rt11fsDirMergeEmpty - merge empty directory entries
 *
 *
 * RETURNS: N/A
 */

void rt11fsDirMergeEmpty (
    RT11FS_DIR_SEG  * pDirSeg,
    int               entryNum
    );

/*******************************************************************************
 *
 * rt11fsFindDirEntry - find directory entry with a given name
 *
 *
 * RETURNS: entry number on success, otherwise ERROR
 */

int rt11fsFindDirEntry (
    RT11FS_DIR_SEG  *   pDirSeg,
    RT11FS_DIR_ENTRY  * pDirEntry,
    int  *              pStartBlock,
    char  *             name
    );

/******************************************************************************
 *
 * rt11fsVolFlush - Flush volume
 *
 * RETURNS: OK or ERROR
 */

STATUS rt11fsVolFlush(
    RT11FS_VOLUME_DESC * pVolDesc
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _rt11fsLib_h */
