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

/* semLibP.h - Private semaphore header */

#ifndef _semLibP_h
#define _semLibP_h

#include <vmx/classLib.h>
#include <util/qLib.h>

/* Mutex kernel give states */
#define SEM_M_Q_GET            0x01
#define SEM_M_SAFE_Q_FLUSH     0x02

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct semaphore
{
    OBJ_CORE objCore;
    unsigned char semType;
    unsigned char options;
    unsigned short recurse;
    Q_HEAD qHead;
    union
    {
        unsigned count;
        void *owner;
    } state;
} SEMAPHORE;

/* Imports */
IMPORT FUNCPTR semGiveTable[];
IMPORT FUNCPTR semTakeTable[];
IMPORT FUNCPTR semFlushTable[];
IMPORT FUNCPTR semGiveDeferTable[];
IMPORT FUNCPTR semFlushDeferTable[];

/******************************************************************************
 * semOwner - Get owner field for semaphore
 *
 * RETURNS: Ownver field
 */

#define semOwner                state.owner

/******************************************************************************
 * semCount - Get counter field field semaphore
 *
 * RETURNS: Counter field
 */

#define semCount                state.count

/******************************************************************************
 * SEM_OWNER_GET
 *
 * RETURNS: Get semaphore owner
 */

#define SEM_OWNER_GET(semId) \
    ((TCB_ID) semId->state.owner)

/******************************************************************************
 * SEM_OWNER_SET
 *
 * RETURNS: Set semaphore owner
 */

#define SEM_OWNER_SET(semId, val) \
    (semId->state.owner = (void *) val)

/******************************************************************************
 * SEM_COUNT
 *
 * RETURNS: Get semaphore counter
 */

#define SEM_COUNT(semId) \
    (semId->state.count)

IMPORT OBJ_CLASS semClass;

/* Functions */

/******************************************************************************
 * semQInit - Initialize semaphore queue
 *
 * RETURNS: OK or ERROR
 */

STATUS semQInit(
    Q_HEAD *pQHead,
    int options
    );

/******************************************************************************
 * semInvalid - Invalid semaphore function
 *
 * RETURNS: ERROR
 */

STATUS semInvalid(
    SEMAPHORE *semId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _semLibP_h */

