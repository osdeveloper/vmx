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

/* ttyLib.h - tty support library header */

#ifndef _ttyLib_h
#define _ttyLib_h

#define XON                     0x11
#define XOFF                    0x13

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#define NO_SELECT
#define NO_EXCHOOKS

#include <sys/types.h>
#include <vmx/semLib.h>
#include <util/rngLib.h>
#include <io/iosLib.h>
#ifndef NO_SELECT
#include <os/selectLib.h>
#endif

typedef struct
{
    DEV_HEADER devHeader;

    RING_ID   readBuffer;
    SEMAPHORE readSync;
    SEMAPHORE mutex;
    struct
    {
        unsigned char xoff;
        unsigned char pending;
        unsigned char canceled;
        unsigned char flushingReadBuffer;
    } readState;

    RING_ID   writeBuffer;
    SEMAPHORE writeSync;
    struct
    {
        unsigned char busy;
        unsigned char xoff;
        unsigned char cr;
        unsigned char canceled;
        unsigned char flushingWriteBuffer;
        unsigned char writeBufferBusy;
    } writeState;

    unsigned char  lnNBytes;
    unsigned char  lnBytesLeft;
    unsigned short options;

    FUNCPTR txStartup;
    FUNCPTR protoHook;
    ARG protoArg;
#ifndef NO_SELECT
  SEL_WAKEUP_LIST selWakeupList;
#endif
    int numOpen;

} TTY_DEV;

typedef TTY_DEV *TTY_DEV_ID;

/******************************************************************************
 * ttyDevInit - Intialize a tty device
 *
 * RETURNS: OR or ERROR
 */

STATUS ttyDevInit(
    TTY_DEV_ID ttyId,
    int readBufferSize,
    int writeBufferSize,
    FUNCPTR txStartup
    );

/******************************************************************************
 * ttyDevRemove - Remove a tty device
 *
 * RETURNS: OR or ERROR
 */

STATUS ttyDevRemove(
    TTY_DEV_ID ttyId
    );

/******************************************************************************
 * ttyAbortFuncSet - Set abort function
 *
 * RETURNS: N/A
 */

void ttyAbortFuncSet(
    FUNCPTR func
    );

/******************************************************************************
 * ttyAbortSet - Set abort char
 *
 * RETURNS: N/A
 */

void ttyAbortSet(
    char c
    );

/******************************************************************************
 * ttyBackSpaceSet - Set backspace char
 *
 * RETURNS: N/A
 */

void ttyBackSpaceSet(
    char c
    );

/******************************************************************************
 * ttyDeleteLineSet - Set line delete char
 *
 * RETURNS: N/A
 */

void ttyDeleteLineSet(
    char c
    );

/******************************************************************************
 * ttyEOFSet - Set end of file char
 *
 * RETURNS: N/A
 */

void ttyEOFSet(
    char c
    );

/******************************************************************************
 * ttyMonitorTrapSet - Set monitor trap char
 *
 * RETURNS: N/A
 */

void ttyMonitorTrapSet(
    char c
    );

/******************************************************************************
 * ttyIoctl - Control request
 *
 * RETURNS: OK or ERROR
 */

int ttyIoctl(
    TTY_DEV_ID ttyId,
    int req,
    int arg
    );

/******************************************************************************
 * ttyWrite - Write to tty
 *
 * RETURNS: Number of bytes written
 */

int ttyWrite(
    TTY_DEV_ID ttyId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * ttyRead - Read from tty
 *
 * RETURNS: Number of bytes read
 */

int ttyRead(
    TTY_DEV_ID ttyId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * ttyTx - tty transmit from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS ttyIntTx(
    TTY_DEV_ID ttyId,
    char *pc
    );

/******************************************************************************
 * ttyIntRd - tty read from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS ttyIntRd(
    TTY_DEV_ID ttyId,
    char c
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ttyLib_h */

