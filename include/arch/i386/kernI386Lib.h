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

/* kernI386.h - Kernel header */

#ifndef _kernI386_h
#define _kernI386_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * kernIntEnt - Called when an interrupt occurs
 *
 * RETURNS: N/A
 */

void kernIntEnt(
    void
    );

/******************************************************************************
 * kernIntExit - Routine only brached to when interrupt ends
 *
 * RETURNS: N/A
 */

void kernIntExit(
    void
    );

/******************************************************************************
 * kernExit - Exit kernel mode
 *
 * RETURNS: OK, ERROR or SIG_RESTART
 */

int kernExit(
    void
    );

/******************************************************************************
 * kernTaskLoadContext - Load task context
 *
 * RETURNS: N/A
 */

void kernTaskLoadContext(
    void
    );

/******************************************************************************
 * kernTaskEntry - Task entry point
 *
 * RETURNS: N/A
 */

void kernTaskEntry(
    void
    );

/******************************************************************************
 * kernIntStackSet - Set interrupt stack pointer
 *
 * RETURNS: N/A
 */

void kernIntStackSet(
    void *pStack
    );

/******************************************************************************
 * kernIntStackEnable - Enable/disable interrupt stack
 *
 * RETURNS: OK or ERROR
 */

STATUS kernIntStackEnable(
    BOOL enable
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _kernI386_h */

