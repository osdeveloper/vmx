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

/* semBLib.c - Binary sempahore library */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/logLib.h>
#include <vmx/semLib.h>
#include <arch/intArchLib.h>
#include <vmx/classLib.h>
#include <vmx/memPartLib.h>
#include <vmx/taskLib.h>
#include <vmx/vmxLib.h>
#include <vmx/sigLib.h>

/* Imports */
IMPORT BOOL kernState;
IMPORT STATUS kernExit(void);
IMPORT TCB_ID kernCurrTaskId;

/* Locals */
LOCAL BOOL semBLibInstalled = FALSE;

/******************************************************************************
* semBLibInit - Initialize binary semaphore library
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semBLibInit(void)
{
  logString("semBLibInit() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Instatiate class */
  if (semBLibInstalled == TRUE)
  {
    logString("WARNING - Binary semaphore library already installed",
              LOG_SEM_LIB,
              LOG_LEVEL_WARNING);
    return(OK);
  }

  /* Install call methods */
  semGiveTable[SEM_TYPE_BINARY] = (FUNCPTR) semBGive;
  semTakeTable[SEM_TYPE_BINARY] = (FUNCPTR) semBTake;

  /* Mark library as installed */
  semBLibInstalled = TRUE;

  return(OK);
}

/******************************************************************************
* semBCreate - Allocate and init semaphore
*
* RETURNS: SEM_ID
******************************************************************************/

SEM_ID semBCreate(int options, SEM_B_STATE state)
{
  SEM_ID semId;

  logString("semBCreate() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if lib is installed */
  if (semBLibInstalled != TRUE)
  {
    logString("ERROR - Binary semaphore library not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(NULL);
  }

  /* Allocate memory */
  semId = (SEM_ID) objAlloc(semClassId);
  if (semId == NULL)
  {
    logString("ERROR - Memory allocation for binary semaphore failed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(NULL);
  }

  /* Initialze structure */
  if (semBInit(semId, options, state) != OK)
  {
    logString("ERROR - Initialization for binary semaphore failed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    objFree(semClassId, semId);
    return(NULL);
  }

  return(semId);
}
  
/******************************************************************************
* semBInit - Init semaphore
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semBInit(SEM_ID semId, int options, SEM_B_STATE state)
{
  logString("semBInit() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if lib is installed */
  if (semBLibInstalled != TRUE)
  {
    logString("ERROR - Binary semaphore library not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Initialize semaphore queue */
  if (semQInit(semId, options) != OK)
  {
    logString("ERROR - Binary semaphore queue initialization failed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Initialize semaphore queue */
  if (semBCoreInit(semId, options, state) != OK)
  {
    logString("ERROR - Binary semaphore core initialization failed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  return(OK);
}

/******************************************************************************
* semBCoreInit - Init semaphore object core
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semBCoreInit(SEM_ID semId, int options, SEM_B_STATE state)
{
  logString("semBCoreInit() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Setup state */
  switch(state)
  {
    case SEM_EMPTY:
      SEM_OWNER_SET(semId, kernCurrTaskId);
      break;

    case SEM_FULL:
      SEM_OWNER_SET(semId, NULL);
      break;

    default: 
      logString("ERROR - Binary semaphore invalid queue type",
                LOG_SEM_LIB,
                LOG_LEVEL_ERROR);
      return(ERROR);
  }

  /* Initialize variables */
  semId->recurse = 0;
  semId->options = options;
  semId->semType = SEM_TYPE_BINARY;

  /* SHOULD INITIALIZE EVENTS HERE */

  /* Initialize object core */
  objCoreInit(&semId->objCore, semClassId);

  return(OK);
}

/******************************************************************************
* semBGive - Give up semaphore
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semBGive(SEM_ID semId)
{
  int level;
  TCB_ID ownerId;

  logString("semBGive() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if lib is installed */
  if (semBLibInstalled != TRUE)
  {
    logString("ERROR - Binary semaphore library not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  logStringAndInteger("Semaphore give request from",
                      SEM_OWNER_GET(semId)->id,
                      LOG_SEM_LIB,
                      LOG_LEVEL_INFO);

  /* Verify class */
  if (OBJ_VERIFY(&semId->objCore, semClassId) != OK)
  {
    logString("ERROR - Trying to give a non semaphore object",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Lock interrupts */
  INT_LOCK(level);

  /* Get owner */
  ownerId = SEM_OWNER_GET(semId);

  logStringAndInteger("This semaphore was taken by",
    		      ownerId->id,
                      LOG_SEM_LIB,
                      LOG_LEVEL_INFO);

  /* Get next listening task from queue */
  SEM_OWNER_SET(semId, Q_FIRST(&semId->qHead));

  /* Check if more tasks are waiting for this semaphore */
  if (SEM_OWNER_GET(semId) != NULL)
  {
    logStringAndInteger("This semaphore also has a take request from",
    			SEM_OWNER_GET(semId)->id,
                	LOG_SEM_LIB,
                	LOG_LEVEL_INFO);

    /* Enter kernel mode */
    kernState = TRUE;
    INT_UNLOCK(level);

    /* Unblock next task waiting */
    vmxPendQGet(&semId->qHead);

    /* Exit kernel mode */
    kernExit();
  }

  /* Unlock interrupts */
  INT_UNLOCK(level);

  logString("Semaphore given",
            LOG_SEM_LIB,
            LOG_LEVEL_INFO);

  return(OK);
}

/******************************************************************************
* semBTake - Take hold of semaphore
*
* RETURNS: OK or ERROR
******************************************************************************/

STATUS semBTake(SEM_ID semId, unsigned timeout)
{
  int level;
  STATUS status;

  logString("semBTake() called:",
            LOG_SEM_LIB,
            LOG_LEVEL_CALLS);

  /* Check if lib is installed */
  if (semBLibInstalled != TRUE)
  {
    logString("ERROR - Binary semaphore library not installed",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  logStringAndInteger("Semaphore take request from",
                      kernCurrTaskId->id,
                      LOG_SEM_LIB,
                      LOG_LEVEL_INFO);

  /* Verify class */
  if (OBJ_VERIFY(&semId->objCore, semClassId) != OK)
  {
    logString("ERROR - Trying to take a non semaphore object",
              LOG_SEM_LIB,
              LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Loop while waing for taker task to give back semaphore */
  do
  {
    /* Lock interrupts */
    INT_LOCK(level);

    /* Check if it is already given back */
    if (SEM_OWNER_GET(semId) == NULL)
    {
      /* Then take it */
      SEM_OWNER_SET(semId, kernCurrTaskId);

      /* Unlock interrupts */
      INT_UNLOCK(level);

      logStringAndInteger("Semaphore taken for",
			  kernCurrTaskId->id,
                	  LOG_SEM_LIB,
                	  LOG_LEVEL_INFO);

      return(OK);
    }
  
    /* Enter kernel mode */
    kernState = TRUE;
    INT_UNLOCK(level);

    logStringAndInteger("Waiting for semaphore give from",
    			SEM_OWNER_GET(semId)->id,
                	LOG_SEM_LIB,
                	LOG_LEVEL_INFO);

    /* Put on pending queue */
    if (vmxPendQPut(&semId->qHead, timeout) != OK)
    {
      logString("ERROR - Failed to put semaphore on pending queue",
                LOG_SEM_LIB,
                LOG_LEVEL_ERROR);
      kernExit();
      return(ERROR);
    }

    /* Exit kernel, loop again if semaphore expired */
  } while ( (status = kernExit()) == SIG_RESTART);

  return(status);
}

