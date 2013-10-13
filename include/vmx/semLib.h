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

/* semLib.h - Semaphore header */

#ifndef _semLib_h
#define _semLib_h

#include <vmx/classLib.h>
#include <vmx/private/semLibP.h>

#define SEM_Q_MASK		0x03
#define SEM_Q_FIFO		0x00
#define SEM_Q_PRIORITY		0x01

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  SEM_EMPTY = 0,
  SEM_FULL
} SEM_B_STATE;

typedef struct semaphore *SEM_ID;

IMPORT CLASS_ID semClassId;

/* Master semaphore */
extern STATUS semLibInit(void);
extern STATUS semGive(SEM_ID semId);
extern STATUS semTake(SEM_ID semId, unsigned timeout);
extern STATUS semInvalid(SEM_ID semId);

extern STATUS semQInit(SEM_ID semId, int options);

/* Binary semaphore */
extern STATUS semBLibInit(void);
extern SEM_ID semBCreate(int options, SEM_B_STATE state);
extern STATUS semBInit(SEM_ID semId, int options, SEM_B_STATE state);
extern STATUS semBCoreInit(SEM_ID semId, int options, SEM_B_STATE state);
extern STATUS semBGive(SEM_ID semId);
extern STATUS semBTake(SEM_ID semId, unsigned timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _semLib_h */

