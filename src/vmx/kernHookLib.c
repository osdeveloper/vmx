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

/* kernHookLib.c - Hooks called when kernel switch tasks */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/logLib.h>
#include <vmx/kernHookLib.h>

/* IMPORTS */
IMPORT FUNCPTR kernCreateHooks[];
IMPORT FUNCPTR kernSwitchHooks[];
IMPORT FUNCPTR kernDeleteHooks[];
IMPORT FUNCPTR kernSwapHooks[];
IMPORT int kernSwapReference[];

/* LOCALS */
LOCAL BOOL kernHookLibInstalled = FALSE;
LOCAL STATUS kernHookAdd(FUNCPTR hook, FUNCPTR table[], int max);
LOCAL STATUS kernHookDelete(FUNCPTR hook, FUNCPTR table[], int max);
LOCAL STATUS kernSwapMaskSet(TCB_ID pTcb,
			     int index,
			     BOOL swapIn,
			     BOOL swapOut);
LOCAL STATUS kernSwapMaskClear(TCB_ID pTcb,
			       int index,
			       BOOL swapIn,
			       BOOL swapOut);

/*******************************************************************************
* kernHookLibInit - Initialize kernel switch hook library
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernHookLibInit(void)
{
  int i;

  logString("kernHookLibInit() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Null all entries */
  for (i = 0; i < MAX_KERNEL_CREATE_HOOKS; i++)
    kernCreateHooks[i] = NULL;

  for (i = 0; i < MAX_KERNEL_SWITCH_HOOKS; i++)
    kernSwitchHooks[i] = NULL;

  for (i = 0; i < MAX_KERNEL_DELETE_HOOKS; i++)
    kernDeleteHooks[i] = NULL;

  for (i = 0; i < MAX_KERNEL_SWAP_HOOKS; i++)
  {
    kernSwapHooks[i] = NULL;
    kernSwapReference[i] = 0;
  }

  /* Mark as installed */
  kernHookLibInstalled = TRUE;

  return(OK);
}

/*******************************************************************************
* kernCreateHookAdd - Add a kernel task create hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernCreateHookAdd(FUNCPTR hook)
{
  logString("kernCreateHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookAdd(hook, kernCreateHooks, MAX_KERNEL_CREATE_HOOKS));
}

/*******************************************************************************
* kernCreateHookDelete - Delete a kernel task create hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernCreateHookDelete(FUNCPTR hook)
{
  logString("kernCreateHookDelete() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookDelete(hook, kernCreateHooks, MAX_KERNEL_CREATE_HOOKS));
}

/*******************************************************************************
* kernSwitchHookAdd - Add a kernel task switch hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwitchHookAdd(FUNCPTR hook)
{
  logString("kernSwitchHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookAdd(hook, kernSwitchHooks, MAX_KERNEL_SWITCH_HOOKS));
}

/*******************************************************************************
* kernSwitchHookDelete - Delete a kernel switch create hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwitchHookDelete(FUNCPTR hook)
{
  logString("kernSwitchHookDelete() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookDelete(hook, kernSwitchHooks, MAX_KERNEL_SWITCH_HOOKS));
}

/*******************************************************************************
* kernDeleteHookAdd - Add a kernel task delete hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernDeleteHookAdd(FUNCPTR hook)
{
  logString("kernDeleteHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookAdd(hook, kernDeleteHooks, MAX_KERNEL_DELETE_HOOKS));
}

/*******************************************************************************
* kernDeleteHookDelete - Delete a kernel switch delete hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernDeleteHookDelete(FUNCPTR hook)
{
  logString("kernDeleteHookDelete() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Call general function */
  return (kernHookDelete(hook, kernDeleteHooks, MAX_KERNEL_DELETE_HOOKS));
}

/*******************************************************************************
* kernHookAdd - Add a kernel hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS kernHookAdd(FUNCPTR hook, FUNCPTR table[], int max)
{
  int i;

  logString("kernHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find next free slot */
  for (i = 0; i < max; i++)
  {
    if (table[i] == NULL)
    {

      /* Insert info fist free spot */
      table[i] = hook;

      /* Exit */
      taskUnlock();
      return(OK);
    }
  }

  /* No slots left */
  taskUnlock();

  logString("ERROR - No more slots for kernel hooks avilable",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernHookDelete - Delete a kernel hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS kernHookDelete(FUNCPTR hook, FUNCPTR table[], int max)
{
  int i;

  logString("kernHookDelete() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find entry */
  for (i = 0; i < max; i++)
  {
    if (table[i] == hook)
    {

      /* Move up all other hooks */
      do {
	table[i] = table[i + 1];
      } while (table[i] != NULL);

      /* Exit */
      taskUnlock();
      return(OK);
    }
  }

  /* Slot not found */
  taskUnlock();

  logString("ERROR - Trying to remove a non existant kernel hook",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernSwapHookAdd - Add a kernel swap hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwapHookAdd(FUNCPTR hook)
{
  int i;

  logString("kernSwapHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find next free slot */
  for (i = 0; i < MAX_KERNEL_SWAP_HOOKS; i++)
  {
    if (kernSwapHooks[i] == NULL)
    {

      /* Insert info fist free spot */
      kernSwapHooks[i] = hook;
      kernSwapReference[i] = 0;

        /* Exit */
        taskUnlock();
	return(OK);
      }
  }
  /* Slot not found */
  taskUnlock();

  logString("ERROR - No more slots for kernel swap hooks avilable",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernSwapHookAttach - Attach a task to kernel swap hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwapHookAttach(FUNCPTR hook,
			  TCB_ID pTcb,
			  BOOL swapIn,
			  BOOL swapOut)
{
  int i;

  logString("kernSwapHookAdd() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find hook */
  for (i = 0; i < MAX_KERNEL_SWAP_HOOKS; i++)
  {

    if (kernSwapHooks[i] == hook)
    {

      if ( (kernSwapMaskSet(pTcb, i, swapIn, swapOut) != OK) )
      {

        logString("ERROR - Unable to set swap mask for task",
	    	   LOG_KERN_HOOK_LIB,
	    	   LOG_LEVEL_ERROR);

	/* Return error */
	taskUnlock();
	return(ERROR);
      }
      else
      {

        /* Set swap in/out */
        kernSwapReference[i] += (swapIn) ? 1 : 0;
        kernSwapReference[i] += (swapOut) ? 1 : 0;

	taskUnlock();
	return(OK);
      }

    }

  }

  /* Not found */
  taskUnlock();

  logString("ERROR - Unable to attach to a non existant kernel swap hook",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernSwapHookDetach - Detach a task to kernel swap hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwapHookDetach(FUNCPTR hook,
			  TCB_ID pTcb,
			  BOOL swapIn,
			  BOOL swapOut)
{
  int i;

  logString("kernSwapHookDetach() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find hook */
  for (i = 0; i < MAX_KERNEL_SWAP_HOOKS; i++)
  {

    if (kernSwapHooks[i] == hook)
    {

      if ( (kernSwapMaskClear(pTcb, i, swapIn, swapOut) != OK) )
      {

        logString("ERROR - Unable to clear swap mask for task",
	    	   LOG_KERN_HOOK_LIB,
	    	   LOG_LEVEL_ERROR);

	/* Return error */
	taskUnlock();
	return(ERROR);
      }
      else
      {

	/* Reset in/out */
        kernSwapReference[i] -= (swapIn) ? 1 : 0;
        kernSwapReference[i] -= (swapOut) ? 1 : 0;

	/* Return error */
	taskUnlock();
	return(OK);
      }

    }

  }

  /* Not found */
  taskUnlock();

  logString("ERROR - Unable to detach to a non existant kernel swap hook",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernSwapHookDelete - Delete a kernel swap hook
*
* RETURNS: OK or ERROR
*******************************************************************************/

STATUS kernSwapHookDelete(FUNCPTR hook)
{
  int i;

  logString("kernSwapHookDelete() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Lock task */
  taskLock();

  /* Find entry */
  for (i = 0; i < MAX_KERNEL_SWAP_HOOKS; i++)
  {
    if (kernSwapHooks[i] == hook)
    {

      /* Check if task are still onnected to this hook */
      if (kernSwapReference[i] != 0)
      {

	/* Unable to remove */
	taskUnlock();
	return(ERROR);
      }
      else
      {

        /* Remove entry */
        kernSwapHooks[i] = NULL;
	taskUnlock();
	return(OK);
      }
    }
  }

  /* Slot not found */
  taskUnlock();

  logString("ERROR - Trying to remove a non existant kernel swap hook",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_ERROR);

  return(ERROR);
}

/*******************************************************************************
* kernSwapMaskSet - Set swap mask for a task
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS kernSwapMaskSet(TCB_ID pTcb,
			     int index,
			     BOOL swapIn,
			     BOOL swapOut)
{
  u_int16_t indexBit;

  logString("kernSwapMaskSet() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Check if valid task */
  if (pTcb == NULL)
    return(NULL);

  /* Calculate index bit */
  indexBit = (1 << (15 - index));

  /* Check if valid  */
  if ( ((swapIn) && (pTcb->swapInMask & indexBit)) ||
       ((swapOut) && (pTcb->swapOutMask & indexBit)) )
  {
    logString("ERROR - Trying to set to invalid mask",
	      LOG_KERN_HOOK_LIB,
	      LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Set bits */
  if (swapIn) pTcb->swapInMask |= indexBit;
  if (swapOut) pTcb->swapOutMask |= indexBit;

  return(OK);
}

/*******************************************************************************
* kernSwapMaskClear - Clear swap mask for a task
*
* RETURNS: OK or ERROR
*******************************************************************************/

LOCAL STATUS kernSwapMaskClear(TCB_ID pTcb,
			       int index,
			       BOOL swapIn,
			       BOOL swapOut)
{
  u_int16_t indexBit;

  logString("kernSwapMaskClear() called",
	    LOG_KERN_HOOK_LIB,
	    LOG_LEVEL_CALLS);

  /* Check if valid task */
  if (pTcb == NULL)
    return(NULL);

  /* Calculate index bit */
  indexBit = (1 << (15 - index));

  /* Check if valid  */
  if ( ((swapIn) && (pTcb->swapInMask & indexBit)) ||
       ((swapOut) && (pTcb->swapOutMask & indexBit)) )
  {
    logString("ERROR - Trying clear invalid mask",
	      LOG_KERN_HOOK_LIB,
	      LOG_LEVEL_ERROR);
    return(ERROR);
  }

  /* Set bits */
  if (swapIn) pTcb->swapInMask &= ~indexBit;
  if (swapOut) pTcb->swapOutMask &= ~indexBit;

  return(OK);
}

