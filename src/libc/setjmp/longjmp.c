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

/* longjmp.c - Non-local goto */

#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/private/sigLibP.h>

#define JMP_DATA(env) \
    ((int *) (((char *) env) + sizeof(jmp_buf) - 2 * sizeof(int)))

/******************************************************************************
 * _setjmpSetup - Setup jump goto
 *
 * RETURNS: N/A
 */

void _setjmpSetup(
    jmp_buf env,
    int     val
    )
{
    int           *ptr;
    struct sigtcb *pSigtcb;

    /* Set ptr to jmp_buf extra area */
    ptr = JMP_DATA(env);

    /* Store current task id */
    ptr[0] = (int) taskIdCurrent;

    /* Store signal info if needed */
    if (val)
    {
        ptr[0] |= 1;

        if (taskIdCurrent->pSignalInfo == NULL)
        {
            ptr[1] = 0;
        }
        else
        {
            pSigtcb = taskIdCurrent->pSignalInfo;
            ptr[1]  = pSigtcb->sigt_blocked;
        }
    }
}

/******************************************************************************
 * longjmp - Do a long goto
 *
 * RETURNS: N/A
 */

void longjmp(
    jmp_buf env,
    int     val
    )
{
    int      *ptr;
    sigset_t *sigset;

    /* Set ptr to jmp_buf extra area */
    ptr = JMP_DATA(env);

    /* Suspend task if needed */
    if ((ptr[0] & ~1) != (int) taskIdCurrent)
    {
        taskSuspend(0);
    }

    /* Call sigprocmask() if needed */
    if (ptr[0] & 1)
    {
        sigset = (sigset_t *) &ptr[1];
        sigprocmask(SIG_SETMASK, sigset, 0);
    }

    /* Set return value */
    if (val == 0)
    {
        _sigCtxRetValueSet((REG_SET *) env, 1);
    }
    else
    {
        _sigCtxRetValueSet((REG_SET *) env, val);
    }

    /* Load task context */
    _sigCtxLoad((REG_SET *) env);
}

