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

/* taskInfo.h - Task data set/get methods header */

#ifndef _taskInfo_h
#define _taskInfo_h

#include <vmx.h>
#include <vmx/taskLib.h>
#include <arch/regs.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * taskIdDefault - Get/set default task
 *
 * RETURNS: Default taskId
 */

int taskIdDefault(
    int taskId
    );

/******************************************************************************
 * taskName - Get task name
 *
 * RETURNS: Task name string
 */

char* taskName(
    int taskId
    );

/******************************************************************************
 * taskNameToId - Get task id from task name
 *
 * RETURNS: Task id
 */

int taskNameToId(
    char *name
    );

/******************************************************************************
 * taskRegsGet - Get register set from task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskRegsGet(
    int taskId,
    REG_SET *pRegSet
    );

/******************************************************************************
 * taskIdListGet - Get a list of active task ids
 *
 * RETURNS: Number of task id active
 */

int taskIdListGet(
    int idList[],
    int maxTasks
    );

/******************************************************************************
 * taskOptionsSet - Set task options
 * 
 * RETURNS: OK or ERROR
 */
 
STATUS taskOptionsSet(
    int tid,
    int mask,
    int options
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _taskInfo_h */

