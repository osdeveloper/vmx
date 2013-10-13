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
#include <vmx/logLib.h>
#include <arch/regs.h>
#include <arch/iv.h>
#include <arch/esf.h>
#include <arch/sysArchLib.h>
#include <arch/excArchLib.h>
#include <arch/intArchLib.h>

/* Globals */
u_int32_t      intCnt                     = 0;
VOIDFUNCPTR    intVecSetEnt               = NULL;
VOIDFUNCPTR    intVecSetExit              = NULL;
VOIDFUNCPTR    intCallTbl[IDT_ENTRIES];
void          *intParamTbl[IDT_ENTRIES];

/* Locals */
LOCAL FUNCPTR *intVecBase = 0;

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
 * intVecSet - Setup a CPU vector
 *
 * RETURNS: N/A
 */

void intVecSet(
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
 * intIntHandle - Interrupt handler
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

        /* Send interrupt completed signal */
        if (vecNum >= 0x28 && vecNum <= 0x2f)
        {
            sysOutByte(0xa0, 0x20);
        }
        if (vecNum >= 0x20 && vecNum <= 0x2f)
        {
            sysOutByte(0x20, 0x20);
        }

    }
    else
    {
        /* If not installed called function in exception part */
        excIntHandle(vecNum, pEsf, pRegs, error);
    }
}

/******************************************************************************
 * intConnectFunction - Connect a C function to interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS intConnectFunction(
    int vecNum,
    VOIDFUNCPTR func,
    void *parameter
    )
{
    STATUS status;

    /* Check if in valid range */
    if (vecNum < IN_USER_START || vecNum > IN_USER_END)
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
 * intRemoveFunction - Remove a C function from interrupt
 *
 * RETURNS: OK or ERROR
 */

STATUS intRemoveFunction(
    int vecNum
    )
{
    STATUS status;

    /* Check if in valid range */
    if (vecNum < IN_USER_START || vecNum > IN_USER_END)
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

