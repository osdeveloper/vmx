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

/* shellLib.c - Shell library */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <os/ioLib.h>
#include <os/excLib.h>
#include <ostool/shellLib.h>

/* Defines */
#define MAX_PROMPT_LEN          80
#define MAX_SHELL_LINE          128

/* Imports */
IMPORT int yyparse(
    void
    );

/* Globals */
int   shellTaskId;
char *shellTaskName      = "tShell";
int   shellTaskPriority  = 1;
int   shellTaskOptions   = TASK_OPTIONS_UNBREAKABLE;
int   shellTaskStackSize = 20000;
int   redirInFd          = -1;
int   redirOutFd         = -1;

/* Locals */
LOCAL BOOL    shellLibInstalled            = FALSE;
LOCAL char    promptString[MAX_PROMPT_LEN] = "-> ";
LOCAL BOOL    shellAbort                   = FALSE;
LOCAL BOOL    startRemoteSession           = FALSE;
LOCAL FUNCPTR loginFunc                    = NULL;
LOCAL int     loginValue                   = 0;
LOCAL FUNCPTR logoutFunc                   = NULL;
LOCAL int     logoutValue                  = 0;
LOCAL BOOL    shellLocked                  = FALSE;
LOCAL BOOL    shellExecuting               = FALSE;
LOCAL int     origFds[3];

LOCAL STATUS getRedir(
    char *line,
    int *pInFd,
    int *pOutFd
    );

LOCAL void stringTrimRight(
    char *str
    );

LOCAL STATUS getSingleRedir(
    char *line,
    int *pInFd,
    int *pOutFd
    );

LOCAL STATUS execShell(
    BOOL interactive
    );

/******************************************************************************
 * shellLibInit - Initialize shell library
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLibInit(
    int stackSize,
    ARG arg
    )
{
    STATUS status;

    /* Check if already installed */
    if (shellLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Not a remote shell */
        startRemoteSession = FALSE;

        /* Mark as installed */
        shellLibInstalled = TRUE;

        /* If shell spawn fails */
        if (shellSpawn(stackSize, arg) != OK)
        {
            status = ERROR;
        }
        else
        {
            shellLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * shellSpawn - Respawn shell task
 *
 * RETURNS: OK or ERROR
 */

STATUS shellSpawn(
    int stackSize,
    ARG arg
    )
{
    STATUS status;
    BOOL spawn;

    /* If shell library already installed */
    if (shellLibInstalled == TRUE)
    {
        spawn = TRUE;

        /* Check if shell task already running */
        if (taskNameToId(shellTaskName) != ERROR)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }
    else
    {
        spawn = FALSE;

        if (shellLibInit(stackSize, arg) != OK)
        {
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    if ((spawn != FALSE) && (status != ERROR))
    {
        /* Check stack size */
        if (stackSize != 0)
        {
            shellTaskStackSize = stackSize;
        }

        /* Spawn shell task */
        shellTaskId = taskSpawn(
                          shellTaskName,
                          shellTaskPriority,
                          shellTaskOptions,
                          shellTaskStackSize,
                          (FUNCPTR) shell,
                          arg,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0,
                          (ARG) 0
        );
        if (shellTaskId == 0)
        {
            status = ERROR;
        }
    }

    return status;
}

/******************************************************************************
 * shellLoginInstall - Install shell login function
 *
 * RETURNS: N/A
 */

void shellLoginInstall(
    FUNCPTR func,
    int value
    )
{
    loginFunc  = func;
    loginValue = value;
}

/******************************************************************************
 * shellLogin - Login using user login function
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLogin(
    int fd
    )
{
    STATUS status;

    /* If login function set */
    if (loginFunc != NULL)
    {
        /* Set task filedescriptors */
        ioTaskStdSet(0, STDIN_FILENO, fd);
        ioTaskStdSet(0, STDOUT_FILENO, fd);

        /* Call user login function */
        status = (*loginFunc)(loginValue);
        if (status == OK)
        {
            printf("\n\n");
        }
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * shellLogoutInstall - Install shell logout function
 *
 * RETURNS: N/A
 */

void shellLogoutInstall(
    FUNCPTR func,
    int value
    )
{
    logoutFunc  = func;
    logoutValue = value;
}

/******************************************************************************
 * shellLogout - Logout using user logout function
 *
 * RETURNS: OK or ERROR
 */

STATUS shellLogout(
    void
    )
{
    STATUS status;

    /* Unlock shell */
    shellLock(FALSE);

    /* If logout function set */
    if (logoutFunc != NULL)
    {
        status = (*logoutFunc)(logoutValue);
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * shellLock - Lock/unlock shell
 *
 * RETURNS: TRUE if mode changed else FALSE
 */

BOOL shellLock(
    BOOL lock
    )
{
    BOOL ret;

    /* If already locked/unlocked */
    if (lock == shellLocked)
    {
        ret = FALSE;
    }
    else
    {
        shellLocked = lock;
        ret = TRUE;
    }

    return ret;
}

/******************************************************************************
 * shell - Shell task
 *
 * RETURNS: N/A
 */

void shell(
    BOOL interactive
    )
{
    BOOL startShell = TRUE;

    /* If shell is not executing */
    if (shellExecuting != TRUE)
    {
        /* If interactive shell */
        if (interactive != FALSE)
        {
            /* Store original standard filedescriptors */
            origFds[STDIN_FILENO]  = ioGlobalStdGet(STDIN_FILENO);
            origFds[STDOUT_FILENO] = ioGlobalStdGet(STDOUT_FILENO);
            origFds[STDERR_FILENO] = ioGlobalStdGet(STDERR_FILENO);

            /* Unlock shell */
            shellLock(FALSE);
        }
        else
        {
            /* Lock for possible remote logins */
            shellLock(TRUE);
        }

        /* Mark shell as executing */
        shellExecuting = TRUE;
    }
    else
    {
        /* If interactive shell */
        if (interactive != FALSE)
        {
            /* Store original standard filedescriptors */
            origFds[STDIN_FILENO]  = ioGlobalStdGet(STDIN_FILENO);
            origFds[STDOUT_FILENO] = ioGlobalStdGet(STDOUT_FILENO);
            origFds[STDERR_FILENO] = ioGlobalStdGet(STDERR_FILENO);

            printf("\n");
        }
        else
        {
            startShell = FALSE;
        }
    }

    if (startShell != FALSE)
    {
        /* Make sure input is setup as a terminal */
        ioctl(STDIN_FILENO, FIOOPTIONS, OPT_TERMINAL);

        /* Execute shell */
        execShell(interactive);
    }

    /* When we arrive here shell has ended */
    shellExecuting = FALSE;
}

/******************************************************************************
 * execute - Execute shell line
 *
 * RETURNS: N/A
 */

STATUS execute(
    char *line
    )
{
    STATUS status;
    int oldInFd, oldOutFd;
    int newInFd, newOutFd;

    /* Store file descriptors */
    oldInFd  = ioGlobalStdGet(STDIN_FILENO);
    oldOutFd = ioGlobalStdGet(STDOUT_FILENO);

    /* Get redirection */
    if (getRedir (line, &newInFd, &newOutFd) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* If empty line */
        if (*line == EOS)
        {
            status = execShell(FALSE);

            /* Restore old filedescriptors */
            ioGlobalStdSet(STDIN_FILENO, oldInFd);
            ioGlobalStdSet(STDOUT_FILENO, oldOutFd);
        }
        else
        {
            /* Store redirect file descriptos */
            redirInFd  = newInFd;
            redirOutFd = newOutFd;

            /* Parse line */
            yystart(line);
            if (yyparse() == 0)
            {
                status = OK;
            }
            else
            {
                status = ERROR;
            }
        }

        /* If in redirected */
        if (newInFd >= 0)
        {
            close(newInFd);
        }

        /* If out redericted */
        if (newOutFd >= 0)
        {
            if (close(newOutFd) == ERROR)
            {
                fprintf(stderr, "can't close output.\n");
            }
        }
    }

    return status;
}

/******************************************************************************
 * getRedir - Get I/O redirection
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS getRedir(
    char *line,
    int *pInFd,
    int *pOutFd
    )
{
    STATUS status;

    /* Reset file descriptor pointers */
    *pInFd  = -1;
    *pOutFd = -1;

    /* If in or out redirection fails */
    if ((getSingleRedir(line, pInFd, pOutFd) != OK) ||
        (getSingleRedir(line, pInFd, pOutFd) != OK))
    {
        if (*pInFd >= 0)
        {
            close(*pInFd);
        }

        if (*pOutFd >= 0)
        {
            close(*pOutFd);
        }

        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * getSingleRedir - Get a single redirection character
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS getSingleRedir(
    char *line,
    int *pInFd,
    int *pOutFd
    )
{
    int len;
    char *pLine;
    char *name;
    STATUS status = OK;

    /* Get line lendth */
    len = strlen(line);

    /* If non-zero line length */
    if (len != 0)
    {
        /* Goto end of line */
        pLine = line + len - 1;

        /* Get last word */
        while ((pLine > line) && (*pLine == ' '))
        {
            pLine--;
        }

        /* Terminate line here */
        *(pLine + 1) = EOS;

        /* Goto start of word */
        while ((pLine > line) &&
               (*pLine != ' ') &&
               (*pLine != '<') && (*pLine != '>') &&
               (*pLine != ')') && (*pLine != ';') &&
               (*pLine != '"'))
        {
            pLine--;
        }

        /* Name is here */
        name = pLine + 1;

        /* Goto previous non blank */
        while ((pLine > line) && (*pLine == ' '))
        {
            pLine--;
        }

        /* If this is an output redirection */
        if ((*pLine == '>') && (*(pLine - 1) != '>'))
        {
            /* If output already open */
            if (*pOutFd >= 0)
            {
                fprintf(stderr, "ambiguous output redirect.\n");
                status = ERROR;
            }
            else
            {
                /* First try to open file */
                *pOutFd = open(name, O_RDONLY, 0);
                if (*pOutFd < 0)
                {
                    /* Try to creat file instead */
                    *pOutFd = creat(name, O_WRONLY);
                    if (*pOutFd < 0)
                    {
                        fprintf(stderr, "can't create output '%s'\n", name);
                        status = ERROR;
                    }
                }
            }

            if (status != ERROR)
            {
                /* Remove redirection */
                *pLine = EOS;
            }
        }
        else if ((*pLine == '<') && ((pLine == line) || ((*pLine - 1) != '<')))
        {
            /* If input already open */
            if (*pInFd >= 0)
            {
                fprintf(stderr, "ambigious input redirect.\n");
                status = ERROR;
            }
            else
            {
                *pInFd = open(name, O_RDONLY, 0);
                if (*pInFd < 0)
                {
                    fprintf(stderr, "can't open input '%s'\n", name);
                    status = ERROR;
                }
            }

            if (status != ERROR)
            {
                /* Remove redirection */
                *pLine = EOS;
            }
        }
    }

    return status;
}

/******************************************************************************
 * stringTrimRight - Remove trailing space from a string
 *
 * RETURNS: N/A
 */

LOCAL void stringTrimRight(
    char *str
    )
{
    char *strPos;
    BOOL terminated = FALSE;

    /* Initialize locals */
    strPos = str + strlen(str) - 1;

    /* While space left to trim */
    while (strPos > str)
    {
        if (__isspace(*strPos))
        {
            strPos--;
        }
        else
        {
            break;
        }
    }

    /* If position is at start of string */
    if (strPos == str)
    {
        /* If space */
        if (__isspace(*strPos))
        {
            *strPos = EOS;
            terminated = TRUE;
        }
    }

    if (terminated != TRUE)
    {
        *(strPos + 1) = EOS;
    }
}

/******************************************************************************
 * execShell - Execute shell commands
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS execShell(
    BOOL interactive
    )
{
    STATUS status;
    char line[MAX_SHELL_LINE + 1];
    int i, nChars;

    /* If remote shell */
    if (startRemoteSession == TRUE)
    {
        /* Will be started */
        startRemoteSession = FALSE;
    }

    /* Reset abort flag */
    shellAbort = FALSE;

    /* While loop */
    while (1)
    {
        /* If interactive shell */
        if (interactive != FALSE)
        {
            /* If unable to get stdin */
            if (ioGlobalStdGet(STDIN_FILENO) == ERROR)
            {
                taskDelay(1);
                continue;
            }

            /* Print prompt */
            printf("%s", promptString);

            /* Read input string */
            fgets(line, MAX_SHELL_LINE, stdin);
            nChars = strlen(line);

            /* If nothing read */
            if (nChars <= 0)
            {
                /* Restart shell */
                excJobAdd(
                    (VOIDFUNCPTR) taskRestart,
                    (ARG) shellTaskId,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0,
                    (ARG) 0
                    );
            }
        }

        /* Terminate input string */
        line[MAX_SHELL_LINE] = EOS;

        /* Eat blanks */
        for (i = 0; __isspace(line[i]); i++);

        /* If line is not a comment */
        if (line[i] != '#' && line[i] != EOS)
        {
            /* Eat blanks */
            stringTrimRight(&line[i]);
            if (line[i] == EOS)
            {
                continue;
            }

            /* If builtin command */
            if (strcmp(&line[i], "logout") == 0)
            {
                sprintf(line, "logout");
                i = 0;
            }

            /* Execute shell command */
            status = execute(&line[i]);
            if ((status != OK) && (interactive == FALSE))
            {
                status = ERROR;
                break;
            }

            /* If shell abort requested */
            if (shellAbort == TRUE)
            {
                /* If not interactive shell */
                if (interactive == FALSE)
                {
                    status = ERROR;
                    break;
                }
                else
                {
                    shellAbort = FALSE;
                }
            }
        }
    }

    return status;
}

