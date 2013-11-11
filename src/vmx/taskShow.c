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

/* taskShow.c - Show task info */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <a.out.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/intArchLib.h>
#include <arch/taskArchLib.h>
#include <util/qLib.h>
#include <vmx/taskLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/errnoLib.h>
#include <os/symLib.h>
#include <os/symbol.h>
#include <vmx/taskInfo.h>
#include <vmx/taskShow.h>

/* Defines */
#define MAX_DSP_TASKS           500             /* Max tasks to be displayed */

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Globals */
char *taskRegsFormat ="%-6s = %8x";
char  taskRegsFmt[]  = "%-6s = %8x";

/* Locals */
LOCAL char infoHead[] =
    "\n"
    "  NAME        ENTRY       TID    PRI   STATUS      PC       SP     ERRNO  DELAY\n"
    "---------- ------------ -------- --- ---------- -------- -------- ------- -----\n";

LOCAL void taskSummary(
    TASK_DESC *pTd
    );

/******************************************************************************
 * taskShowInit - Install task show routine
 *
 * RETURNS: OK or ERROR
 */

STATUS taskShowInit(
    void
    )
{
    return classShowConnect(taskClassId, taskShow);
}

/*******************************************************************************
 * taskInfoGet - Get info about task
 *
 * RETURNS: OK or ERROR
 */

STATUS taskInfoGet(
    int taskId,
    TASK_DESC *pTaskDesc
    )
{
    STATUS status;
    char *pStackHigh;
    TASK_DESC  *pTd   = pTaskDesc;
    TCB_ID      tcbId = taskTcb(taskId);

    /* Check task */
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Get information */
        pTd->td_id          = (int) tcbId;
        pTd->td_name        = tcbId->name;
        pTd->td_priority    = (int) tcbId->priority;
        pTd->td_status      = tcbId->status;
        pTd->td_options     = tcbId->options;
        pTd->td_entry       = tcbId->entry;
        pTd->td_sp          = (char *) ((int) tcbId->regs.spReg );
        pTd->td_pStackBase  = tcbId->pStackBase;
        pTd->td_pStackLimit = tcbId->pStackLimit;
        pTd->td_pStackEnd   = tcbId->pStackEnd;

        /* Calculate stack status */
#if (_STACK_DIR == _STACK_GROWS_DOWN)
        if (tcbId->options & TASK_OPTIONS_NO_STACK_FILL)
        {
            pStackHigh = tcbId->pStackLimit;
        }
        else
        {
            for (pStackHigh = tcbId->pStackLimit;
                 *(unsigned char *) pStackHigh == 0xee;
                 pStackHigh++);
        }
#else /* _STACK_GROWS_UP */
        if (tcbId->options & TASK_OPTIONS_NO_STACK_FILL)
        {
            pStackHigh = tcbId->pStackLimit - 1;
        }
        else
        {
            for (pStackHigh = tcbId->pStackLimit - 1;
                 *(unsigned char *) pStackHigh == 0xee;
                 pStackHigh--);
        }
#endif /* _STACK_DIR */

        /* Store stack status */
        pTd->td_stackSize    = (int) (tcbId->pStackLimit - tcbId->pStackBase) *
                                   _STACK_DIR;
        pTd->td_stackHigh    = (int) (pStackHigh - tcbId->pStackBase) *
                                   _STACK_DIR;
        pTd->td_stackMargin  = (int) (tcbId->pStackLimit - pStackHigh) *
                                   _STACK_DIR;
        pTd->td_stackCurrent = (int) (pTd->td_sp - tcbId->pStackBase) *
                                   _STACK_DIR;

        /* Get error status */
        pTd->td_errorStatus = errnoOfTaskGet((int) tcbId);

        /* If task is delayed, get end time */
        if (tcbId->status & TASK_DELAY)
        {
            pTd->td_delay = Q_KEY(&tickQHead, &tcbId->tickNode, 1);
        }
        else
        {
            pTd->td_delay = 0;
        }
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskShow - Print information about task(s)
 *
 * RETURNS: OK or ERROR
 */

STATUS taskShow(
    int taskId,
    int level
    )
{
    STATUS status;
    TCB_ID tcbId;
    TASK_DESC td;
    int i, nTasks;
    int idList[MAX_DSP_TASKS];
    char optString[256];

    /* Get/set default task id */
    taskId = taskIdDefault(taskId);

    /* Select show level */
    switch (level)
    {
        /* Summarize info for one task */
        case 0:
            if (taskInfoGet(taskId, &td) != OK)
            {
                fprintf(stderr, "Task not found.\n");
                status = ERROR;
            }
            else
            {
                /* Print info header */
                printf(infoHead);

                /* Print task summary */
                taskSummary(&td);
                status = OK;
            }
            break;

        /* Detailed info about a single task */
        case 1:
            if (taskInfoGet(taskId, &td) != OK)
            {
                fprintf(stderr, "Task not found.\n");
                status = ERROR;
            }
            else
            {
                /* Get task options string */
                taskOptionsString(taskId, optString);

                /* Print info header */
                printf(infoHead);

                /* Print task summary */
                taskSummary(&td);

                /* Print stack info */
                printf(
                    "\nstack: base 0x%-6x  end 0x%-6x  size %-5d  ",
                    (int) td.td_pStackBase,
                    (int) td.td_pStackEnd,
                    td.td_stackSize
                    );

                /* Extra stack info */
                if (td.td_options & TASK_OPTIONS_NO_STACK_FILL)
                {
                    printf("high %5s  margin %5s\n", "???", "???");
                }
                else
                {
                    printf(
                        "high: %-5d  margin %-5d\n",
                        td.td_stackHigh,
                        td.td_stackMargin
                        );
                }

                /* Print task options */
                printf("\noptions: 0x%x\n%s\n", td.td_options, optString);

                /* If not myself */
                if (taskId != taskIdSelf())
                {
                    /* Show task regs */
                    taskRegsShow(taskId);
                }

                tcbId = taskTcb(taskId);
                if (tcbId == NULL)
                {
                    status = ERROR;
                }
                else
                {
                    excInfoShow(&tcbId->excInfo, FALSE);
                    status = OK;
                }
                break;

            /* Summarize info for all tasks */
            case 2:
                /* Print info header */
                printf(infoHead);

                /* Get number of active tasks */
                nTasks = taskIdListGet(idList, MAX_DSP_TASKS);

                /* Sort by priority level */
                taskIdListSort(idList, nTasks);

                /* Print summary for each task */
                for (i = 0; i < nTasks; i++)
                {
                    if (taskInfoGet(idList[i], &td) == OK)
                    {
                        taskSummary(&td);
                    }
                }

                status = OK;
                break;
        }
    }

    return status;
}

/******************************************************************************
 * taskStatusString - Get task status string
 *
 * RETURNS: OK or ERROR
 */

STATUS taskStatusString(
    int taskId,
    char *str
    )
{
    STATUS status;
    TCB_ID tcbId;

    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Select status string */
        switch (tcbId->status)
        {
            case TASK_READY:
                strcpy(str, "READY");
                status = OK;
                break;

            case TASK_DELAY:
                strcpy(str, "DELAY");
                status = OK;
                break;

            case TASK_PEND:
                strcpy(str, "PEND");
                status = OK;
                break;

            case TASK_SUSPEND:
                strcpy(str, "SUSPEND");
                status = OK;
                break;

            case TASK_DEAD:
                strcpy(str, "DEAD");
                status = OK;
                break;

            case TASK_DELAY |
                 TASK_SUSPEND:
                strcpy(str, "DELAY+S");
                break;

            case TASK_PEND |
                 TASK_DELAY:
                strcpy(str, "PEND+T");
                status = OK;
                break;

            case TASK_PEND |
                 TASK_SUSPEND:
                 strcpy(str, "PEND+S");
                status = OK;
                 break;

            case TASK_PEND  |
                 TASK_DELAY |
                 TASK_SUSPEND:
                strcpy(str, "PEND+S+T");
                status = OK;
                break;

            default:
                strcpy(str, "UNKNOWN");
                status = ERROR;
                break;
        }
    }

    return status;
}

/******************************************************************************
 * taskOptionsString - Get task options string
 *
 * RETURNS: OK or ERROR
 */

STATUS taskOptionsString(
    int taskId,
    char *str
    )
{
    STATUS status;
    TCB_ID tcbId;

    /* Get task TCB */
    tcbId = taskTcb(taskId);
    if (tcbId == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Null terminate string */
        str[0] = EOS;

        /* Get options string */
        if (tcbId->options & TASK_OPTIONS_SUPERVISOR_MODE)
        {
            strcat(str, "SUPERVISOR_MODE     ");
        }

        if (tcbId->options & TASK_OPTIONS_UNBREAKABLE)
        {
            strcat(str, "UNBREAKABLE         ");
        }

        if (tcbId->options & TASK_OPTIONS_DEALLOC_STACK)
        {
            strcat(str, "DEALLOC_STACK       ");
        }

        if (tcbId->options & TASK_OPTIONS_NO_STACK_FILL)
        {
            strcat(str, "NO_STACK_FILL       ");
        }

        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskSummary - Print task summary
 *
 * RETURNS: N/A
 */

LOCAL void taskSummary(
    TASK_DESC *pTd
    )
{
    char str[10];
    REG_SET regSet;
    SYMBOL_ID symId;
    char *name;
    ARG value;

    /* Get status string */
    taskStatusString(pTd->td_id, str);

    /* Print name */
    printf("%-11.11s", pTd->td_name);

    /* If symbol found */
    if (symFindSymbol(
            sysSymTable,
            NULL,
            pTd->td_entry,
            N_TEXT | N_EXT, N_TEXT | N_EXT,
            &symId) == OK
            )
    {
        /* Get name and value */
        symNameGet(symId, &name);
        symValueGet(symId, &value);
    }

    /* Print entry */
    if (pTd->td_entry == (FUNCPTR) value)
    {
        printf("%-12.12s", name);
    }
    else
    {
        printf("%-12x", (int) pTd->td_entry);
    }

    /* Get register set */
    taskRegsGet(pTd->td_id, &regSet);

    /* Print summary */
    printf(
        " %8x %3d %-10.10s %8x %8x %7x %5u\n",
        pTd->td_id,
        pTd->td_priority,
        str,
        ((taskIdSelf() == pTd->td_id) ? (int) taskSummary : (int) regSet.pc),
        (int) regSet.spReg,
        pTd->td_errorStatus,
        pTd->td_delay
        );
}

/******************************************************************************
 * taskRegsShow - Show task register set
 *
 * RETURNS: N/A
 */

void taskRegsShow(
    int taskId
    )
{
    int i;
    int *pRegValue;
    REG_SET regSet;

    /* Get register set */
    if (taskRegsGet(taskId, &regSet) != OK)
    {
        fprintf(stderr, "taskRegsShow: invalid task id %#x\n", taskId);
    }
    else
    {
        /* For all register names */
        for (i = 0; taskRegName[i].regName != NULL; i++)
        {
            if ((i % 4) == 0)
            {
                printf("\n");
            }
            else
            {
                printf("%3s", "");
            }

            /* If register name found */
            if (taskRegName[i].regName[0] != EOS)
            {
                pRegValue = (int *) ((int) &regSet + taskRegName[i].regOffset);
                printf(taskRegsFmt, taskRegName[i].regName, *pRegValue);
            }
            else
            {
                printf("%17s", "");
            }
        }

        printf("\n");
    }
}

/******************************************************************************
 * taskIdListSort - Sort list of task ids by priority
 *
 * RETURNS: N/A
 */

void taskIdListSort(
    int idList[],
    int nTasks
    )
{
    int tmp;
    int currPrio, prevPrio;
    int *pCurrId, *pEndId;
    BOOL change;

    if (nTasks != 0)
    {
        pEndId = &idList[nTasks];
        change = TRUE;

        /* While change */
        while (change == TRUE)
        {
            change = FALSE;

            /* Get start priority */
            taskPriorityGet(idList[0], &prevPrio);

            /* For all tasks */
            for (pCurrId = &idList[1];
                 pCurrId < pEndId;
                 ++pCurrId, prevPrio = currPrio)
            {
                /* Get current task priority */
                taskPriorityGet(*pCurrId, &currPrio);

                /* If swap */
                if (prevPrio > currPrio)
                {
                    tmp = *pCurrId;
                    *pCurrId = *(pCurrId - 1);
                    *(pCurrId - 1) = tmp;
                    change = TRUE;
                }
            }
        }
    }
}

