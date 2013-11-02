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

/* taskInfo.c - Task data set/get methods */

#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>

/* Globals */
int defaultTaskId = 0;

/* Locals */
LOCAL BOOL taskNameNoMatch(
    Q_NODE *pNode,
    char *name
    );

/******************************************************************************
 * taskIdDefault - Get/set default task
 *
 * RETURNS: Default taskId
 */

int taskIdDefault(
    int taskId
    )
{

    /* If nonzero agument, set default id */
    if (taskId != 0)
    {
        defaultTaskId = taskId;
    }

    return defaultTaskId;
}

/******************************************************************************
 * taskName - Get task name
 *
 * RETURNS: Task name string
 */

char* taskName(
    int taskId
    )
{
    char *name;
    TCB_ID tcbId;

    /* Get task tcb */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        name = NULL;
    }
    else
    {
        if (tcbId->name == NULL)
        {
            name = "\0";
        }
        else
        {
            name = tcbId->name;
        }
    }

    return name;
}

/******************************************************************************
 * taskNameToId - Get task id from task name
 *
 * RETURNS: Task id or ERROR
 */

int taskNameToId(
    char *name
    )
{
    int result;
    int taskId;

    /* Try to match each node in the active queue against name */
    taskId = (int) Q_EACH(&activeQHead, taskNameNoMatch, name);

    /* If no match was found */
    if (taskId == 0)
    {
        result = ERROR;
    }
    else
    {
        result = taskId - OFFSET(TCB, activeNode);
    }

    return result;
}

/******************************************************************************
 * taskNameNoMatch - Check for task name mismatch
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL taskNameNoMatch(
    Q_NODE *pNode,
    char *name
    )
{
    BOOL result;
    TCB_ID tcbId;

    /* Get active node */
    tcbId = (TCB_ID) ( (int) pNode - OFFSET(TCB, activeNode) );

    if ((tcbId->name == NULL) || (strcmp(tcbId->name, name) != 0))
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

/******************************************************************************
 * taskRegsGet - Get register set from task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskRegsGet(
    int taskId,
    REG_SET *pRegSet
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get tcb */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Copy exception regs to regs set */
        if (tcbId->pExcRegSet != NULL)
        {
            memcpy(&tcbId->regs, tcbId->pExcRegSet, sizeof(REG_SET));
            tcbId->pExcRegSet = NULL;
        }

        memcpy(pRegSet, &tcbId->regs, sizeof(REG_SET));
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskIdListGet - Get a list of active task ids
 *
 * RETURNS: Number of task id active
 */

int taskIdListGet(
    int idList[],
    int maxTasks
    )
{
    int i, count;

    /* Lock task */
    taskLock();

    /* Get number of nodes in active queue */
    count = Q_INFO(&activeQHead, idList, maxTasks);

    /* Unlock task */
    taskUnlock();

    /* Extract active node from TCB */
    for (i = 0; i < count; i++)
    {
        idList[i] -= OFFSET(TCB, activeNode);
    }

    return count;
}

/******************************************************************************
 * taskOptionsSet - Set task options
 *
 * RETURNS: OK or ERROR
 */

STATUS taskOptionsSet(
    int tid,
    int mask,
    int options
    )
{
    STATUS status;
    TCB_ID tcbId;

    if (INT_RESTRICT() != OK)
    {
        status = ERROR;
    }
    else
    {
        taskLock();

        tcbId = taskTcb(tid);
        if (tcbId == NULL)
        {
            taskUnlock();
            status = ERROR;
        }
        else
        {
            tcbId->options = (tcbId->options & mask) | options;
            taskUnlock();
            status = OK;
        }
    }

    return status;
}

