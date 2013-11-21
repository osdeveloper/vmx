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

/* taskVarLib.c - Task variable library */

/* Includes */

#include <stdlib.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <vmx/taskLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/taskHookLib.h>
#include <os/taskVarLib.h>
#include <os/errnoLib.h>

/* Defines */

/* Locals */
LOCAL BOOL taskVarLibInstalled = FALSE;

LOCAL void taskVarSwitchHook(
    TCB_ID oldId,
    TCB_ID newId
    );

LOCAL void taskVarDeleteHook(
    TCB_ID tcbId
    );

/* Globals */

/* Functions */

/******************************************************************************
 * taskVarLibInit - Initialize task variable library
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarLibInit(
    void
    )
{
    STATUS status;

    /* Check if already installed */
    if (taskVarLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        if (taskSwitchHookAdd((FUNCPTR) taskVarSwitchHook) != OK)
        {
            status = ERROR;
        }
        else
        {
            if (taskDeleteHookAdd((FUNCPTR) taskVarDeleteHook) != OK )
            {
                taskSwitchHookDelete((FUNCPTR) taskVarSwitchHook);
                status = ERROR;
            }
            else
            {
                /* Mark as installed */
                taskVarLibInstalled = TRUE;
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskVarAdd - Add a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarAdd(
    int  taskId,
    int *pVar
    )
{
    STATUS    status;
    TCB_ID    tcbId;
    TASK_VAR *pNewVar;
    int       level;

    /* Check library */
    if (taskVarLibInstalled != TRUE)
    {
        status = ERROR;
    }
    else
    {
        /* Get task tcb */
        tcbId = taskTcb(taskId);
        if (tcbId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Allocate variable storage */
            pNewVar = (TASK_VAR *) malloc(sizeof(TASK_VAR));
            if (pNewVar == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Initialize task variable struct */
                pNewVar->addr  = pVar;
                pNewVar->value = *pVar;

                /* Lock interrupts */
                INT_LOCK(level);

                /* Add task variable to list in tcb */
                pNewVar->next   = tcbId->pTaskVar;
                tcbId->pTaskVar = pNewVar;

                /* Unlock interrupts */
                INT_UNLOCK(level);
                status = OK;
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskVarSet - Set value of a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarSet(
    int  taskId,
    int *pVar,
    int  value
    )
{
    STATUS status;
    TASK_VAR *pTaskVar;
    TCB_ID tcbId;

    /* Get task tcb */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        status = ERROR;

        /* For all variables in list */
        for (pTaskVar = tcbId->pTaskVar;
             pTaskVar != NULL;
             pTaskVar = pTaskVar->next)
        {
            /* If match */
            if (pTaskVar->addr == pVar)
            {
                /* If current task owner */
                if (tcbId == taskIdCurrent)
                {
                    *pVar = value;
                }
                else
                {
                    pTaskVar->value = value;
                }

                status = OK;
                break;
            }
        }

        if (status != OK)
        {
            errnoSet(S_taskLib_TASK_VAR_NOT_FOUND);
        }
    }

    return status;
}

/******************************************************************************
 * taskVarGet - Get value of a task variable
 *
 * RETURNS: Value of task variable or ERROR
 */

int taskVarGet(
    int taskId,
    int *pVar
    )
{
    int       ret;
    TASK_VAR *pTaskVar;
    TCB_ID    tcbId;
    BOOL      varFound;

    /* Get task tcb */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        ret = ERROR;
    }
    else
    {
        varFound = FALSE;

        /* For all variables in list */
        for (pTaskVar = tcbId->pTaskVar;
             pTaskVar != NULL;
             pTaskVar = pTaskVar->next)
        {
            /* If match */
            if (pTaskVar->addr == pVar)
            {
                /* If current task owner */
                if (tcbId == taskIdCurrent)
                {
                    ret = *pVar;
                }
                else
                {
                    ret = pTaskVar->value;
                }

                varFound = TRUE;
                break;
            }
        }

        if (varFound != TRUE)
        {
            errnoSet(S_taskLib_TASK_VAR_NOT_FOUND);
            ret = ERROR;
        }

    }

    return ret;
}

/******************************************************************************
 * taskVarInfo - Get task variable info for a task
 *
 * RETURNS: Number of variables in tasks variable list
 */

int taskVarInfo(
    int      taskId,
    TASK_VAR varList[],
    int max
    )
{
    int       ret;
    int       i;
    TASK_VAR *pTaskVar;
    TCB_ID    tcbId;

    /* Get task tcb */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        ret = ERROR;
    }
    else
    {
        /* Safety first */
        taskLock();

        /* For all variables in list */
        for (pTaskVar = tcbId->pTaskVar, i = 0;
             (pTaskVar != NULL) && (i < max);
             pTaskVar = pTaskVar->next, i++)
        {
            /* Get value */
            if (varList != NULL)
            {
                varList[i] = *pTaskVar;
            }
        }

        taskUnlock();
        ret = i;
    }

    return ret;
}

/******************************************************************************
 * taskVarDelete - Remove a task variable
 *
 * RETURNS: OK or ERROR
 */

STATUS taskVarDelete(
    int  taskId,
    int *pVar
    )
{
    STATUS     status;
    TCB_ID     tcbId;
    TASK_VAR **ppTaskVar;
    TASK_VAR  *pTaskVar;
    int        level;

    /* Check library */
    if (taskVarLibInstalled == TRUE)
    {
        status = ERROR;
    }
    else
    {
        /* Get task tcb */
        tcbId = taskTcb(taskId);
        if (tcbId == NULL)
        {
            status = ERROR;
        }
        else
        {
            status = ERROR;

            /* For all variables tasks variable list */
            for (ppTaskVar = &tcbId->pTaskVar;
                 *ppTaskVar != NULL;
                 ppTaskVar = &((*ppTaskVar)->next))
            {
                /* Get varable pointer */
                pTaskVar = *ppTaskVar;

                /* If variable matches */
                if (pTaskVar->addr == pVar)
                {
                    /* Store variable if current task */
                    if (tcbId == taskIdCurrent)
                    {
                        *pVar = pTaskVar->value;
                    }

                    /* Get next variable in list */
                    *ppTaskVar = pTaskVar->next;

                    /* Free old variable */
                    free(pTaskVar);

                    status = OK;
                    break;
                }
            }

            if (status != OK)
            {
                errnoSet(S_taskLib_TASK_VAR_NOT_FOUND);
            }
        }
    }

    return status;
}

/******************************************************************************
 * taskVarSwitchHook - Called when task switch occurs
 *
 * RETURNS: N/A
 */

LOCAL void taskVarSwitchHook(
    TCB_ID oldTcbId,
    TCB_ID newTcbId
    )
{
    int       value;
    TASK_VAR *pTaskVar;

    if (TASK_ID_VERIFY(oldTcbId) == OK)
    {
        /* For all variables in list */
        for (pTaskVar = oldTcbId->pTaskVar;
             pTaskVar != NULL;
             pTaskVar = pTaskVar->next)
        {
            /* Swap */
            value             = pTaskVar->value;
            pTaskVar->value   = *(pTaskVar->addr);
            *(pTaskVar->addr) = value;
        }
    }

    /* For all variables in list */
    for (pTaskVar = newTcbId->pTaskVar;
         pTaskVar != NULL;
         pTaskVar = pTaskVar->next)
    {
        /* Swap */
        value             = pTaskVar->value;
        pTaskVar->value   = *(pTaskVar->addr);
        *(pTaskVar->addr) = value;
    }
}

/******************************************************************************
 * taskVarDeleteHook - Called when task delete occurs
 *
 * RETURNS: N/A
 */

LOCAL void taskVarDeleteHook(
    TCB_ID tcbId
    )
{
    TASK_VAR *pTaskVar;
    TASK_VAR *pTaskVarNext = NULL;

    /* For all variables in list */
    for (pTaskVar = tcbId->pTaskVar;
         pTaskVar != NULL;
         pTaskVar = pTaskVarNext)
    {
        /* Get next variable */
        pTaskVarNext = pTaskVar->next;

        /* Free storage */
        free(pTaskVar);
    }
}

