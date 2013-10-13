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

/* semLib.c - Sempahore library */

#include <stdlib.h>
#include <vmx.h>
#include <util/qLib.h>
#include <util/qFifoLib.h>
#include <util/qPrioLib.h>
#include <vmx/logLib.h>
#include <vmx/memPartLib.h>
#include <vmx/semLib.h>

/* Imports */
IMPORT BOOL kernState;

/* Locals */
LOCAL BOOL semLibInstalled = FALSE;

/* Internals */
OBJ_CLASS	semClass;

FUNCPTR semGiveTable [MAX_SEM_TYPE] =
{
  (FUNCPTR) semInvalid
};

FUNCPTR semTakeTable [MAX_SEM_TYPE] =
{
  (FUNCPTR) semInvalid
};

/* Externals */
CLASS_ID	semClassId = &semClass;

/******************************************************************************
* semLibInit - Initialize semaphore library
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semLibInit(void)
{
  logString("semLibInit() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Instatiate class */
  if (semLibInstalled == TRUE)
  {
      logString("WARNING - Semaphore library already installed",
            	LOG_SEM_LIB,
            	LOG_LEVEL_WARNING);
    return(OK);
  }

  if (classInit(semClassId,
		sizeof(SEMAPHORE), OFFSET(SEMAPHORE, objCore),
                memSysPartId,
		(FUNCPTR) NULL,
		(FUNCPTR) NULL,
		(FUNCPTR) NULL) != OK)
  {
      logString("ERROR - Semaphore class initialization failed",
            	LOG_SEM_LIB,
            	LOG_LEVEL_ERROR);
      return(ERROR);
  }

  if (semBLibInit() != OK)
  {
    logString("ERROR - Binary semaphore library initialization failed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  semLibInstalled = TRUE;

  return(OK);
}

/******************************************************************************
* semGive - Give up semaphore
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semGive(SEM_ID semId)
{
  logString("semGive() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if library is installed */
  if (semLibInstalled == FALSE)
  {
    logString("ERROR - semLib not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* SHOULD IMPLEMENT SEM GIVE IN KERNEL MODE */
  if (kernState)
  {
    logString("ERROR - Not implemented",
              LOG_SEM_LIB,
              LOG_LEVEL_CALLS);
    return(ERROR);
  }

  return( (*semGiveTable[semId->semType & SEM_TYPE_MASK]) (semId) );
}

/******************************************************************************
* semTake - Take hold of semaphore
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semTake(SEM_ID semId, unsigned timeout)
{
  logString("semTake() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if library is installed */
  if (semLibInstalled == FALSE)
  {
    logString("ERROR - semLib not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  return( (*semTakeTable[semId->semType & SEM_TYPE_MASK]) (semId, timeout) );
}

/******************************************************************************
* semInvalid - Invalid semaphore function
*
* RETURNS: ERROR
******************************************************************************/

STATUS semInvalid(SEM_ID semId)
{
  logString("semInvalid() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  return(ERROR);
}

/******************************************************************************
* semQInit - Initialize semaphore queue
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semQInit(SEM_ID semId, int options)
{
  logString("semQInit() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if library is installed */
  if (semLibInstalled == FALSE)
  {
    logString("ERROR - semLib not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Initilaize queue according to options */
  switch(options & SEM_Q_MASK)
  {
    case SEM_Q_FIFO:
      qInit(&semId->qHead, qFifoClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      break;

    case SEM_Q_PRIORITY:
      qInit(&semId->qHead, qPrioClassId, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
      break;

    default:
      logString("ERROR - Invalid semaphore quque type",
            	LOG_SEM_LIB,
            	LOG_LEVEL_ERROR);
      return(ERROR);
  }

  return(OK);
}

