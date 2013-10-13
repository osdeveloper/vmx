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

/* excArchLib.h - Exception library */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/iv.h>
#include <arch/esf.h>
#include <arch/intArchLib.h>
#include <arch/excArchLib.h>

#include <vmx/logLib.h>

/* Externals */
IMPORT u_int8_t		excCallTbl[];
IMPORT VOIDFUNCPTR	intCallTbl[];
IMPORT u_int32_t	sysIntIdtType;

/* Forward declarations */
LOCAL BOOL	programError(int vecNum);

/**************************************************************
* excVecInit - Setup exception interrupt vector
*
* RETURNS: OK
***************************************************************/

STATUS excVecInit(void)
{
  FAST int vecNum;

  /* Clear interrupt table */
  for (vecNum = LOW_VEC; vecNum <= HIGH_VEC; ++vecNum)
    intCallTbl[vecNum] = NULL;

  /* Setup pointers in idt to all procedures defined in
     excALib.s */

  for (vecNum = LOW_VEC; vecNum <= HIGH_VEC; ++vecNum)
  {
    intVecSet((FUNCPTR *) INUM_TO_IVEC(vecNum),
	      (FUNCPTR)   &excCallTbl[vecNum * 5],
	      programError(vecNum) ? IDT_TRAP_GATE : sysIntIdtType,
	      programError(vecNum) ? sysCsExc : sysCsInt);
  }

  return(OK);
}

/**************************************************************
* excExcHandle - Default exception handler
*
* RETURNS: N/A
***************************************************************/

void excExcHandle(int vecNum,
		  ESF0 *pEsf,
		  REG_SET *pRegs,
		  BOOL error)
{
  logStringAndInteger("ERROR - Exception occured",
                      vecNum,
                      LOG_ARCH_LIB,
                      LOG_LEVEL_ERROR);

  /* Hang */
  __asm__ __volatile__("cli");
  for(; ;);
}

/**************************************************************
* excIntHandle - Uninitialized interrput handler
*
* RETURNS: N/A
***************************************************************/

void excIntHandle(int vecNum,
		  ESF0 *pEsf,
		  REG_SET *pRegs,
		  BOOL error)
{

  logString("WARNING - Unexpected event occured:",
            LOG_ARCH_LIB,
            LOG_LEVEL_WARNING);

  /* Print message */
  if (vecNum >= 0x20 && vecNum <= 0x2f)
    logStringAndInteger("Hardware interrupt",
	                vecNum,
                        LOG_ARCH_LIB,
                        LOG_LEVEL_WARNING);
  else if (vecNum <= 0xff)
    logStringAndInteger("Software interrupt",
	                vecNum,
                        LOG_ARCH_LIB,
                        LOG_LEVEL_WARNING);
  else
    logStringAndInteger("Unknown interrupt",
	                vecNum,
                        LOG_ARCH_LIB,
                        LOG_LEVEL_WARNING);
}

/**************************************************************
* programError - Determine if exception is a software error
*
* RETURNS:
* 	TRUE if error
* 	FALSE if hw interrupt failure
***************************************************************/

LOCAL BOOL programError(int vecNum)
{
  return (vecNum <= IN_CP_ERROR);
}

