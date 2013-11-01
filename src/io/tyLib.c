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

/* tyLib.c - Typewriter support library */

#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <vmx/errnoLib.h>
#include <io/ioLib.h>
#include <io/tyLib.h>

#define XON                     0x11
#define XOFF                    0x13

/* Imports */
#ifndef NO_EXCHOOKS
IMPORT FUNCPTR _func_excJobAdd;
#endif
#ifndef NO_SELECT
IMPORT FUNCPTR _func_selWakeupListInit;
IMPORT FUNCPTR _func_selWakeupAll;
#endif

/* Locals */
LOCAL int       tyMutexOptions          = SEM_Q_FIFO | SEM_DELETE_SAFE;
LOCAL FUNCPTR   tyAbortFunc             = NULL;
LOCAL char      tyAbortChar             = 0x03;
LOCAL char      tyBackSpaceChar         = 0x08;
LOCAL char      tyDeleteLineChar        = 0x15;
LOCAL char      tyEofChar               = 0x04;
LOCAL char      tyMonitorTrapChar       = 0x18;
LOCAL int       tyXoffTreshold          = 80;
LOCAL int       tyXonTreshold           = 100;
LOCAL int       tyWriteTreshold         = 20;
LOCAL int       tyXoffChars             = 0;
LOCAL int       tyXoffMaxChars          = 0;

LOCAL void tyFlush(
    TY_DEV_ID tyId
    );

LOCAL void tyFlushRead(
    TY_DEV_ID tyId
    );

LOCAL void tyFlushWrite(
    TY_DEV_ID tyId
    );

LOCAL void tyReadXoff(
    TY_DEV_ID tyId,
    BOOL xoff
    );

LOCAL void tyWriteXoff(
    TY_DEV_ID tyId,
    BOOL xoff
    );

LOCAL void tyTxStartup(
    TY_DEV_ID tyId
    );

#ifndef NO_SELECT
LOCAL void tySelAdd(
    TY_DEV_ID tyId,
    int arg
    );

LOCAL void tySelDelete(
    TY_DEV_ID tyId,
    int arg
    );
#endif

/******************************************************************************
 * tyDevInit - Intialize a typewriter device
 *
 * RETURNS: OR or ERROR
 */

STATUS tyDevInit(
    TY_DEV_ID tyId,
    int readBufferSize,
    int writeBufferSize,
    FUNCPTR txStartup
    )
{
    STATUS status;

    /* Clear struct */
    memset(tyId, 0, sizeof(TY_DEV));

    /* Allocate ring buffers */
    tyId->readBuffer = rngCreate(readBufferSize);
    if (tyId->readBuffer == NULL)
    {
        status = ERROR;
    }
    else
    {
        tyId->writeBuffer = rngCreate(writeBufferSize);
        if (tyId->writeBuffer == NULL)
        {
            rngDelete(tyId->readBuffer);
            status = ERROR;
        }
        else
        {
            /* Initialize struct */
            tyId->txStartup = txStartup;

            /* Initialize semaphores */
            semBInit(&tyId->readSync, SEM_Q_PRIORITY, SEM_EMPTY);
            semBInit(&tyId->writeSync, SEM_Q_PRIORITY, SEM_EMPTY);
            semMInit(&tyId->mutex, tyMutexOptions);

#ifndef NO_SELECT
            /* Initialize select list if installed */
            if (_func_selWakeupListInit != NULL)
            {
                (*_func_selWakeupListInit)(&tyId->selWakeupList);
            }
#endif

            tyFlush(tyId);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * tyDevRemove - Remove a typewriter device
 *
 * RETURNS: OR or ERROR
 */

STATUS tyDevRemove(
    TY_DEV_ID tyId
    )
{
    STATUS status;

    /* Remove buffers */
    semTake(&tyId->mutex, WAIT_FOREVER);

    if (tyId->readBuffer == NULL)
    {
        semGive(&tyId->mutex);
        status = ERROR;
    }
    else
    {
        tyId->readState.flushingReadBuffer = TRUE;
        rngDelete(tyId->readBuffer);

        if (tyId->writeBuffer == NULL)
        {
            semGive(&tyId->mutex);
            status = ERROR;
        }
        else
        {
            rngDelete(tyId->writeBuffer);
            tyId->writeState.flushingWriteBuffer = TRUE;
            semGive(&tyId->mutex);
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * tyAbortFuncSet - Set abort function
 *
 * RETURNS: N/A
 */

void tyAbortFuncSet(
    FUNCPTR func
    )
{
    tyAbortFunc = func;
}

/******************************************************************************
 * tyAbortSet - Set abort char
 *
 * RETURNS: N/A
 */

void tyAbortSet(
    char c
    )
{
    tyAbortChar = c;
}

/******************************************************************************
 * tyBackSpaceSet - Set backspace char
 *
 * RETURNS: N/A
 */

void tyBackSpaceSet(
    char c
    )
{
    tyBackSpaceChar = c;
}

/******************************************************************************
 * tyDeleteLineSet - Set line delete char
 *
 * RETURNS: N/A
 */

void tyDeleteLineSet(
    char c
    )
{
    tyDeleteLineChar = c;
}

/******************************************************************************
 * tyEOFSet - Set end of file char
 *
 * RETURNS: N/A
 */

void tyEOFSet(
    char c
    )
{
    tyEofChar = c;
}

/******************************************************************************
 * tyMonitorTrapSet - Set monitor trap char
 *
 * RETURNS: N/A
 */

void tyMonitorTrapSet(
    char c
    )
{
    tyMonitorTrapChar = c;
}

/******************************************************************************
 * tyIoctl - Control request
 *
 * RETURNS: OK or ERROR
 */

int tyIoctl(
    TY_DEV_ID tyId,
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
            *((int *) arg) = rngNBytes(tyId->readBuffer);
            rv = OK;
            break;

        case FIONWRITE:
            *((int *) arg) = rngNBytes(tyId->writeBuffer);
            rv = OK;
            break;

        case FIOFLUSH:
            tyFlush(tyId);
            rv = OK;
            break;

        case FIOCANCEL:
            semTake(&tyId->mutex, WAIT_FOREVER);

            /* Cancel read transaction */
            tyId->readState.canceled = TRUE;
            semGive(&tyId->readSync);

            /* Cancel write transaction */
            tyId->writeState.canceled = TRUE;
            semGive(&tyId->writeSync);

            semGive(&tyId->mutex);
            rv = OK;
            break;

        case FIORFLUSH:
            tyFlushRead(tyId);
            rv = OK;
            break;

        case FIOWFLUSH:
            tyFlushWrite(tyId);
            rv = OK;
            break;

        case FIOGETOPTIONS:
            rv = tyId->options;
            break;

        case FIOSETOPTIONS:
            /* Store old */
            old_opts = tyId->options;

            /* Set new */
            tyId->options = arg;

            /* Check if read flushing is needed */
            if ((old_opts & OPT_LINE) != (tyId->options & OPT_LINE))
            {
                tyFlushRead(tyId);
            }

            /* Check xoff options */
            if ((old_opts & OPT_TANDEM) && !(tyId->options & OPT_TANDEM))
            {
                tyReadXoff(tyId, FALSE);
                tyWriteXoff(tyId, FALSE);
            }

            rv = OK;
            break;

        case FIOISATTY:
            rv = TRUE;
            break;

        case FIOPROTOHOOK:
            tyId->protoHook = (FUNCPTR) arg;
            rv = OK;
            break;

        case FIOPROTOARG:
            tyId->protoArg = (ARG) arg;
            rv = OK;
            break;

        case FIORBUFSET:
            semTake(&tyId->mutex, WAIT_FOREVER);

            tyId->readState.flushingReadBuffer = TRUE;

            /* Delete old read buffer */
            if (tyId->readBuffer != NULL)
            {
                rngDelete(tyId->readBuffer);
            }

            /* Create new read buffer with arumented size */
            tyId->readBuffer = rngCreate(arg);
            if (tyId->readBuffer == NULL)
            {
                rv = ERROR;
            }
            else
            {
                rv = OK;
            }

            tyId->readState.flushingReadBuffer = FALSE;

            semGive(&tyId->mutex);
            break;

        case FIOWBUFSET:
            semTake(&tyId->mutex, WAIT_FOREVER);

            tyId->writeState.flushingWriteBuffer = TRUE;

            /* Delete old write buffer */
            if (tyId->writeBuffer != NULL)
            {
                rngDelete(tyId->writeBuffer);
            }

            /* Create new write buffer with arumented size */
            tyId->writeBuffer = rngCreate(arg);
            if (tyId->writeBuffer == NULL)
            {
                rv = ERROR;
            }
            else
            {
                rv = OK;
            }

            tyId->writeState.flushingWriteBuffer = FALSE;

            semGive(&tyId->mutex);
            break;

#ifndef NO_SELECT
        case FIOSELECT:
            tySelAdd(tyId, arg);
            rv = OK;
            break;

        case FIOUNSELECT:
            tySelDelete(tyId, arg);
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
 * tyWrite - Write to typewriter
 *
 * RETURNS: Number of bytes written
 */

int tyWrite(
    TY_DEV_ID tyId,
    char *buffer,
    int nBytes
    )
{
    int bwrote;
    int nStart = nBytes;
    int result = 0;

    tyId->writeState.canceled = FALSE;

    while (nBytes > 0)
    {
        semTake(&tyId->writeSync, WAIT_FOREVER);
        semTake(&tyId->mutex, WAIT_FOREVER);

        /* Check if write was canceled */
        if (tyId->writeState.canceled == TRUE)
        {
            semGive(&tyId->mutex);
            errnoSet(S_ioLib_CANCELLED);
            result = nStart - nBytes;
            break;
        }
        else
        {
            tyId->writeState.writeBufferBusy = TRUE;
            bwrote = rngBufPut(tyId->writeBuffer, buffer, nBytes);
            tyId->writeState.writeBufferBusy = FALSE;

            tyTxStartup(tyId);

            nBytes -= bwrote;
            result += bwrote;
            buffer += bwrote;

            /* Check if more room is avilable */
            if (rngFreeBytes(tyId->writeBuffer) > 0)
            {
                semGive(&tyId->writeSync);
            }

            semGive(&tyId->mutex);
        }
    }

    return result;
}

/******************************************************************************
 * tyRead - Read from typewriter
 *
 * RETURNS: Number of bytes read
 */

int tyRead(
    TY_DEV_ID tyId,
    char *buffer,
    int nBytes
    )
{
    int n, nn, freeBytes;
    BOOL canceled;
    RING_ID ringId;

    tyId->readState.canceled = FALSE;
    canceled = FALSE;

    /* Loop until read ring is not empty */
    while (1)
    {
        /* Don't know why sleep is needed here */
        /* taskDelay(1); */

        semTake(&tyId->readSync, WAIT_FOREVER);
        semTake(&tyId->mutex, WAIT_FOREVER);

        /* Check if write was canceled */
        if (tyId->readState.canceled == TRUE)
        {
            semGive(&tyId->mutex);
            errnoSet(S_ioLib_CANCELLED);
            n = 0;
            canceled = TRUE;
            break;
        }
        else
        {
            ringId = tyId->readBuffer;
            if (rngIsEmpty(ringId) == FALSE)
            {
                break;
            }

            semGive(&tyId->mutex);
        }
    }

    if (canceled == FALSE)
    {
        /* Get characters from ring buffer */
        if (tyId->options & OPT_LINE)
        {
            if (tyId->lnBytesLeft == 0)
            {
                RNG_ELEM_GET(ringId, &tyId->lnBytesLeft, nn);
            }

            n = min((int) tyId->lnBytesLeft, nBytes);
            rngBufGet(ringId, buffer, n);
            tyId->lnBytesLeft -= n;
        }
        else
        {
            n = rngBufGet(ringId, buffer, nBytes);
        }

        /* Check xon */
        if ((tyId->options & OPT_TANDEM) && (tyId->readState.xoff == TRUE))
        {
            freeBytes = rngFreeBytes(ringId);
            if (tyId->options & OPT_LINE)
            {
                freeBytes -= tyId->lnNBytes + 1;
            }

            if (freeBytes > tyXonTreshold)
            {
                tyReadXoff(tyId, FALSE);
            }
        }

        /* Check if there is more to read */
        if (rngIsEmpty(ringId) == FALSE)
        {
            semGive(&tyId->readSync);
        }

        semGive(&tyId->mutex);
    }

    return n;
}

/******************************************************************************
 * tyTx - Typewriter transmit from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS tyIntTx(
    TY_DEV_ID tyId,
    char *pc
    )
{
    STATUS status;
    RING_ID ringId;
    int nn;

    ringId = tyId->writeBuffer;

    /* Check xon/xoff */
    if (tyId->readState.pending == TRUE)
    {
        tyId->readState.pending = FALSE;
        *pc = (tyId->readState.xoff == TRUE) ? XOFF : XON;

        if (tyId->readState.xoff == TRUE)
        {
            if (tyXoffChars > tyXoffMaxChars)
            {
                tyXoffMaxChars = tyXoffChars;
            }
            tyXoffChars = 0;
        }
    }
    else if ((tyId->writeState.xoff == TRUE) ||
             (tyId->writeState.flushingWriteBuffer == TRUE))
    {
        tyId->writeState.busy = FALSE;
    }
    else if (tyId->writeState.cr == TRUE)
    {
        *pc = '\n';
        tyId->writeState.cr = FALSE;
    }
    else if (RNG_ELEM_GET(ringId, pc, nn) == FALSE)
    {
        tyId->writeState.busy = FALSE;
    }
    else
    {
        tyId->writeState.busy = TRUE;

        if ((tyId->options & OPT_CRMOD) && (*pc == '\n'))
        {
            *pc = '\r';
            tyId->writeState.cr = TRUE;
        }

        if (rngFreeBytes(ringId) == tyWriteTreshold)
        {
            semGive(&tyId->writeSync);
#ifndef NO_SELECT
            if (_func_selWakeupAll != NULL)
            {
                (*_func_selWakeupAll)(&tyId->selWakeupList, SELWRITE);
            }
#endif
        }
    }

    if (tyId->writeState.busy == FALSE)
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
 * tyIntRd - Typewriter read from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS tyIntRd(
    TY_DEV_ID tyId,
    char c
    )
{
    RING_ID ringId;
    int nn, freeBytes;
    BOOL hookRv;
    BOOL releaseTaskLevel;
    int options = tyId->options;
    BOOL echoed = FALSE;
    STATUS status = OK;

    if (tyId->readState.flushingReadBuffer == TRUE)
    {
        status = ERROR;
    }
    else
    {
        if (tyId->protoHook != NULL)
        {
            hookRv = (*tyId->protoHook)(tyId->protoArg, c);
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
            if ((c == tyAbortChar) && (options & OPT_ABORT))
            {
                if (tyAbortFunc != NULL)
                {
                    (*tyAbortFunc)();
                }
            }
            else if ((c == tyMonitorTrapChar) && (options & OPT_MON_TRAP))
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
                tyWriteXoff(tyId, (c == XOFF) ? TRUE : FALSE);
            }
            else
            {
                /* Count number of chars while in xoff */
                if (tyId->readState.xoff == TRUE)
                {
                    tyXoffChars++;
                }

                /* Check carriage return */
                if ((options & OPT_CRMOD) && (c == '\r'))
                {
                    c = '\n';
                }

                /* Check for echo on */
                if ((options & OPT_ECHO) &&
                    (tyId->writeState.writeBufferBusy == FALSE) &&
                    (tyId->writeState.flushingWriteBuffer == FALSE))
                {
                    ringId = tyId->writeBuffer;

                    /* Check for line options */
                    if (options & OPT_LINE)
                    {
                        if (c == tyDeleteLineChar)
                        {
                            RNG_ELEM_PUT(ringId, '\n', nn);
                            echoed = TRUE;
                        }
                        else if (c == tyBackSpaceChar)
                        {
                            if (tyId->lnNBytes != 0)
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
                        tyTxStartup(tyId);
                    }
                }

                /* Put char in read buffer */
                ringId = tyId->readBuffer;
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

                    if (c == tyBackSpaceChar)
                    {
                        if (tyId->lnNBytes != 0)
                        {
                            tyId->lnNBytes--;
                        }
                    }
                    else if (c == tyDeleteLineChar)
                    {
                        tyId->lnNBytes = 0;
                    }
                    else if (c == tyEofChar)
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
                            if (freeBytes >= (tyId->lnNBytes + 2))
                            {
                                tyId->lnNBytes++;
                            }
                            else
                            {
                                status = ERROR;
                            }

                            rngPutAhead(ringId, c, (int) tyId->lnNBytes);

                            if ((c == '\n') || (tyId->lnNBytes == 255))
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
                        rngPutAhead(ringId, (char) tyId->lnNBytes, 0);
                        rngMoveAhead(ringId, (int) tyId->lnNBytes + 1);
                        tyId->lnNBytes = 0;
                    }
                }

                /* Check for xon/xoff */
                if (options & OPT_TANDEM)
                {
                    freeBytes = rngFreeBytes(ringId);
                    if (tyId->options & OPT_LINE)
                    {
                        freeBytes -= tyId->lnNBytes + 1;
                    }

                    if (tyId->readState.xoff == FALSE)
                    {
                        if (freeBytes < tyXoffTreshold)
                        {
                            tyReadXoff(tyId, TRUE);
                        }
                    }
                    else
                    {
                        if (freeBytes > tyXonTreshold)
                        {
                            tyReadXoff(tyId, FALSE);
                        }
                    }
                }

                if (releaseTaskLevel == TRUE)
                {
                    semGive(&tyId->readSync);
#ifndef NO_SELECT
                    if (_func_selWakeupAll != NULL)
                    {
                        (*_func_selWakeupAll)(&tyId->selWakeupList, SELREAD);
                    }
#endif
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * tyFlush - Flush a typewriter device
 *
 * RETURNS: N/A
 */

LOCAL void tyFlush(
    TY_DEV_ID tyId
    )
{
    tyFlushRead(tyId);
    tyFlushWrite(tyId);
}

/******************************************************************************
 * tyFlushRead - Flush a typewriter devices read buffer
 *
 * RETURNS: N/A
 */

LOCAL void tyFlushRead(
    TY_DEV_ID tyId
    )
{
    int oldErrno;

    semTake(&tyId->mutex, WAIT_FOREVER);

    tyId->readState.flushingReadBuffer = TRUE;
    rngFlush(tyId->readBuffer);

    oldErrno = errnoGet();
    semTake(&tyId->readSync, WAIT_NONE);
    if (errnoGet() == S_objLib_UNAVAILABLE)
    {
        errnoSet(oldErrno);
    }

    tyId->lnNBytes = 0;
    tyId->lnBytesLeft = 0;

    tyReadXoff(tyId, FALSE);

    tyId->readState.flushingReadBuffer = FALSE;

    semGive(&tyId->mutex);
}

/******************************************************************************
 * tyFlushWrite - Flush a typewriter device write buffer
 *
 * RETURNS: N/A
 */

LOCAL void tyFlushWrite(
    TY_DEV_ID tyId
    )
{
    semTake(&tyId->mutex, WAIT_FOREVER);

    tyId->writeState.flushingWriteBuffer = TRUE;
    rngFlush(tyId->writeBuffer);
    semGive(&tyId->writeSync);

    tyId->writeState.flushingWriteBuffer = FALSE;

    semGive(&tyId->mutex);

#ifndef NO_SELECT
    /* Wakeup select if installed */
    if (_func_selWakeupAll != NULL)
    {
        (*_func_selWakeupAll) (&tyId->selWakeupList, SELWRITE);
    }
#endif
}

/******************************************************************************
 * tyReadXoff - Set read xon/xoff
 *
 * RETURNS: N/A
 */

LOCAL void tyReadXoff(
    TY_DEV_ID tyId,
    BOOL xoff
    )
{
    int level;

    INT_LOCK(level);

    if (tyId->readState.xoff != xoff)
    {
        tyId->readState.xoff = xoff;
        tyId->readState.pending = TRUE;

        if (tyId->writeState.busy == FALSE)
        {
            tyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*tyId->txStartup)(tyId);
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
 * tyWriteXoff - Set write xon/xoff
 *
 * RETURNS: N/A
 */

LOCAL void tyWriteXoff(
    TY_DEV_ID tyId,
    BOOL xoff
    )
{
    int level;

    INT_LOCK(level);

    if (tyId->writeState.xoff != xoff)
    {
        tyId->writeState.xoff = xoff;

        if ((xoff == FALSE) && (tyId->writeState.busy == FALSE))
        {
            tyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*tyId->txStartup)(tyId);
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
 * tyTxStartup - Start transmitter if nessasary
 *
 * RETURNS: N/A
 */

LOCAL void tyTxStartup(
    TY_DEV_ID tyId
    )
{
    int level;

    if (tyId->writeState.busy == FALSE)
    {
        INT_LOCK(level);

        if (tyId->writeState.busy == FALSE)
        {
            tyId->writeState.busy = TRUE;
            INT_UNLOCK(level);
            (*tyId->txStartup)(tyId);
        }
        else
        {
            INT_UNLOCK(level);
        }
    }
}

#ifndef NO_SELECT
/*****************************************************************************
 * tySelAdd - Ioctl add select on file descriptor
 *
 * RETURNS: N/A
 */

LOCAL void tySelAdd(
    TY_DEV_ID tyId,
    int arg
    )
{

    /* Add select node to ty wakeup list */
    selNodeAdd(&tyId->selWakeupList, (SEL_WAKEUP_NODE *) arg);

    /* If select on read */
    if ((selWakeupType((SEL_WAKEUP_NODE *) arg) == SELREAD) &&
        (rngNBytes(tyId->readBuffer) > 0))
    {
        selWakeup((SEL_WAKEUP_NODE *) arg);
    }

    /* If select on write */
    if ((selWakeupType((SEL_WAKEUP_NODE *) arg) == SELWRITE) &&
       (rngFreeBytes(tyId->writeBuffer) > 0))
    {
        selWakeup((SEL_WAKEUP_NODE *) arg);
    }
}

/*****************************************************************************
 * tySelDelete - Ioctl delete select on file descriptor
 *
 * RETURNS: N/A
 */

LOCAL void tySelDelete(
    TY_DEV_ID tyId,
    int arg
    )
{
    selNodeDelete(&tyId->selWakeupList, (SEL_WAKEUP_NODE *) arg);
}
#endif

