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

/* taskHookLib.c - Hooks called when kernel switch tasks */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/taskHookLib.h>

/* GLOBALS */
FUNCPTR taskCreateHooks[MAX_TASK_CREATE_HOOKS + 1];
FUNCPTR taskSwitchHooks[MAX_TASK_SWITCH_HOOKS + 1];
FUNCPTR taskDeleteHooks[MAX_TASK_DELETE_HOOKS + 1];
FUNCPTR taskSwapHooks[MAX_TASK_SWAP_HOOKS + 1];
int     taskSwapReference[MAX_TASK_SWAP_HOOKS + 1];

/* LOCALS */
LOCAL BOOL taskHookLibInstalled = FALSE;

LOCAL STATUS taskHookAdd(
    FUNCPTR hook,
    FUNCPTR table[],
    int max
    );

LOCAL STATUS taskHookDelete(
    FUNCPTR hook,
    FUNCPTR table[],
    int max
    );

LOCAL STATUS taskSwapMaskSet(
    int taskId,
    int index,
    BOOL swapIn,
    BOOL swapOut
    );

LOCAL STATUS taskSwapMaskClear(
    int taskId,
    int index,
    BOOL swapIn,
    BOOL swapOut
    );

/******************************************************************************
 * taskHookLibInit - Initialize task switch hook library
 *
 * RETURNS: OK
 */

STATUS taskHookLibInit(
    void
    )
{
    STATUS status;
    int i;

    if (taskHookLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Null all entries */
        for (i = 0; i < MAX_TASK_CREATE_HOOKS; i++)
        {
            taskCreateHooks[i] = NULL;
        }

        for (i = 0; i < MAX_TASK_SWITCH_HOOKS; i++)
        {
            taskSwitchHooks[i] = NULL;
        }

        for (i = 0; i < MAX_TASK_DELETE_HOOKS; i++)
        {
            taskDeleteHooks[i] = NULL;
        }

        for (i = 0; i < MAX_TASK_SWAP_HOOKS; i++)
        {
            taskSwapHooks[i]     = NULL;
            taskSwapReference[i] = 0;
        }

        /* Mark as installed */
        taskHookLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskCreateHookAdd - Add a task create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskCreateHookAdd(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookAdd(hook, taskCreateHooks, MAX_TASK_CREATE_HOOKS);
}

/******************************************************************************
 * taskCreateHookDelete - Delete a task create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskCreateHookDelete(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookDelete(hook, taskCreateHooks, MAX_TASK_CREATE_HOOKS);
}

/******************************************************************************
 * taskSwitchHookAdd - Add a task switch hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwitchHookAdd(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookAdd(hook, taskSwitchHooks, MAX_TASK_SWITCH_HOOKS);
}

/******************************************************************************
 * taskSwitchHookDelete - Delete a task switch hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwitchHookDelete(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookDelete(hook, taskSwitchHooks, MAX_TASK_SWITCH_HOOKS);
}

/******************************************************************************
 * taskDeleteHookAdd - Add a task delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteHookAdd(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookAdd(hook, taskDeleteHooks, MAX_TASK_DELETE_HOOKS);
}

/******************************************************************************
 * taskDeleteHookDelete - Delete a task switch delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskDeleteHookDelete(
    FUNCPTR hook
    )
{
    /* Call general function */
    return taskHookDelete(hook, taskDeleteHooks, MAX_TASK_DELETE_HOOKS);
}

/******************************************************************************
 * taskHookAdd - Add a task hook
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS taskHookAdd(
    FUNCPTR hook,
    FUNCPTR table[],
    int max
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find next free slot */
    for (i = 0; i < max; i++)
    {
        if (table[i] == NULL)
        {
            /* Insert into fist free spot */
            table[i] = hook;
            status   = OK;
            break;
        }
    }

    taskUnlock();

    /* No slots left */
    if (status != OK)
    {
        errnoSet(S_taskLib_TASK_HOOK_TABLE_FULL);
    }

    return status;
}

/******************************************************************************
 * taskHookDelete - Delete a task hook
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS taskHookDelete(
    FUNCPTR hook,
    FUNCPTR table[],
    int max
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find entry */
    for (i = 0; i < max; i++)
    {
        if (table[i] == hook)
        {
            /* Move up all other hooks */
            do
            {
                table[i] = table[i + 1];
            } while (table[i] != NULL);

            /* Done */
            status = OK;
            break;
        }
    }

    taskUnlock();

    /* Slot not found */
    if (status != OK)
    {
        errnoSet(S_taskLib_TASK_HOOK_NOT_FOUND);
    }

    return status;
}

/******************************************************************************
 * taskSwapHookAdd - Add a task swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookAdd(
    FUNCPTR hook
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find next free slot */
    for (i = 0; i < MAX_TASK_SWAP_HOOKS; i++)
    {
        if (taskSwapHooks[i] == NULL)
        {
            /* Insert into fist free spot */
            taskSwapHooks[i]     = hook;
            taskSwapReference[i] = 0;

            /* Done */
            status = OK;
            break;
        }
    }

    taskUnlock();

    /* Slot not found */
    if (status != OK)
    {
        errnoSet(S_taskLib_TASK_HOOK_TABLE_FULL);
    }

    return status;
}

/******************************************************************************
 * taskSwapHookAttach - Attach a task to swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookAttach(
    FUNCPTR hook,
    int taskId,
    BOOL swapIn,
    BOOL swapOut
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find hook */
    for (i = 0; i < MAX_TASK_SWAP_HOOKS; i++)
    {
        if (taskSwapHooks[i] == hook)
        {
            if ((taskSwapMaskSet(taskId, i, swapIn, swapOut) != OK))
            {
                /* Error */
                status = ERROR;
                break;
            }
            else
            {
                /* Set swap in/out */
                taskSwapReference[i] += (swapIn)  ? 1 : 0;
                taskSwapReference[i] += (swapOut) ? 1 : 0;
                status = OK;
                break;
            }
        }
    }

    taskUnlock();

    if (status != OK)
    {
        /* Slot not found */
        if (i == MAX_TASK_SWAP_HOOKS)
        {
            errnoSet(S_taskLib_TASK_HOOK_NOT_FOUND);
        }
    }

    return status;
}

/******************************************************************************
 * taskSwapHookDetach - Detach a task to swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookDetach(
    FUNCPTR hook,
    int taskId,
    BOOL swapIn,
    BOOL swapOut
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find hook */
    for (i = 0; i < MAX_TASK_SWAP_HOOKS; i++)
    {
        if (taskSwapHooks[i] == hook)
        {
            if ((taskSwapMaskClear(taskId, i, swapIn, swapOut) != OK))
            {
                /* Error */
                status = ERROR;
                break;
            }
            else
            {
                /* Reset in/out */
                taskSwapReference[i] -= (swapIn)  ? 1 : 0;
                taskSwapReference[i] -= (swapOut) ? 1 : 0;
                status = OK;
                break;
            }
        }
    }

    taskUnlock();

    if (status != OK)
    {
        /* Slot not found */
        if (i == MAX_TASK_SWAP_HOOKS)
        {
            errnoSet(S_taskLib_TASK_HOOK_NOT_FOUND);
        }
    }

    return status;
}

/******************************************************************************
 * taskSwapHookDelete - Delete a task swap hook
 *
 * RETURNS: OK or ERROR
 */

STATUS taskSwapHookDelete(
    FUNCPTR hook
    )
{
    int i;
    STATUS status = ERROR;

    taskLock();

    /* Find entry */
    for (i = 0; i < MAX_TASK_SWAP_HOOKS; i++)
    {
        if (taskSwapHooks[i] == hook)
        {
            /* Check if task are still connected to this hook */
            if (taskSwapReference[i] != 0)
            {
                /* Unable to remove */
                errnoSet(S_taskLib_TASK_SWAP_HOOK_REFERENCED);
                status = ERROR;
                break;
            }
            else
            {
                /* Remove entry */
                taskSwapHooks[i] = NULL;
                status = OK;
                break;
            }
        }
    }

    taskUnlock();

    if (status != OK)
    {
        /* Slot not found */
        if (i == MAX_TASK_SWAP_HOOKS)
        {
            errnoSet(S_taskLib_TASK_HOOK_NOT_FOUND);
        }
    }

    return status;
}

/******************************************************************************
 * taskSwapMaskSet - Set swap mask for a task
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS taskSwapMaskSet(
    int taskId,
    int index,
    BOOL swapIn,
    BOOL swapOut
    )
{
    STATUS status;
    TCB_ID tcbId;
    u_int16_t indexBit;

    /* Get TCB */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Calculate index bit */
        indexBit = (1 << (15 - index));

        /* Check if valid  */
        if (((swapIn  == TRUE) && (tcbId->swapInMask & indexBit)) ||
            ((swapOut == TRUE) && (tcbId->swapOutMask & indexBit)))
        {
            errnoSet(S_taskLib_TASK_SWAP_HOOK_SET);
            status = ERROR;
        }
        else
        {
            /* Set bits */
            if (swapIn == TRUE)
            {
                tcbId->swapInMask |= indexBit;
            }

            if (swapOut == TRUE)
            {
                tcbId->swapOutMask |= indexBit;
            }

            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * taskSwapMaskClear - Clear swap mask for a task
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS taskSwapMaskClear(
    int taskId,
    int index,
    BOOL swapIn,
    BOOL swapOut
    )
{
    STATUS status;
    TCB_ID tcbId;
    u_int16_t indexBit;

    /* Get TCB */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Calculate index bit */
        indexBit = (1 << (15 - index));

        /* Check if valid  */
        if (((swapIn  == TRUE) && (tcbId->swapInMask & indexBit)) ||
            ((swapOut == TRUE) && (tcbId->swapOutMask & indexBit)))
        {
            errnoSet(S_taskLib_TASK_SWAP_HOOK_CLEAR);
            status = ERROR;
        }
        else
        {
            /* Reset bits */
            if (swapIn == TRUE)
            {
                tcbId->swapInMask &= ~indexBit;
            }

            if (swapOut == TRUE)
            {
                tcbId->swapOutMask &= ~indexBit;
            }

            status = OK;
        }
    }

    return status;
}

