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

/* envShow.c - Environment variables show */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <os/envLib.h>
#include <os/envShow.h>

/* Defines */

/* Locals */

/* Globals */

/* Functions */

/******************************************************************************
 * envShowInit - Initialize environment show library
 *
 * RETURNS: N/A
 */

void envShowInit(
    void
    )
{
}

/******************************************************************************
 * envShow - Show environment
 *
 * RETURNS: N/A
 */

void envShow(
    int taskId
    )
{
    TCB_ID tcbId;
    char **ppEnv;
    int    i;
    int    nEnvEntries;

    /* Initialize locals */
    tcbId = taskTcb(taskId);
    if (tcbId != NULL)
    {
        /* If global environment */
        if (tcbId->ppEnviron == NULL)
        {
            printf("(global environment)\n");
            ppEnv       = ppGlobalEnviron;
            nEnvEntries = globalEnvNEntries;
        }
        else
        {
            printf("(private environment)\n");
            ppEnv       = tcbId->ppEnviron;
            nEnvEntries = tcbId->nEnvVarEntries;
        }

        /* For all environment variables */
        for (i = 0; i < nEnvEntries; i++, ppEnv++)
        {
            printf("%d: %s\n", i, *ppEnv);
        }
    }
    else
    {
        fprintf(stderr, "Invalid object.\n");
    }
}

