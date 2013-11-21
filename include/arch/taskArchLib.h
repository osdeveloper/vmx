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

/* taskLib.h - Task context switching stuff */

#ifndef _taskArchLib_h
#define _taskArchLib_h

#include <types/vmxCpu.h>
#include <vmx.h>
#include <arch/regs.h>

#if      CPU_FAMILY==I386
#include <arch/i386/taskI386Lib.h>
#endif

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx/taskLib.h>

/* Imports */

IMPORT REG_INDEX taskRegName[];

/* Functions */

/******************************************************************************
 * taskRegsInit - Initialize architecture depedant tcb data
 *
 * RETURNS: N/A
 */

void taskRegsInit(
    TCB_ID tcbId,
    char *pStackBase
    );

/******************************************************************************
 * taskRetValueSet - Set task return value
 *
 * RETURNS: N/A
 */

void taskRetValueSet(
    TCB_ID tcbId,
    int val
    );

/******************************************************************************
 * taskArgSet - Setup task arguments
 *
 * RETURNS: N/A
 */

void taskArgSet(
    TCB_ID tcbId,
    char *pStackBase,
    ARG args[]
    );

/******************************************************************************
 * taskArgGet - Read task arguments
 *
 * RETURNS: N/A
 */

void taskArgGet(
    TCB_ID tcbId,
    char *pStackBase,
    ARG args[]
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUANGE */

#endif /* _taskArchLib_h */

