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
#include <util/listLib.h>
#include <arch/intArchLib.h>
#include <vmx/private/kernelLibP.h>
#include <os/logLib.h>
#include <os/unixLib.h>

/* Defines */

/* Locals */
LOCAL SEM_ID splSemId;
LOCAL BOOL   unixLibInstalled = FALSE;

/* Globals */
int     splTaskId;
int     splMutexOptions = SEM_Q_PRIORITY | SEM_DELETE_SAFE;
BOOL    panicSuspend    = TRUE;
FUNCPTR panicHook       = NULL;


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
    STATUS status;

    if (unixLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        splSemId = semMCreate(splMutexOptions);
        if (splSemId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Mark as installed */
            unixLibInstalled = TRUE;
            status = OK;
        }
    }

    return status;
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

/******************************************************************************
 * splnet - Get network processor level
 *
 * RETURNS: TRUE or FALSE
 */

int splnet(
    void
    )
{
    int ret;

    if (splTaskId == ((int) taskIdCurrent))
    {
        ret = TRUE;
    }
    else
    {
        semTake(splSemId, WAIT_FOREVER);
        splTaskId = (int) taskIdCurrent;
        ret = FALSE;
    }

    return ret;
}

/******************************************************************************
 * splimp - Set imp processor level
 *
 * RETURNS: TRUE or FALSE
 */

int splimp(
    void
    )
{
    int ret;

    if (splTaskId == ((int) taskIdCurrent))
    {
        ret = TRUE;
    }
    else
    {
        semTake(splSemId, WAIT_FOREVER);
        splTaskId = (int) taskIdCurrent;
        ret = FALSE;
    }

    return ret;
}

/******************************************************************************
 * splx - Set processor level
 *
 * RETURNS: N/A
 */

void splx(
    int x
    )
{
    if (!x)
    {
        splTaskId = 0;
        semGive(splSemId);
    }
}

/******************************************************************************
 * ksleep - Got to sleep
 *
 * RETURNS: N/A
 */

void ksleep(
    SEM_ID semId
    )
{
    BOOL gotSplSem;

    /* Check if current task is spl task */
    if (splTaskId == (int) taskIdCurrent)
    {
        gotSplSem = TRUE;
    }
    else
    {
        gotSplSem = FALSE;
    }

    /* If had spl semaphore */
    if (gotSplSem == TRUE)
    {
        splTaskId = 0;
        semGive(splSemId);
    }

    semTake(semId, WAIT_FOREVER);

    /* If had spl semaphore */
    if (gotSplSem == TRUE)
    {
        semTake(splSemId, WAIT_FOREVER);
        splTaskId = (int) taskIdCurrent;
    }
}

/******************************************************************************
 * wakeup - Wakeup
 *
 * RETURNS: N/A
 */

void wakeup(
    SEM_ID semId
    )
{
    semGive(semId);
}

/******************************************************************************
 * insque - Insert into queue
 *
 * RETURNS: N/A
 */

void insque(
    void *node,
    void *prev
    )
{
    LIST_NODE *pNode = (LIST_NODE *) node;
    LIST_NODE *pPrev = (LIST_NODE *) prev;
    LIST_NODE *pNext = pPrev->next;

    /* Insert node */
    pPrev->next = pNode;
    pNext->prev = pNode;
    pNode->next = pNext;
    pNode->prev = pPrev;
}

/******************************************************************************
 * remque - Remove from queue
 *
 * RETURNS: N/A
 */

void remque(
    void *node
    )
{
    LIST_NODE *pNode = (LIST_NODE *) node;

    /* Remove node */
    pNode->prev->next = pNode->next;
    pNode->next->prev = pNode->prev;
}

/******************************************************************************
 * hashinit - Initialize hash table
 *
 * RETURNS: Pointer to table
 */

void* hashinit(
    int            elements,
    int            type,
    unsigned long *hashmask
    )
{
    LIST *hashtable;
    long  hashsize;
    int   i;

    /* If le. zero elements */
    if (elements <= 0)
    {
        panic("hashinit: bad elements");
    }

    /* For all elements log 2 */
    for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
    {
        continue;
    }

    /* Shift size size */
    hashsize >>= 1;

    /* Allocate hash table */
    hashtable = malloc((size_t) hashsize * sizeof(LIST));
    if (hashtable != NULL)
    {
        /* For all lists */
        for (i = 0; i < hashsize; i++)
        {
            listInit(&hashtable[i]);
        }

        /* Set mask parameter */
        *hashmask = hashsize - 1;
    }

  return hashtable;
}

