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

/* tyLib.h - Typewriter support library header */

#ifndef _tyLib_h
#define _tyLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#define NO_SELECT
#define NO_EXCHOOKS

#include <sys/types.h>
#include <vmx/semLib.h>
#include <util/rngLib.h>
#include <os/iosLib.h>
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

} TY_DEV;

typedef TY_DEV *TY_DEV_ID;

/******************************************************************************
 * tyDevInit - Intialize a typwriter device
 *
 * RETURNS: OR or ERROR
 */

STATUS tyDevInit(
    TY_DEV_ID tyId,
    int readBufferSize,
    int writeBufferSize,
    FUNCPTR txStartup
    );

/******************************************************************************
 * tyDevRemove - Remove a typewriter device
 *
 * RETURNS: OR or ERROR
 */

STATUS tyDevRemove(
    TY_DEV_ID tyId
    );

/******************************************************************************
 * tyAbortFuncSet - Set abort function
 *
 * RETURNS: N/A
 */

void tyAbortFuncSet(
    FUNCPTR func
    );

/******************************************************************************
 * tyAbortSet - Set abort char
 *
 * RETURNS: N/A
 */

void tyAbortSet(
    char c
    );

/******************************************************************************
 * tyBackSpaceSet - Set backspace char
 *
 * RETURNS: N/A
 */

void tyBackSpaceSet(
    char c
    );

/******************************************************************************
 * tyDeleteLineSet - Set line delete char
 *
 * RETURNS: N/A
 */

void tyDeleteLineSet(
    char c
    );

/******************************************************************************
 * tyEOFSet - Set end of file char
 *
 * RETURNS: N/A
 */

void tyEOFSet(
    char c
    );

/******************************************************************************
 * tyMonitorTrapSet - Set monitor trap char
 *
 * RETURNS: N/A
 */

void tyMonitorTrapSet(
    char c
    );

/******************************************************************************
 * tyIoctl - Control request
 *
 * RETURNS: OK or ERROR
 */

int tyIoctl(
    TY_DEV_ID tyId,
    int req,
    int arg
    );

/******************************************************************************
 * tyWrite - Write to typewriter
 *
 * RETURNS: Number of bytes written
 */

int tyWrite(
    TY_DEV_ID tyId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * tyRead - Read from typewriter
 *
 * RETURNS: Number of bytes read
 */

int tyRead(
    TY_DEV_ID tyId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * tyTx - Typewriter transmit from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS tyIntTx(
    TY_DEV_ID tyId,
    char *pc
    );

/******************************************************************************
 * tyIntRd - Typwriter read from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS tyIntRd(
    TY_DEV_ID tyId,
    char c
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tyLib_h */

