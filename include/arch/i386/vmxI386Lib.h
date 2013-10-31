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

/* vmxI386.h - Kernel header */

#ifndef _vmxI386_h
#define _vmxI386_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * intEnt - Called when an interrupt occurs
 *
 * RETURNS: N/A
 */

void intEnt(
    void
    );

/******************************************************************************
 * intExit - Routine only brached to when interrupt ends
 *
 * RETURNS: N/A
 */

void intExit(
    void
    );

/******************************************************************************
 * vmxExit - Exit kernel mode
 *
 * RETURNS: OK, ERROR or SIG_RESTART
 */

int vmxExit(
    void
    );

/******************************************************************************
 * vmxTaskContextLoad - Load task context
 *
 * RETURNS: N/A
 */

void vmxTaskContextLoad(
    void
    );

/******************************************************************************
 * vmxTaskEntry - Task entry point
 *
 * RETURNS: N/A
 */

void vmxTaskEntry(
    void
    );

/******************************************************************************
 * intStackSet - Set interrupt stack pointer
 *
 * RETURNS: N/A
 */

void intStackSet(
    void *pStack
    );

/******************************************************************************
 * intStackEnable - Enable/disable interrupt stack
 *
 * RETURNS: OK or ERROR
 */

STATUS intStackEnable(
    BOOL enable
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vmxI386_h */

