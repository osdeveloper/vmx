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

/* envLib.c - Environment variable library */

/* Includes */
#include <stdlib.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <os/taskHookLib.h>
#include <os/logLib.h>
#include <os/envLib.h>

/* Defines */
#define ENV_MAX_ENTRIES                 50

/* Imports */

/* Locals */
LOCAL char** envFind(
    char *name,
    int   namelen
    );

LOCAL void envCreateHook(
    TCB_ID tcbId
    );

LOCAL void envDeleteHook(
    TCB_ID tcbId
    );

LOCAL STATUS envDuplicate(
    char **srcEnv,
    int srcEnvTableSize,
    int srcNEnvVarEntries,
    TCB_ID tcbId
    );

LOCAL void envDestroy(
    char **ppEnv,
    int nEnvVarEntries
    );

/* Globals */
char **ppGlobalEnviron;
int    globalEnvTableSize;
int    globalEnvNEntries;

/* Functions */

/******************************************************************************
 * envLibInit - Initialize environment variable library
 *
 * RETURNS: OK or ERROR
 */

STATUS envLibInit(
    BOOL installHooks
    )
{
    STATUS status;

    /* Allocate storage */
    ppGlobalEnviron = (char **) malloc(ENV_MAX_ENTRIES * sizeof(char *));
    if (ppGlobalEnviron == NULL)
    {
        status = ERROR;
    }
    else
    {
        globalEnvTableSize = ENV_MAX_ENTRIES;
        globalEnvNEntries  = 0;

        /* Optional hooks */
        if (installHooks == TRUE)
        {
            if (taskCreateHookAdd((FUNCPTR) envCreateHook) != OK)
            {
                status = ERROR;
            }
            else
            {
                if (taskDeleteHookAdd((FUNCPTR) envDeleteHook) != OK)
                {
                    status = ERROR;
                }
                else
                {
                    status = OK;
                }
            }
        }
        else
        {
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * envPrivateCreate - Create a private environment for task
 *
 * RETURNS: OK or ERROR
 */

STATUS envPrivateCreate(
    int taskId,
    int envSource
    )
{
    STATUS   status;
    TCB_ID   tcbId;
    char   **ppEnv;

    /* Select source */
    switch(envSource)
    {
        /* Create new environment for task */
        case -1:
            ppEnv = (char **) malloc(ENV_MAX_ENTRIES * sizeof(char *));
            if (ppEnv == NULL)
            {
                status = ERROR;
                break;
            }

            tcbId = taskTcb(taskId);
            if (tcbId == NULL)
            {
                status = ERROR;
                break;
            }

            tcbId->envTableSize   = ENV_MAX_ENTRIES;
            tcbId->nEnvVarEntries = 0;
            tcbId->ppEnviron      = ppEnv;

            status = OK;
            break;

        /* Duplicate global environment */
        case 0:
            if (envDuplicate(
                    ppGlobalEnviron,
                    globalEnvTableSize,
                    globalEnvNEntries,
                    (TCB_ID) taskIdSelf()
                    ) == ERROR)
            {
                status = ERROR;
                break;
            }

            status = OK;
            break;

        /* Duplicate current tasks environment */
        default:
            tcbId = taskTcb(taskId);
            if (tcbId == NULL)
            {
                status = ERROR;
                break;
            }

            if (envDuplicate(
                    tcbId->ppEnviron,
                    tcbId->envTableSize,
                    tcbId->nEnvVarEntries,
                    (TCB_ID) taskTcb(0)
                    ) == ERROR)
            {
                status = ERROR;
                break;
            }

            status = OK;
            break;
    }

    taskOptionsSet(taskId, TASK_OPTIONS_PRIVATE_ENV, TASK_OPTIONS_PRIVATE_ENV);

    return OK;
}

/******************************************************************************
 * envPrivateDestroy - Free a tasks environment
 *
 * RETURNS: OK or ERROR
 */

STATUS envPrivateDestroy(
    int taskId
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
        envDestroy(tcbId->ppEnviron, tcbId->nEnvVarEntries);
        tcbId->ppEnviron = NULL;

        taskOptionsSet(taskId, TASK_OPTIONS_PRIVATE_ENV, 0);
        status = OK;
    }

    return OK;
}

/******************************************************************************
 * getenv - Get environment
 *
 * RETURNS: Pointer to value string or NULL
 */

char* getenv(
    const char *name
    )
{
    char **pEnvLine;
    char  *pValue;

    /* Find entry */
    pEnvLine = envFind((char *) name, strlen(name));
    if (pEnvLine == NULL)
    {
        pValue = NULL;
    }
    else
    {
        /* Go beyound name */
        pValue = *pEnvLine + strlen(name);

        /* Eat whitespace and separator */
        while ((*pValue == ' ') || (*pValue == '\t') || (*pValue == '='))
        {
            pValue++;
        }
    }

    return pValue;
}

/******************************************************************************
 * putenv - Put environment variable
 *
 * RETURNS: OK or ERROR
 */

int putenv(
    char *string
    )
{
    TCB_ID tcbId;
    int    currEnvTableSize;
    int    currEnvNEntries;
    char **ppCurrEnv, **buf;
    char **ppEnvLine;
    char  *pChar;
    STATUS status = OK;

    /* Initialize locals */
    tcbId = taskTcb(0);
    pChar = string;

    while ((*pChar != ' ') && (*pChar != '\t') && (*pChar != '=') &&
           (*pChar != EOS))
    {
        pChar++;
    }

    /* Find entry */
    ppEnvLine = envFind(string, pChar - string);
    if (ppEnvLine == NULL)
    {
        /* If global environment */
        if (tcbId->ppEnviron == NULL)
        {
            ppCurrEnv        = ppGlobalEnviron;
            currEnvTableSize = globalEnvTableSize;
            currEnvNEntries  = globalEnvNEntries;
            ppEnvLine        = &ppGlobalEnviron[globalEnvNEntries++];
        }
        else
        {
            ppCurrEnv        = tcbId->ppEnviron;
            currEnvTableSize = tcbId->envTableSize;
            currEnvNEntries  = tcbId->nEnvVarEntries;
            ppEnvLine        = &tcbId->ppEnviron[tcbId->nEnvVarEntries++];
        }

        /* Allocate more memory if needed */
        if (currEnvTableSize == currEnvNEntries)
        {
            buf = (char **) realloc(
                                (char *) ppCurrEnv,
                                (unsigned) (currEnvTableSize +
                                    ENV_MAX_ENTRIES) * sizeof(char *)
                                );
            if (buf == NULL)
            {
                status = ERROR;
            }
            else
            {
                /* Clear */
                memset(&buf, 0, ENV_MAX_ENTRIES * sizeof(char *));

                /* Update size */
                if (tcbId->ppEnviron == NULL)
                {
                    ppGlobalEnviron     = buf;
                    globalEnvTableSize += ENV_MAX_ENTRIES;
                }
                else
                {
                    tcbId->ppEnviron     = buf;
                    tcbId->envTableSize += ENV_MAX_ENTRIES;
                }

                ppEnvLine = &buf[currEnvNEntries];
            }
        }
    }
    else
    {
        free(*ppEnvLine);
    }

    if (status == OK)
    {
        /* Allocate memory from string */
        *ppEnvLine = (char *) malloc(strlen(string) + 1);
        strcpy(*ppEnvLine, string);
    }

    return status;
}

/******************************************************************************
 * envFind - Find environment variable
 *
 * RETURNS: Pointer to environment variable or NULL
 */

LOCAL char** envFind(
    char *name,
    int   namelen
    )
{
    TCB_ID tcbId;
    int    i;
    int    nEnvEntries;
    char **envVar;
    char **ret;
    char   endChar;
    BOOL   found = FALSE;

    /* Initialize locals */
    tcbId = taskTcb(0);

    /* If no local enironment */
    if (tcbId->ppEnviron == NULL)
    {
        envVar      = ppGlobalEnviron;
        nEnvEntries = globalEnvNEntries;
    }
    else
    {
        envVar      = tcbId->ppEnviron;
        nEnvEntries = tcbId->nEnvVarEntries;
    }

    /* For all entries */
    for (i = 0; i < nEnvEntries; i++, envVar++)
    {
        if (strncmp(name, *envVar, namelen) == 0)
        {
            endChar = (*envVar)[namelen];
            if ((endChar == ' ') || (endChar == '\t') || (endChar == '=') ||
                (endChar == EOS))
            {
                found = TRUE;
                break;
            }
        }
    }

    if (found == TRUE)
    {
        ret = envVar;
    }
    else
    {
        ret = NULL;
    }

    return ret;
}

/******************************************************************************
 * envCreateHook - Environment variable create hook
 *
 * RETURNS: N/A
 */

LOCAL void envCreateHook(
    TCB_ID tcbId
    )
{
    TCB_ID currTask;
    char **ppSrcEnviron;
    int    srcEnvTableSize;
    int    srcNEnvVarEntries;

    if (tcbId->options & TASK_OPTIONS_PRIVATE_ENV)
    {
        currTask = taskTcb(0);

        /* If tasks environment */
        ppSrcEnviron = currTask->ppEnviron;
        if (ppSrcEnviron == NULL)
        {
            /* Get global environment */
            ppSrcEnviron      = ppGlobalEnviron;
            srcEnvTableSize   = globalEnvTableSize;
            srcNEnvVarEntries = globalEnvNEntries;
        }
        else
        {
            /* Get tasks environment */
            srcEnvTableSize   = currTask->envTableSize;
            srcNEnvVarEntries = currTask->nEnvVarEntries;
        }

        /* Duplicate environment */
        if (envDuplicate(
                ppSrcEnviron,
                srcEnvTableSize,
                srcNEnvVarEntries,
                tcbId
                ) == ERROR)
        {
            logMsg(
                   "evnCreateHook: couldn't create private environment!\n",
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0,
                   (ARG) 0
                   );
            taskOptionsSet((int) tcbId, TASK_OPTIONS_PRIVATE_ENV, 0);
        }
    }
    else
    {
        tcbId->ppEnviron = NULL;
    }
}

/******************************************************************************
 * envDeleteHook - Environment variable delete hook
 *
 * RETURNS: N/A
 */

LOCAL void envDeleteHook(
    TCB_ID tcbId
    )
{
    if (tcbId->options & TASK_OPTIONS_PRIVATE_ENV)
    {
        if (tcbId->ppEnviron != NULL)
        {
            envDestroy(tcbId->ppEnviron, tcbId->nEnvVarEntries);
        }
    }
}

/******************************************************************************
 * envDuplicate - Duplicate a given environment
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS envDuplicate(
    char **srcEnv,
    int    srcEnvTableSize,
    int    srcNEnvVarEntries,
    TCB_ID tcbId
    )
{
    int    i;
    char  *src;
    char  *dest;
    char **ppEnv;
    STATUS status = OK;

    /* Allocate */
    ppEnv = (char **) malloc(srcEnvTableSize * sizeof(char *));
    if (ppEnv == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Setup struct */
        tcbId->envTableSize   = srcEnvTableSize;
        tcbId->nEnvVarEntries = 0;
        tcbId->ppEnviron      = ppEnv;

        /* Copy environment */
        for (i = 0; i < srcNEnvVarEntries; i++)
        {
            src  = srcEnv[i];
            dest = (char *) malloc(strlen(src) + 1);
            if (dest == NULL)
            {
                envDestroy(ppEnv, tcbId->nEnvVarEntries);
                status = ERROR;
                break;
            }

            strcpy(dest, src);
            ppEnv[i] = dest;
            tcbId->nEnvVarEntries++;
        }
    }

    return status;
}

/******************************************************************************
 * envDestroy - Free environment entry
 *
 * RETURNS: N/A
 */

LOCAL void envDestroy(
    char **ppEnv,
    int    nEnvVarEntries
    )
{
    int i;

    for (i = 0; i < nEnvVarEntries; i++)
    {
        free(ppEnv[i]);
    }

    free(ppEnv);
}

