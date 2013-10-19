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

/* semLib.h - Semaphore header */

#ifndef _semLib_h
#define _semLib_h

#include <vmx/moduleNumber.h>
#include <vmx/private/semLibP.h>

#define MAX_SEM_TYPE                        3
#define SEM_TYPE_MASK                    0x03
#define SEM_TYPE_BINARY                  0x00
#define SEM_TYPE_MUTEX                   0x01
#define SEM_TYPE_COUNTING                0x02

#define SEM_Q_MASK                       0x03
#define SEM_Q_FIFO                       0x00
#define SEM_Q_PRIORITY                   0x01
#define SEM_DELETE_SAFE                  0x04

#define S_semLib_NOT_INSTALLED           (M_semLib | 0x0001)
#define S_semLib_INVALID_STATE           (M_semLib | 0x0002)
#define S_semLib_INVALID_OPTION          (M_semLib | 0x0003)
#define S_semLib_INVALID_Q_TYPE          (M_semLib | 0x0004)
#define S_semLib_INVALID_OPERATION       (M_semLib | 0x0005)
#define S_semLib_NOT_OWNER               (M_semLib | 0x0006)
#define S_semLib_INVALID_MAX_READERS     (M_semLib | 0x0007)
#define S_semLib_RECURSION_LIMIT         (M_semLib | 0x0008)
#define S_semLib_LOST_LOCK               (M_semLib | 0x0009)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SEM_EMPTY = 0,
    SEM_FULL
} SEM_B_STATE;

typedef SEMAPHORE *SEM_ID;

IMPORT CLASS_ID semClassId;

/* Functions */

/******************************************************************************
 * semLibInit - Initialize semaphore library
 *
 * RETURNS: OK or ERROR
 */

STATUS semLibInit(
    void
    );

/******************************************************************************
 * semCreate - Create a semaphore
 *
 * RETURNS: Semaphore Id or NULL
 */

SEM_ID semCreate(
    int type,
    int options
    );

/******************************************************************************
 * semInit - Initialize a semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semInit(
    SEM_ID semId,
    int type,
    int options
    );

/******************************************************************************
 * semDelete - Delete semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semDelete(
    SEM_ID semId
    );

/******************************************************************************
 * semTerminate - Terminate semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semTerminate(
    SEM_ID semId
    );

/******************************************************************************
 * semDestroy - Destroy semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semDestroy(
    SEM_ID semId,
    BOOL deallocate
    );

/******************************************************************************
 * semGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semGive(
    SEM_ID semId
    );

/******************************************************************************
 * semGive - Give up semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semGive(
    SEM_ID semId
    );

/******************************************************************************
 * semTake - Take hold of semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semTake(
    SEM_ID semId,
    unsigned timeout
    );

/******************************************************************************
 * semFlush - Flush all tasks depending on semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semFlush(
    SEM_ID semId
    );

/******************************************************************************
 * semGiveDefer - Give sempahore defered
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semGiveDefer(
    SEM_ID semId
    );

/******************************************************************************
 * semFlushDefer - Flush all tasks depending on semaphore
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS semFlushDefer(
    SEM_ID semId
    );

/******************************************************************************
 * semQFlush - Flush semaphore queue
 *
 * RETURNS: OK or ERROR
 */

STATUS semQFlush(
    SEM_ID semId
    );

/******************************************************************************
 * semQFlushDefer - Flush semaphore queue in defered mode
 *
 * RETURNS: N/A
 */

void semQFlushDefer(
    SEM_ID semId
    );

/******************************************************************************
 * semBLibInit - Initialize binary semaphore library
 *
 * RETURNS: OK
 */

STATUS semBLibInit(
    void
    );

/******************************************************************************
 * semBCreate - Allocate and init semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semBCreate(
    int options,
    SEM_B_STATE state
    );

/******************************************************************************
 * semBInit - Init semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semBInit(
    SEM_ID semId,
    int options,
    SEM_B_STATE state
    );

/******************************************************************************
 * semMLibInit - Initialize mutex semaphore library
 *
 * RETURNS: OK
 */

STATUS semMLibInit(
    void
    );

/******************************************************************************
 * semMCreate - Allocate and init mutex semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semMCreate(
    int options
    );

/******************************************************************************
 * semMInit - Init mutex semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semMInit(
    SEM_ID semId,
    int options
    );

/***************************************************************************
 * semMGiveForce - forcibly give a mutex (for debugging only)
 *
 * This routine forcibly releases a mutex semaphore.  It passes ownership to
 * the next in the queue (if any).
 *
 * RETURNS: OK or ERROR
 */

STATUS semMGiveForce(
    SEM_ID  semId      /* mutex semaphore to forcibly give */
    );

/******************************************************************************
 * semCLibInit - Initialize counting semaphore library
 *
 * RETURNS: OK
 */

STATUS semCLibInit(
    void
    );

/******************************************************************************
 * semCCreate - Allocate and init counting semaphore
 *
 * RETURNS: SEM_ID or NULL
 */

SEM_ID semCCreate(
    int options,
    int initialCount
    );

/******************************************************************************
 * semCInit - Init counting semaphore
 *
 * RETURNS: OK or ERROR
 */

STATUS semCInit(
    SEM_ID semId,
    int options,
    int initialCount
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _semLib_h */

