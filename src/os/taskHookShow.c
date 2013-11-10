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

/* taskHookShow.c - Show installed task hooks */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <os/taskHookLib.h>
#include <os/private/taskHookLibP.h>
#include <os/taskHookShow.h>

/* Defines */

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Locals */
LOCAL void taskHookShow(
    FUNCPTR table[],
    int max
    );

/* Globals */

/* Functions */

/******************************************************************************
 * taskHookShowInit - Initialize show hooks module
 *
 * RETURNS: N/A
 */

void taskHookShowInit(
    void
    )
{
}

/******************************************************************************
 * taskCreateHookShow - Show task create hooks
 *
 * RETURNS: N/A
 */

void taskCreateHookShow(
    void
    )
{
    taskHookShow(taskCreateHooks, MAX_TASK_CREATE_HOOKS);
}

/******************************************************************************
 * taskSwitchHookShow - Show task switch hooks
 *
 * RETURNS: N/A
 */

void taskSwitchHookShow(
    void
    )
{
    taskHookShow(taskSwitchHooks, MAX_TASK_SWITCH_HOOKS);
}

/******************************************************************************
 * taskDeleteHookShow - Show task delete hooks
 *
 * RETURNS: N/A
 */

void taskDeleteHookShow(
    void
    )
{
    taskHookShow(taskDeleteHooks, MAX_TASK_DELETE_HOOKS);
}

/******************************************************************************
 * taskSwapHookShow - Show task swap hooks
 *
 * RETURNS: N/A
 */

void taskSwapHookShow(
    void
    )
{
    taskHookShow(taskSwapHooks, MAX_TASK_SWAP_HOOKS);
}

/******************************************************************************
 * taskHookShow - General show function for task hooks
 *
 * RETURNS: N/A
 */

LOCAL void taskHookShow(
    FUNCPTR table[],
    int max
    )
{
    int i;
    char *name;
    ARG symValue;
    SYMBOL_ID symId;
    FUNCPTR value;

    /* Initialize locals */
    symValue = (ARG) 0;

    /* For max task hooks */
    for (i = 0; i < max; i++)
    {
        /* Get value */
        value = table[i];

        /* If no more hooks */
        if (value == NULL)
        {
            break;
        }

        /* If symbol found */
        if (symFindSymbol(
                sysSymTable,
                NULL,
                value,
                SYM_MASK_NONE,
                SYM_MASK_NONE,
                &symId
                ) == OK)
        {
            /* Get name and value */
            symNameGet(symId, &name);
            symValueGet(symId, &symValue);
        }

        /* If symbol found */
        if (symValue == value)
        {
            printf("%-15s", name);
        }
        else
        {
            printf("%#21x", table[i]);
        }

        printf("\n");
    }
}

