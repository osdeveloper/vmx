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

/* excArchShow.c - Show exception info */

#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <arch/iv.h>
#include <arch/excArchLib.h>
#include <arch/intArchLib.h>
#include <arch/excArchShow.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/private/kernelLibP.h>
#include <os/excLib.h>

/* Locals */
LOCAL char *excMsgs[] =
{
    "Divide Error",                     /* Exception number  0 */
    "Debug",                            /* Exception number  1 */
    "Nonmaskable Interrupt",            /* Exception number  2 */
    "Breakpoint",                       /* Exception number  3 */
    "Overflow",                         /* Exception number  4 */
    "Bound",                            /* Exception number  5 */
    "Invalid Opcode",                   /* Exception number  6 */
    "Device Not Available",             /* Exception number  7 */
    "Double Fault",                     /* Exception number  8 */
    "Coprocessor Overrun",              /* Exception number  9 */
    "Invalid TSS",                      /* Exception number 10 */
    "Segment Not Present",              /* Exception number 11 */
    "Stack Fault",                      /* Exception number 12 */
    "General Protection Fault",         /* Exception number 13 */
    "Page Fault",                       /* Exception number 14 */
    "Intel Reserved",                   /* Exception number 15 */
    "Coprocessor Error",                /* Exception number 16 */
    "Alignment Check",                  /* Exception number 17 */
    "Streaming SIMD"                    /* Exception number 18 */
};

LOCAL char *excIntInfoStr =
    "\n"
    "Uninitialized Interrupt!\n"
    "Vector number %d (0-255). %s\n"
    "Supervisor ESP : 0x%08x\n"
    "Program Counter: 0x%08x\n"
    "Code Selector  : 0x%08x\n"
    "Eflags register: 0x%08x\n";

/******************************************************************************
 * excShowInit - Initialize exception info show library
 *
 * RETURNS: OK
 */

STATUS excShowInit(
    void
    )
{
    return OK;
}

/******************************************************************************
 * excInfoShow - Show exception info
 *
 * RETURNS: N/A
 */

void excInfoShow(
    EXC_INFO *pExcInfo,
    BOOL doBell
    )
{
    int valid  = pExcInfo->valid;
    int vecNum = pExcInfo->vecNum;

    /* If valid exception vector number */
    if (valid & EXC_VEC_NUM)
    {
        /* If exception and message set */
        if ((vecNum < NELEMENTS(excMsgs)) &&
            (excMsgs[vecNum] != NULL))
        {
            printExc(
                "\n%s\n",
                (ARG) excMsgs[vecNum],
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );
        }
        else
        {
            printExc(
                "\nTrap to uninitialized vector number %d (0-255).\n",
                (ARG) vecNum,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0,
                (ARG) 0
                );
        }

        printExc(
            "Page Dir Base   : 0x%08x\n",
            (ARG) pExcInfo->cr3,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );

        printExc(
            "Esp0 0x%08x : 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
            (ARG) pExcInfo->esp0,
            (ARG) pExcInfo->esp00,
            (ARG) pExcInfo->esp01,
            (ARG) pExcInfo->esp02,
            (ARG) pExcInfo->esp03
            );

        printExc(
            "Esp0 0x%08x : 0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
            (ARG) pExcInfo->esp0 + 16,
            (ARG) pExcInfo->esp04,
            (ARG) pExcInfo->esp05,
            (ARG) pExcInfo->esp06,
            (ARG) pExcInfo->esp07
            );

        printExc(
            "Program Counter : 0x%08x\n",
            (ARG) pExcInfo->pc,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );

        printExc("Code Selector   : 0x%08x\n",
            (ARG) ((int) pExcInfo->cs),
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );

        printExc("Eflags Register : 0x%08x\n",
            (ARG) pExcInfo->eflags,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }

    /* If exception error code */
    if (valid & EXC_ERROR_CODE)
    {
        printExc(
            "Error Code      : 0x%08x\n",
            (ARG) pExcInfo->errCode,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }

    /* If page fault */
    if (valid & EXC_CR2)
    {
        printExc(
            "Page Fault Addr : 0x%08x %s\n",
            (ARG) pExcInfo->cr2,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }

    if (doBell == TRUE)
    {
        printExc(
            "Task: %#x \"%s\"\007\n",
            (ARG) taskIdCurrent,
            (ARG) taskName((int) taskIdCurrent),
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }
    else
    {
        printExc(
            "Task: %#x \"%s\"\n",
            (ARG) taskIdCurrent,
            (ARG) taskName((int) taskIdCurrent),
            (ARG) 0,
            (ARG) 0,
            (ARG) 0
            );
    }
}

/******************************************************************************
 * excIntInfoShow - Show interrupt info
 *
 * RETURNS: N/A
 */

void excIntInfoShow(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegSet,
    EXC_INFO *pExcInfo
    )
{
}

/******************************************************************************
 * excPanicShow - Show fatal exception info
 *
 * RETURNS: N/A
 */
void excPanicShow(
    int vecNum,
    ESF0 *pEsf,
    REG_SET *pRegSet,
    EXC_INFO *pExcInfo
    )
{
}

