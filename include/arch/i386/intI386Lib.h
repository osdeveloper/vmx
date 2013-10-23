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

/* intI386Lib.h - Interrupt handeling */

#ifndef _intI386Lib_h
#define _intI386Lib_h

#define SYS_INT_TRAPGATE        0x0000ef00
#define SYS_INT_INTGATE         0x0000ff00
#define INT_NUM_IRQ0                  0x20

#define INT_KERNEL_ENTRY         1
#define INT_BOI_PUSH             8
#define INT_BOI_PARAM            9
#define INT_BOI_CALL            13
#define INT_BOI_ROUTINE         14
#define INT_HANDLER_PUSH        18
#define INT_HANDLER_PARAM       19
#define INT_HANDLER_CALL        23
#define INT_HANDLER_ROUTINE     24
#define INT_EOI_PUSH            28
#define INT_EOI_PARAM           29
#define INT_EOI_CALL            33
#define INT_EOI_ROUTINE         34
#define INT_ADD_N               40
#define INT_KERNEL_EXIT         45

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/esf.h>

IMPORT u_int32_t    intLockMask;
IMPORT u_int32_t    intCnt;
IMPORT VOIDFUNCPTR  intVecSetEnt;
IMPORT VOIDFUNCPTR  intVecSetExit;
IMPORT VOIDFUNCPTR  intEoi;

/******************************************************************************
 * INT_ENABLE - Unable interrupts
 *
 * RETURNS: N/A
 */

#define INT_ENABLE() \
    __asm__ __volatile__("sti");

/******************************************************************************
 * INT_DISABLE - Disable interrupts
 *
 * RETURNS: N/A
 */

#define INT_DISABLE() \
    __asm__ __volatile__("sti");

/******************************************************************************
 * INT_LOCK - Lock out interrupts
 *
 * RETURNS: N/A
 */

#define INT_LOCK(oldLevel) \
    __asm__ __volatile__("pushf; popl %0; andl $0x00000200, %0; cli" \
                         : "=rm" (oldLevel) : /* no input */ : "memory")

/******************************************************************************
 * INT_UNLOCK - Unlock interrupts
 *
 * RETURNS: N/A
 */

#define INT_UNLOCK(oldLevel) \
    __asm__ __volatile__ ("testl $0x00000200, %0; jz 0f; sti; 0:" \
                          : /* no output */ : "rm" (oldLevel) : "memory")

#ifdef NEW_INTALIB
extern int intLevelSet(int level);
extern int intLock(void);
extern void intUnlock(int level);
#endif

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
 * intVBRSet - Setup interrupt table descriptor
 *
 * RETURNS N/A
 */

void intVBRSet(
    FUNCPTR *baseAddr
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
 * intVecSet2 - Setup a CPU vector with gate and selector
 *
 * RETURNS: N/A
 */

void intVecSet2(
    FUNCPTR *vector,
    FUNCPTR function,
    int idtGate,
    int idtSelector
    );

/******************************************************************************
 * intIntHandle - Default interrupt handler
 *
 * RETURNS: N/A
 */

void intIntHandle(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    BOOL error
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

/******************************************************************************
 * intConnectDefault - Connect function to default interrupt handler table
 *
 * RETURNS: OK or ERROR
 */

STATUS intConnectDefault(
    int vecNum,
    VOIDFUNCPTR func,
    void *parameter
    );

/******************************************************************************
 * intRemoveDefault - Remove function from default interrupt handler table
 *
 * RETURNS: OK or ERROR
 */

STATUS intRemoveDefault(
    int vecNum
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* _intI386Lib_h */

