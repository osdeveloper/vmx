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

/* sysArchLib.h - System library */

#ifndef _sysArchLib_h
#define _sysArchLib_h

#include <sys/types.h>

#define GDT_ENTRIES              5
#define	IDT_ENTRIES            256

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * sysInByte - Read one byte from I/O space
 *
 * RETURNS: Byte data from I/O port
 */

u_int8_t sysInByte(
    u_int32_t address
    );

/******************************************************************************
 * sysInWord - Read one word from I/O space
 *
 * RETURNS: Word data from I/O port
 */

u_int16_t sysInWord(
    u_int32_t address
    );

/******************************************************************************
 * sysInLong - Read one long-word from I/O space
 *
 * RETURNS: Long-word data from I/O port
 */

u_int32_t sysInLong(
    u_int32_t address
    );

/******************************************************************************
 * sysOutByte - Output one byte to I/O space
 *
 * RETURNS: N/A
 */

void sysOutByte(
    u_int32_t address,
    u_int8_t data
    );

/******************************************************************************
 * sysOutWord - Output one word to I/O space
 *
 * RETURNS: N/A
 */

void sysOutWord(
    u_int32_t address,
    u_int16_t data
    );

/******************************************************************************
 * sysOutLong - Output one long-word to I/O space
 *
 * RETURNS: N/A
 */

void sysOutLong(
    u_int32_t address,
    u_int32_t data
    );

/******************************************************************************
 * sysInWordString - Input word string from I/O space
 *
 * RETURNS: N/A
 */

void sysInWordString(
    u_int32_t port,
    u_int16_t *address,
    u_int32_t count
    );

/*****************************************************************************
 * sysInLongString - Input long string from I/O space
 *
 * RETURNS:   N/A
 */

void sysInLongString(
    u_int32_t port,
    u_int32_t *address,
    u_int32_t count
    );

/******************************************************************************
 * sysOutWordString - Output word string to I/O space
 *
 * RETURNS: N/A
 */

void sysOutWordString(
    u_int32_t port,
    u_int16_t *address,
    u_int32_t count
    );

/******************************************************************************
 * sysOutLongString - Output word string to I/O space
 *
 * RETURNS: N/A
 */

void sysOutLongString(
    u_int32_t port,
    u_int32_t *address,
    u_int32_t count
    );

/* System controll functions */

/******************************************************************************
 * sysWait - Wait for input buffer to become empty
 *
 * RETURNS: N/A
 */

void sysWait(
    void
    );

/******************************************************************************
 * sysReboot - Reset system
 *
 * RETURNS:   N/A
 */

void sysReboot(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _sysArchLib_h */

