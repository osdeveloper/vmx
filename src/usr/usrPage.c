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

/* usrPage.c - Page display functions for shell */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <vmx.h>
#include <usr/usrPage.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL int lineCount = 0;
LOCAL int pageSize = 22;

/* Globals */

/* Functions */

/******************************************************************************
 * usrPageInit - Initialize page display facilities
 *
 * RETURNS: N/A
 */

void usrPageInit(
    int numLines
    )
{
    /* Reset line count */
    lineCount = 0;

    /* Setup number of lines per page */
    pageSize = numLines;
}

/******************************************************************************
 * usrPageReset - Reset line counter for page
 *
 * RETURNS: N/A
 */

void usrPageReset(
    void
    )
{
    lineCount = 0;
}

/******************************************************************************
 * usrPageNumLinesSet - Set number of lines for a page
 *
 * RETURNS: N/A
 */

void usrPageNumLinesSet(
    int numLines
    )
{
    pageSize = numLines;
}

/******************************************************************************
 * usrPageCheck - Check if max page lines are reached and wait for input
 *
 * RETURNS: TRUE if continue or FALSE if quit
 */

BOOL usrPageCheck(
    void
    )
{
    char ch;
    BOOL ret = TRUE;

    /* Increase line count */
    lineCount++;

    /* If page line number exceeded */
    if (lineCount >= pageSize)
    {
        /* Reset line count */
        lineCount = 0;

        printf("Type <CR> to continue, Q<CR> to stop: ");

        /* Get answer */
        ch = getchar();
        if ((ch == 'q') || (ch == 'Q'))
        {
            usrPageReset();
            ret = FALSE;
        }
    }

    return ret;
}

