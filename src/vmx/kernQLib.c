/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* kernQLib.c - Kernel work queue library */

#include <vmx.h>
#include <arch/intArchLib.h>
#include <arch/sysArchLib.h>
#include <vmx/kernQLib.h>

/* Imports */
IMPORT int 		errno;

/* Globals */
volatile unsigned	kernQReadIndex;
volatile unsigned	kernQWriteIndex;
volatile BOOL		kernQEmpty;
KERN_JOB		kernJobs[255];

/******************************************************************************
* kernQLibInit - Initialize kernel work queue
*
* RETURNS: OK
******************************************************************************/

STATUS kernQLibInit(void)
{
  kernQReadIndex = 0;
  kernQWriteIndex = 0;
  kernQEmpty = TRUE;

  return(OK);
}

/******************************************************************************
* kernQAdd0 - Add a kernel job with no aguments
*
* RETURNS: N/A
******************************************************************************/

void kernQAdd0(FUNCPTR func)
{
  int oldLevel;
  FAST KERN_JOB *pJob;

  INT_LOCK(oldLevel);					/* LOCK INTERRUPTS */

  /* Get place to store job in buffer */
  pJob = (KERN_JOB *) &kernJobs[kernQWriteIndex];

  /* Increase pointer */
  kernQWriteIndex++;

  if (kernQWriteIndex == kernQReadIndex)
    kernQPanic();

  INT_UNLOCK(oldLevel);					/* UNLOCK INTERRUPTS */

  /* Queue not empty */
  kernQEmpty = FALSE;

  /* Store data */
  pJob->func = func;
  pJob->numArgs = 0;
}

/******************************************************************************
* kernQAdd1 - Add a kernel job with one agument
*
* RETURNS: N/A
******************************************************************************/

void kernQAdd1(FUNCPTR func, ARG arg1)
{
  int oldLevel;
  FAST KERN_JOB *pJob;

  INT_LOCK(oldLevel);					/* LOCK INTERRUPTS */

  /* Get place to store job in buffer */
  pJob = (KERN_JOB *) &kernJobs[kernQWriteIndex];

  /* Increase pointer */
  kernQWriteIndex++;

  if (kernQWriteIndex == kernQReadIndex)
    kernQPanic();

  INT_UNLOCK(oldLevel);					/* UNLOCK INTERRUPTS */

  /* Queue not empty */
  kernQEmpty = FALSE;

  /* Store data */
  pJob->func = func;
  pJob->arg1 = arg1;
  pJob->numArgs = 1;
}

/******************************************************************************
* kernQAdd2 - Add a kernel job with two aguments
*
* RETURNS: N/A
******************************************************************************/

void kernQAdd2(FUNCPTR func,ARG arg1,ARG arg2)
{
  int oldLevel;
  FAST KERN_JOB *pJob;

  INT_LOCK(oldLevel);					/* LOCK INTERRUPTS */

  /* Get place to store job in buffer */
  pJob = (KERN_JOB *) &kernJobs[kernQWriteIndex];

  /* Increase pointer */
  kernQWriteIndex++;

  if (kernQWriteIndex == kernQReadIndex)
    kernQPanic();

  INT_UNLOCK(oldLevel);					/* UNLOCK INTERRUPTS */

  /* Queue not empty */
  kernQEmpty = FALSE;

  /* Store data */
  pJob->func = func;
  pJob->arg1 = arg1;
  pJob->arg2 = arg2;
  pJob->numArgs = 2;
}

/******************************************************************************
* kernQDoWork - Do job on the kernel queue
*
* RETURNS: N/A
******************************************************************************/

void kernQDoWork(void)
{
  FAST KERN_JOB *pJob;
  int keepErrno;

  /* Store current errno */
  keepErrno = errno;

  /* Do all jobs on store */
  while (kernQReadIndex != kernQWriteIndex) {

    /* Get next job */
    pJob = (KERN_JOB *) &kernJobs[kernQReadIndex];

    /* Increase pointer */
    kernQReadIndex++;

    /* Call job procedure */
    switch(pJob->numArgs) {
      case 1 : (FUNCPTR *) (pJob->func)(pJob->arg1);
	       break;
      case 2 : (FUNCPTR *) (pJob->func)(pJob->arg1, pJob->arg2);
	       break;
      default: (FUNCPTR *) (pJob->func)();
	       break;
    }

  }

  /* Queue must be empty */
  kernQEmpty = TRUE;

  /* Restore errno */
  errno = keepErrno;
}

/******************************************************************************
* kernQPanic - Fatal error in queue
*
* RETURNS: N/A
******************************************************************************/

void kernQPanic(void)
{
  sysReboot();
}

