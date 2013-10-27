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

/* rngLib.c - Ring buffer library */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <util/rngLib.h>

/******************************************************************************
 * rngCreate - Create a ring buffer
 *
 * RETURNS: Pointer to ring buffer or NULL
 */

RING_ID rngCreate(
    int size
    )
{
    char *buffer;
    RING_ID ringId;

    ringId = (RING_ID) malloc(sizeof(RING));
    if (ringId != NULL)
    {
        buffer = (char *) malloc(++size);
        if (buffer == NULL)
        {
            free(ringId);
            ringId = NULL;
        }
        else
        {
            ringId->bufferSize = size;
            ringId->buffer     = buffer;

            rngFlush(ringId);
        }
    }

    return ringId;
}

/******************************************************************************
 * rngDelete - Delete a ring buffer
 *
 * RETURNS: OK or ERROR
 */

STATUS rngDelete(
    RING_ID ringId
    )
{
    STATUS status;

    if (ringId == NULL)
    {
        status = ERROR;
    }
    else
    {
        free(ringId->buffer);
        free(ringId);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * rngFlush - Flush a ring buffer
 *
 * RETURNS: OK or ERROR
 */

STATUS rngFlush(
    RING_ID ringId
    )
{
    STATUS status;

    if (ringId == NULL)
    {
        status = ERROR;
    }
    else
    {
        ringId->offsetToBuffer   = 0;
        ringId->offsetFromBuffer = 0;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * rngElemGet - Get element from ring buffer
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngElemGet(
    RING_ID ringId,
    char *pChar,
    int *pOffsetFrom
    )
{
    BOOL ret;
    int offset = ringId->offsetFromBuffer;

    if (ringId->offsetToBuffer == offset)
    {
        ret = FALSE;
    }
    else
    {
        *pChar = ringId->buffer[offset];
        if (++offset == ringId->bufferSize)
        {
            ringId->offsetFromBuffer = 0;
        }
        else
        {
            ringId->offsetFromBuffer = offset;
        }
        ret = TRUE;
    }

    *pOffsetFrom = offset;
    return ret;
}

/******************************************************************************
 * rngElemPut - Put element on ring buffer
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngElemPut(
    RING_ID ringId,
    char c,
    int *pOffsetTo
    )
{
    BOOL ret;
    int offset = ringId->offsetToBuffer;

    if (offset == ringId->offsetFromBuffer - 1)
    {
        ret = FALSE;
    }
    else
    {
        if (offset == (ringId)->bufferSize - 1)
        {
            if (ringId->offsetFromBuffer == 0)
            {
                ret = FALSE;
            }
            else
            {
                ringId->buffer[offset] = c;
                ringId->offsetToBuffer = 0;
                ret = TRUE;
            }
        }
        else
        {
            ringId->buffer[offset] = c;
            ringId->offsetToBuffer++;
            ret = TRUE;
        }
    }

    *pOffsetTo = offset;
    return ret;
}

/******************************************************************************
 * rngBufGet - Get from ring buffer
 *
 * RETURNS: Number of bytes read
 */

int rngBufGet(
    RING_ID ringId,
    char *buffer,
    int nBytes
    )
{
    int bread, bytes, offsetTmp;
    int offsetToBuffer;

    if (ringId == NULL)
    {
        bread = 0;
    }
    else
    {
        bread = 0;
        offsetTmp = 0;
        offsetToBuffer = ringId->offsetToBuffer;

        if (offsetToBuffer >= ringId->offsetFromBuffer)
        {
            /* To offset has not wrapped around */
            bread = min(nBytes, offsetToBuffer - ringId->offsetFromBuffer);
            memcpy(buffer, &ringId->buffer[ringId->offsetFromBuffer], bread);
            ringId->offsetFromBuffer += bread;
        }
        else
        {
            /* To offset has wrapped around */
            bread = min(nBytes, ringId->bufferSize - ringId->offsetFromBuffer);
            memcpy(buffer, &ringId->buffer[ringId->offsetFromBuffer], bread);
            offsetTmp = ringId->offsetFromBuffer + bread;

            /* Check wrapping */
            if (offsetTmp == ringId->bufferSize)
            {
                bytes = min(nBytes - bread, offsetToBuffer);
                memcpy(buffer + bread, ringId->buffer, bytes);
                ringId->offsetFromBuffer = bytes;
                bread += bytes;
            }
            else
            {
                ringId->offsetFromBuffer = offsetTmp;
            }
        }
    }

    return bread;
}

/******************************************************************************
 * rngBufPut - Put to ring buffer
 *
 * RETURNS: Number of bytes written
 */

int rngBufPut(
    RING_ID ringId,
    char *buffer,
    int nBytes
    )
{
    int bwrote, bytes, offsetTmp;
    int offsetFromBuffer;

    if (ringId == NULL || buffer == NULL)
    {
        bwrote = 0;
    }
    else
    {
        bwrote = 0;
        offsetTmp = 0;
        offsetFromBuffer = ringId->offsetFromBuffer;

        if (offsetFromBuffer > ringId->offsetToBuffer)
        {
            /* offsetFrom is ahead of offsetTo */
            bwrote = min(nBytes, offsetFromBuffer - ringId->offsetToBuffer - 1);
            memcpy(&ringId->buffer[ringId->offsetToBuffer], buffer, bwrote);
            ringId->offsetToBuffer += bwrote;
        }
        else if (offsetFromBuffer == 0)
        {
            /* offsetFrom is at beginning */
            bwrote = min(
                nBytes,
                ringId->bufferSize - ringId->offsetToBuffer - 1
                );
            memcpy(&ringId->buffer[ringId->offsetToBuffer], buffer, bwrote);
            ringId->offsetToBuffer += bwrote;
        }
        else
        {
            /* From offset has wrapped around */
            bwrote = min(nBytes, ringId->bufferSize - ringId->offsetToBuffer);
            memcpy(&ringId->buffer[ringId->offsetToBuffer], buffer, bwrote);
            offsetTmp = ringId->offsetToBuffer + bwrote;

            /* Check wraping */
            if (offsetTmp == ringId->bufferSize)
            {
                bytes = min(nBytes - bwrote, offsetFromBuffer - 1);
                memcpy(ringId->buffer, buffer + bwrote, bytes);
                ringId->offsetToBuffer = bytes;
                bwrote += bytes;
            }
            else
            {
                ringId->offsetToBuffer = offsetTmp;
            }
        }
    }

    return bwrote;
}

/******************************************************************************
 * rngIsEmpty - Check if buffer is empty
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngIsEmpty(
    RING_ID ringId
    )
{
    BOOL ret;

    if (ringId == NULL)
    {
        ret = TRUE;
    }
    else
    {
        if (ringId->offsetToBuffer == ringId->offsetFromBuffer)
        {
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }
    }

    return ret;
}

/******************************************************************************
 * rngIsFull - Check if buffer is full
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngIsFull(
    RING_ID ringId
    )
{
    BOOL full;
    int n;

    if (ringId == NULL)
    {
        full = FALSE;
    }
    else
    {
        n = ringId->offsetToBuffer - ringId->offsetFromBuffer + 1;
        if ((n == 0) || (n == ringId->bufferSize))
        {
            full = TRUE;
        }
        else
        {
            full = FALSE;
        }
    }

    return full;
}

/******************************************************************************
 * rngFreeBytes - Check space left in ring buffer
 *
 * RETURNS: Bytes free
 */

int rngFreeBytes(
    RING_ID ringId
    )
{
    int n;

    if (ringId == NULL)
    {
        n = 0;
    }
    else
    {
        n = ringId->offsetFromBuffer - ringId->offsetToBuffer - 1;
        if (n < 0)
        {
            n += ringId->bufferSize;
        }
    }

    return n;
}

/******************************************************************************
 * rngNBytes - Get number of bytes in ring buffer
 *
 * RETURNS: Bytes in buffer
 */

int rngNBytes(
    RING_ID ringId
    )
{
    int n;

    if (ringId == NULL)
    {
        n = 0;
    }
    else
    {
        n = ringId->offsetToBuffer - ringId->offsetFromBuffer;
        if (n < 0)
        {
            n += ringId->bufferSize;
        }
    }

    return n;
}

/******************************************************************************
 * rngPutAhead - Put a char ahead of ringbuffer with no offset change
 *
 * RETURNS: OK or ERROR
 */

STATUS rngPutAhead(
    RING_ID ringId,
    char c,
    int offset
    )
{
    STATUS status;
    int n;

    if (ringId == NULL)
    {
        status = ERROR;
    }
    else
    {
        n = ringId->offsetToBuffer + offset;
        if (n >= ringId->bufferSize)
        {
            n -= ringId->bufferSize;
        }

        *(ringId->buffer + n) = c;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * rngMoveAhead - Advance offset n bytes
 *
 * RETURNS: OK or ERROR
 */

STATUS rngMoveAhead(
    RING_ID ringId,
    int offset
    )
{
    STATUS status;

    if (ringId == NULL)
    {
        status = ERROR;
    }
    else
    {
        offset += ringId->offsetToBuffer;
        if (offset >= ringId->bufferSize)
        {
            offset -= ringId->bufferSize;
        }

        ringId->offsetToBuffer = offset;
        status = OK;
    }

    return status;
}

