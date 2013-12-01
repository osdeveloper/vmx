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

/* usrLib.h - Shell user functions header */

#ifndef _usrLib_h
#define _usrLib_h

#include <vmx.h>
#include <ostool/moduleLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * usrLibInit - Initialize user library
 *
 * RETURNS: OK
 */

STATUS usrLibInit(
    void
    );

/******************************************************************************
 * taskIdFigure - Translate task name or id to task id
 *
 * RETURNS: Task id
 */

int taskIdFigure(
    int taskNameOrId
    );

/******************************************************************************
 * show - Call object show method
 *
 * RETURNS: N/A
 */

void show(
    int objId,
    int level
    );

/******************************************************************************
 * help - Show help
 *
 * RETURNS: N/A
 */

void help(
    void
    );

/******************************************************************************
 * i - Show task summary
 *
 * RETURNS: N/A
 */

void i(
    int taskNameOrId
    );

/******************************************************************************
 * ti - Show task info
 *
 * RETURNS: N/A
 */

void ti(
    int taskNameOrId
    );

/******************************************************************************
 * sp - Spawn a task
 *
 * RETURNS: Task id or ERROR
 */

int sp(
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8,
    ARG arg9
    );

/******************************************************************************
 * ts - Suspend task
 *
 * RETURNS: N/A
 */

void ts(
    int taskNameOrId
    );

/******************************************************************************
 * tr - Resume task
 *
 * RETURNS: N/A
 */

void tr(
    int taskNameOrId
    );

/******************************************************************************
 * td - Delete task
 *
 * RETURNS: N/A
 */

void td(
    int taskNameOrId
    );

/******************************************************************************
 * d - Display memory
 *
 * RETURNS: N/A
 */

void d(
    void *addr,
    int n,
    int size
    );

/******************************************************************************
 * m - Modify memory
 *
 * RETURNS: N/A
 */

void m(
    void *addr,
    int size
    );

/******************************************************************************
 * pc - Get task program counter
 *
 * RETURNS: Task program counter or ERROR
 */

int pc(
    int taskNameOrId
    );

/******************************************************************************
 * devs - Show a list of known devices
 *
 * RETURNS: N/A
 */

void devs(
    void
    );

/******************************************************************************
 * ld - Load object module into memory
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID ld(
    int symFlags,
    BOOL noAbort,
    char *name
    );

/******************************************************************************
 * lkup - List symbols in system symbol table
 *
 * RETURNS: N/A
 */

void lkup(
    char *str
    );

/******************************************************************************
 * periodicRun - Call a function periodically
 *
 * RETURNS: N/A
 */

void periodRun(
    int secs,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    );

/******************************************************************************
 * checkStack - Print stack info
 *
 * RETURNS: N/A
 */

void checkStack(
    int taskNameOrId
    );

/******************************************************************************
 * period - Spawn a task that calls a function periodically
 *
 * RETURNS: Task id or ERROR
 */

int period(
    int secs,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    );

/******************************************************************************
 * repeatRun - Call a function a specified number of times
 *
 * RETURNS: N/A
 */

void repeatRun(
    int n,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    );

/******************************************************************************
 * repeat - Spawn a task that calls a function a specified number of times
 *
 * RETURNS: Task id
 */

int repeat(
    int n,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    );

/******************************************************************************
 * version - Printf kernel version
 *
 * RETURNS: N/A
 */

void version(
    void
    );

/******************************************************************************
 * printLogo - Print the Real VMX logo
 *
 * RETURNS: N/A
 */

void printLogo(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _usrLib_h */

