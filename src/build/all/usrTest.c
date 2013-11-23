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

/* useTest.o - Test suite for Real VMX */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <a.out.h>
#include <os/symLib.h>

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

static jmp_buf jbuf;

void setjmp_second(
    void
    )
{
    printf("Begin:\tsecond()\n");
    printf("Calling longjmp()\n");
    longjmp(jbuf, 1);
    printf("End:\tsecond()\n");
}

void setjmp_first(
    void
    )
{
    printf("Begin:\tfirst()\n");
    setjmp_second();
    printf("End:\tfirst()\n");
}

int setjmpTest(
    void
    )
{
    printf("Begin:\t'main()'\n");

    if (!setjmp(jbuf))
    {
        printf("Main:\tsetjmp() == 0\n");
        setjmp_first();
    }
    else
    {
        printf("Main:\tsetjmp() != 0\n");
    }

    printf("End:\t'main()'\n");

    return 0;
}

void handleKill(
    int        signo,
    siginfo_t *info,
    void      *f
    )
{
    printf("Caught signal sent by ");
    switch(info->si_code)
    {
        case SI_KILL:
            printf("kill()\n");
            break;

        case SI_QUEUE:
            printf("sigqueue()\n");
            break;

        default:
            printf("unknown\n");
            break;
    }

    exit(1);
}

int installSigkillHandler(
    void
    )
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = handleKill;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    /* Install handler */
    if (sigaction(SIGCHLD, &sa, NULL) != OK)
    {
        printf("Error - Unable to install signal handler\n");
        return ERROR;
    }

    return 0;
}

int waitSignalTest(
    void
    )
{
    if (installSigkillHandler() != 0)
    {
        return ERROR;
    }

    printf("Wating for signal SIGCHLD (%d):\n", SIGCHLD);
    for (;;)
    {
        pause();
        printf(".");
    }

    printf("Out of signal wait.\n");

    return 0;
}

int selfSignalTest(
    void
    )
{
    if (installSigkillHandler() != 0)
    {
        return ERROR;
    }

    printf("Wating for signal SIGCHLD (%d):\n", SIGCHLD);
    raise(SIGCHLD);
    printf("Out of signal wait.\n");

    return 0;
}

int sendSignalTest(
    int taskId
    )
{
    if (kill(taskId, SIGCHLD) != OK)
    {
        printf("Error sending signal.\n");
        return ERROR;
    }

    return 0;
}

int sendQSignalTest(
    int taskId,
    int i
    )
{
    union sigval value;

    value.sival_int = i;
    if (sigqueue(taskId, SIGCHLD, value) != OK)
    {
        printf("Error sending signal.\n");
        return ERROR;
    }

    return 0;
}

void usrTestInit(
    void
    )
{
    static SYMBOL symTableTest[] =
    {
        {NULL, "_setjmpTest", setjmpTest, 0, N_TEXT | N_EXT},
        {NULL, "_waitSignalTest", waitSignalTest, 0, N_TEXT | N_EXT},
        {NULL, "_selfSignalTest", selfSignalTest, 0, N_TEXT | N_EXT},
        {NULL, "_sendSignalTest", sendSignalTest, 0, N_TEXT | N_EXT},
        {NULL, "_sendQSignalTest", sendQSignalTest, 0, N_TEXT | N_EXT}
    };

    int i;

    for (i = 0; i < NELEMENTS(symTableTest); i++)
    {
        symTableAdd(sysSymTable, &symTableTest[i]);
    }
}

