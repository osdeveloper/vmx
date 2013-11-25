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

/* nec765Fd.h - NEC 765 floppy disk controller header */

#ifndef _nec765Fd_h
#define _nec765Fd_h

#include <vmx.h>
#include <fs/blkIo.h>

/* Number of floppy drives */
#define FD_MAX_DRIVES                   4

/* Error codes */
#define FD_UNFORMATED                   -2
#define FD_WRITE_PROTECTED              -3
#define FD_DISK_NOT_PRESENT             -4

/* Floppy dma channel */
#define FD_DMA_CHAN                     2

/* Floppy I/O addresses */
#define FD_REG_OUTPUT                   0x3f2
#define FD_REG_STATUS                   0x3f4
#define FD_REG_COMMAND                  0x3f4
#define FD_REG_DATA                     0x3f5
#define FD_REG_INPUT                    0x3f7
#define FD_REG_CONFIG                   0x3f7

/* Floppy output register */
#define FD_DOR_DRIVE0_SEL               0x00
#define FD_DOR_DRIVE1_SEL               0x01
#define FD_DOR_DRIVE2_SEL               0x02
#define FD_DOR_DRIVE3_SEL               0x03
#define FD_DOR_RESET                    0x00
#define FD_DOR_CLEAR_RESET              0x04
#define FD_DOR_DMA_DISABLE              0x00
#define FD_DOR_DMA_ENABLE               0x08
#define FD_DOR_DRIVE_ON                 0x10

/* Floppy status register */
#define FD_MSR_DRIVE0_SEEK              0x01
#define FD_MSR_DRIVE1_SEEK              0x02
#define FD_MSR_DRIVE2_SEEK              0x04
#define FD_MSR_DRIVE3_SEEK              0x08
#define FD_MSR_FD_BUSY                  0x10
#define FD_MSR_EXEC_MODE                0x20
#define FD_MSR_DIRECTION                0x40
#define FD_MSR_RQM                      0x80

/* Floppy commands */
#define FD_CMD_SPECIFY                  0x03
#define FD_CMD_SENSEDRIVE               0x04
#define FD_CMD_RECALIBRATE              0x07
#define FD_CMD_SENSEINT                 0x08
#define FD_CMD_SEEK                     0x0f
#define FD_CMD_READ                     0x06
#define FD_CMD_WRITE                    0x05
#define FD_CMD_FORMAT                   0x0d

#define FD_CMD_LEN_SPECIFY              3
#define FD_CMD_LEN_SENSEDRIVE           2
#define FD_CMD_LEN_RECALIBRATE          2
#define FD_CMD_LEN_SENSEINT             1
#define FD_CMD_LEN_SEEK                 3
#define FD_CMD_LEN_RW                   9
#define FD_CMD_LEN_FORMAT               6

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdDev
{
    BLK_DEV blkDev;
    int     fdType;             /* Floppy disk type */
    int     drive;              /* Drive number */
    int     blkOffset;          /* Block offset */
} FD_DEV;

typedef struct fdType
{
    int   sectors;              /* Number of sectors */
    int   sectorsPerTrack;      /* Number of sectors per track */
    int   heads;                /* Number of heades */
    int   cylinders;            /* Number of cylinders */
    int   bytesPerSector;       /* Bytes per sector */
    char  gap1;                 /* Gap 1 size read/write */
    char  gap2;                 /* Gap 2 size format */
    char  dataRate;             /* Data transfer rate */
    char  stepRate;             /* Stepping rate */
    char  headUnload;           /* Head unload time */
    char  headLoad;             /* Head load time */
    char  mfm;                  /* MFM bit for read/write format */
    char  sk;                   /* SK bit for read */
    char *name;                 /* Name */
} FD_TYPE;

/* Functions */

/******************************************************************************
 * fdDrvInit - Initialize floppy driver
 *
 * RETURNS: OK or ERROR
 */

STATUS fdDrvInit(
    int vector,
    int level
    );

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
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _nec765Fd_h */

