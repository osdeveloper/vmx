/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2010 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* unixLib.c - Unix routines library */

/* Includes */
#include <stdlib.h>
#include <errno.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <os/logLib.h>

/* Defines */

/* Locals */

/* Globals */
BOOL    panicSuspend = TRUE;
FUNCPTR panicHook    = NULL;

/* Functions */

/******************************************************************************
 * unixLibInit - Inititalize unix library
 *
 * RETURNS: OK or ERROR
 */

STATUS unixLibInit(
    void
    )
{
    return OK;
}

/******************************************************************************
 * panic - Suspend task
 *
 * RETURNS: N/A
 */

void panic(
    char *msg
    )
{

    /* If panic hook set */
    if (panicHook != NULL)
    {
        (*panicHook)(msg);
    }
    else
    {
      logMsg(
          "panic: %s\n",
          (ARG) msg,
          (ARG) 0,
          (ARG) 0,
          (ARG) 0,
          (ARG) 0,
          (ARG) 0
          );

          if ((INT_CONTEXT() == FALSE) && (panicSuspend == TRUE))
          {
              taskSuspend(0);
          }
    }
}

