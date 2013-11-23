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

void usrTestInit(
    void
    )
{
    static SYMBOL symTableTest[] =
    {
        {NULL, "_setjmpTest", setjmpTest, 0, N_TEXT | N_EXT}
    };

    int i;

    for (i = 0; i < NELEMENTS(symTableTest); i++)
    {
        symTableAdd(sysSymTable, &symTableTest[i]);
    }
}

