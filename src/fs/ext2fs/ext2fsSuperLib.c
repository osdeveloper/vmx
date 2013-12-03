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

/* ext2fsSuperLib.c - Ext2 superblock library */

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <fs/ext2fsLib.h>
#include <fs/xbd.h>

LOCAL void ext2fsSuperBlkDone (
    struct bio * pBio
    );

/******************************************************************************
 *
 * ext2fsSuperBlkRead - read ext2fs super block
 *
 * RETURNS: OK on success, otherwise error
 */

int ext2fsSuperBlkRead (
    EXT2FS_DEV * pFsDev
    ) {
    EXT2FS_SUPERBLOCK_DISK * pSuperBlkDisk;
    EXT2FS_SUPERBLOCK *      pSuperBlk;
    struct bio               bio;
    unsigned                 sectorSize;
    int                      error;

    error = xbdBlockSize (pFsDev->ext2fsVolDesc.device, &sectorSize);
    if (error != OK) {
        return (error);
    }

    bio.bio_dev     = pFsDev->ext2fsVolDesc.device;
    bio.bio_data    = pFsDev->pScratchBlk;
    bio.bio_flags   = BIO_READ;
    bio.bio_done    = ext2fsSuperBlkDone;
    bio.bio_caller1 = pFsDev->bioSem;
    bio.bio_chain   = NULL;
    bio.bio_bcount  = (sectorSize > EXT2FS_SUPERBLK_SIZE)
                          ? sectorSize
                          : EXT2FS_SUPERBLK_SIZE;
    bio.bio_blkno   = EXT2FS_SUPERBLK_SIZE / sectorSize;
    bio.bio_error   = OK;
    bio.bio_resid   = 0;

    xbdStrategy (pFsDev->ext2fsVolDesc.device, &bio);
    semTake (pFsDev->bioSem, WAIT_FOREVER);

    if (bio.bio_error != OK) {
        return (bio.bio_error);
    }

    if (bio.bio_resid != 0) {
        return (EIO);
    }

    pSuperBlkDisk = (EXT2FS_SUPERBLOCK_DISK *) pFsDev->pScratchBlk;
    if (sectorSize > EXT2FS_SUPERBLK_SIZE) {
        pSuperBlkDisk =
            (EXT2FS_SUPERBLOCK_DISK *) (((u_int8_t *) pSuperBlkDisk) +
                                        EXT2FS_SUPERBLK_SIZE);
    }

    pSuperBlk = pFsDev->ext2fsVolDesc.pSuperBlk;

    ext2fsSuperBlockDiskToHost (pSuperBlkDisk, pSuperBlk);
    ext2fsSuperBlkShow (pSuperBlk);

    return (OK);
}

/******************************************************************************
 *
 * ext2fsSuperBlkDone - super block bio operation complete
 *
 * RETURNS: N/A
 */

LOCAL void ext2fsSuperBlkDone (
    struct bio * pBio
    ) {

    semGive ((SEM_ID) pBio->bio_caller1);
}

