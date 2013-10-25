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

/* ttyLib.c - tty support library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <vmx/errnoLib.h>
#include <io/ioLib.h>
#include <io/ttyLib.h>

/* Imports */
#ifndef NO_EXCHOOKS
IMPORT FUNCPTR _func_excJobAdd;
#endif
#ifndef NO_SELECT
IMPORT FUNCPTR _func_selWakeupListInit;
IMPORT FUNCPTR _func_selWakeupAll;
#endif

/* Locals */
LOCAL int       ttyMutexOptions         = SEM_Q_FIFO | SEM_DELETE_SAFE;
LOCAL FUNCPTR   ttyAbortFunc            = NULL;
LOCAL char      ttyAbortChar            = 0x03;
LOCAL char      ttyBackSpaceChar        = 0x08;
LOCAL char      ttyDeleteLineChar       = 0x15;
LOCAL char      ttyEofChar              = 0x04;
LOCAL char      ttyMonitorTrapChar      = 0x18;
LOCAL int       ttyXoffTreshold         = 80;
LOCAL int       ttyXonTreshold          = 100;
LOCAL int       ttyWriteTreshold        = 20;
LOCAL int       ttyXoffChars            = 0;
LOCAL int       ttyXoffMaxChars         = 0;

LOCAL void ttyFlush(
    TTY_DEV_ID ttyId
    );

LOCAL void ttyFlushRead(
    TTY_DEV_ID ttyId
    );

LOCAL void ttyFlushWrite(
    TTY_DEV_ID ttyId
    );

LOCAL void ttyReadXoff(
    TTY_DEV_ID ttyId,
    BOOL xoff
    );

LOCAL void ttyWriteXoff(
    TTY_DEV_ID ttyId,
    BOOL xoff
    );

LOCAL void ttyTxStartup(
    TTY_DEV_ID ttyId
    );

#ifndef NO_SELECT
LOCAL void ttySelAdd(
    TTY_DEV_ID ttyId,
    int arg
    );

LOCAL void ttySelDelete(
    TTY_DEV_ID ttyId,
    int arg
    );
#endif

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
    )
{
    STATUS status;

    /* Clear struct */
    memset(ttyId, 0, sizeof(TTY_DEV));

    /* Allocate ring buffers */
    ttyId->readBuffer = rngCreate(readBufferSize);
    if (ttyId->readBuffer == NULL)
    {
        status = ERROR;
    }
    else
    {
        ttyId->writeBuffer = rngCreate(writeBufferSize);
        if (ttyId->writeBuffer == NULL)
        {
            rngDelete(ttyId->readBuffer);
            status = ERROR;
        }
        else
        {
            /* Initialize struct */
            ttyId->txStartup = txStartup;

            /* Initialize semaphores */
            semBInit(&ttyId->readSync, SEM_Q_PRIORITY, SEM_EMPTY);
            semBInit(&ttyId->writeSync, SEM_Q_PRIORITY, SEM_EMPTY);
            semMInit(&ttyId->mutex, ttyMutexOptions);

#ifndef NO_SELECT
            /* Initialize select list if installed */
            if (_func_selWakeupListInit != NULL)
            {
                (*_func_selWakeupListInit)(&ttyId->selWakeupList);
            }
#endif

            ttyFlush(ttyId);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * ttyDevRemove - Remove a tty device
 *
 * RETURNS: OR or ERROR
 */

STATUS ttyDevRemove(
    TTY_DEV_ID ttyId
    )
{
    STATUS status;

    /* Remove buffers */
    semTake(&ttyId->mutex, WAIT_FOREVER);

    if (ttyId->readBuffer == NULL)
    {
        semGive(&ttyId->mutex);
        status = ERROR;
    }
    else
    {
        ttyId->readState.flushingReadBuffer = TRUE;
        rngDelete(ttyId->readBuffer);

        if (ttyId->writeBuffer == NULL)
        {
            semGive(&ttyId->mutex);
            status = ERROR;
        }
        else
        {
            rngDelete(ttyId->writeBuffer);
            ttyId->writeState.flushingWriteBuffer = TRUE;
            semGive(&ttyId->mutex);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * ttyAbortFuncSet - Set abort function
 *
 * RETURNS: N/A
 */

void ttyAbortFuncSet(
    FUNCPTR func
    )
{
    ttyAbortFunc = func;
}

/******************************************************************************
 * ttyAbortSet - Set abort char
 *
 * RETURNS: N/A
 */

void ttyAbortSet(
    char c
    )
{
    ttyAbortChar = c;
}

/******************************************************************************
 * ttyBackSpaceSet - Set backspace char
 *
 * RETURNS: N/A
 */

void ttyBackSpaceSet(
    char c
    )
{
    ttyBackSpaceChar = c;
}

/******************************************************************************
 * ttyDeleteLineSet - Set line delete char
 *
 * RETURNS: N/A
 */

void ttyDeleteLineSet(
    char c
    )
{
    ttyDeleteLineChar = c;
}

/******************************************************************************
 * ttyEOFSet - Set end of file char
 *
 * RETURNS: N/A
 */

void ttyEOFSet(
    char c
    )
{
    ttyEofChar = c;
}

/******************************************************************************
 * ttyMonitorTrapSet - Set monitor trap char
 *
 * RETURNS: N/A
 */

void ttyMonitorTrapSet(
    char c
    )
{
    ttyMonitorTrapChar = c;
}

/******************************************************************************
 * ttyIoctl - Control request
 *
 * RETURNS: OK or ERROR
 */

int ttyIoctl(
    TTY_DEV_ID ttyId,
    int req,
    int arg
    )
{
    int old_opts;
    int rv;

    /* Handle request */
    switch(req)
    {
        case FIONREAD:
            *((int *) arg) = rngNBytes(ttyId->readBuffer);
            rv = OK;
            break;

        case FIONWRITE:
            *((int *) arg) = rngNBytes(ttyId->writeBuffer);
            rv = OK;
            break;

        case FIOFLUSH:
            ttyFlush(ttyId);
            rv = OK;
            break;

        case FIOCANCEL:
            semTake(&ttyId->mutex, WAIT_FOREVER);

            /* Cancel read transaction */
            ttyId->readState.canceled = TRUE;
            semGive(&ttyId->readSync);

            /* Cancel write transaction */
            ttyId->writeState.canceled = TRUE;
            semGive(&ttyId->writeSync);

            semGive(&ttyId->mutex);
            rv = OK;
            break;

        case FIORFLUSH:
            ttyFlushRead(ttyId);
            rv = OK;
            break;

        case FIOWFLUSH:
            ttyFlushWrite(ttyId);
            rv = OK;
            break;

        case FIOGETOPTIONS:
            rv = ttyId->options;

        case FIOSETOPTIONS:
            /* Store old */
            old_opts = arg;

            /* Set new */
            ttyId->options = arg;

            /* Check if read flushing is needed */
            if ((old_opts & OPT_LINE) != (ttyId->options & OPT_LINE))
            {
                ttyFlushRead(ttyId);
            }

            /* Check xoff options */
            if ((old_opts & OPT_TANDEM) && !(ttyId->options & OPT_TANDEM))
            {
                ttyReadXoff(ttyId, FALSE);
                ttyWriteXoff(ttyId, FALSE);
            }

            rv = OK;
            break;

        case FIOISATTY:
            rv = TRUE;
            break;

        case FIOPROTOHOOK:
            ttyId->protoHook = (FUNCPTR) arg;
            rv = OK;
            break;

        case FIOPROTOARG:
            ttyId->protoArg = (ARG) arg;
            rv = OK;
            break;

        case FIORBUFSET:
            semTake(&ttyId->mutex, WAIT_FOREVER);
            ttyId->readState.flushingReadBuffer = TRUE;

            /* Delete old read buffer */
            if (ttyId->readBuffer != NULL)
            {
                rngDelete(ttyId->readBuffer);
            }

            /* Create new read buffer with arumented size */
            ttyId->readBuffer = rngCreate(arg);
            if (ttyId->readBuffer == NULL)
            {
                rv = ERROR;
            }
            else
            {
                rv = OK;
            }

            ttyId->readState.flushingReadBuffer = FALSE;
            semGive(&ttyId->mutex);
            break;

        case FIOWBUFSET:
            semTake(&ttyId->mutex, WAIT_FOREVER);
            ttyId->writeState.flushingWriteBuffer = TRUE;

            /* Delete old write buffer */
            if (ttyId->writeBuffer != NULL)
            {
                rngDelete(ttyId->writeBuffer);
            }

            /* Create new write buffer with arumented size */
            ttyId->writeBuffer = rngCreate(arg);
            if (ttyId->writeBuffer == NULL)
            {
                rv = ERROR;
            }
            else
            {
                rv = OK;
            }

            ttyId->writeState.flushingWriteBuffer = FALSE;
            semGive(&ttyId->mutex);
            break;

#ifndef NO_SELECT
        case FIOSELECT:
            ttySelAdd(ttyId, arg);
            rv = OK;
            break;

        case FIOUNSELECT:
            ttySelDelete(ttyId, arg);
            rv = OK;
            break;
#endif

        default:
            rv = ERROR;
            break;

    }

    return rv;
}

/******************************************************************************
 * ttyWrite - Write to tty
 *
 * RETURNS: Number of bytes written
 */

int ttyWrite(
    TTY_DEV_ID ttyId,
    char *buffer,
    int nBytes
    )
{
    int bwrote;
    int nStart;
    int result = 0;

    nStart = nBytes;
    ttyId->writeState.canceled = FALSE;

    while (nBytes > 0)
    {
        semTake(&ttyId->writeSync, WAIT_FOREVER);
        semTake(&ttyId->mutex, WAIT_FOREVER);

        /* Check if write was canceled */
        if (ttyId->writeState.canceled == TRUE)
        {
            semGive(&ttyId->mutex);
            result = nStart - nBytes;
            break;
        }
        else
        {
            ttyId->writeState.writeBufferBusy = TRUE;
            bwrote = rngBufPut(ttyId->writeBuffer, buffer, nBytes);
            ttyId->writeState.writeBufferBusy = FALSE;

            ttyTxStartup(ttyId);

            nBytes -= bwrote;
            result += bwrote;
            buffer += bwrote;

            /* Check if more room is avilable */
            if (rngFreeBytes(ttyId->writeBuffer) > 0)
            {
                semGive(&ttyId->writeSync);
            }

            semGive(&ttyId->mutex);
        }
    }

    return result;
}

/******************************************************************************
 * ttyRead - Read from tty
 *
 * RETURNS: Number of bytes read
 */

int ttyRead(
    TTY_DEV_ID ttyId,
    char *buffer,
    int nBytes
    )
{
    int n, nn, freeBytes;
    BOOL canceled;
    RING_ID ringId;

    ttyId->readState.canceled = FALSE;
    canceled = FALSE;

    semTake(&ttyId->readSync, WAIT_FOREVER);

    /* Loop until read ring is not empty */
    while (1)
    {
        /* Don't know why sleep is needed here */
        taskDelay(1);

        semTake(&ttyId->mutex, WAIT_FOREVER);

        /* Check if write was canceled */
        if (ttyId->readState.canceled == TRUE)
        {
            semGive(&ttyId->mutex);
            n = 0;
            canceled = TRUE;
            break;
        }
        else
        {
            ringId = ttyId->readBuffer;
            if (rngIsEmpty(ringId) == FALSE)
            {
                break;
            }

            semGive(&ttyId->mutex);
        }
    }

    if (canceled == FALSE)
    {
        /* Get characters from ring buffer */
        if (ttyId->options & OPT_LINE)
        {
            if (ttyId->lnBytesLeft == 0)
            {
                RNG_ELEM_GET(ringId, &ttyId->lnBytesLeft, nn);
            }

            n = min((int) ttyId->lnBytesLeft, nBytes);
            rngBufGet(ringId, buffer, n);
            ttyId->lnBytesLeft -= n;
        }
        else
        {
            n = rngBufGet(ringId, buffer, nBytes);
        }

        /* Check xon */
        if ((ttyId->options & OPT_TANDEM) && (ttyId->readState.xoff == TRUE))
        {
            freeBytes = rngFreeBytes(ringId);
            if (ttyId->options & OPT_LINE)
            {
                freeBytes -= ttyId->lnNBytes + 1;
            }

            if (freeBytes > ttyXonTreshold)
            {
                ttyReadXoff(ttyId, FALSE);
            }
        }

        /* Check if there is more to read */
        if (rngIsEmpty(ringId) == FALSE)
        {
            semGive(&ttyId->readSync);
        }

        semGive(&ttyId->mutex);
    }

    return n;
}

/******************************************************************************
 * ttyTx - tty transmit from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS ttyIntTx(
    TTY_DEV_ID ttyId,
    char *pc
    )
{
    STATUS status;
    RING_ID ringId;
    int nn;

    ringId = ttyId->writeBuffer;

    /* Check xon/xoff */
    if (ttyId->readState.pending == TRUE)
    {
        ttyId->readState.pending = FALSE;
        *pc = (ttyId->readState.xoff == TRUE) ? XOFF : XON;

        if (ttyId->readState.xoff == TRUE)
        {
            if (ttyXoffChars > ttyXoffMaxChars)
            {
                ttyXoffMaxChars = ttyXoffChars;
            }
            ttyXoffChars = 0;
        }
    }
    else if ((ttyId->writeState.xoff == TRUE) ||
             (ttyId->writeState.flushingWriteBuffer == TRUE))
    {
        ttyId->writeState.busy = FALSE;
    }
    else if (ttyId->writeState.cr == TRUE)
    {
        *pc = '\n';
        ttyId->writeState.cr = FALSE;
    }
    else if (RNG_ELEM_GET(ringId, pc, nn) == FALSE)
    {
        ttyId->writeState.busy = FALSE;
    }
    else
    {
        ttyId->writeState.busy = TRUE;
        if ((ttyId->options & OPT_CRMOD) && (*pc == '\n'))
        {
            *pc = '\r';
            ttyId->writeState.cr = TRUE;
        }

        if (rngFreeBytes(ringId) == ttyWriteTreshold)
        {
            semGive(&ttyId->writeSync);
#ifndef NO_SELECT
            if (_func_selWakeupAll != NULL)
            {
                (*_func_selWakeupAll)(&ttyId->selWakeupList, SELWRITE);
            }
#endif
        }
    }

    if (ttyId->writeState.busy == FALSE)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * ttyIntRd - tty read from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS ttyIntRd(
    TTY_DEV_ID ttyId,
    char c
    )
{
    RING_ID ringId;
    int nn, freeBytes;
    BOOL hookRv;
    BOOL echoed;
    BOOL releaseTaskLevel;
    int options = ttyId->options;
    STATUS status = OK;

    if (ttyId->readState.flushingReadBuffer == TRUE)
    {
        status = ERROR;
    }
    else
    {
        if (ttyId->protoHook != NULL)
        {
            hookRv = (*ttyId->protoHook)(ttyId->protoArg, c);
        }
        else
        {
            hookRv = FALSE;
        }

        if (hookRv != TRUE)
        {
            /* Check 7 bit */
            if (options & OPT_7_BIT)
            {
                c &= 0x7f;
            }

            /* Check for abort */
            if ((c == ttyAbortChar) && (options & OPT_ABORT))
            {
                if (ttyAbortFunc != NULL)
                {
                    (*ttyAbortFunc)();
                }
            }
            else if ((c == ttyMonitorTrapChar) && (options & OPT_MON_TRAP))
            {
#ifndef NO_EXCHOOKS
                if (_func_excJobAdd != NULL)
                {
                    (*_func_excJobAdd)(
                        sysReboot,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0,
                        (ARG) 0
                        );
                }
                else
                {
                    sysReboot();
                }
#endif
            }
            else if (((c == XOFF) || (c == XOFF)) && (options & OPT_TANDEM))
            {
                ttyWriteXoff (ttyId, (c == XOFF));
            }
            else
            {
                /* Count number of chars while in xoff */
                if (ttyId->readState.xoff == TRUE)
                {
                    ttyXoffChars++;
                }

                /* Check carriage return */
                if ((options & OPT_CRMOD) && (c == '\r'))
                {
                    c = '\n';
                }

                /* Check for echo on */
                if ((options & OPT_ECHO) &&
                    (ttyId->writeState.writeBufferBusy == FALSE) &&
                    (ttyId->writeState.flushingWriteBuffer == FALSE))
                {
                    ringId = ttyId->writeBuffer;
                    echoed = FALSE;

                    /* Check for line options */
                    if (options & OPT_LINE)
                    {
                        if (c == ttyDeleteLineChar)
                        {
                            RNG_ELEM_PUT(ringId, '\n', nn);
                            echoed = TRUE;
                        }
                        else if (c == ttyBackSpaceChar)
                        {
                            if (ttyId->lnNBytes != 0)
                            {
                                /* echo backspace-space-backspace */
                                rngBufPut(ringId, "\b \b", 3);
                                echoed = TRUE;
                            }
                        }
                        else if ((c < 0x20) && (c != '\n'))
                        {
                            /* echo ^-c */
                            RNG_ELEM_PUT(ringId, '^', nn);
                            RNG_ELEM_PUT(ringId, c + '@', nn);
                            echoed = TRUE;
                        }
                        else
                        {
                            RNG_ELEM_PUT(ringId, c, nn);
                            echoed = TRUE;
                        }
                    }
                    else
                    {
                        RNG_ELEM_PUT(ringId, c, nn);
                        echoed = TRUE;
                    }

                    /* If echo start tx */
                    if (echoed == TRUE)
                    {
                        ttyTxStartup(ttyId);
                    }
                }

                /* Put char in read buffer */
                ringId = ttyId->readBuffer;
                releaseTaskLevel = FALSE;

                /* Check for non-line options */
                if (!(options & OPT_LINE))
                {
                    if(RNG_ELEM_PUT(ringId, c, nn) == FALSE)
                    {
                        status = ERROR;
                    }

                    if (rngNBytes(ringId) == 1)
                    {
                        releaseTaskLevel = TRUE;
                    }
                }
                else
                {
                    freeBytes = rngFreeBytes(ringId);

                    if (c == ttyBackSpaceChar)
                    {
                        if (ttyId->lnNBytes != 0)
                        {
                            ttyId->lnNBytes--;
                        }
                    }
                    else if (c == ttyDeleteLineChar)
                    {
                        ttyId->lnNBytes = 0;
                    }
                    else if (c == ttyEofChar)
                    {
                        if (freeBytes > 0)
                        {
                            releaseTaskLevel = TRUE;
                        }
                    }
                    else
                    {
                        /* Check for freeBytes >= 2 */
                        if (freeBytes >= 2)
                        {
                            if (freeBytes >= (ttyId->lnNBytes + 2))
                            {
                                ttyId->lnNBytes++;
                            }
                            else
                            {
                                status = ERROR;
                            }

                            rngPutAhead(ringId, c, (int) ttyId->lnNBytes);

                            if ((c == '\n') || (ttyId->lnNBytes == 255))
                            {
                                releaseTaskLevel = TRUE;
                            }
                        }
                        else
                        {
                            status = ERROR;
                        }
                    }

                    if (releaseTaskLevel == TRUE)
                    {
                        rngPutAhead(ringId, (char) ttyId->lnNBytes, 0);
                        rngMoveAhead(ringId, (int) ttyId->lnNBytes + 1);
                        ttyId->lnNBytes = 0;
                    }
                }

                /* Check for xon/xoff */
                if (options & OPT_TANDEM)
                {
                    freeBytes = rngFreeBytes(ringId);
                    if (ttyId->options & OPT_LINE)
                    {
                        freeBytes -= ttyId->lnNBytes + 1;
                    }

                    if (ttyId->readState.xoff == FALSE)
                    {
                        if (freeBytes < ttyXoffTreshold)
                        {
                            ttyReadXoff(ttyId, TRUE);
                        }
                    }
                    else
                    {
                        if (freeBytes > ttyXonTreshold)
                        {
                            ttyReadXoff(ttyId, FALSE);
                        }
                    }
                }

                if (releaseTaskLevel == TRUE)
                {
                    semGive(&ttyId->readSync);
#ifndef NO_SELECT
                    if (_func_selWakeupAll != NULL)
                    {
                        (*_func_selWakeupAll)(&ttyId->selWakeupList, SELREAD);
                    }
#endif
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * ttyFlush - Flush a tty device
 *
 * RETURNS: N/A
 */

LOCAL void ttyFlush(
    TTY_DEV_ID ttyId
    )
{
    ttyFlushRead(ttyId);
    ttyFlushWrite(ttyId);
}

/******************************************************************************
 * ttyFlushRead - Flush a tty devices read buffer
 *
 * RETURNS: N/A
 */

LOCAL void ttyFlushRead(
    TTY_DEV_ID ttyId
    )
{
    semTake(&ttyId->mutex, WAIT_FOREVER);
    ttyId->readState.flushingReadBuffer = TRUE;
    rngFlush(ttyId->readBuffer);

    ttyId->lnNBytes = 0;
    ttyId->lnBytesLeft = 0;
    ttyReadXoff(ttyId, FALSE);
    ttyId->readState.flushingReadBuffer = FALSE;
    semGive(&ttyId->mutex);
}

/******************************************************************************
 * ttyFlushWrite - Flush a tty devices write buffer
 *
 * RETURNS: N/A
 */

LOCAL void ttyFlushWrite(
    TTY_DEV_ID ttyId
    )
{
    semTake(&ttyId->mutex, WAIT_FOREVER);
    ttyId->writeState.flushingWriteBuffer = TRUE;
    rngFlush(ttyId->writeBuffer);
    semGive(&ttyId->writeSync);

    ttyId->writeState.flushingWriteBuffer = FALSE;
    semGive(&ttyId->mutex);

#ifndef NO_SELECT
    /* Wakeup select if installed */
    if (_func_selWakeupAll != NULL)
    {
        (*_func_selWakeupAll) (&ttyId->selWakeupList, SELWRITE);
    }
#endif
}

/******************************************************************************
 * ttyReadXoff - Set read xon/xoff
 *
 * RETURNS: N/A
 */

LOCAL void ttyReadXoff(
    TTY_DEV_ID ttyId,
    BOOL xoff
    )
{
    int level;

    INT_LOCK(level);

    if (ttyId->readState.xoff != xoff)
    {
        ttyId->readState.xoff = xoff;
        ttyId->readState.pending = TRUE;

        if (ttyId->writeState.busy == FALSE)
        {
            ttyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*ttyId->txStartup)(ttyId);
        }
        else
        {
            INT_UNLOCK(level);
        }
    }
    else
    {
        INT_UNLOCK(level);
    }
}

/******************************************************************************
 * ttyWriteXoff - Set write xon/xoff
 *
 * RETURNS: N/A
 */

LOCAL void ttyWriteXoff(
    TTY_DEV_ID ttyId,
    BOOL xoff
    )
{
    int level;

    INT_LOCK(level);

    if (ttyId->writeState.xoff != xoff)
    {
        ttyId->writeState.xoff = xoff;

        if ((xoff == FALSE) && (ttyId->writeState.busy == FALSE))
        {
            ttyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*ttyId->txStartup)(ttyId);
        }
        else
        {
            INT_UNLOCK(level);
        }
    }
    else
    {
        INT_UNLOCK(level);
    }
}

/******************************************************************************
 * ttyTxStartup - Start transmitter if nessasary
 *
 * RETURNS: N/A
 */

LOCAL void ttyTxStartup(
    TTY_DEV_ID ttyId
    )
{
    int level;

    if (ttyId->writeState.busy == FALSE)
    {
        INT_LOCK(level);

        if (ttyId->writeState.busy == FALSE)
        {
            ttyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*ttyId->txStartup)(ttyId);
        }
        else
        {
            INT_UNLOCK(level);
        }
    }
}

#ifndef NO_SELECT
/*****************************************************************************
 * ttySelAdd - Ioctl add select on file descriptor
 *
 * RETURNS: N/A
 */

LOCAL void ttySelAdd(
    TTY_DEV_ID ttyId,
    int arg
    )
{

    /* Add select node to tty wakeup list */
    selNodeAdd(&ttyId->selWakeupList, (SEL_WAKEUP_NODE *) arg);

    /* If select on read */
    if ((selWakeupType((SEL_WAKEUP_NODE *) arg) == SELREAD) &&
        (rngNBytes(ttyId->readBuffer) > 0))
    {
        selWakeup((SEL_WAKEUP_NODE *) arg);
    }

    /* If select on write */
    if ((selWakeupType((SEL_WAKEUP_NODE *) arg) == SELWRITE) &&
       (rngFreeBytes(ttyId->writeBuffer) > 0))
    {
        selWakeup((SEL_WAKEUP_NODE *) arg);
    }
}

/*****************************************************************************
 * ttySelDelete - Ioctl delete select on file descriptor
 *
 * RETURNS: N/A
 */

LOCAL void ttySelDelete(
    TTY_DEV_ID ttyId,
    int arg
    )
{
    selNodeDelete(&ttyId->selWakeupList, (SEL_WAKEUP_NODE *) arg);
}
#endif

