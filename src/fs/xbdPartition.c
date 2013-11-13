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

/* xbdPartition.c */

/*
DESCRIPTION
  For a good reference on partitions and how they are supposed to be set up,
please refer to the following webpages ....

http://www.nondot.org/sabre/os/files/Disk/CHSTranslation.txt
http://www.nondot.org/sabre/os/files/Partitions/PartitionTables.txt

These were the pages that were used as reference material when coding this
module.

  Example partitioning of Lower-Layer XBD Device:
  +----------------+----------------------------+-------------+
  |   Part #1      |         Part #2            |  Part #3    |
  +----------------+----------------------------+-------------+
  |              Lower-Layer XBD Device                       |
  +-----------------------------------------------------------+

Each XBD request comes from an upper layer, and get funnelled down to the
lower layer as necessary.  In the above diagram, there are three partitions;
each partition is an XBD device.  As each XBD partition shares the underlying
lower-layer XBD device, it is necessary to be able to identify which 
*/

/* includes */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <os/erfLib.h>
#include <fs/bio.h>
#include <fs/xbd.h>
#include <fs/fsEventUtilLib.h>

/* defines */

/*
 * This semi-arbitrary limit for the number of XBD partitions comes from the
 * fact that there are 26 letters in the alphabet.  Unless extended partitions
 * are used, there will not be more than four (4) partitions per device.  When
 * extended partitions are eventually implemented, it is not expected that the
 * total number of partitions will exceed 26.
 */
#define XBD_MAX_PARTS    26     /* semi-arbitrary limit */

/* typedefs */

typedef struct partition_entry {
    u_char  bootIndicator;     /* Boot indicator: 0x80--used for booting */
    u_char  headStart;         /* Starting head for the partition */
    u_char  secStart;          /* Starting sector for the partition */
    u_char  cylStart;          /* Starting cylinder for the partition */

    u_char  systemIndicator;   /* System indicator (F/S type hint) */
    u_char  headEnd;           /* Ending head for the partition */
    u_char  secEnd;            /* Ending sector for the partition */
    u_char  cylEnd;            /* Ending cylinder for the partition */

    u_int   sector;            /* Starting sector of partition */
    u_int   nSectors;          /* # of sectors in the partition */
} PARTITION_ENTRY;

typedef struct xbd_partition  XBD_PARTITION;

typedef struct xbd_part {
    XBD              xbd;
    device_t         device;
    BOOL             isBootable;       /* TRUE if bootable */
    unsigned         head[2];          /* [0]: start value, [1]: end value */
    unsigned         sector[2];        /* [0]: start value, [1]: end value */
    unsigned         cylinder[2];      /* [0]: start value, [1]: end value */
    lblkno_t         startBlock;       /* starting block */
    lblkno_t         nBlocks;          /* number of blocks in the partition */
    XBD_PARTITION *  xbdPartitions;    /* back pointer */
} XBD_PART;

struct xbd_partition {
    device_t  subDevice;                /* ID's lower layer XBD device */
    int       nPartitions;              /* number of partitions */
    XBD_PART  xbdParts[XBD_MAX_PARTS];  /* list of XBD partitions */
};

/* forward declarations */
LOCAL int xbdPartitionIoctl(XBD *xbd, unsigned command, void *data);
LOCAL int xbdPartitionStrategy(XBD *xbd, struct bio *pBio);
LOCAL int xbdPartitionDump(XBD *xbd, lblkno_t lblkno,
    void *data, size_t nBytes);

LOCAL int xbdSectorRead (device_t  device, lblkno_t sector,
    unsigned sectorSize, char *data);
LOCAL void xbdSectorReadDone (struct bio *bio);
LOCAL STATUS xbdPartInit (XBD_PART *pXbdPart, XBD_GEOMETRY *pXbdGeometry,
    lblkno_t startBlock, lblkno_t endBlock);
LOCAL int xbdPartitionsDetect (device_t device);
LOCAL void xbdPartitionHandler (int category, int type,
    void *pEventData, void *pUserData);
LOCAL void xbdPartitionEject (int category, int type,
    void *pEventData, void *pUserData);

/* locals */
LOCAL XBD_FUNCS  xbdPartitionFuncs = {
    &xbdPartitionIoctl,
    &xbdPartitionStrategy,
    &xbdPartitionDump,
};

LOCAL BOOL xbdPartitionLibInstalled = FALSE;

/***************************************************************************
 *
 * xbdPartitionLibInit -
 *
 * RETURNS: OK on success, error otherwise
 */

STATUS xbdPartitionLibInit (void) {
    if (erfHandlerRegister (xbdEventCategory, xbdEventPrimaryInsert,
                            xbdPartitionHandler, NULL, 0) != OK) {
        return (ERROR);
    }

    xbdPartitionLibInstalled = TRUE;

    return (OK);
}

/***************************************************************************
 *
 * xbdPartitionDump -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdPartitionDump (
    XBD *     xbd,
    lblkno_t  lblkno,
    void *    data,
    size_t    nBytes
    ) {
    XBD_PART *  xbdPart = (XBD_PART *) xbd;
    int         rv;

    if ((lblkno < 0) || (lblkno > xbdPart->nBlocks)) {
        return (EINVAL);
    }

    lblkno += xbdPart->startBlock;

    rv = xbdDump (xbdPart->xbdPartitions->subDevice, lblkno, data, nBytes);

    return (rv);
}

/***************************************************************************
 *
 * xbdPartitionStrategy -
 *
 * RETURNS OK on success, error otherwise
 */

LOCAL int xbdPartitionStrategy (
    XBD *         xbd,
    struct bio *  pBio
    ) {
    XBD_PART *  xbdPartition = (XBD_PART *) xbd;
    lblkno_t    blkOffset;
    bio_t *     bio;
    int         rv;

    /* Get block offset for partition */
    blkOffset = xbdPartition->startBlock;

    /* Add block offset to all block numbers */
    for (bio = pBio; bio != NULL; bio = bio->bio_chain) {
        bio->bio_blkno += blkOffset;
    }

    rv = xbdStrategy (xbdPartition->xbdPartitions->subDevice, pBio);

    return (rv);
}

/***************************************************************************
 *
 * xbdPartitionIoctl -
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int  xbdPartitionIoctl (
    XBD *     xbd,
    unsigned  command,
    void *    data
    ) {
    XBD_PART *      xbdPartition = (XBD_PART *) xbd;
    int             rv = OK;
    XBD_GEOMETRY  * pGeometry;

    switch (command) {
        case XBD_GEOMETRY_GET:
            pGeometry = (XBD_GEOMETRY *) data;
            if ((rv = xbdIoctl (xbdPartition->xbdPartitions->subDevice,
                                XBD_GEOMETRY_GET, pGeometry)) != OK) {
                return (rv);
            }
            pGeometry->numHeads = xbdPartition->head[1] -
                                  xbdPartition->head[0] + 1;
            pGeometry->numCylinders = xbdPartition->cylinder[1] -
                                      xbdPartition->cylinder[0] + 1;
            break;

        default:
            /* Pass down to lower layer */
            rv = xbdIoctl (xbdPartition->xbdPartitions->subDevice,
                           command, data);
            break;
    }

    return (rv);
}

/***************************************************************************
 *
 * xbdCreatePartition- partition an XBD device
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS xbdCreatePartition (
    char *  pathName,     /* path to device to partition */
    int     nPartitions,  /* number of partitions to create */
    int     size1,        /* % of device for first partition */
    int     size2,        /* % of device for second partition */
    int     size3         /* % of device for third partition */
    ) {
    /*
     * Stub routine.   Eventually will need to ...
     * 
     * 1.  Eject/unmount any file system currently residing on each partition.
     * 2.  Eject/unmount the partitions.
     * 3.  This should leave an unpartitioned device that uses rawFS.
     *     Hmmm ... if not rawFS, then directly access underlying device
     *     with XBDs.
     * 4.  Create the new partitions using rawFS (or XBDs if required).
     */
    devname_t           devname, devnameBase, devnameRoot;
    device_t            device, deviceBase;
    lblkno_t            nBlks;
    lblkno_t            nBlks1, nBlks2, nBlks3;
    XBD_GEOMETRY        xbdGeometry;
    PARTITION_ENTRY     partTable[4];
    XBD_PART  *         pXbdPart;
    char                nStr[3];
    FS_PATH_WAIT_STRUCT pathWait;
    char                volName[PATH_MAX];
    char  *             pc;
    int                 fd, i, len, bytesWritten;
    int                 error;

    /* Get XBD driver name */
    if (fsmGetDriver (pathName, devnameRoot, XBD_MAX_NAMELEN) != OK) {
        return (ERROR);
    }

    /* Check if partitioning is supported and extract root name */
    if (((pc = strrchr (devnameRoot, ':')) != NULL) &&
         (isdigit(pc[1]))) {
        *pc = EOS;
    }
    else {
        return (ERROR);
    }

    /* Get root partition device */
    strcpy (devnameBase, devnameRoot);
    strcat (devnameBase, ":0");
    if ((error = xbdDevFind(&deviceBase, devnameBase)) != OK) {
        errnoSet (error);
        return (ERROR);
    }

    /* Hard eject all partition devices */
    for (i = 1; i < XBD_MAX_PARTS; i++) {
        strcpy (devname, devnameRoot);
        strcat (devname, ":");
        strcat (devname, itoa(i, nStr, 10));

        if (xbdDevFind(&device, devname) != OK) {
            break;
        }

        if ((error = xbdIoctl (device, XBD_HARD_EJECT, NULL)) != OK) {
            errnoSet (error);
            return (ERROR);
        }
    }

    /* Soft eject device, which means it will be re-mounted with rawFS */
    if ((error = xbdIoctl (deviceBase, XBD_SOFT_EJECT, NULL)) != OK) {
        errnoSet (error);
        return (ERROR);
    }

    /* Get number of blocks and device geometry */
    if (((error = xbdNBlocks (deviceBase, &nBlks)) != OK) ||
        ((error = xbdIoctl (deviceBase, XBD_GEOMETRY_GET,
                            (void *) &xbdGeometry)) != OK)) {
        errnoSet (error);
        return (ERROR);
    }

    /* Clear partition table */
    len = sizeof (XBD_PART) * 4;

    /* Allocate and clear */
    pXbdPart = (XBD_PART *) malloc (len);
    if (pXbdPart == NULL) {
        return (ERROR);
    }

    memset (pXbdPart, 0, len);

    if (nPartitions == 0) {
        xbdPartInit (pXbdPart, &xbdGeometry, 1, nBlks + 1);
    }
    else if (nPartitions == 1) {
        nBlks1 = (size1 * nBlks) / 100;
        xbdPartInit (pXbdPart, &xbdGeometry, 1, nBlks1 + 1);
    }
    else {
        nBlks1 = (size1 * nBlks) / 100;
        nBlks2 = (size2 * nBlks) / 100;
        nBlks3 = (size3 * nBlks) / 100;
        xbdPartInit (&pXbdPart[0], &xbdGeometry,
                     1,
                     nBlks1 + 1);
        xbdPartInit (&pXbdPart[1], &xbdGeometry,
                     nBlks1 + 2,
                     nBlks1 + nBlks2 + 1);
        xbdPartInit (&pXbdPart[2], &xbdGeometry,
                     nBlks1 + nBlks2 + 2,
                     nBlks1 + nBlks2 + nBlks3 + 1);
    }

    for (i = 0; i < 4; i++) {
        partTable[i].bootIndicator = (u_char) ((pXbdPart->isBootable != 0x00) &
                                               0x80);
        partTable[i].headStart = (u_char) pXbdPart->head[0];
        partTable[i].headEnd = (u_char) pXbdPart->head[1];
        partTable[i].secStart = (u_char) pXbdPart->sector[0];
        partTable[i].secEnd = (u_char) pXbdPart->sector[1];
        partTable[i].cylStart = (u_char) pXbdPart->cylinder[0];
        partTable[i].cylEnd = (u_char) pXbdPart->cylinder[1];
        partTable[i].sector = (u_int) pXbdPart->startBlock;
        partTable[i].nSectors = (u_int) pXbdPart->nBlocks;

        /* Advance to next partition entry */
        pXbdPart++;
    }

    /* Free temporary storage */
    free (pXbdPart - 3);

    /* Calculate partition table length */
    len = sizeof(PARTITION_ENTRY) * 4;

    /* Get path for the underlying device */
    if (fsmGetVolume (devnameBase, volName, PATH_MAX) != OK) {
        return (ERROR);
    }

    /* Setup path to wait for */
    if (fsPathAddedEventSetup(&pathWait, volName) != OK) {
        return (ERROR);
    }

#ifdef DEBUG_XBD_PART
    printf("Wait for path %s...", volName);
#endif

    /* Wait for soft insert to complete */
    if (fsWaitForPath (&pathWait) != OK) {
        return (ERROR);
    }

#ifdef DEBUG_XBD_PART
    printf("Done.\n");
#endif

    /* Open device, which now is mounted with rawFS */
    fd = open (volName, O_RDWR, 0777);
    if (fd == ERROR) {
        return (ERROR);
    }

    /* Goto partition table offset */
    lseek (fd, 0x1be, SEEK_SET);

    /* Write partition table */
    bytesWritten = write (fd, partTable, len);
    if (bytesWritten != len) {
        close (fd);
        return (ERROR);
    }

    /* Close file */
    close (fd);

    /* Hard eject device */
    if ((error = xbdIoctl (deviceBase, XBD_HARD_EJECT, NULL)) != OK) {
        errnoSet (error);
        return (ERROR);
    }

    /*
     * Primary insert device,
     * which means that partitions will re-deteced and mounted
     */
    if (erfEventRaise (xbdEventCategory, xbdEventPrimaryInsert,
                       ERF_ASYNC_PROCESS, (void *) deviceBase, NULL) != OK) {
        return (ERROR);
    }

    return (OK);
}

/***************************************************************************
 *
 * xbdSectorRead - read a sector from the underlying XBD device
 *
 * RETURNS: OK on success, error otherwise
 */

LOCAL int xbdSectorRead (
    device_t  device,       /* lower layer XBD device */
    lblkno_t  sector,       /* sector number to read */
    unsigned  sectorSize,   /* # of bytes in the sector */
    char *    data          /* pointer to data to read */
    ) {
    SEMAPHORE  sem;
    bio_t      bio;

    semBInit (&sem, SEM_Q_PRIORITY, SEM_EMPTY);

    bio.bio_dev     = device;               /* device to read */
    bio.bio_data    = data;                 /* data buffer */
    bio.bio_flags   = BIO_READ;             /* flag it as a read operation */
    bio.bio_chain   = NULL;                 /* no subsequent bios */
    bio.bio_bcount  = sectorSize;           /* sector size (bytes to read) */
    bio.bio_blkno   = sector;               /* sector # to read */
    bio.bio_done    = xbdSectorReadDone;    /* rtn to call when done reading */
    bio.bio_caller1 = &sem;                 /* semaphore on which to wait */
    bio.bio_error   = OK;
    bio.bio_resid   = 0;

    xbdStrategy (device, &bio);
    semTake (&sem, WAIT_FOREVER);           /* wait for completion */

    return (bio.bio_error);
}


/***************************************************************************
 *
 * xbdSectorReadDone -
 *
 * RETURNS: N/A
 */

LOCAL void xbdSectorReadDone (
    struct bio *  bio
    ) {
    semGive ((SEM_ID) bio->bio_caller1);
}

/***************************************************************************
 *
 * xbdPartitionsDetect - detect any pre-existing partitions
 *
 * This routine searches for any existing partitions on the device.  It should
 * only be called by the file system monitor.  For each found partition, it
 * raises a secondary insertion event to be handled by the file system monitor.
 *
 * RETURNS: Number of partitions on success, ERROR otherwise
 */

LOCAL int xbdPartitionsDetect (
    device_t  device
    ) {
    lblkno_t  nBlks;
    unsigned  blkSize;
    int       i;
    int       error;
    char *    data;
    PARTITION_ENTRY  partTable[4];
    XBD_PARTITION *  pXbdPartitions;
    XBD_PART *       pXbdPart;
    device_t         partitionDevice;
    devname_t        devname;
    devname_t        devnameRoot;
    char             nStr[3];
    char  *          pc;
    STATUS           status;

    if (((error = xbdBlockSize (device, &blkSize)) != OK) ||
        ((error = xbdNBlocks (device, &nBlks)) != OK)) {
        errnoSet (error);
        return (ERROR);
    }

    if ((data = (char *) malloc (blkSize)) == NULL) {
        errnoSet (ENOMEM);
        return (ERROR);
    }

    pXbdPartitions = (XBD_PARTITION *) malloc (sizeof (XBD_PARTITION));
    if (pXbdPartitions == NULL) {
        errnoSet (ENOMEM);
        free (data);
        return (ERROR);
    }

    if ((error = xbdSectorRead (device, 0, blkSize, data)) != OK) {
        goto errorReturn;
    }

    /*
     * For now, it shall be assumed that block size of the XBD device is not
     * less than 512 bytes.  The partition table is supposed to be located at
     * byte offset 0x1be (446).  Should the block size be smaller than 512
     * bytes, a developer should be able to spot the potential problem.
     */

    memcpy (partTable, data + 0x1be, sizeof (partTable));
    memset (pXbdPartitions, 0, sizeof (XBD_PARTITION));

    pXbdPartitions->subDevice = device;
    pXbdPart = &(pXbdPartitions->xbdParts[0]);

    if (xbdDevName (device, devnameRoot) != OK) {
        goto errorReturn;
    }

    /* Check if partitioning is supported and extract root name */
    if (((pc = strrchr (devnameRoot, ':')) != NULL) &&
         (strcmp (pc, ":0") == 0)) {
        *pc = EOS;
    }
    else {
        return (ERROR);
    }


    /*
     * Initialize XBD partitions.
     * TODO: Add code to handle detection of extended partitions.
     */

    for (i = 0; i < 4; i++) {
        if (partTable[i].nSectors == 0) {
            pXbdPart->device = NULLDEV;
            continue;
        }

        pXbdPart->isBootable = ((partTable[i].bootIndicator & 0x80) != 0x00);
        pXbdPart->head[0] = partTable[i].headStart;
        pXbdPart->head[1] = partTable[i].headEnd;
        pXbdPart->sector[0] = partTable[i].secStart;
        pXbdPart->sector[1] = partTable[i].secEnd;
        pXbdPart->cylinder[0] = partTable[i].cylStart;
        pXbdPart->cylinder[1] = partTable[i].cylEnd;
        pXbdPart->startBlock = (lblkno_t) partTable[i].sector;
        pXbdPart->nBlocks = (lblkno_t) partTable[i].nSectors;
        pXbdPart->xbdPartitions = pXbdPartitions;

        /*
         * TODO:
         * Add code to sanity check the settings.  Skipping this for now.
         */

        /* Create name for device:
           underlying device name + ":" + pXbdPartitions->nPartitions
         */
        strcpy (devname, devnameRoot);
        strcat (devname, ":");
        strcat (devname, itoa(pXbdPartitions->nPartitions + 1, nStr, 10));

        /* Attach the XBD */
        status = xbdAttach (&pXbdPart->xbd, &xbdPartitionFuncs, devname,
                            blkSize, pXbdPart->nBlocks, &partitionDevice);

        if (status != OK) {
            error = errnoGet();
            goto errorReturn;
        }

        /* Store attached  XBD device */
        pXbdPart->device = partitionDevice;

        if ((erfHandlerRegister (xbdEventCategory, xbdEventRemove,
                                 xbdPartitionEject, pXbdPart, 0) != OK) ||
            (erfHandlerRegister (xbdEventCategory, xbdEventMediaChanged,
                                 xbdPartitionEject, pXbdPart, 0) != OK)) {
            error = errnoGet();
            goto errorReturn;
        }

        /* Attachment successful.  Raise the secondary insertion event */
        if (erfEventRaise (xbdEventCategory,
                           xbdEventSecondaryInsert,
                           ERF_ASYNC_PROCESS,
                           (void *) partitionDevice,
                           NULL) != OK) {
            error = errnoGet ();
            goto errorReturn;
        }

        /* Successful raising of the event.  Accept the partition as valid. */
        pXbdPart++;
        pXbdPartitions->nPartitions++;

    }

    return (pXbdPartitions->nPartitions);

errorReturn:
    free (data);
    free (pXbdPartitions);
    errnoSet (error);
    return (ERROR);
}

/***************************************************************************
 *
 * xbdPartInit - inititalize partition parameters
 *
 * This function calculates the CHS and LBA parameters based on
 * start and end block.
 * The result is stored in the XBD_PART structure.
 *
 * RETURNS: OK on success, ERROR otherwise
 */

LOCAL STATUS xbdPartInit (
    XBD_PART *      pXbdPart,
    XBD_GEOMETRY  * pXbdGeometry,
    lblkno_t        startBlock,
    lblkno_t        endBlock
    ) {
    unsigned      temp;

    /* Inititalize partition structure */
    pXbdPart->cylinder[0] = startBlock /
        (pXbdGeometry->numHeads * pXbdGeometry->sectorsPerTrack);
    temp = startBlock % (pXbdGeometry->numHeads *
                         pXbdGeometry->sectorsPerTrack);
    pXbdPart->head[0] = temp / pXbdGeometry->sectorsPerTrack;
    pXbdPart->sector[0] = temp % pXbdGeometry->sectorsPerTrack + 1;

    pXbdPart->cylinder[1] = endBlock /
        (pXbdGeometry->numHeads * pXbdGeometry->sectorsPerTrack);
    temp = endBlock % (pXbdGeometry->numHeads *
                       pXbdGeometry->sectorsPerTrack);
    pXbdPart->head[1] = temp / pXbdGeometry->sectorsPerTrack;
    pXbdPart->sector[1] = temp % pXbdGeometry->sectorsPerTrack + 1;

    pXbdPart->startBlock = startBlock;
    pXbdPart->nBlocks = endBlock - startBlock;

#ifdef DEBUG_XBD_PART
    printf("Partition set up\n");
    printf("isBootable: %d\n", pXbdPart->isBootable);
    printf("Head      : [%d,%d]\n",
        pXbdPart->head[0], pXbdPart->head[1]);
    printf("Sector    : [%d,%d]\n",
        pXbdPart->sector[0], pXbdPart->sector[1]);
    printf("Cylinder  : [%d,%d]\n",
        pXbdPart->cylinder[0], pXbdPart->cylinder[1]);
    printf("startBlock: %d\n", pXbdPart->startBlock);
    printf("nBlocks   : %d\n", pXbdPart->nBlocks);
    printf("Check LBA1: %d\n",
        ((pXbdPart->cylinder[0] * pXbdGeometry->numHeads + pXbdPart->head[0]) *
            pXbdGeometry->sectorsPerTrack) + pXbdPart->sector[0] - 1);
    printf("Check LBA2: %d\n",
        ((pXbdPart->cylinder[1] * pXbdGeometry->numHeads + pXbdPart->head[1]) *
            pXbdGeometry->sectorsPerTrack) + pXbdPart->sector[1] - 1);
#endif

    return (OK);
}

/***************************************************************************
 *
 * xbdPartitionHandler -
 *
 * RETURNS: N/A
 */

LOCAL void xbdPartitionHandler (
    int     category,      /* event category */
    int     type,          /* event type */
    void *  pEventData,    /* device ID */
    void *  pUserData      /* not used--path extracted from XBD & mapping */
    ) {
    device_t     device;
    devname_t    devname;
    char  *      pc;

    if (category != xbdEventCategory) {
        return;
    }

    if (type == xbdEventPrimaryInsert) {
        /*
         * XXX
         * Media that supports partitions has been inserted.  Something will
         * have to be done to check the media for any existing partitions.
         * If partitions are present, each partition will raise its own
         * <xbdEventSecondaryInsert> event ASYNCHRONOUSLY.
         * XXX
         */
        device = (device_t) pEventData;
        if (xbdDevName (device, devname)) {
            return;
        }

        /* Check if partitioning is supported */
        if (((pc = strrchr (devname, ':')) != NULL) &&
             (strcmp (pc, ":0") == 0)) {
            if (xbdPartitionsDetect (device) <= 0) {
                /*
                 * If no partitions where detected,
                 * raise secondary insert event for base device.
                 */
                erfEventRaise (xbdEventCategory,
                               xbdEventSecondaryInsert,
                               ERF_ASYNC_PROCESS,
                               (void *) device,
                               NULL);
            }
        }
    }
}

/***************************************************************************
 *
 * xbdPartitionEject -
 *
 * RETURNS: N/A
 */

LOCAL void xbdPartitionEject (
    int     category,      /* event category */
    int     type,          /* event type */
    void *  pEventData,    /* device ID */
    void *  pUserData      /* pointer to XBD */
    ) {
    device_t         device;
    XBD_PART  *      pXbdPart;
    XBD_PARTITION  * pXbdPartitions;
    int              i;
    BOOL             dealloc = FALSE;

    /* Extract device */
    device = (device_t) pEventData;

    /* Extract XBD device */
    pXbdPart = (XBD_PART *) pUserData;

    /* Check if this request is for us */
    if (pXbdPart->device != device) {
        return;
    }

    /* Detach device */
    xbdDetach ((XBD *) pXbdPart);
    pXbdPart->device = NULLDEV;

    /* Check if all partitions has been removed/ejected */
    pXbdPartitions = pXbdPart->xbdPartitions;
    for (i = 0, dealloc = TRUE, pXbdPart = &(pXbdPartitions->xbdParts[0]);
         i < pXbdPartitions->nPartitions;
         i++, pXbdPart++) {
        if (pXbdPart->device != NULLDEV) {
            dealloc = FALSE;
            break;
        }
    }

    /* Unregister handler */
    erfHandlerUnregister (category, xbdEventRemove,
                          pEventData, pUserData);
    erfHandlerUnregister (category, xbdEventMediaChanged,
                          pEventData, pUserData);

    if (dealloc) {
#ifdef DEBUG_XBD_PART
        printf("No more partition(s).\n");
#endif
        free (pXbdPartitions);
    }
}

