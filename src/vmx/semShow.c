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

/* semShow.c - Semaphore show library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <vmx/classLib.h>
#include <vmx/taskLib.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/semLib.h>
#include <vmx/semShow.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL char *semTypeName[MAX_SEM_TYPE] =
{
    "BINARY",
    "MUTEX",
    "COUNTING"
};

/* Globals */

/* Functions */

/******************************************************************************
 * semShowInit - Initialize semaphore info facilities
 *
 * RETURNS: OK or ERROR
 */

STATUS semShowInit(
    void
    )
{
    return classShowConnect(semClassId, (FUNCPTR) semShow);
}

/******************************************************************************
 * semInfo - Get list of task blocking on a semaphore
 *
 * RETURNS: Number of tasks
 */

int semInfo(
    SEM_ID semId,
    int idList[],
    int max
    )
{
    int n, level;

    if (INT_RESTRICT() != OK)
    {
        n = ERROR;
    }
    else
    {
        /* Lock interrupts */
        INT_LOCK(level);

        /* Verify object class */
        if (OBJ_VERIFY(semId, semClassId) != OK)
        {
            INT_UNLOCK(level);
            n = ERROR;
        }
        else
        {
            /* Get number of tasks blocking on semaphore */
            n = Q_INFO(&semId->qHead, idList, max);

            /* Unlock interrupts */
            INT_UNLOCK(level);
        }
    }

    return n;
}

/******************************************************************************
 * semShow - Show semaphore info
 *
 * RETURNS: OK or ERROR
 */

STATUS semShow(
    SEM_ID semId,
    int mode
    )
{
    STATUS status;
    TCB_ID tcbId;
    int i, nTask;
    int level;
    int taskIdList[20], taskDList[20];
    char tmpString[128];

    /* Lock interrupts */
    INT_LOCK(level);

    /* Get number of tasks blocking on semaphore */
    nTask = semInfo(semId, taskIdList, 20);
    if (nTask == ERROR)
    {
        INT_UNLOCK(level);
        printf("Invalid semaphore id: %#x\n", (int) semId);
        status = ERROR;
    }
    else
    {
        /* If any tasks in list */
        if (nTask > 0)
        {
            /* For all tasks */
            for (i = 0;
                 i < min(nTask, NELEMENTS(taskIdList));
                 i++)
            {
                tcbId = (TCB_ID) taskIdList[i];
                if (tcbId->status & TASK_DELAY)
                {
                    taskDList[i] = Q_KEY(&tickQHead, &tcbId->tickNode, 1);
                }
                else
                {
                    taskDList[i] = 0;
                }
            }
        }

        /* Unlock interrupts */
        INT_UNLOCK(level);

        /* Get queue type */
        if ((semId->options & SEM_Q_MASK) == SEM_Q_FIFO)
        {
            strcpy(tmpString, "FIFO");
        }
        else
        {
            strcpy(tmpString, "PRIORITY");
        }

        /* Print info */
        printf("\n");
        printf("Semaphore Id        : 0x%-10x\n", (int) semId);
        printf("Semaphore Type      : %-10s\n", semTypeName[semId->semType]);
        printf("Task Queuing        : %-10s\n", tmpString);
        printf("Pended Tasks        : %-10d\n", nTask);

        /* Select semaphore type */
        switch(semId->semType)
        {
            case SEM_TYPE_BINARY:
                if (tcbId != NULL)
                {
                    printf("State               : %-10s\n", "EMPTY");
                }
                else
                {
                    printf("State               : %-10s\n", "FULL");
                }
                break;

            case SEM_TYPE_MUTEX:
                /* If non null tcb */
                if (tcbId != NULL)
                {
                    printf("Owner               : 0x%-10x", (int) tcbId);
                    if (taskIdVerify((int) tcbId) != OK)
                    {
                        printf(" Deleted!\n");
                    }
                    else if (tcbId->name != NULL)
                    {
                        printf(" (%s)\n", tcbId->name);
                    }
                    printf("\n");
                }
                else
                {
                    printf("Owner               : %-10s\n", "NONE");
                }
                break;

            case SEM_TYPE_COUNTING:
                printf("Count:              : %-10d\n", SEM_COUNT(semId));
                break;

            default:
                printf("State:             : 0x%-10x\n", (int) tcbId);
                break;
        }

        /* Get options string */
        if ((semId->options & SEM_Q_MASK) == SEM_Q_FIFO)
        {
            strcpy(tmpString, "SEM_Q_FIFO");
        }
        else
        {
            strcpy(tmpString, "SEM_Q_PRIORITY");
        }

        /* Show options */
        printf("Options             : 0x%x\t%s\n", semId->options, tmpString);

        /* If detailed info requested */
        if (mode >= 1)
        {
            /* If tasks pending */
            if (nTask > 0)
            {
                printf(
                    "\nPended Tasks\n"
                    "------------\n"
                    );

                printf("   NAME      TID    PRI TIMEOUT\n");
                printf("---------- -------- --- -------\n");

                /* For all tasks */
                for (i = 0; i < min( nTask, NELEMENTS(taskIdList) ); i++)
                {
                    printf(
                        "%-11.11s%8x %3d %7u\n",
                        taskName(taskIdList[i]),
                        taskIdList[i],
                        ((TCB_ID) taskIdList[i])->priority,
                        taskDList[i]
                        );
                }
            }
        }

        printf("\n");
        status = OK;
    }

    return status;
}

