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

/* rngLib.h - Ring buffer library */

#ifndef _rngLib_h
#define _rngLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int offsetToBuffer;
    int offsetFromBuffer;
    int bufferSize;
    char *buffer;
} RING;

typedef RING *RING_ID;

/* Macros */

/******************************************************************************
 * RING_ELEM_GET - Get character from ring buffer
 *
 * RETURNS: TRUE if there is character to return, FALSE if not
 */

#define RNG_ELEM_GET(ringId, pc, fp)                                          \
    rngElemGet((ringId), (pc), (&fp))

/******************************************************************************
 * RNG_ELEM_PUT - Put character on ring buffer
 *
 * RETURNS: TRUE if there was room left, FALSE if not
 */

#define RNG_ELEM_PUT(ringId, c, tp)                                           \
    rngElemPut((ringId), (c), (&tp))

/******************************************************************************
 * rngCreate - Create a ring buffer
 *
 * RETURNS: Pointer to ring buffer or NULL
 */

RING_ID rngCreate(
    int size
    );

/******************************************************************************
 * rngDelete - Delete a ring buffer
 *
 * RETURNS: OK or ERROR
 */

STATUS rngDelete(
    RING_ID ringId
    );

/******************************************************************************
 * rngFlush - Flush a ring buffer
 *
 * RETURNS: OK or ERROR
 */

STATUS rngFlush(
    RING_ID ringId
    );

/******************************************************************************
 * rngElemGet - Get element from ring buffer
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngElemGet(
    RING_ID ringId,
    char *pChar,
    int *pOffsetFrom
    );

/******************************************************************************
 * rngElemPut - Put element on ring buffer
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngElemPut(
    RING_ID ringId,
    char c,
    int *pOffsetTo
    );

/******************************************************************************
 * rngBufGet - Get from ring buffer
 *
 * RETURNS: Number of bytes read
 */

int rngBufGet(
    RING_ID ringId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * rngBufPut - Put to ring buffer
 *
 * RETURNS: Number of bytes written
 */

int rngBufPut(
    RING_ID ringId,
    char *buffer,
    int nBytes
    );

/******************************************************************************
 * rngIsEmpty - Check if buffer is empty
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngIsEmpty(
    RING_ID ringId
    );

/******************************************************************************
 * rngIsFull - Check if buffer is full
 *
 * RETURNS: TRUE or FALSE
 */

BOOL rngIsFull(
    RING_ID ringId
    );

/******************************************************************************
 * rngFreeBytes - Check space left in ring buffer
 *
 * RETURNS: Bytes free
 */

int rngFreeBytes(
    RING_ID ringId
    );

/******************************************************************************
 * rngNBytes - Get number of bytes in ring buffer
 *
 * RETURNS: Bytes in buffer
 */

int rngNBytes(
    RING_ID ringId
    );

/******************************************************************************
 * rngPutAhead - Put a char ahead of ringbuffer with no offset change
 *
 * RETURNS: OK or ERROR
 */

STATUS rngPutAhead(
    RING_ID ringId,
    char c,
    int offset
    );

/******************************************************************************
 * rngMoveAhead - Advance offset n bytes
 *
 * RETURNS: OK or ERROR
 */

STATUS rngMoveAhead(
    RING_ID ringId,
    int offset
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ringLib_h */

