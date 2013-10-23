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

/* intArchLib.c - Interrupt functions */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/iv.h>
#include <arch/esf.h>
#include <arch/excArchLib.h>
#include <arch/i386/sysI386Lib.h>
#include <arch/intArchLib.h>

/* Imports */
IMPORT void             kernIntEnt(void);
IMPORT void             kernIntExit(void);
IMPORT u_int32_t        sysIntIdtType;
IMPORT u_int32_t        sysCsInt;

/* Globals */
u_int32_t               intLockMask     = 0x0;
u_int32_t               intCnt          = 0;
VOIDFUNCPTR             intVecSetEnt    = NULL;
VOIDFUNCPTR             intVecSetExit   = NULL;
VOIDFUNCPTR             intEoi          = NULL;
VOIDFUNCPTR             intCallTbl[IDT_ENTRIES];
void                    *intParamTbl[IDT_ENTRIES];

/* Locals */
LOCAL FUNCPTR           *intVecBase = 0;
LOCAL u_int8_t          intConnectCode[] =
{
/* 00   e8 kk kk kk kk  call    kernIntEnt      * Kernel enter hook
 * 05   50              pushl   %eax            * Save regs
 * 06   52              pushl   %edx
 * 07   51              pushl   %ecx
 * 08   68 pp pp pp pp  pushl   $paramBoi       * push BOI param
 * 13   e8 rr rr rr rr  call    _routineBoi     * call BOI routine
 * 18   68 pp pp pp pp  pushl   $param          * push parameter
 * 23   e8 rr rr rr rr  call    _routine        * call handler
 * 28   68 pp pp pp pp  pushl   $paramEoi       * push EOI param
 * 33   e8 rr rr rr rr  call    $routineEoi     * call EOI routine
 * 38   83 c4 0c        addl    $12,%esp        * pop param
 * 41   59              popl    %ecx            * Restore regs
 * 42   5a              popl    %edx
 * 43   58              popl    %eax
 * 44   e9 kk kk kk kk  jmp     kernIntExit     * Kernel exit hook
 */

        0xe8, 0x00, 0x00, 0x00, 0x00,           /* kernIntEnt fill in later */
        0x50,                                   /* Save regs */
        0x52,
        0x51,
        0x68, 0x00, 0x00, 0x00, 0x00,           /* BOI param fill in later */
        0xe8, 0x00, 0x00, 0x00, 0x00,           /* routineBoi fill in later */
        0x68, 0x00, 0x00, 0x00, 0x00,           /* param fill in later */
        0xe8, 0x00, 0x00, 0x00, 0x00,           /* routine fill in later */
        0x68, 0x00, 0x00, 0x00, 0x00,           /* EOI param fill in later */
        0xe8, 0x00, 0x00, 0x00, 0x00,           /* EOI param fill in later */
        0x83, 0xc4, 0x0c,                       /* pop param */
        0x59,                                   /* Restore regs */
        0x5a,
        0x58,
        0xe9, 0x00, 0x00, 0x00, 0x00            /* kernIntExit fill in later */
};

/******************************************************************************
 * intVecBaseSet - Set interrupt vector base address
 *
 * RETURNS: N/A
 */

void intVecBaseSet(
    FUNCPTR *baseAddr
    )
{
    u_int8_t idt[6];
    u_int8_t *p = idt;

    /* Store pointer locally */
    intVecBase = baseAddr;

    /* Setup idt descriptor pointer */
    *(u_int16_t *) p = 0x07ff;
    *(u_int32_t *) (p + 2) = (u_int32_t) baseAddr;

    /* Call external assembly function to update IDT */
    intVBRSet((FUNCPTR *) idt);
}

/******************************************************************************
 * intVecBaseGet - Get interrupt vector base address
 *
 * RETURNS: Current vector base address
 */

FUNCPTR* intVecBaseGet(
    void
    )
{
    return intVecBase;
}

/******************************************************************************
 * intVecSet - Setup a CPU vector for interrupt/exception
 *
 * RETURNS: N/A
 */

void intVecSet(
    FUNCPTR *vector,
    FUNCPTR function
    )
{
    FUNCPTR *newVector;

    /* Call enter hook */
    if (intVecSetEnt != NULL)
    {
        (*intVecSetEnt)(vector, function);
    }

    /* Calculate offset in IDT */
    newVector = (FUNCPTR *)
        ((u_int32_t) vector + (u_int32_t) intVecBaseGet());

    /* Update entry in IDT */
    *(u_int32_t *) newVector =
        (sysCsInt << 16) | ((u_int32_t) function & 0xffff);
    *((u_int16_t *) newVector + 3) = (u_int16_t)
        (((u_int32_t) function & 0xffff0000) >> 16);

    /* Call exit hook */
    if (intVecSetExit != NULL)
    {
        (*intVecSetExit)(vector, function);
    }
}

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
    )
{
    FUNCPTR *newVector;

    /* Call enter hook */
    if (intVecSetEnt != NULL)
    {
        (*intVecSetEnt)(vector, function);
    }

    /* Calculate offset in IDT */
    newVector = (FUNCPTR *)
        ((u_int32_t) vector + (u_int32_t) intVecBaseGet());

    /* Update entry in IDT */
    *(u_int32_t *) newVector =
        (idtSelector << 16) | ((u_int32_t) function & 0x0000ffff);
    *((u_int32_t *) newVector + 1) =
        ((u_int32_t) function & 0xffff0000) | idtGate;

    /* Call exit hook */
    if (intVecSetExit != NULL)
    {
        (*intVecSetExit)(vector, function);
    }
}

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
    )
{
    VOIDFUNCPTR irq;
    void *parameter;

    /* Check if user interrupt is intalled */
    irq = intCallTbl[vecNum];
    if (irq != NULL)
    {
        /* Call function with assosiated parameter */
        parameter = intParamTbl[vecNum];
        (*irq)(parameter);
    }
    else
    {
        /* If not installed called function in exception part */
        excIntHandle(vecNum, pEsf, pRegs, error);
    }

    /* Call end of interrupt handler */
    if (intEoi != NULL)
    {
        intEoi();
    }
}

/******************************************************************************
 * intHandleCreate - Create an interrupt handler
 *
 * RETURNS: Pointer to handler or NULL
 */

FUNCPTR intHandlerCreate(
    FUNCPTR routine,
    int param
    )
{
    char *pCode;
    int i;

    /* Allocated buffer */
    pCode = (char *) malloc(sizeof(intConnectCode));
    if (pCode != NULL)
    {
        /* Copy call code */
        memcpy(
            (char *) pCode,
            (char *) intConnectCode,
            sizeof(intConnectCode)
            );

        /* Setup parameters and function calls */

        /* Kernel entry hook */
        *(int *) &pCode[INT_KERNEL_ENTRY] = (int) kernIntEnt -
            (int) &pCode[INT_KERNEL_ENTRY + 4];

        /* Replace BOI stuff with NOP */
        for (i = 0; i < 10; i++)
        {
            pCode[INT_BOI_PUSH + i] = 0x90;
        }

        /* Now only two parameters to pop */
        pCode[INT_ADD_N] = 8;

        /* Store handler parameters and routine */
        *(int *) &pCode[INT_HANDLER_PARAM] = (int) param;
        *(int *) &pCode[INT_HANDLER_ROUTINE] = (int) routine -
            (int) &pCode[INT_HANDLER_ROUTINE + 4];

        /* Check if there no EOI function */
        if (intEoi == NULL)
        {
            /* Replace with NOP */
            for (i = 0; i < 5; i++)
            {
                pCode[INT_EOI_CALL + i] = 0x90;
            }
        }
        else
        {
            /* Store call */
            *(int *) &pCode[INT_EOI_ROUTINE] = (int) intEoi -
                (int) &pCode[INT_EOI_ROUTINE + 4];
        }

        /* Store kernel exit hook */
        *(int *) &pCode[INT_KERNEL_EXIT] = (int) kernIntExit -
            (int) &pCode[INT_KERNEL_EXIT + 4];
    }

    return((FUNCPTR) pCode);
}

/******************************************************************************
 * intConnect - Connect an interrupt handler
 *
 * RETURNS: OK or ERROR
 */

STATUS intConnect(
    VOIDFUNCPTR *vec,
    VOIDFUNCPTR routine,
    int param
    )
{
    STATUS status;
    FUNCPTR intCode;

    intCode = intHandlerCreate((FUNCPTR) routine, param);
    if (intCode == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Set vector */
        intVecSet((FUNCPTR *) vec, (FUNCPTR) intCode);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * intLockLevelSet - Set interrupt lock out level
 *
 * RETURNS: N/A
 */

void intLockLevelSet(
    int level
    )
{
    intLockMask = level;
}

/******************************************************************************
 * intLockLevelGet - Get interrupt lock out level
 *
 * RETURNS: Interrupt lock mask
 */

int intLockLevelGet(
    void
    )
{
    return (int) intLockMask;
}

/******************************************************************************
 * intConnectDefault - Connect function to default interrupt handler table
 *
 * RETURNS: OK or ERROR
 */

STATUS intConnectDefault(
    int vecNum,
    VOIDFUNCPTR func,
    void *parameter
    )
{
    STATUS status;

    /* Check if in valid range */
    if ((vecNum < IN_USER_START) || (vecNum > IN_USER_END))
    {
        status = ERROR;
    }
    else
    {
        /* Store function and parameter in global table */
        intCallTbl[vecNum] = func;
        intParamTbl[vecNum] = parameter;
        status = OK;
    }
  
    return status;
}

/******************************************************************************
 * intRemoveDefault - Remove function from default interrupt handler table
 *
 * RETURNS: OK or ERROR
 */

STATUS intRemoveDefault(
    int vecNum
    )
{
    STATUS status;

    /* Check if in valid range */
    if ((vecNum < IN_USER_START) || (vecNum > IN_USER_END))
    {
        status = ERROR;
    }
    else
    {
        /* Set funcptr to null */
        intCallTbl[vecNum] = NULL;
        status = OK;
    }

    return status;
}

