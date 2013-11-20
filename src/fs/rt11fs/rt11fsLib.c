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

/* rt11fsLib.c - Rt11 compatible filesystem library */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/erfLib.h>
#include <fs/bio.h>
#include <fs/xbd.h>
#include <fs/mount.h>
#include <fs/fsMonitor.h>
#include <fs/rt11fsLib.h>

/* defines */
#define RT11FS_NAME_LEN                 11

/* Forward declarations */
LOCAL int rt11fsDirSegBlocks (unsigned blkSize, int maxEntries);
LOCAL int rt11fsSectorRW (device_t  device, lblkno_t sector,
    unsigned sectorSize, unsigned flags, char *data);
LOCAL void rt11fsSectorRWDone (struct bio *bio);
LOCAL int rt11fsR50out (char *string);
LOCAL void rt11fsR50in (unsigned int r50, char *string);
LOCAL void rt11fsSwapBytes (char *pSrc, char *pDest, size_t nBytes);

/* Locals */
BOOL rt11fsLibInstalled = FALSE;
LOCAL char rad50[] = " abcdefghijklmnopqrstuvwxyz$.\3770123456789";

/* Globals */
int rt11fsMaxBuffers = 0;
int rt11fsMaxFiles = 0;
int rt11fsMaxEntries = 0;

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
    ) {
    int i;

    /* Store globals */
    rt11fsMaxBuffers = maxBufs;
    rt11fsMaxFiles   = maxFiles;
    rt11fsMaxEntries = maxEntries;

    rt11fsLibInstalled = TRUE;
    return (OK);
}

/***************************************************************************
 *
 * rt11fsEject - eject the rt11 file system
 *
 * RETURNS: N/A
 */

LOCAL void rt11fsEject (
    int     category,
    int     type,
    void *  pEventData,   /* device to eject */
    void *  pUserData     /* ptr to device structure */
    ) {
    device_t     device;
    RT11FS_DEV * pFsDev;

    if ((category != xbdEventCategory) ||
        ((type != xbdEventRemove) && (type != xbdEventMediaChanged))) {
        return;
    }

    device = (device_t) pEventData;
    pFsDev = (RT11FS_DEV *) pUserData;

    /* If this event is not for us, then return */
    if (pFsDev->volDesc.vd_device != device) {
        return;
    }

    /*
     * Unregister the registered events.  Then inform the vnode layer to
     * unmount the rt11 file system.
     */

    erfHandlerUnregister (xbdEventCategory, xbdEventRemove,
                          rt11fsEject, pFsDev);
    erfHandlerUnregister (xbdEventCategory, xbdEventMediaChanged,
                          rt11fsEject, pFsDev);


    mountEject (pFsDev->volDesc.vd_pMount, type == xbdEventMediaChanged);
}

/***************************************************************************
 *
 * rt11fsDiskProbe - check if device is formated for rt11fs
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS rt11fsDiskProbe (
    device_t device
    ) {
    RT11FS_DIR_SEG  * pDirSeg;
    lblkno_t          nBlks;
    size_t            len;
    long long         totalSize;
    unsigned          sectorSize;
    int               i, nSegBlks;
    int               error;

    /* Get setup from device */
    if (((error = xbdBlockSize (device, &sectorSize)) != OK) ||
        ((error = xbdNBlocks (device, &nBlks)) != OK) ||
        ((error = xbdSize (device, &totalSize)) != OK)) {
        errnoSet (error);
        return (ERROR);
    }

    /* Calculate root directory size */
    nSegBlks = rt11fsDirSegBlocks (sectorSize, rt11fsMaxEntries);

    /* Calculate super-block length */
    len = nSegBlks * sectorSize;

    /* Allocate memory for super-block */
    pDirSeg = (RT11FS_DIR_SEG *) bio_alloc (device, nSegBlks);
    if (pDirSeg == NULL) {
        return (ERROR);
    }

    /* Read super-block */
    if ((error = rt11fsSectorRW (device, RT11FS_DIR_BLOCK, len,
                                 BIO_READ, (char *) pDirSeg)) != OK) {
        errnoSet (error);
        bio_free (pDirSeg);
        return (ERROR);
    }

    /* Swap bytes */
    rt11fsSwapBytes((char *) pDirSeg, (char *) pDirSeg, len);

    /* Check directory segment */
    if ((pDirSeg->ds_nsegs == 1) && (pDirSeg->ds_next_seg == 0) &&
        (pDirSeg->ds_last_seg == 1) && (pDirSeg->ds_extra == 0)) {
        bio_free (pDirSeg);
        return (OK);
    }

    bio_free (pDirSeg);
    return (ERROR);
}

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
    ) {
    mount_t  *    pMount;
    RT11FS_DEV  * pFsDev;
    int           error;

    if ((!rt11fsLibInstalled) || (pDevName == NULL) ||
        (pDevName[0] == '\0')) {
        errnoSet (EINVAL);
        return (NULL);
    }

    if (numBufs < RT11FS_MIN_BUFFERS) {
        numBufs = RT11FS_MIN_BUFFERS;
    }

    error = mountCreate (&rt11VfsOps, device, maxFiles, pDevName, &pMount);
    if (error != OK) {
        errnoSet (error);
        return (NULL);
    }

    pFsDev = (RT11FS_DEV *) pMount->mnt_data;
    mountBufFree (pMount);

    error = mountBufAlloc (pMount, numBufs, pFsDev->volDesc.vd_blkSize);
    if (error != OK) {
        mountDelete (pMount, 0);
        errnoSet (error);
        return (NULL);
    }

    return (pFsDev);
}

/***************************************************************************
 *
 * rt11fsDevCreate2 - create rt11fs device callback
 *
 * RETURNS: Pointer to device, NULL otherwise
 */

STATUS rt11fsDevCreate2 (
    device_t  device,
    char  *   pDevName
    ) {
    RT11FS_DEV  * pFsDev;
    int           fd;

    pFsDev = rt11fsDevCreate (pDevName, device, rt11fsMaxBuffers,
                              rt11fsMaxFiles, rt11fsMaxEntries);
    if (pFsDev == NULL) {
        return (ERROR);
    }

    fd = open (pDevName, O_RDONLY, 0777);
    if (fd < 0) {
        goto errorReturn;
    }

    close (fd);

    if ((erfHandlerRegister (xbdEventCategory, xbdEventRemove,
                             rt11fsEject, pFsDev, 0) != OK) ||
        (erfHandlerRegister (xbdEventCategory, xbdEventMediaChanged,
                             rt11fsEject, pFsDev, 0) != OK)) {
        goto errorReturn;
    }

    xbdIoctl (device, XBD_STACK_COMPLETE, NULL);
    fsPathAddedEventRaise (pDevName);
    return (OK);

errorReturn:

    return (ERROR);
}

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
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    lblkno_t              nBlks;
    size_t                len;
    long long             totalSize;
    unsigned              sectorSize;
    int                   i;
    int                   error;

    pFsDev = (RT11FS_DEV *) pMount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    memset (pFsDev, 0, sizeof(RT11FS_DEV));

    /* Get setup from device */
    if (((error = xbdBlockSize (pMount->mnt_dev, &sectorSize)) != OK) ||
        ((error = xbdNBlocks (pMount->mnt_dev, &nBlks)) != OK) ||
        ((error = xbdSize (pMount->mnt_dev, &totalSize)) != OK)) {
        return (error);
    }

    /* Inititalize syncer vnode */
    pMount->mnt_syncer->v_ops = (struct vnode_ops *) &rt11VopsSyncer;

    /* Inititalize semaphore */
    pFsDev->bioSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
    if (pFsDev->bioSem == NULL) {
        return (ENOBUFS);
    }

    /* Inititalize volume desciptor */
    pVolDesc->vd_pMount = pMount;
    pVolDesc->vd_device = pMount->mnt_dev;
    pVolDesc->vd_blkSize = sectorSize;
    pVolDesc->vd_blkSize2 = ffsMsb(pVolDesc->vd_blkSize) - 1;
    pVolDesc->vd_secPerBlk2 = 1 + pVolDesc->vd_blkSize2 - ffsMsb(sectorSize);
    pVolDesc->vd_secPerBlk = (1 << pVolDesc->vd_blkSize2);
    pVolDesc->vd_nBlks = nBlks;
    pVolDesc->vd_maxEntries = rt11fsMaxEntries;
    pVolDesc->vd_diskSize = totalSize;

    /* Calculate root directory size */
    pVolDesc->vd_nSegBlks = rt11fsDirSegBlocks (sectorSize,
                                                pVolDesc->vd_maxEntries);

    /* Calculate super-block length */
    len = pVolDesc->vd_nSegBlks * pVolDesc->vd_blkSize;

    /* Allocate memory for super-block */
    pVolDesc->vd_pDirSeg = (RT11FS_DIR_SEG *) bio_alloc (pMount->mnt_dev,
                                                         pVolDesc->vd_nSegBlks);
    if (pVolDesc->vd_pDirSeg == NULL) {
        return (ENOMEM);
    }

    /* Read super-block */
    if ((error = rt11fsSectorRW (pVolDesc->vd_device, RT11FS_DIR_BLOCK, len,
                                 BIO_READ,
                                 (char *) pVolDesc->vd_pDirSeg)) != OK) {
        free (pVolDesc->vd_pDirSeg);
        return (error);
    }

    /* Swap bytes */
    rt11fsSwapBytes((char *) pVolDesc->vd_pDirSeg,
                    (char *) pVolDesc->vd_pDirSeg,
                    len);

    return (OK);
}

/*******************************************************************************
 *
 * rt11fsVolFormat - format volume
 *
 * RETURNS: OK or ERROR
 */

STATUS rt11fsVolFormat (
    char  * pathName,
    int     maxEntries
    ) {
    struct stat       st;
    RT11FS_DIR_SEG  * pDirSeg;
    size_t            len;
    devname_t         devname;
    device_t          device;
    int               fd, i, nSegBlks, freeBlks;
    STATUS            status;

    if (!rt11fsLibInstalled) {
        return (ERROR);
    }

    if (maxEntries == 0) {
        maxEntries = rt11fsMaxEntries;
    }

    fd = open (pathName, O_RDWR, 0777);
    if (fd == ERROR) {
        return (ERROR);
    }

    /* Get volume status */
    if (fstat (fd, &st) != OK) {
        close (fd);
        return (ERROR);
    }

    /* Calculate root directory size */
    nSegBlks = rt11fsDirSegBlocks (st.st_blksize, maxEntries);

    /* Calculate super-block size */
    len = nSegBlks * st.st_blksize;

    /* Allocate memory for directory segment */
    pDirSeg = (RT11FS_DIR_SEG *) malloc(len);
    if (pDirSeg == NULL) {
        close (fd);
        return (ERROR);
    }

    /* Initialize directory segment */
    pDirSeg->ds_nsegs = 1;
    pDirSeg->ds_next_seg = 0;
    pDirSeg->ds_last_seg = 1;
    pDirSeg->ds_extra = 0;

    /* Fist data block */
    pDirSeg->ds_start = RT11FS_DIR_BLOCK + nSegBlks;

    /* Free blocks */
    freeBlks = st.st_blocks - pDirSeg->ds_start;

    /* Inititalize all directory entries */
    i = 0;
    while (freeBlks > RT11FS_MAX_BLOCKS_PER_FILE) {
        pDirSeg->ds_entries[i].de_status = DES_EMPTY;
        pDirSeg->ds_entries[i++].de_nblocks = RT11FS_MAX_BLOCKS_PER_FILE;
        freeBlks -= RT11FS_MAX_BLOCKS_PER_FILE;
    }

    /* Initialize last directory entry */
    if (freeBlks > 0) {
        pDirSeg->ds_entries[i].de_status = DES_EMPTY;
        pDirSeg->ds_entries[i++].de_nblocks = freeBlks;
    }

    /* Terminating entry */
    pDirSeg->ds_entries[i].de_status = DES_END;

    /* Swap bytes */
    rt11fsSwapBytes((char *) pDirSeg, (char *) pDirSeg, len);

    /* Goto super-block position on disk */
    lseek (fd, RT11FS_DIR_BLOCK * st.st_blksize, SEEK_SET);

    /* Write super-block to disk */
    if (write (fd, pDirSeg, len) != len) {
        free (pDirSeg);
        close (fd);
        return (ERROR);
    }

    free (pDirSeg);

    /* Eject device */
    if ((status = ioctl (fd, XBD_HARD_EJECT, 0)) != OK) {
        close (fd);
        return (status);
    }

    close (fd);

    if ((fsmGetDriver (pathName, devname, XBD_MAX_NAMELEN) != OK) ||
        (xbdDevFind (&device, devname) != OK)) {
        return (ERROR);
    }

    if (erfEventRaise (xbdEventCategory, xbdEventSecondaryInsert,
                       ERF_ASYNC_PROCESS, (void *) device, NULL) != OK) {
        return (ERROR);
    }

    return (status);
}

/*******************************************************************************
 *
 * rt11fsInodeGet - get disk inode
 *
 * RETURNS: OK on success, errno otherwise
 */

int rt11fsInodeGet (
    struct vnode *pVnode
    ) {
    RT11FS_DEV  *         pFsDev;
    RT11FS_VOLUME_DESC  * pVolDesc;
    RT11FS_DIR_DESC  *    pDirDesc;
    RT11FS_FILE_DESC  *   pFileDesc;
    RT11FS_INODE  *       pInode;

    pFsDev = (RT11FS_DEV *) pVnode->v_mount->mnt_data;
    pVolDesc = &(pFsDev->volDesc);

    RT11FS_VTOI (pInode, pVnode);

    /* If root directory inode */
    if (pInode->in_inode == RT11FS_ROOT_INODE) {
        pDirDesc = (RT11FS_DIR_DESC *) malloc (sizeof (RT11FS_DIR_DESC));
        if (pDirDesc == NULL) {
            return (ENOMEM);
        }

        /* Inititalize directory descriptor */
        pDirDesc->rdd_maxEntries = pVolDesc->vd_maxEntries;
        pDirDesc->rdd_pDirSeg = pVolDesc->vd_pDirSeg;

        pInode->in_type = VDIR;
        pInode->in_data = pDirDesc;
    }

    /* Else file inode */
    else {
        pFileDesc = (RT11FS_FILE_DESC *) malloc (sizeof (RT11FS_FILE_DESC));
        if (pFileDesc == NULL) {
            return (ENOMEM);
        }

        /* Inititalize file descriptor */
        memset (pFileDesc, 0, sizeof(RT11FS_FILE_DESC));

        pInode->in_type = VREG;
        pInode->in_data = pFileDesc;
    }

    return (OK);
}

/*******************************************************************************
 *
 * rt11fsInodeRelease - release disk inode
 *
 * RETURNS: OK on success, errno otherwise
 */

int rt11fsInodeRelease (
    struct vnode  * pVnode
    ) {
    RT11FS_INODE  * pInode;

    RT11FS_VTOI (pInode, pVnode);

    return (OK);
}


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
    ) {

    memcpy (pDirEntry,
            &pDirSeg->ds_entries[entryNum],
            sizeof (RT11FS_DIR_ENTRY));
}

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
    ) {

    pDirSeg->ds_entries[entryNum] = *pDirEntry;
}

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
    ) {
    char    tmp[RT11FS_NAME_LEN];
    char  * cp;

    strncpy (tmp, string, RT11FS_NAME_LEN - 1);
    tmp[RT11FS_NAME_LEN - 1] = EOS;

    /* Find dot in filenmae */
    cp = strchr (tmp, '.');

    if (cp == NULL) {
        pName->nm_type = 0;
    }
    else {
        *cp = EOS;
        pName->nm_type = rt11fsR50out (cp + 1);
    }

    /* Convert rest of filename */
    pName->nm_name1 = rt11fsR50out (tmp);

    if (strlen (tmp) <= 3) {
        pName->nm_name2 = 0;
    }
    else {
        pName->nm_name2 = rt11fsR50out (tmp + 3);
    }
}

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
    ) {
    char  * pString;

    rt11fsR50in (name.nm_name1, string);
    rt11fsR50in (name.nm_name2, string + 3);

    for (pString = string;
         (pString < (string + 6)) && (*pString != ' ');
         ++pString);

    *pString++ = '.';

    rt11fsR50in (name.nm_type, pString);

    *(pString + 3) = EOS;
}

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
    ) {
    RT11FS_DIR_ENTRY  dirEntry, dirEntryMax;
    int               i, iMax, startBlock, startBlockMax;

    startBlock = pDirSeg->ds_start;

    /* Search for biggest free entry */
    for (i = 0, iMax = ERROR; i < maxEntries; i++) {
        rt11fsGetDirEntry (pDirSeg, i, &dirEntry);

        if (dirEntry.de_status == DES_END) {
            break;
        }

        if (dirEntry.de_status == DES_TENTATIVE) {
            return (ERROR);
        }

        if (dirEntry.de_status == DES_EMPTY) {
            if ((iMax == ERROR) ||
                (dirEntry.de_nblocks > dirEntryMax.de_nblocks)) {
                iMax = i;
                dirEntryMax = dirEntry;
                startBlockMax = startBlock;
            }
        }

        /* Inclrease start block */
        startBlock += dirEntry.de_nblocks;
    }

    /* Maximum number of directory entries reached */
    if ((i >= maxEntries - 1) || (iMax == ERROR)) {
        return (ERROR);
    }

    /* Inititalize entry */
    memcpy (pNewDirEntry, &dirEntryMax, sizeof(RT11FS_DIR_ENTRY));
    pNewDirEntry->de_status = DES_TENTATIVE;
    rt11fsNameR50 (name, &pNewDirEntry->de_name);

    /* Set start block */
    *pStartBlock = startBlockMax;

    return (iMax);
}

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
    ) {
    RT11FS_DIR_ENTRY currEntry, prevEntry;

    /* Replace current entry with new */
    rt11fsGetDirEntry (pDirSeg, entryNum, &currEntry);
    rt11fsPutDirEntry (pDirSeg, entryNum, pDirEntry);

    /* Replace entry with previous entry */
    while (currEntry.de_status != DES_END) {
        prevEntry = currEntry;
        entryNum++;
        rt11fsGetDirEntry (pDirSeg, entryNum, &currEntry);
        rt11fsPutDirEntry (pDirSeg, entryNum, &prevEntry);
    }

    /* Put end indicator in next slot */
    rt11fsPutDirEntry (pDirSeg, entryNum + 1, &currEntry);
}

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
    ) {
    RT11FS_DIR_ENTRY currEntry, nextEntry;
    int i = 0;

    /* Get current entry */
    rt11fsGetDirEntry (pDirSeg, --entryNum, &currEntry);
    if ((entryNum < 0) || (currEntry.de_status != DES_EMPTY)) {
        rt11fsGetDirEntry (pDirSeg, ++entryNum, &currEntry);
    }

    /* Get next entry */
    rt11fsGetDirEntry (pDirSeg, entryNum + 1, &nextEntry);

    while (nextEntry.de_status == DES_EMPTY) {
        if ((currEntry.de_nblocks + nextEntry.de_nblocks) >
             RT11FS_MAX_BLOCKS_PER_FILE) {
            if (i > 0) {
                break;
            }
            else {
                entryNum++;
                currEntry = nextEntry;
            }
        }
        else {
            currEntry.de_nblocks += nextEntry.de_nblocks;
            ++i;
        }

        /* Get next entry */
        rt11fsGetDirEntry (pDirSeg, entryNum + 1 + i, &nextEntry);
    }

    /* If any entries where merged */
    if (i > 0) {
        rt11fsPutDirEntry (pDirSeg, entryNum, &currEntry);
        rt11fsPutDirEntry (pDirSeg, ++entryNum, &nextEntry);

        while (nextEntry.de_status != DES_END) {
            rt11fsGetDirEntry (pDirSeg, ++entryNum + i, &nextEntry);
            rt11fsPutDirEntry (pDirSeg, entryNum, &nextEntry);
        }
    }
}

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
    ) {
    RT11FS_DIR_ENTRY dirEntry;
    RT11FS_NAME      rname;
    int              i;

    /* Remove leading slashes */
    while ((*name == '/') || (*name == '\\')) {
        name++;
    }

    /* Get radix-50 name */
    rt11fsNameR50 (name, &rname);

    /* Inititalize starting block */
    *pStartBlock = pDirSeg->ds_start;

    /* Search for entry */
    for (i = 0; ; i++) {
        rt11fsGetDirEntry (pDirSeg, i, &dirEntry);
        if (dirEntry.de_status == DES_END) {
            return (ERROR);
        }

        /* Check file */
        if (((dirEntry.de_status == DES_PERMANENT) ||
             (dirEntry.de_status == DES_TENTATIVE)) &&
            ((dirEntry.de_name.nm_name1 == rname.nm_name1) &&
                (dirEntry.de_name.nm_name2 == rname.nm_name2) &&
                (dirEntry.de_name.nm_type == rname.nm_type))) {
                /* Copy directory entry if wanted */
            if (pDirEntry != NULL) {
                memcpy (pDirEntry, &dirEntry, sizeof(RT11FS_DIR_ENTRY));
            }
            return (i);
        }

        /* Increase starting block */
        *pStartBlock += dirEntry.de_nblocks;
    }

    return (ERROR);
}

/******************************************************************************
 *
 * rt11fsVolFlush - Flush volume
 *
 * RETURNS: OK or ERROR
 */

STATUS rt11fsVolFlush(
    RT11FS_VOLUME_DESC * pVolDesc
    )
{
    int               error;
    int               len;
    char *            pSegTop;
    RT11FS_DIR_SEG  * pDirSeg;

    /* Initialize locals */
    len = pVolDesc->vd_nSegBlks * pVolDesc->vd_blkSize;
    pSegTop = (char *) pVolDesc->vd_pDirSeg;

    /* Allocate memory for directory block */
    pDirSeg = (RT11FS_DIR_SEG *) bio_alloc (pVolDesc->vd_device,
                                            pVolDesc->vd_nSegBlks);
    if (pDirSeg == NULL) {
        return (ERROR);
    }

    /* Copy current superblock */
    memcpy (pDirSeg, pSegTop, len);

    /* Swap bytes */
    rt11fsSwapBytes ((char *) pDirSeg, (char *) pDirSeg, len);

    /* Write superblock to device */
    if ((error = rt11fsSectorRW (pVolDesc->vd_device, RT11FS_DIR_BLOCK, len,
                                 BIO_WRITE, (char *) pDirSeg)) != OK) {
        errnoSet (error);
        bio_free (pDirSeg);
        return (ERROR);
    }

    bio_free (pDirSeg);

    return (OK);
}

/***************************************************************************
 *
 * rt11fsDirSegBlock - calculate number of blocks that root dir occupies
 *
 * RETURNS: number of blocks
 */

LOCAL int rt11fsDirSegBlocks (
    unsigned blkSize,
    int      maxEntries
    ) {
    int i, seglen;

    /* Calculate root directory size */
    seglen = sizeof(RT11FS_DIR_SEG) +
        (maxEntries - 1) * sizeof(RT11FS_DIR_ENTRY);
    if ((i = 1 + (seglen / blkSize)) < 2) {
        return (2);
    }

    return (i);
}

/***************************************************************************
 *
 * rt11fsSectorRW - read or write sector from the underlying XBD device
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int rt11fsSectorRW (
    device_t  device,       /* lower layer XBD device */
    lblkno_t  sector,       /* sector number to read */
    unsigned  sectorSize,   /* # of bytes in the sector */
    unsigned  flags,        /* BIO_READ or BIO_WRITE */
    char *    data          /* pointer to data to read */
    ) {
    SEMAPHORE  sem;
    bio_t      bio;

    semBInit (&sem, SEM_Q_PRIORITY, SEM_EMPTY);

    bio.bio_dev     = device;               /* device to read */
    bio.bio_data    = data;                 /* data buffer */
    bio.bio_flags   = flags;                /* flag it as a read operation */
    bio.bio_chain   = NULL;                 /* no subsequent bios */
    bio.bio_bcount  = sectorSize;           /* sector size (bytes to read) */
    bio.bio_blkno   = sector;               /* sector # to read */
    bio.bio_done    = rt11fsSectorRWDone;   /* rtn to call when done reading */
    bio.bio_caller1 = &sem;                 /* semaphore on which to wait */
    bio.bio_error   = OK;
    bio.bio_resid   = 0;

    xbdStrategy (device, &bio);
    semTake (&sem, WAIT_FOREVER);           /* wait for completion */

    return (bio.bio_error);
}

/***************************************************************************
 *
 * rt11fsSectorRWDone -
 *
 * RETURNS: N/A
 */

LOCAL void rt11fsSectorRWDone (
    struct bio *  bio
    ) {
    semGive ((SEM_ID) bio->bio_caller1);
}

/*******************************************************************************
 *
 * rt11fsR50out - Convert up to 3 ASCII chars to radix-50
 *
 * RETURNS: Radix-50 number
 */

LOCAL int rt11fsR50out (
    char  * string
    ) {
    unsigned int r50 = 0;
    int          i, r;
    char         ch;

    for (i = 0; i < 3; i++) {
        if (*string == EOS) {
            r = 0;
        }

        else {
            ch = *string;

            /* Convert to lowercase */
            if (isupper (ch)) {
                ch = tolower (ch);
            }

            r = (char *) strchr (rad50, ch) - rad50;
            string++;
        }
        
        r50 = (50 * r50) + r;
  }

    return (r50);
}

/*******************************************************************************
 *
 * rt11fsR50in - Convert radix-50 to 3 ASCII chars
 *
 * RETURNS: N/A
 */

LOCAL void rt11fsR50in (
    unsigned int r50,
    char  *      string
    ) {
    int i;

    for (i = 2; i >= 0; i--) {
        string[i] = rad50[r50 % 50];
        r50 /= 50;
    }
}

/*******************************************************************************
 *
 * rt11fsSwapBytes -
 *
 * RETURNS: N/A
 */

LOCAL void rt11fsSwapBytes (
    char  * pSrc,
    char  * pDest,
    size_t  nBytes) {
    short *src, *dst, *dst_end;

    /* Initialize locals */
    src = (short *) pSrc;
    dst = (short *) pDest;
    dst_end = (short *) dst + (nBytes / 2);

    for (; dst < dst_end; dst++, src++)
        *dst = ((*src & 0x00ff) << 8) | ((*src & 0xff00) >> 8);
}

