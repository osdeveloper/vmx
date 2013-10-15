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

/* kernQLib.h - Kernel work queue library */

#ifndef _kernQLib_h
#define _kernQLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Struct */
typedef struct
{
    FUNCPTR func;    /* ptr to function that was added */
    int     numArgs; /* # of arguments to added item */
    ARG     arg1;    /* 1st argument of added function (if applicable) */
    ARG     arg2;    /* 2nd argument of added function (if applicable) */
} KERN_JOB;

/* Globals */
extern volatile BOOL kernQEmpty;

/* Functions */
/******************************************************************************
 * kernQLibInit - Initialize kernel work queue
 *
 * RETURNS: OK
 */

STATUS kernQLibInit(
    void
    );

/******************************************************************************
 * kernQAdd0 - Add a kernel job with no aguments
 *
 * RETURNS: N/A
 */

void kernQAdd0(
    FUNCPTR func    /* ptr to deferred function */
    );

/******************************************************************************
 * kernQAdd1 - Add a kernel job with one agument
 *
 * RETURNS: N/A
 */

void kernQAdd1(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1          /* 1st argument to deferred function */
    );

/******************************************************************************
 * kernQAdd2 - Add a kernel job with two aguments
 *
 * RETURNS: N/A
 */

void kernQAdd2(
    FUNCPTR func,     /* ptr to function to add */
    ARG arg1,         /* 1st argument to deferred function */
    ARG arg2          /* 2nd argument to deferred function */
    );

/******************************************************************************
 * kernQDoWork - Do job on the kernel queue
 *
 * RETURNS: N/A
 */

void kernQDoWork(
    void
    );

/******************************************************************************
 * kernQPanic - Fatal error in queue
 *
 * RETURNS: N/A
 */

void kernQPanic(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _kernQLib_h */

