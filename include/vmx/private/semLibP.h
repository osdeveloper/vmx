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

/* semLibP.h - Private semaphore header */

#ifndef _semLibP_h
#define _semLibP_h

#include <vmx/private/classLibP.h>
#include <vmx/private/objLibP.h>
#include <util/qLib.h>

#define MAX_SEM_TYPE		1
#define SEM_TYPE_MASK		0x7
#define SEM_TYPE_BINARY		0x0

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#define SEM_OWNER_GET(semId) 		((TCB_ID) semId->state.owner)
#define SEM_OWNER_SET(semId, val)	(semId->state.owner = (void *) val)
#define SEM_COUNT(semId)		semId->state.count

typedef struct semaphore
{
  OBJ_CORE objCore;
  unsigned char semType;
  unsigned char options;
  unsigned short recurse;
  Q_HEAD qHead;
  union {
    unsigned count;
    void *owner;
  } state;
} SEMAPHORE;

extern OBJ_CLASS	semClass;

extern FUNCPTR semGiveTable[];
extern FUNCPTR semTakeTable[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _semLibP_h */

