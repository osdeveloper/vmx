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

/* taskArchLib.c - Task switching functions */

#include <stdlib.h>
#include <vmx.h>
#include <arch/vmxArchLib.h>
#include <arch/regs.h>
#include <vmx/taskLib.h>
#include <arch/taskArchLib.h>

/* Globals */
REG_INDEX taskRegName[] =
{
    {"edi",    G_REG_OFFSET(0)},
    {"esi",    G_REG_OFFSET(1)},
    {"ebp",    G_REG_OFFSET(2)},
    {"esp",    G_REG_OFFSET(3)},
    {"ebx",    G_REG_OFFSET(4)},
    {"edx",    G_REG_OFFSET(5)},
    {"ecx",    G_REG_OFFSET(6)},
    {"eax",    G_REG_OFFSET(7)},
    {"eflags", SR_OFFSET      },
    {"pc",     PC_OFFSET      },
    {NULL,     0              }
};

/******************************************************************************
 * taskRegsInit - Initialize architecture depedant tcb data
 *
 * RETURNS: N/A
 */

void taskRegsInit(
    TCB_ID tcbId,
    char *pStackBase
    )
{
    ARG args[MAX_TASK_ARGS];
    int i;
    for (i = 0; i < MAX_TASK_ARGS; i++)
    {
        args[i] = (ARG) i;
    }

    tcbId->regs.eflags = TASK_INITIAL_FLAGS;
    tcbId->regs.pc = (INSTR *) vmxTaskEntry;

    tcbId->regs.edi = 0;
    tcbId->regs.esi = 0;
    tcbId->regs.ebp = 0;
    tcbId->regs.ebx = 0;
    tcbId->regs.edx = 0;
    tcbId->regs.ecx = 0;
    tcbId->regs.eax = 0;

    /* Setup initial stack */
    tcbId->regs.esp = (u_int32_t)
                      (pStackBase - (MAX_TASK_ARGS * sizeof(u_int32_t)));
}

/******************************************************************************
 * taskRetValueSet - Set task return value
 *
 * RETURNS: N/A
 */

void taskRetValueSet(
    TCB_ID tcbId,
    int val
    )
{
    tcbId->regs.eax = val;
}

/******************************************************************************
 * taskArgSet - Setup task arguments
 *
 * RETURNS: N/A
 */

void taskArgSet(
    TCB_ID tcbId,
    char *pStackBase,
    ARG args[]
    )
{
    int i;
    ARG *sp;

    sp = (ARG *) pStackBase;

    /* Push args on stack */
    for (i = MAX_TASK_ARGS - 1; i >= 0; --i)
    {
        *--sp = (ARG) args[i];
    }
}

/******************************************************************************
 * taskArgGet - Read task arguments
 *
 * RETURNS: N/A
 */

void taskArgGet(
    TCB_ID tcbId,
    char *pStackBase,
    ARG args[]
    )
{
    int i;
    ARG *sp;

    sp = (ARG *) pStackBase;

    /* Push args on stack */
    for (i = MAX_TASK_ARGS - 1; i >= 0; --i)
    {
        args[i] = *--sp;
    }
}

