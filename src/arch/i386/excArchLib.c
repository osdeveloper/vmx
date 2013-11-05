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

/* excArchLib.h - Exception library */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <vmx.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <util/qLib.h>
#include <arch/regs.h>
#include <arch/iv.h>
#include <arch/esf.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <arch/excArchLib.h>

/* Imports */
IMPORT u_int8_t         excCallTbl[];
IMPORT VOIDFUNCPTR      intCallTbl[];
IMPORT u_int32_t        sysIntIdtType;
IMPORT u_int32_t        sysCsExc;
IMPORT u_int32_t        sysCsInt;

#define NO_EXCHOOKS
#ifndef NO_EXCHOOKS
IMPORT FUNCPTR          _func_excBaseHook;
IMPORT FUNCPTR          _func_excIntHook;
IMPORT FUNCPTR          _func_excInfoShow;
IMPORT FUNCPTR          _func_sigExcKill;
#endif

/* Locals */
LOCAL void excGetInfoFromESF(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    EXC_INFO *pExcInfo,
    BOOL errorCode
    );

LOCAL BOOL programError(
    int vecNum
    );

/******************************************************************************
 * excVecInit - Setup exception interrupt vector
 *
 * RETURNS: OK
 */

STATUS excVecInit(
    void
    )
{
    int vecNum;

    /* Clear interrupt table */
    for (vecNum = LOW_VEC; vecNum <= HIGH_VEC; ++vecNum)
    {
        intCallTbl[vecNum] = NULL;
    }

    /* Setup pointers in idt to all procedures defined in excALib.s */

    for (vecNum = LOW_VEC; vecNum <= HIGH_VEC; ++vecNum)
    {
        intVecSet2(
            (FUNCPTR *) INUM_TO_IVEC(vecNum),
            (FUNCPTR)   &excCallTbl[vecNum * 5],
            programError(vecNum) ? IDT_TRAP_GATE : sysIntIdtType,
            programError(vecNum) ? sysCsExc : sysCsInt
            );
    }

    return OK;
}

/******************************************************************************
 * excExcHandle - Default exception handler
 *
 * RETURNS: N/A
 */

void excExcHandle(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    BOOL error
    )
{
    EXC_INFO excInfo;

    /* Get exception info */
    excGetInfoFromESF(vecNum, pEsf, pRegs, &excInfo, error);

#ifndef NO_EXCHOOKS
    /* Call os hook if it exists */
    if (_func_excBaseHook != NULL)
    {
        /* If os hook returns success (non zero) */
        if (((*_func_excBaseHook)(vecNum, pEsf, pRegs, &excInfo)))
        {
            return;
        }
    }
#endif

    /* If exception in isr or pre-kernel */
    if ((INT_CONTEXT() == TRUE) || (Q_FIRST(&activeQHead) == NULL))
    {
        sysReboot();
    }
    else
    {
        /* If we are here a task caused the exception */
        taskIdCurrent->pExcRegSet = pRegs;
        taskIdDefault( (int) taskIdCurrent );
        memcpy(&taskIdCurrent->excInfo, &excInfo, sizeof(excInfo));

#ifndef NO_EXCHOOKS
        /* Call kill signal if set */
        if (_func_sigExcKill != NULL)
        {
            (*_func_sigExcKill)(vecNum, INUM_TO_IVEC(vecNum), pRegs);
        }

        /* Call exception show if set */
        if (_func_excInfoShow != NULL)
        {
            (*_func_excInfoShow )(&excInfo, TRUE);
        }
#endif

        /* Suspend task and invalidate exception regs */
        taskSuspend(0);
        taskIdCurrent->pExcRegSet = NULL;
    }
}

/******************************************************************************
 * excIntHandle - Uninitialized interrput handler
 *
 * RETURNS: N/A
 */

void excIntHandle(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    BOOL error
    )
{
    EXC_INFO excInfo;

    /* Get exception info */
    excGetInfoFromESF(vecNum, pEsf, pRegs, &excInfo, error);

#ifndef NO_EXCHOOKS
    /* Call os hook if existing */
    if (_func_excIntHook != NULL)
    {
        (*_func_excIntHook)(vecNum, pEsf, pRegs, &excInfo);
    }
#endif

    if (Q_FIRST(&activeQHead) == NULL)
    {
        sysReboot();
    }
}

/******************************************************************************
 * excGetInfoFromESF - Get information from exception stack frame
 *
 * RETURNS: N/A
 */

LOCAL void excGetInfoFromESF(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegs,
    EXC_INFO *pExcInfo,
    BOOL errorCode
    )
{
    int sz;

    /* Verify vector and vecNum into info structure */
    pExcInfo->valid = EXC_VEC_NUM;
    pExcInfo->vecNum = vecNum;

    /* If in supervisor mode, ESF1 */
    if (errorCode)
    {
        /* Validate error code */
        pExcInfo->valid |= EXC_ERROR_CODE;

        /* Store exception info */
        pExcInfo->errCode = ((ESF1 * ) pEsf)->errCode;
        pExcInfo->pc =      ((ESF1 * ) pEsf)->pc;
        pExcInfo->cs =      ((ESF1 * ) pEsf)->cs;
        pExcInfo->eflags =  ((ESF1 * ) pEsf)->eflags;
        sz =                ESF1_NBYTES;
    }
    else
    {
        pExcInfo->pc =      ((ESF0 * ) pEsf)->pc;
        pExcInfo->cs =      ((ESF0 * ) pEsf)->cs;
        pExcInfo->eflags =  ((ESF0 * ) pEsf)->eflags;
        sz =                ESF0_NBYTES;
    }

    /* If page fault */
    if (vecNum == IN_PAGE_FAULT)
    {
        pExcInfo->valid |= EXC_CR2;
        pExcInfo->cr2 = 0;
    }

    /* Store stack fault */
    pExcInfo->cr3 = 0;
    pExcInfo->esp0 = (u_int32_t) pEsf + sz;

    /* Adjust stack pointer in REG_SET */
    pRegs->spReg = (u_int32_t) ((char *) pEsf + sz);

    /* Get info from supervisor stack */
    pExcInfo->esp00 =     *(u_int32_t *)  pExcInfo->esp0;
    pExcInfo->esp01 =     *(u_int32_t *) (pExcInfo->esp0 + 1);
    pExcInfo->esp02 =     *(u_int32_t *) (pExcInfo->esp0 + 2);
    pExcInfo->esp03 =     *(u_int32_t *) (pExcInfo->esp0 + 3);
    pExcInfo->esp04 =     *(u_int32_t *) (pExcInfo->esp0 + 4);
    pExcInfo->esp05 =     *(u_int32_t *) (pExcInfo->esp0 + 5);
    pExcInfo->esp06 =     *(u_int32_t *) (pExcInfo->esp0 + 6);
    pExcInfo->esp07 =     *(u_int32_t *) (pExcInfo->esp0 + 7);
}

/******************************************************************************
 * programError - Determine if exception is a software error
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL programError(
    int vecNum
    )
{
    return (vecNum <= IN_CP_ERROR);
}

