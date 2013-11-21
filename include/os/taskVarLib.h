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

/* taskVarLib.h - Task variables library header */

#ifndef _taskVarLib_h
#define _taskVarLib_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct taskVar
{
    struct taskVar *next;
    int            *addr;
    int             value;
} TASK_VAR;

/* Functions */

/******************************************************************************
 * taskVarLibInit - Initialize task variable library
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarLibInit(
    void
    );

/******************************************************************************
 * taskVarAdd - Add a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarAdd(
    int  taskId,
    int *pVar
    );

/******************************************************************************
 * taskVarSet - Set value of a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarSet(
    int  taskId,
    int *pVar,
    int  value
    );

/******************************************************************************
 * taskVarGet - Get value of a task variable
 *
 * RETURNS: Value of task variable or ERROR
 */

int taskVarGet(
    int taskId,
    int *pVar
    );

/******************************************************************************
 * taskVarInfo - Get task variable info for a task
 *
 * RETURNS: Number of variables in tasks variable list
 */

int taskVarInfo(
    int      taskId,
    TASK_VAR varList[],
    int max
    );

/******************************************************************************
 * taskVarDelete - Remove a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarDelete(
    int  taskId,
    int *pVar
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskVarLib_h */

