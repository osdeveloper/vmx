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

/* taskI386Lib.h - Architecture dependent task context switch */

#ifndef _taskI386Lib_h
#define _taskI386Lib_h

#define TIMER_INTERRUPT_NUM    0x20

#include <arch/regs.h>

#define TASK_TCB_EDI           TASK_TCB_REGS + REGS_EDI
#define TASK_TCB_ESI           TASK_TCB_REGS + REGS_ESI
#define TASK_TCB_EBP           TASK_TCB_REGS + REGS_EBP
#define TASK_TCB_ESP           TASK_TCB_REGS + REGS_ESP
#define TASK_TCB_EBX           TASK_TCB_REGS + REGS_EBX
#define TASK_TCB_EDX           TASK_TCB_REGS + REGS_EDX
#define TASK_TCB_ECX           TASK_TCB_REGS + REGS_ECX
#define TASK_TCB_EAX           TASK_TCB_REGS + REGS_EAX
#define TASK_TCB_EFLAGS        TASK_TCB_REGS + REGS_EFLAGS
#define TASK_TCB_PC            TASK_TCB_REGS + REGS_PC

#define TASK_INITIAL_FLAGS     0x202
#define TASK_TCB_ENTRY          0x3c
#define TASK_TCB_ERRNO          0x40
#define TASK_TCB_ID             0x44
#define TASK_TCB_STATUS         0x48
#define TASK_TCB_LOCK_COUNT     0x4c
#define TASK_TCB_SWAP_IN        0x50
#define TASK_TCB_SWAP_OUT       0x52
#define TASK_TCB_REGS           0x54

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskI386Lib_h */

