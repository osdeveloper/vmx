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

/* usrLib.c - User functions used by shell */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <a.out.h>
#include <vmx.h>
#include <arch/regs.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/taskShow.h>
#include <vmx/kernelLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/classLib.h>
#include <os/errnoLib.h>
#include <os/iosLib.h>
#include <os/iosShow.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <ostool/moduleLib.h>
#include <ostool/loadLib.h>
#include <usr/usrLib.h>

/* Defines */
#define MAX_BYTES_PER_LINE               16
#define MAXLINE                          80
#define MAX_DISP_TASKS                  500

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

IMPORT int sysClockRateGet(
    void
    );

/* Gloabals */
int spTaskPriority    = 100;
int spTaskOptions     = 0;
int spTaskStackSize   = 20000;
BOOL ldCommonMatchAll = TRUE;

/* Locals */
LOCAL BOOL usrLibInstalled = FALSE;

LOCAL void printStackSummary(
    TASK_DESC *pTd
    );

/******************************************************************************
 * usrLibInit - Initialize user library
 *
 * RETURNS: OK
 */

STATUS usrLibInit(
    void
    )
{
    STATUS status;
    int i;

    /* Check if already installed */
    if (usrLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Mark as installed */
        usrLibInstalled = TRUE;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * taskIdFigure - Translate task name or id to task id
 *
 * RETURNS: Task id
 */

int taskIdFigure(
    int taskNameOrId
    )
{
    char name[10];
    int taskId;

    /* If it is zero */
    if (taskNameOrId == 0)
    {
        taskId = taskIdDefault(0);
    }
    else if (taskIdVerify(taskNameOrId) == OK)
    {
        taskId = taskNameOrId;
    }
    else
    {
        /* Copy task name string */
        strcpy(name, (char *) taskNameOrId);
        taskId = taskNameToId(name);
    }

    return taskId;
}

/******************************************************************************
 * show - Call object show method
 *
 * RETURNS: N/A
 */

void show(
    int objId,
    int level
    )
{
    /* Zero will default to current task */
    if (objId == 0)
    {
        objId = (int) taskIdCurrent;
    }

    /* If object show fails */
    if (objShow(taskClassId, (OBJ_ID) objId, level) != OK)
    {
        if (errnoGet() == S_classLib_OBJ_NO_METHOD)
        {
            fprintf(
                stderr,
                "Show routine of this object not configured into system.\n"
                );
        }
        else
        {
            fprintf(stderr, "Object not found.\n");
        }
    }
}

/******************************************************************************
 * help - Show help
 *
 * RETURNS: N/A
 */

void help(
    void
    )
{
    static char *helpMessages[] =
    {
        "help                           Print this list",
        "ioHelp                         Print I/O utilities help info",
        "i         [task]               Summary of tasks' TCBs",
        "ti        task                 Complete info on TCB for task",
        "sp        adr,args...          Spawn a task, pri=100, opt=0, stk=20000",
        "taskSpawn name,pri,opt,stk,adr,args... Spawn a task",
        "td        task                 Delete a task",
        "ts        task                 Suspend a task",
        "tr        task                 Resume a task",
        "d         [adr,[nunits[,width]]] Display memory",
        "m         adr[,width]          Modify memory",
        "pc        [task]               Return task's program counter",
        "devs                           List devices",
        "ld        [syms[,noAbort][,\"name\"]] Load stdin, or file into memory",
        "lkup      [\"substr\"]         List symbols in system symbol table",
        "checkStack  [task]             List task stack sizes and usage",
        "period    secs,adr,args... Spawn task to call function periodically",
        "repeat    n,adr,args...    Spawn task to call function n times (0=forever)",
        "version                        Print Real VMX version info",
        ""
    };

    int i;

    printf("\n");
    for (i = 0; i < NELEMENTS(helpMessages); i++)
    {
        printf(helpMessages[i]);
        printf("\n");
    }

    printf("\n");
}

/******************************************************************************
 * i - Show task summary
 *
 * RETURNS: N/A
 */

void i(
    int taskNameOrId
    )
{
    int taskId;

    /* If task id is zero show summary */
    if (taskNameOrId == 0)
    {
        show(taskIdSelf(), 2);
    }
    else
    {
        taskId = taskIdFigure(taskNameOrId);
        if (taskId == ERROR)
        {
            fprintf(stderr, "Task not found.\n");
        }
        else
        {
            show(taskId, 0);
        }
    }
}

/******************************************************************************
 * ti - Show task info
 *
 * RETURNS: N/A
 */

void ti(
    int taskNameOrId
    )
{
    int taskId;

    /* Get task id */
    taskId = taskIdFigure(taskNameOrId);
    if ((taskId == ERROR) || (taskIdVerify(taskId) != OK))
    {
        fprintf(stderr, "Task not found.\n");
    }
    else
    {
        /* Set as default task id */
        taskId = taskIdDefault(taskId);

        show(taskId, 1);
    }
}

/******************************************************************************
 * sp - Spawn a task
 *
 * RETURNS: Task id or ERROR
 */

int sp(
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8,
    ARG arg9
    )
{
    int taskId;

    /* If task id is zero task id */
    if (func == NULL)
    {
        fprintf(stderr, "sorry, won't spawn task at PC = 0.\n");
    }
    else
    {
        /* Spawn task */
        taskId = taskSpawn(
                     NULL,
                     spTaskPriority,
                     spTaskOptions,
                     spTaskStackSize,
                     (FUNCPTR) func,
                     arg1,
                     arg2,
                     arg3,
                     arg4,
                     arg5,
                     arg6,
                     arg7,
                     arg8,
                     arg9,
                     (ARG) 0
                     );
        if (taskId == ERROR)
        {
            fprintf(stderr, "not able to spawn task.\n");
        }
        else
        {
            printf(
                "task spawned: id = %#x, name = %s\n",
                taskId,
                taskName(taskId)
                );
        }
    }

    return taskId;
}

/******************************************************************************
 * ts - Suspend task
 *
 * RETURNS: N/A
 */

void ts(
    int taskNameOrId
    )
{
    int taskId;

    /* Get task id */
    taskId = taskIdFigure(taskNameOrId);
    if (taskId == ERROR)
    {
        fprintf(stderr, "Task not found.\n");
    }
    else
    {
        /* If task id self */
        if (taskId == 0 || taskId == taskIdSelf())
        {
            fprintf(stderr, "sorry, the shell can't suspend itself.\n");
        }
        else
        {
            /* If action fails */
            if (taskSuspend(taskId) != OK)
            {
                fprintf(stderr, "unable to suspend task.\n");
            }
        }
    }
}

/******************************************************************************
 * tr - Resume task
 *
 * RETURNS: N/A
 */

void tr(
    int taskNameOrId
    )
{
    int taskId;

    /* Get task id */
    taskId = taskIdFigure(taskNameOrId);
    if (taskId == ERROR)
    {
        fprintf(stderr, "Task not found.\n");
    }
    else
    {
        /* If action fails */
        if (taskResume(taskId) != OK)
        {
            fprintf(stderr, "unable to resume task.\n");
        }
    }
}

/******************************************************************************
 * td - Delete task
 *
 * RETURNS: N/A
 */

void td(
    int taskNameOrId
    )
{
    int taskId;

    /* Get task id */
    taskId = taskIdFigure(taskNameOrId);
    if (taskId == ERROR)
    {
        fprintf(stderr, "Task not found.\n");
    }
    else
    {
        /* If task id self */
        if (taskId == 0 || taskId == taskIdSelf())
        {
            fprintf(stderr, "sorry, the task can't delete itself.\n");
        }
        else
        {
            /* If action fails */
            if (taskDelete(taskId) != OK)
            {
                fprintf(stderr, "unable to delete task.\n");
            }
        }
    }
}

/******************************************************************************
 * d - Display memory
 *
 * RETURNS: N/A
 */

void d(
    void *addr,
    int n,
    int size
    )
{
    static void *lastAddr = (void *) 0;
    static int lastSize   = 2;
    static int lastN      = 0x80;

    int i, count, itm;
    char line[MAX_BYTES_PER_LINE + 1];
    unsigned char *pByte;

    /* Terminate line */
    line[MAX_BYTES_PER_LINE] = EOS;

    /* If address non-zero */
    if (addr != (void *) 0)
    {
        lastAddr = addr;
    }

    /* if non-zero size */
    if (size != 0)
    {
        if (size != 1 &&
            size != 2 &&
#ifdef INCLUDE_LONG_LONG
            size != 4 &&
            size != 8)
#else
            size != 4)
#endif /* INCLUDE_LONG_LONG */
        {
            size = 1;
        }

        /* Set new size */
        lastSize = size;
    }

    /* If non-zero number of units */
    if (n != 0)
    {
        lastN = n;
    }

    /* Set start address /*
    lastAddr = (void *) ( (int) lastAddr & ~(lastSize - 1) );

    /* Insert spaces at start of line */
    memset(line, '.', MAX_BYTES_PER_LINE);

    /* Print address */
    printf("%08x:  ", (int) lastAddr & ~0xf);

    /* For all items */
    for (itm = 0; itm < ( (int) lastAddr & 0xf) / lastSize; itm++)
    {
        printf("%*s ", 2 * lastSize, " ");
        memset(&line[itm * lastSize], ' ', 2 * lastSize);
    }

    /* Store numner of items to print */
    count = lastN;

    /* While units left to print */
    while((count--) > 0)
    {
        /* If end of line */
        if (itm == MAX_BYTES_PER_LINE / lastSize)
        {
            printf("  *%16s*\n%08x:  ", line, (int) lastAddr);
            memset(line, '.', MAX_BYTES_PER_LINE);
            itm = 0;
        }

        /* Select size */
        switch(lastSize)
        {
            case 1:
                printf("%02x", *(unsigned char *) lastAddr);
                break;

            case 2:
                printf("%04x", *(unsigned short *) lastAddr);
                break;

            case 4:
                printf("%08lx", *(unsigned long *) lastAddr);
                break;

#ifdef INCLUDE_LONG_LONG
            case 8:
                printf("%016lx", *(unsigned long long *) lastAddr);
                break;
#endif /* INCLUDE_LONG_LONG */

            default:
                printf("%02x", *(unsigned char *) lastAddr);
                break;
        }

        /* Add space between units */
        printf(" ");

        /* Set line buffer */
        pByte = (unsigned char *) lastAddr;

        /* For byte size */
        for (i = 0; i < lastSize; i++)
        {
            /* If character is printable */
            if (*pByte == ' ' || (__isascii(*pByte) && __isprint(*pByte)))
            {
                line[itm * lastSize + i] = *pByte;
            }

            /* Advance */
            pByte++;
        }

        /* Advance */
        lastAddr = (void *) ( (int) lastAddr + lastSize );
        itm++;
    }

    /* Print end of last line */
    for (; itm < MAX_BYTES_PER_LINE / lastSize; itm++)
    {
        printf("%*s ", 2 * lastSize, " ");
    }

    printf("  *%16s*\n", line);
}

/******************************************************************************
 * m - Modify memory
 *
 * RETURNS: N/A
 */

void m(
    void *addr,
    int size
    )
{
    static void *lastAddr = (void *) 0;
    static int lastSize = 2;
    char line[MAXLINE + 1];
    unsigned long value;

#ifdef INCLUDE_LONG_LONG
    long long lValue;
#endif /* INCLUDE_LONG_LONG */

    /* If address non-zero */
    if (addr != (void *) 0)
    {
        lastAddr = addr;
    }

    /* if non-zero size */
    if (size != 0)
    {
        if (size != 1 &&
            size != 2 &&
#ifdef INCLUDE_LONG_LONG
            size != 4 &&
            size != 8)
#else
            size != 4)
#endif /* INCLUDE_LONG_LONG */
        {
            size = 1;
        }

        /* Set new size */
        lastSize = size;
    }

    /* Set start address /*
    lastAddr = (void *) ( (int) lastAddr & ~(lastSize - 1) );

    /* While loop */
    while (1)
    {
        /* Select size */
        switch(lastSize)
        {
            case 1:
                printf(
                    "%08x:  %02x-",
                    (int) lastAddr,
                    *(unsigned char *) lastAddr
                    );
                break;

            case 2:
                printf(
                    "%08x:  %04x-",
                    (int) lastAddr,
                    *(unsigned short *) lastAddr
                    );
                break;

            case 4:
                printf(
                    "%08x:  %08lx-",
                    (int) lastAddr,
                    *(unsigned long *) lastAddr
                    );
                break;

#ifdef INCLUDE_LONG_LONG
            case 4:
                printf(
                    "%016x:  %08lx-",
                    (int) lastAddr,
                    *(unsigned long long *) lastAddr
                    );
                break;
#endif /* INCLUDE_LONG_LONG */

            default:
                printf(
                    "%08x:  %02x-",
                    (int) lastAddr,
                    *(unsigned char *) lastAddr
                    );
                break;
        }

        /* Get input */
        fgets(line, MAXLINE, stdin);
        if (strlen(line) <= 1)
        {
            break;
        }

        /* Select size */
        switch(lastSize)
        {
            case 1:
                sscanf(line, "%x", &value);
                *(unsigned char *) lastAddr = (unsigned char) value;
                break;

            case 2:
                sscanf(line, "%x", &value);
                *(unsigned short *) lastAddr = (unsigned short) value;
                break;

            case 4:
                sscanf(line, "%lx", &value);
                *(unsigned long *) lastAddr = (unsigned long) value;
                break;

#ifdef INCLUDE_LONG_LONG
            case 8:
                sscanf(line, "%lx", &lValue);
                *(unsigned long long *) lastAddr = (unsigned long long) lValue;
                break;
#endif /* INCLUDE_LONG_LONG */

            default:
                sscanf(line, "%x", &value);
                *(unsigned char *) lastAddr = (unsigned char) value;
                break;
        }

        /* Advance */
        lastAddr = (void *) ( (int) lastAddr + lastSize );
    }
}

/******************************************************************************
 * pc - Get task program counter
 *
 * RETURNS: Task program counter or ERROR
 */

int pc(
    int taskNameOrId
    )
{
    int ret;
    REG_SET regSet;
    int taskId;

    /* Get task id */
    taskId = taskIdFigure(taskNameOrId);
    if (taskId == ERROR)
    {
        ret = ERROR;
    }
    else
    {
        /* Set as default task */
        taskId = taskIdDefault(taskId);

        /* Get registers */
        if (taskRegsGet(taskId, &regSet) != OK)
        {
            ret = ERROR;
        }
        else
        {
            ret = (int) regSet.pc;
        }
    }

    return ret;
}

/******************************************************************************
 * devs - Show a list of known devices
 *
 * RETURNS: N/A
 */

void devs(
    void
    )
{
    iosDevShow();
}

/******************************************************************************
 * ld - Load object module into memory
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID ld(
    int symFlags,
    BOOL noAbort,
    char *name
    )
{
    int loadFlag;
    MODULE_ID modId = NULL;
    int       fd    = ERROR;

    /* If filename given */
    if (name != NULL)
    {
        fd = open(name, O_RDONLY, 0);
        if (fd == ERROR)
        {
            fprintf(stderr, "ld error: unable to open \"%s\"\n", name);
        }
    }
    else
    {
        fd = STDIN_FILENO;
    }

    if (fd != ERROR)
    {
        /* Select load flags */
        switch (symFlags)
        {
            case NO_SYMBOLS:
                loadFlag = LOAD_NO_SYMBOLS;
                break;

            case GLOBAL_SYMBOLS:
                loadFlag = LOAD_GLOBAL_SYMBOLS;
                break;

            case ALL_SYMBOLS:
                loadFlag = LOAD_ALL_SYMBOLS;
                break;

            default:
                loadFlag = LOAD_ALL_SYMBOLS;
                break;
        }

        /* If match all symbols */
        if (ldCommonMatchAll == TRUE)
        {
            loadFlag = (loadFlag | LOAD_COMMON_MATCH_ALL);
        }

        /* Load module */
        modId = loadModule(fd, loadFlag);

        /* Close file if opened */
        if (name != NULL)
        {
            close(fd);
        }

        /* If error */
        if (modId == NULL)
        {
            /* Select error */
            switch (errnoGet())
            {
                case S_loadLib_TOO_MANY_SYMBOLS:
                    fprintf(stderr, "ld error: to many symbols.\n");
                    break;

                case S_symLib_SYMBOL_NOT_FOUND:
                    fprintf(
                        stderr, "ld error: Module contains undefined symbol(s) "
                        "and may be unusable.\n"
                        );
                    break;

                default:
                    fprintf(
                        stderr,
                        "ld error: error loading file (errno = %#x).\n",
                        errnoGet()
                        );
                    break;
            }
        }
    }

    return modId;
}

/******************************************************************************
 * lkup - List symbols in system symbol table
 *
 * RETURNS: N/A
 */

void lkup(
    char *str
    )
{
    show((int) sysSymTable, (int) str);
}

/******************************************************************************
 * periodicRun - Call a function periodically
 *
 * RETURNS: N/A
 */

void periodRun(
    int secs,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    )
{
    /* While loop */
    while (1)
    {
        /* Call function */
        (*func)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

        /* Sleep */
        taskDelay(secs * sysClockRateGet());
    }
}

/******************************************************************************
 * checkStack - Print stack info
 *
 * RETURNS: N/A
 */

void checkStack(
    int taskNameOrId
    )
{
    static char checkStackHeader [] =
        "  NAME        ENTRY       TID    SIZE   CUR  HIGH  MARGIN\n"
        "---------- ------------ -------- ----- ----- ----- ------\n";
    int i, numTasks;
    char *pIntStackHigh;
    int taskId;
    TASK_DESC td;
    int idList[MAX_DISP_TASKS];
    char s[20];

    /* If task id is zero */
    if (taskNameOrId == 0)
    {
        /* Print stack info */
        printf(checkStackHeader);

        /* Get all tasks */
        numTasks = taskIdListGet(idList, NELEMENTS(idList));
        taskIdListSort(idList, numTasks);

        /* For all tasks */
        for (i = 0; i < numTasks; i++)
        {
            if (taskInfoGet(idList[i], &td) == OK)
            {
                printStackSummary(&td);
            }
        }

        /* Calculate interrupt stack usage */
#if (_STACK_DIR == _STACK_GROWS_DOWN)
        for (pIntStackHigh = pExcStackEnd;
             *(unsigned char *) pIntStackHigh  == 0xee;
             pIntStackHigh++);
#else /* _STACK_GROWS_UP */
        for (pIntStackHigh = pExcStackEnd - 1;
             *(unsigned char *) pIntStackHigh == 0xee;
             pIntStackHigh--);
#endif /* _STACK_DIR */

        /* Check for overflow */
        if ((pIntStackHigh == pExcStackEnd) &&
            (pExcStackEnd != pExcStackBase != 0))
        {
            strcpy(s, "OVERFLOW");
        }
        else
        {
            strcpy(s, "");
        }

        /* Print interrupt stack usage */
        printf("%-11.11s%-12.12s", "INTERRUPT", "");
        printf(
            " %8s %5d %5d %5d %6d %s\n",
           "",
           (int) ((pExcStackEnd - pExcStackBase) * _STACK_DIR),
           0,
           (int) ((pIntStackHigh - pExcStackBase) * _STACK_DIR),
           (int) ((pExcStackEnd - pIntStackHigh) * _STACK_DIR),
           s
           );
    }
    else
    {
        /* Get task id */
        taskId = taskIdFigure(taskNameOrId);
        if (taskId == ERROR)
        {
            fprintf(stderr, "Task not found.\n");
        }
        else
        {
            /* Get task info */
            if (taskInfoGet(taskId, &td) != OK)
            {
                fprintf(stderr, "Unable to get task info.\n");
            }
            else
            {
                /* Print stack info */
                printf(checkStackHeader);
                printStackSummary(&td);
            }
        }
    }
}

/******************************************************************************
 * period - Spawn a task that calls a function periodically
 *
 * RETURNS: Task id or ERROR
 */

int period(
    int secs,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    )
{
    return sp(
               (FUNCPTR) periodRun,
               (ARG) secs,
               (ARG) func,
               arg1,
               arg2,
               arg3,
               arg4,
               arg5,
               arg6,
               arg7
               );
}

/******************************************************************************
 * repeatRun - Call a function a specified number of times
 *
 * RETURNS: N/A
 */

void repeatRun(
    int n,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    )
{
    BOOL endless;

    if (n == 0)
    {
        endless = TRUE;
    }
    else
    {
        endless = FALSE;
    }

    /* While endless n greater than zero */
    while ((endless == TRUE) || (--n >= 0))
    {
        (*func) (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }
}

/******************************************************************************
 * repeat - Spawn a task that calls a function a specified number of times
 *
 * RETURNS: Task id
 */

int repeat(
    int n,
    FUNCPTR func,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5,
    ARG arg6,
    ARG arg7,
    ARG arg8
    )
{

    return sp(
               (FUNCPTR) repeatRun,
               (ARG) n,
               (ARG) func,
               arg1,
               arg2,
               arg3,
               arg4,
               arg5,
               arg6,
               arg7
               );
}

/******************************************************************************
 * version - Printf kernel version
 *
 * RETURNS: N/A
 */

void version(
    void
    )
{
    printf("Kernel version: %s.\n", kernelVersion());
}

/******************************************************************************
 * printLogo - Print the Real VMX logo
 *
 * RETURNS: N/A
 */

void printLogo(
    void
    )
{
    int i;
    static char *logo[] =
    {
        "+-++-++-++-+   _           _  _           _  _           _ ",
        "|R||e||a||l|  (_)         (_)(_) _     _ (_)(_)_       _(_)",
        "+-++-++-++-+  (_)         (_)(_)(_)   (_)(_)  (_)_   _(_)  ",
        "              (_)_       _(_)(_) (_)_(_) (_)    (_)_(_)    ",
        "                (_)     (_)  (_)   (_)   (_)     _(_)_     ",
        "                 (_)   (_)   (_)         (_)   _(_) (_)_   ",
        "                  (_)_(_)    (_)         (_) _(_)     (_)_ ",
        "                    (_)      (_)         (_)(_)         (_)"
    };
                                       

    for (i = 0; i < NELEMENTS(logo); i++)
    {
        printf("%s\n", logo[i]);
    }

    printf("KERNEL: %s.\n", kernelVersion());
    printf("This system is released under GNU general public license.\n");

    printf("\n");
}

/******************************************************************************
 * printStackSummary - Print stack summary from task info
 *
 * RETURNS: N/A
 */

LOCAL void printStackSummary(
    TASK_DESC *pTd
    )
{
    char s[18];
    char *name;
    FUNCPTR func;
    ARG value;
    SYMBOL_ID symId;

    /* Get task entry */
    func = pTd->td_entry;

    /* If symbol found */
    if (symFindSymbol(
            sysSymTable,
            NULL,
            func,
            N_TEXT | N_EXT, N_TEXT | N_EXT,
            &symId
            ) == OK)
    {
        symNameGet(symId, &name);
        symValueGet(symId, &value);
    }

    /* Print task name */
    printf("%-11.11s", pTd->td_name);

    /* Print symbol or function address */
    if (func == (FUNCPTR) value)
    {
        printf("%-12.12s", name);
    }
    else
    {
        printf("%-12x", (int) func);
    }

    /* Check for overflow */
    if (pTd->td_stackMargin <= 0)
    {
        strcpy(s, "OVERFLOW");
    }
    else
    {
        strcpy(s, "");
    }

    /* Print stack sizes */
    printf(
        " %8x %5d %5d ",
        pTd->td_id,
        pTd->td_stackSize,
        pTd->td_stackCurrent
        );

    /* Calculate stack usage */
    if (pTd->td_options & TASK_OPTIONS_NO_STACK_FILL)
    {
        printf("%5s %6s NO_STACK_FILL\n", "???", "???");
    }
    else
    {
        printf("%5d %6d %s\n", pTd->td_stackHigh, pTd->td_stackMargin, s);
    }
}

