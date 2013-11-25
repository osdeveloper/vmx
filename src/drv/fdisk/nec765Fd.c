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

/* nec765Fd.c - NEC 765 floppy driver controller driver */

/* Includes */
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/sysArchLib.h>
#include <arch/intArchLib.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>
#include <vmx/wdLib.h>
#include <os/errnoLib.h>
#include <os/ioLib.h>
#include <drv/fdisk/nec765Fd.h>

/* Defines */

/* Imports */
IMPORT unsigned int sysFdBuf;
IMPORT unsigned int sysFdBufSize;

int sysClockRateGet(
    void
    );

STATUS sysIntEnablePIC(
    int level
    );

STATUS dmaSetup(
    int           dir,
    void         *buf,
    unsigned int  nBytes,
    unsigned int  c
    );

/* Locals */
LOCAL fdDrvInstalled = FALSE;
LOCAL fdCylinder     = 1;

LOCAL char fdDORvalues[] =
{
    0x1c,
    0x2d,
    0x4e,
    0x8f
};

LOCAL char fdAccess[] =
{
    0,
    0,
    0,
    0
};

LOCAL FD_TYPE fdTypes[] =
{
    /* 1.44 MBytes floppy */
    {
        2880,                   /* Number of sectors */
        18,                     /* Number of sectors per track */
        2,                      /* Number of heads */
        80,                     /* Number of cylinders */
        2,                      /* Bytes per sector */
        0x1b,                   /* Gap 1 size read/write */
        0x54,                   /* Gap 2 size format */
        0x00,                   /* Data transfer rate */
        0x0c,                   /* Stepping rate */
        0x0f,                   /* Head unload time */
        0x02,                   /* Head load time */
        1,                      /* MFM bit for read/write format */
        1,                      /* SK bit for read */
        "1.44M"                 /* Name */
    },

    /* 1.2 MBytes floppy */
    {
        2400,                        /* Number of sectors */
        15,                          /* Number of sectors per track */
        2,                           /* Number of heads */
        80,                          /* Number of cylinders */
        2,                           /* Bytes per sector */
        0x24,                        /* Gap 1 size read/write */
        0x50,                        /* Gap 2 size format */
        0x00,                        /* Data transfer rate */
        0x0d,                        /* Stepping rate */
        0x0f,                        /* Head unload time */
        0x02,                        /* Head load time */
        1,                           /* MFM bit for read/write format */
        1,                           /* SK bit for read */
        "1.2M"                       /* Name */
    }
};

LOCAL BOOL fdIsWP(
    int drive
    );

LOCAL void fdRelease(
    void
    );

LOCAL void fdSelect(
    int fdType,
    int drive
    );

LOCAL STATUS fdRecalibrate(
    int drive
    );

LOCAL void fdInit(
    void
    );

LOCAL STATUS fdReset(
    FD_DEV *pFdDev
    );

LOCAL STATUS fdStatusChk(
    FD_DEV *pFdDev
    );

LOCAL STATUS fdSeek(
    int drive,
    int cylinder,
    int head
    );

LOCAL STATUS fdBlkRd(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer
    );

LOCAL STATUS fdBlkWrt(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer
    );

LOCAL STATUS fdIoctl(
    FD_DEV *pFdDev,
    int     cmd,
    ARG     arg
    );

LOCAL void fdIntr(
    int ctrl
    );

LOCAL void fdOff(
    void
    );

LOCAL STATUS fdBlkRW(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer,
    int     direction
    );

LOCAL int fdRW(
    int   fdType,
    int   drive,
    int   cylinder,
    int   head,
    int   sector,
    void *pBuf,
    int   nSecs,
    int   direction
    );

LOCAL STATUS fdFormat(
    int fdType,
    int drive,
    int cylinder,
    int head,
    int interleave
    );

LOCAL int fdCmdSend(
    u_int8_t *pCmd,
    int       nBytes
    );

LOCAL int fdResult(
    u_int8_t *buffer,
    BOOL      immediate,
    int       nBytes
    );

LOCAL STATUS fdIntSence(
    BOOL seekEnd
    );

/* Globals */
int fdIntCount   = 0;           /* Interrupt count */
int fdRetry      = 2;           /* Retry count */
int fdTimeout    = 10000;       /* Timeout count */
int fdSemSeconds = 2;           /* Semaphore timeout seconds */
int fdWdSeconds  = 4;           /* Timeout for drive motor */
SEMAPHORE fdSyncSem;            /* Sychronization semaphore */
SEMAPHORE fdMutexSem;           /* Mutex semaphore */
WDOG_ID fdWdId;                 /* Disk motor timer */

/* Functions */

/******************************************************************************
 * fdDrvInit - Initialize floppy driver
 *
 * RETURNS: OK or ERROR
 */

STATUS fdDrvInit(
    int vector,
    int level
    )
{
    if (fdDrvInstalled == TRUE)
    {
        return OK;
    }

    /* Initialize semaphores */
    semBInit(&fdSyncSem, SEM_Q_FIFO, SEM_EMPTY);
    semMInit(&fdMutexSem, SEM_Q_PRIORITY | SEM_DELETE_SAFE);

    /* Create drive motor timer */
    fdWdId = wdCreate();
    if (fdWdId == NULL)
    {
        return ERROR;
    }

    intConnectDefault(vector, fdIntr, (void *) 0);
    sysIntEnablePIC(level);

    fdInit();

    /* Mark as installed */
    fdDrvInstalled = TRUE;

    return OK;
}

/******************************************************************************
 * fdDrvCreate - Create floppy device
 *
 * RETURNS: Block device or NULL
 */

BLK_DEV* fdDevCreate(
    int drive,
    int fdType,
    int nBlocks,
    int blkOffset
    )
{
    FD_DEV  *pFdDev;
    BLK_DEV *pBlkDev;
    FD_TYPE *pFdType;

    /* If not installed */
    if (fdDrvInstalled != TRUE)
    {
        return NULL;
    }

    /* Initialize type */
    pFdType = &fdTypes[fdType];

    /* Check driver number */
    if (drive >= FD_MAX_DRIVES)
    {
        return NULL;
    }

    /* Check number of blocks */
    if (nBlocks == 0)
    {
        nBlocks = pFdType->sectors;
    }

    /* Allocate memory for fd device structure */
    pFdDev = (FD_DEV *) malloc( sizeof(FD_DEV) );
    if (pFdDev == NULL)
    {
        return NULL;
    }

    /* Setup fd device struct */
    pFdDev->fdType    = fdType;
    pFdDev->drive     = drive;
    pFdDev->blkOffset = blkOffset;

    /* Setup block device structure */
    pBlkDev = &pFdDev->blkDev;
    pBlkDev->bd_nBlocks      = nBlocks;
    pBlkDev->bd_bytesPerBlk  = 128 << pFdType->bytesPerSector;
    pBlkDev->bd_blksPerTrack = pFdType->sectorsPerTrack;
    pBlkDev->bd_nHeads       = pFdType->heads;
    pBlkDev->bd_removable    = TRUE;
    pBlkDev->bd_retry        = 1;
    pBlkDev->bd_readyChanged = TRUE;
    pBlkDev->bd_blkRd        = (FUNCPTR) fdBlkRd;
    pBlkDev->bd_blkWrt       = (FUNCPTR) fdBlkWrt;
    pBlkDev->bd_ioctl        = (FUNCPTR) fdIoctl;
    pBlkDev->bd_reset        = (FUNCPTR) fdReset;
    pBlkDev->bd_statusChk    = (FUNCPTR) fdStatusChk;

    /* Power up device */
    sysOutByte(FD_REG_OUTPUT, fdDORvalues[drive]);
    sysDelay();

    /* Set mode according to write protect jumper on floppy disk */
    if (fdIsWP(drive) == TRUE)
    {
        pBlkDev->bd_mode = O_RDONLY;
    }
    else
    {
        pBlkDev->bd_mode = O_RDWR;
    }

    /* Power down device */
    fdRelease();
    sysDelay();

    return pBlkDev;
}

/******************************************************************************
 * fdIsWP - Check if floppy is write protected
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL fdIsWP(
    int drive
    )
{
    BOOL     ret;
    u_int8_t cmd[FD_CMD_SENSEDRIVE];
    u_int8_t result;

    /* Send drive sense command */
    cmd[0] = FD_CMD_SENSEDRIVE;
    cmd[1] = drive & 0x03;
    fdCmdSend(cmd, FD_CMD_LEN_SENSEDRIVE);

    fdResult(&result, FALSE, 1);
    if ((result & 0x40) == 0x40)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

/******************************************************************************
 * fdRelease - Release and power down drive
 *
 * RETURNS: N/A
 */

LOCAL void fdRelease(
    void
    )
{
    /* Power down drive */
    sysOutByte(FD_REG_OUTPUT, (FD_DOR_CLEAR_RESET | FD_DOR_DMA_ENABLE));
    sysDelay();
}

/******************************************************************************
 * fdSelect - Select and turn off drive
 *
 * RETURNS: N/A
 */

LOCAL void fdSelect(
    int fdType,
    int drive
    )
{
    FD_TYPE  *pFdType;
    u_int8_t  cmd[FD_CMD_LEN_SPECIFY];

    /* Get floppy type */
    pFdType = &fdTypes[fdType];

    /* Power down device */
    sysOutByte(FD_REG_OUTPUT, fdDORvalues[drive]);
    sysDelay();

    /* Set data transfer rate */
    sysOutByte(FD_REG_CONFIG, pFdType->dataRate);
    sysDelay();

    /* Send specify drive command */
    cmd[0] = FD_CMD_SPECIFY;
    cmd[1] = (pFdType->stepRate << 4) | pFdType->headUnload;
    cmd[2] = pFdType->headLoad << 1;
    fdCmdSend(cmd, FD_CMD_LEN_SPECIFY);
    sysDelay();
}

/******************************************************************************
 * fdRecalibrate - Recalibrate drive
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdRecalibrate(
    int drive
    )
{
    int      i;
    STATUS   ret[2];
    u_int8_t cmd[FD_CMD_LEN_RECALIBRATE];

    /* Send recalibrate command */
    cmd[0] = FD_CMD_RECALIBRATE;
    cmd[1] = drive & 0x03;
    for (i = 0; i < 2; i++)
    {
        fdCmdSend(cmd, FD_CMD_LEN_RECALIBRATE);

        if (semTake(&fdSyncSem, sysClockRateGet() * fdSemSeconds) != OK)
        {
            return ERROR;
        }

        ret[i] = fdIntSence(TRUE);
    }

    if ((ret[0] == OK) && (ret[1] == OK))
    {
        return OK;
    }

    return ERROR;
}

/******************************************************************************
 * fdInit - Initialize drive
 *
 * RETURNS: N/A
 */

LOCAL void fdInit(
    void
    )
{
    int i;

    /* Reset and disable dma */
    sysOutByte(FD_REG_OUTPUT, (FD_DOR_RESET | FD_DOR_DMA_DISABLE));
    taskDelay(sysClockRateGet() >> 1);

    /* Clear reset and enable dma */
    sysOutByte(FD_REG_OUTPUT, (FD_DOR_CLEAR_RESET | FD_DOR_DMA_ENABLE));
    taskDelay(sysClockRateGet() >> 1);

    if (semTake(&fdSyncSem, sysClockRateGet() * fdSemSeconds) == OK)
    {
        for (i = 0; i < FD_MAX_DRIVES; i++)
        {
            fdIntSence(FALSE);
        }
    }
}

/******************************************************************************
 * fdReset - Reset drive
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdReset(
    FD_DEV *pFdDev
    )
{
    fdInit();

    fdSelect(pFdDev->fdType, pFdDev->drive);
    fdRecalibrate(pFdDev->drive);
    fdRelease();

    return OK;
}

/******************************************************************************
 * fdStatusChk - Check floppy driver ready status
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdStatusChk(
    FD_DEV *pFdDev
    )
{
    FD_TYPE *pFdType;

    /* Initialize floppy type */
    pFdType = &fdTypes[pFdDev->fdType];

    /* Cancel timed disk release */
    wdCancel(fdWdId);

    semTake(&fdMutexSem, WAIT_FOREVER);

    sysOutByte(FD_REG_OUTPUT, fdDORvalues[pFdDev->drive]);
    sysDelay();

    /* If diskette changed */
    if (sysInByte(FD_REG_INPUT) & 0x80)
    {
        pFdDev->blkDev.bd_readyChanged = TRUE;

        if (++fdCylinder >= pFdType->cylinders)
        {
            fdCylinder = 1;
        }

        /* Perform seek operation */
        fdSelect(pFdDev->fdType, pFdDev->drive);
        fdSeek(pFdDev->drive, fdCylinder, 0);

        /* Setup write protected flag */
        if (fdIsWP(pFdDev->drive) == TRUE)
        {
            pFdDev->blkDev.bd_mode = O_RDONLY;
        }
        else
        {
            pFdDev->blkDev.bd_mode = O_RDWR;
        }
    }

    semGive(&fdMutexSem);

    /* Resume timed disk release */
    wdStart(
        fdWdId,
        sysClockRateGet() * fdWdSeconds,
        (FUNCPTR) fdOff,
        (ARG) 0
        );

    return OK;
}

/******************************************************************************
 * fdSeek - Seek to position
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdSeek(
    int drive,
    int cylinder,
    int head
    )
{
    STATUS   status;
    u_int8_t cmd[FD_CMD_LEN_SEEK];
 
    /* Send seek command */
    cmd[0] = FD_CMD_SEEK;
    cmd[1] = (head << 2) | (drive & 0x03);
    cmd[2] = cylinder;
    fdCmdSend(cmd, FD_CMD_LEN_SEEK);

    if (semTake(&fdSyncSem, sysClockRateGet() * fdSemSeconds) != OK)
    {
        status = ERROR;
    }
    else
    {
        status = fdIntSence(TRUE);
    }

    return status;
}

/******************************************************************************
 * fdBlkRd - Block read function
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdBlkRd(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer
    )
{
    return fdBlkRW(pFdDev, startBlk, nBlks, buffer, O_RDONLY);
}

/******************************************************************************
 * fdBlkWrt - Block write function
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdBlkWrt(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer
    )
{
    return fdBlkRW(pFdDev, startBlk, nBlks, buffer, O_WRONLY);
}

/******************************************************************************
 * fdBlkIoctl - Ioctl function
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdIoctl(
    FD_DEV *pFdDev,
    int     cmd,
    ARG     arg
    )
{
    FD_TYPE *pFdType;
    int      cylinder;
    int      head;
    int      timeout;
    STATUS   status = ERROR;

    /* Initialize type */
    pFdType = &fdTypes[pFdDev->fdType];

    /* Cancel timed drive release */
    wdCancel(fdWdId);

    semTake(&fdMutexSem, WAIT_FOREVER);

    /* Select command */
    switch (cmd)
    {
        case FIODISKFORMAT:
            fdSelect(pFdDev->fdType, pFdDev->drive);
            fdRecalibrate(pFdDev->drive);

            /* For all cylinders */
            for (cylinder = 0; cylinder < pFdType->cylinders; cylinder++)
            {
                /* For all heads */
                for (head = 0; head < pFdType->heads; head++)
                {
                    /* While seek */
                    timeout = 0;
                    while (fdSeek(pFdDev->drive, cylinder, head) != OK)
                    {
                        if (++timeout > fdRetry)
                        {
                            errnoSet(S_ioLib_DEVICE_ERROR);
                            goto ioctlDone;
                        }
                    }

                    /* While format */
                    timeout = 0;
                    while (fdFormat(
                               pFdDev->fdType,
                               pFdDev->drive,
                               cylinder,
                               head,
                               (int) arg
                               ) != OK )
                    {
                        if (++timeout > fdRetry)
                        {
                            errnoSet(S_ioLib_DEVICE_ERROR);
                            goto ioctlDone;
                        }
                    }
                }
            }

            status = OK;
            break;

        default:
            errnoSet(S_ioLib_UNKNOWN_REQUEST);
            break;
    }

ioctlDone:
    semGive(&fdMutexSem);

    /* Resume timed drive release */
    wdStart(
        fdWdId,
        sysClockRateGet() * fdWdSeconds,
        (FUNCPTR) fdOff,
        (ARG) 0
        );

    return status;
}

/******************************************************************************
 * fdIntr - Floppy drive interrupt handler
 *
 * RETURNS: N/A
 */

LOCAL void fdIntr(
    int ctrl
    )
{
    fdIntCount++;
    semGive(&fdSyncSem);
}

/******************************************************************************
 * fdOff - Release and turn off drive
 *
 * RETURNS: N/A
 */

LOCAL void fdOff(
    void
    )
{
    sysOutByte(FD_REG_OUTPUT, (FD_DOR_CLEAR_RESET | FD_DOR_DMA_ENABLE));
    sysDelay();
}

/******************************************************************************
 * fdBlkRW - Read/write blocks to/from floppy disk
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdBlkRW(
    FD_DEV *pFdDev,
    int     startBlk,
    int     nBlks,
    char   *buffer,
    int     direction
    )
{
    BLK_DEV *pBlkDev;
    FD_TYPE *pFdType;
    int      head;
    int      cylinder;
    int      sector;
    int      nSecs;
    int      i;
    int      tryRW;
    int      tryRecalibrate;
    int      trySeek;
    int      rwStatus;
    STATUS   status = ERROR;

    /* Initialize locals */
    pBlkDev = &pFdDev->blkDev;
    pFdType = &fdTypes[pFdDev->fdType];

    /* Cancel timed disk release */
    wdCancel(fdWdId);

    semTake(&fdMutexSem, WAIT_FOREVER);

    /* Calculate start block */
    startBlk += pFdDev->blkOffset;

    /* Select drive and recalibrate if needed */
    fdSelect(pFdDev->fdType, pFdDev->drive);
    if (fdAccess[pFdDev->drive] == 0)
    {
        fdRecalibrate(pFdDev->drive);
        fdAccess[pFdDev->drive] = 1;
    }

    /* For all blocks */
    for (i = 0; i < nBlks; i += nSecs)
    {
        /* Setup track */
        cylinder = startBlk / (pFdType->sectorsPerTrack * pFdType->heads);
        sector   = startBlk % (pFdType->sectorsPerTrack * pFdType->heads);
        head     = sector / pFdType->sectorsPerTrack;
        sector   = sector % pFdType->sectorsPerTrack + 1;

        /* Store current */
        fdCylinder = cylinder;

        /* Seek to track */
        trySeek = 0;
        while (fdSeek(pFdDev->drive, cylinder, head) != OK)
        {
            if (++trySeek > fdRetry)
            {
                goto rwDone;
            }
        }

        /* Calculate number of sectors */
        nSecs = min(nBlks - i, pFdType->sectorsPerTrack - sector + 1);
        while ((pBlkDev->bd_bytesPerBlk * nSecs) > sysFdBufSize)
        {
            nSecs--;
        }

        /* While read/write track data */
        tryRW = 0;
        tryRecalibrate = 0;
        while ((rwStatus = fdRW(
                               pFdDev->fdType,
                               pFdDev->drive,
                               cylinder,
                               head,
                               sector,
                               buffer,
                               nSecs,
                               direction
                               )) != OK)
        {
            if ((rwStatus == FD_UNFORMATED) ||
                (rwStatus == FD_WRITE_PROTECTED) ||
                (rwStatus == FD_DISK_NOT_PRESENT))
            {
                tryRW = fdRetry;
            }

            /* If read/write timeout */
            if (++tryRW > fdRetry)
            {
                /* Try to recalibrate */
                fdRecalibrate(pFdDev->drive);
                if (++tryRecalibrate > fdRetry)
                {
                    goto rwDone;
                }

                /* Try to seek */
                trySeek = 0;
                while (fdSeek(pFdDev->drive, cylinder, head) != OK)
                {
                    if (++trySeek > fdRetry)
                    {
                        goto rwDone;
                    }
                }

                tryRW = 0;
            }
        }

        /* Advance */
        startBlk += nSecs;
        buffer   += pBlkDev->bd_bytesPerBlk * nSecs;
    }

    /* Success if here */
    status = OK;

rwDone:

    /* Select error */
    switch (rwStatus)
    {
        case FD_UNFORMATED:
            errnoSet(S_ioLib_UNFORMATED);
            break;

        case FD_WRITE_PROTECTED:
            pBlkDev->bd_mode == O_RDONLY;
            errnoSet(S_ioLib_WRITE_PROTECTED);
            break;

        case FD_DISK_NOT_PRESENT:
            errnoSet(S_ioLib_DISK_NOT_PRESENT);
            break;

        case ERROR:
            errnoSet(S_ioLib_DEVICE_ERROR);
            break;

        default:
            break;
    }

    semGive(&fdMutexSem);

    /* Resume timed disk release */
    wdStart(
        fdWdId,
        sysClockRateGet() * fdWdSeconds,
        (FUNCPTR) fdOff,
        (ARG) 0
        );

    return status;
}

/******************************************************************************
 * fdRW - Read/write a number of sectors to the current track
 *
 * RETURNS: OK, ERROR or error code
 */

LOCAL int fdRW(
    int   fdType,
    int   drive,
    int   cylinder,
    int   head,
    int   sector,
    void *pBuf,
    int   nSecs,
    int   direction
    )
{
    FD_TYPE      *pFdType;
    unsigned int  nBytes;
    u_int8_t      cmd[FD_CMD_LEN_RW];
    u_int8_t      result[7];
    char          fd_mt;
    int           ret;

    /* Initialize locals */
    pFdType = &fdTypes[fdType];
    nBytes  = nSecs * (128 << pFdType->bytesPerSector);

    /* If header gt. one */
    if (pFdType->heads > 1)
    {
        fd_mt = 1;
    }
    else
    {
        fd_mt = 0;
    }

    /* If read only */
    if (direction == O_RDONLY)
    {
        /* Setup dma for reading */
        if (dmaSetup(O_RDONLY, (void *) sysFdBuf, nBytes, FD_DMA_CHAN) != OK)
        {
            return ERROR;
        }

        /* Setup read command */
        cmd[0] = FD_CMD_READ |
                 (fd_mt << 7) | (pFdType->mfm << 6) | (pFdType->sk << 5);
    }
    else
    {
        /* Setup dma for writing */
        if (dmaSetup(O_WRONLY, (void *) sysFdBuf, nBytes, FD_DMA_CHAN) != OK)
        {
            return ERROR;
        }

        /* Setup write command */
        cmd[0] = FD_CMD_WRITE | (fd_mt << 7) | (pFdType->mfm << 6);
        memcpy((void *) sysFdBuf, pBuf, nBytes);
    }

    /* Setup the rest of the command fields */
    cmd[1] = (head << 2) | (drive & 0x03);
    cmd[2] = cylinder;
    cmd[3] = head;
    cmd[4] = sector;
    cmd[5] = pFdType->bytesPerSector;
    cmd[6] = pFdType->sectorsPerTrack;
    cmd[7] = pFdType->gap1;
    cmd[8] = 0xff;

    /* Send command */
    fdCmdSend(cmd, FD_CMD_LEN_RW);

    if (semTake(&fdSyncSem, sysClockRateGet() * fdSemSeconds) != OK)
    {
        return FD_DISK_NOT_PRESENT;
    }

    /* Get results */
    ret = fdResult(result, FALSE, 7);
    if (ret == 0)
    {
        if ((result[0] & 0xc0) == 0x00)
        {
            if (direction == O_RDONLY)
            {
                memcpy(pBuf, (void *) sysFdBuf, nBytes);
            }

            return OK;
        }
        else
        {
            if ((result[1] & 0x04) == 0x04)
            {
                return FD_UNFORMATED;
            }
            else if ((result[1] & 0x02) == 0x02)
            {
                return FD_WRITE_PROTECTED;
            }
            else
            {
                return ERROR;
            }
        }
    }

    return ERROR;
}

/******************************************************************************
 * fdFormat - Format track
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdFormat(
    int fdType,
    int drive,
    int cylinder,
    int head,
    int interleave
    )
{
    int       ret;
    int       i;
    int       sector;
    FD_TYPE  *pFdType;
    u_int8_t  cmd[FD_CMD_LEN_FORMAT];
    u_int8_t  result[7];
    u_int8_t *pBuf = (u_int8_t *) sysFdBuf;

    /* Initialize type */
    pFdType = &fdTypes[fdType];

    for (i = 0, sector = 1; i < pFdType->sectorsPerTrack; i++)
    {
        *pBuf++ = (char) cylinder;
        *pBuf++ = (char) head;
        *pBuf++ = (char) sector;
        *pBuf++ = pFdType->bytesPerSector;
        sector++;
    }

    /* Setup dma for writing */
    if (dmaSetup(
            O_WRONLY,
            (void *) sysFdBuf,
            pFdType->sectorsPerTrack * 4,
            FD_DMA_CHAN
            ) != OK)
    {
        return ERROR;
    }

    /* Send format command */
    cmd[0] = FD_CMD_FORMAT | (pFdType->mfm << 6);
    cmd[1] = (head << 2) | (drive & 0x03);
    cmd[2] = pFdType->bytesPerSector;
    cmd[3] = pFdType->sectorsPerTrack;
    cmd[4] = pFdType->gap2;
    cmd[5] = 0xff;
    fdCmdSend(cmd, FD_CMD_LEN_FORMAT);

    if (semTake(&fdSyncSem, sysClockRateGet() * fdSemSeconds) != OK)
    {
        return ERROR;
    }

    ret = fdResult(result, FALSE, 7);
    if ((ret == 0) && ((result[0] & 0xc0) == 0x00))
    {
        return OK;
    }

    return ERROR;
}

/******************************************************************************
 * fdCmdSend - Send command to floppy controller
 *
 * RETURNS: Zero or errors -1 or -2
 */

LOCAL int fdCmdSend(
    u_int8_t *pCmd,
    int       nBytes
    )
{
    int i;
    int timeout;

    for (i = 0; i < nBytes; i++)
    {
        timeout = 0;

      /* While RQM flag not set */
      while ((sysInByte(FD_REG_STATUS) & FD_MSR_RQM) != FD_MSR_RQM)
      {
          if (++timeout > fdTimeout)
          {
              return (-1);
          }
      }

      /* Check if controller is ready for input */
      if ((sysInByte(FD_REG_STATUS) & FD_MSR_DIRECTION) != 0)
      {
          return (-2);
      }

      sysOutByte(FD_REG_DATA, (*pCmd++));
      sysDelay();
    }

    return 0;
}

/******************************************************************************
 * fdResult - Get results from floppy drive controller
 *
 * RETURNS: Zero or errors -1, -2, -3 or -4
 */

LOCAL int fdResult(
    u_int8_t *buffer,
    BOOL      immediate,
    int       nBytes
    )
{
    int i;
    int timeout;

    if ((immediate == TRUE) ||
        ((sysInByte(FD_REG_STATUS) & FD_MSR_FD_BUSY) != 0))
    {
        /* Get result bytes */
        for (i = 0; i < nBytes; i++)
        {
            timeout = 0;

            /* While REQ flag not set */
            while ((sysInByte(FD_REG_STATUS) & FD_MSR_RQM) != FD_MSR_RQM)
            {
                if (++timeout > fdTimeout)
                {
                    return (-1);
                }
            }

            /* If ready for output */
            if ((sysInByte(FD_REG_STATUS) & FD_MSR_DIRECTION) == 0)
            {
                return (-2);
            }

            buffer[i] = sysInByte(FD_REG_DATA);
            sysDelay();
        }

        timeout = 0;

        /* While not completed */
        while ((sysInByte(FD_REG_STATUS) & FD_MSR_FD_BUSY) != 0)
        {
            if (++timeout > fdTimeout)
            {
                return (-3);
            }
        }
    }
    else
    {
        timeout = 0;

        /* While interrupt sence */
        while (fdIntSence(FALSE) != OK)
        {
            if (++timeout > fdRetry)
            {
                return (-4);
            }
        }
    }

    return 0;
}

/******************************************************************************
 * fdIntSence - Get info about last drive interrupt
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS fdIntSence(
    BOOL seekEnd
    )
{
    STATUS   status;
    int      ret;
    u_int8_t cmd[FD_CMD_LEN_SENSEINT];
    u_int8_t result[2];

    /* Send sence interrupt command */
    cmd[0] = FD_CMD_SENSEINT;
    fdCmdSend(cmd, FD_CMD_LEN_SENSEINT);

    ret = fdResult(result, TRUE, 2);
    if ((ret == 0) && (seekEnd == FALSE) &&
        (((result[0] & 0xc0) == 0x00) || ((result[0] & 0xc0) == 0xc0)))
    {
        status = OK;
    }
    else if ((ret == 0) && (seekEnd == TRUE) &&
             ((result[0] & 0xe0) == 0x20))
    {
        status = OK;
    }
    else
    {
        status = ERROR;
    }

    return status;
}

