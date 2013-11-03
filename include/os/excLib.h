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

/* excLib.h - Exception library header */

#ifndef _excLib_h
#define _excLib_h

#include <vmx.h>
#include <vmx/taskLib.h>

/* Defines */
#define EXC_MAX_ARGS            6
#define EXC_MAX_MSGS            10

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Typedefs */
typedef struct excfaultTab
{
    int faultType;
    int subtype;
    int signal;
    int code;
} EXC_FAULT_TAB;

typedef struct
{
    VOIDFUNCPTR func;
    ARG         arg[EXC_MAX_ARGS];
} EXC_MSG;

/* Functions */

/******************************************************************************
 * excLibInit - Initialize exception library
 *
 * RETURNS: OK or ERROR
 */

STATUS excLibInit(
    void
    );

/******************************************************************************
 * excJobAdd - Add work to exception task
 *
 * RETURNS: OK or ERROR
 */

STATUS excJobAdd(
    VOIDFUNCPTR func,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    );

/******************************************************************************
 * printErr - Print error message
 *
 * RETURNS: Return from printf
 */

int printErr(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4
    );

/******************************************************************************
 * printExc - Print exception message
 *
 * RETURNS: N/A
 */

void printExc(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3, 
    ARG arg4
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _excLib_h */

