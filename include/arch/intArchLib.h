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

/* intLib.h - Interrupt library */

#ifndef _intArchLib_h
#define _intArchLib_h

#include <ostool/moduleNumber.h>
#include <types/vmxCpu.h>

#if      CPU_FAMILY==I386
#include <arch/i386/intI386Lib.h>
#endif

#define S_intLib_NOT_ISR_CALLABLE      (M_intLib | 0x001)

/* Macros */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * INT_CONTEXT - Check if in interrupt context
 *
 * RETURNS: Non-zero if in interrupt context
 */

#define INT_CONTEXT()           ((intCnt > 0) ? TRUE : FALSE)

/******************************************************************************
 * INT_RESTRICT - Restrict usage from interrupt context
 *
 * RETURNS: ERROR if in interrupt contexte, otherwise OK
 */

#define INT_RESTRICT()          ((intCnt > 0) ? ERROR : OK)

/******************************************************************************
 * intLevelSet - Setup interrupt lockout level
 *
 * RETURNS: Zero
 */

int intLevelSet(
    int level
    );

/******************************************************************************
 * intLock - Lock interrupts
 *
 * RETURNS: Interrupt lock level
 */

int intLock(
    void
    );

/******************************************************************************
 * intUnlock - Unlock interrupts
 *
 * RETURNS: N/A
 */

void intUnlock(
    int level
    );

/******************************************************************************
 * intVecBaseSet - Set interrupt vector base address
 *
 * RETURNS: N/A
 */

void intVecBaseSet(
    FUNCPTR *baseAddr
    );

/******************************************************************************
 * intVecBaseGet - Get interrupt vector base address
 *
 * RETURNS: Current vector base address
 */

FUNCPTR* intVecBaseGet(
    void
    );

/******************************************************************************
 * intVecSet - Setup a CPU vector for interrupt/exception
 *
 * RETURNS: N/A
 */

void intVecSet(
    FUNCPTR *vector,
    FUNCPTR function
    );

/******************************************************************************
 * intHandleCreate - Create an interrupt handler
 *
 * RETURNS: Pointer to handler or NULL
 */

FUNCPTR intHandlerCreate(
    FUNCPTR routine,
    int param
    );

/******************************************************************************
 * intConnect - Connect an interrupt handler
 *
 * RETURNS: OK or ERROR
 */

STATUS intConnect(
    VOIDFUNCPTR *vec,
    VOIDFUNCPTR routine,
    int param
    );

/******************************************************************************
 * intLockLevelSet - Set interrupt lock out level
 *
 * RETURNS: N/A
 */

void intLockLevelSet(
    int level
    );

/******************************************************************************
 * intLockLevelGet - Get interrupt lock out level
 *
 * RETURNS: Interrupt lock mask
 */

int intLockLevelGet(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUANGE */

#endif /* _intArchLib_h */

