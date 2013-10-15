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

/* workQLib.h - Kernel work queue library */

#ifndef _workQLib_h
#define _workQLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>

/* Globals */
IMPORT volatile BOOL workQEmpty;

/* Functions */
/******************************************************************************
 * workQLibInit - Initialize kernel work queue
 *
 * RETURNS: OK
 */

STATUS workQLibInit(
    void
    );

/******************************************************************************
 * workQAdd0 - Add a kernel job with no aguments
 *
 * RETURNS: N/A
 */

void workQAdd0(
    FUNCPTR func    /* ptr to deferred function */
    );

/******************************************************************************
 * workQAdd1 - Add a kernel job with one agument
 *
 * RETURNS: N/A
 */

void workQAdd1(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1          /* 1st argument to deferred function */
    );

/******************************************************************************
 * workQAdd2 - Add a kernel job with two aguments
 *
 * RETURNS: N/A
 */

void workQAdd2(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1,         /* 1st argument to deferred function */
    ARG arg2          /* 2nd argument to deferred function */
    );

/******************************************************************************
 * workQDoWork - Do job on the kernel queue
 *
 * RETURNS: N/A
 */

void workQDoWork(
    void
    );

/******************************************************************************
 * workQPanic - Fatal error in queue
 *
 * RETURNS: N/A
 */

void workQPanic(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _workQLib_h */

