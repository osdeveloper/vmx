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

/* selectLib.h - Select library header */

#ifndef _selectLib_h
#define _selectLib_h

#include <tools/moduleNumber.h>
#define S_selectLib_NO_SELECT_CONTEXT                   (M_selectLib | 0x0001)
#define S_selectLib_NO_SELECT_SUPPORT_IN_DRIVER         (M_selectLib | 0x0002)
#define S_selectLib_WIDTH_OUT_OF_RANGE                  (M_selectLib | 0x0003)

#include <sys/time.h>

#ifndef FD_SETSIZE
#define FD_SETSIZE              2048
#endif

#ifndef _ASMLANGUAGE

typedef long fd_mask;
#define NFDBITS         (sizeof(fd_mask) * 8)
#ifndef howmany
#define howmany(x, y)   ((unsigned int)(((x)+((y)-1)))/(unsigned int)(y))
#endif

typedef struct fd_set {
  fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((p), 0, sizeof(*(p)))

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SELREAD,
    SELWRITE
} SELECT_TYPE;

#include <os/private/selectLibP.h>

typedef struct selContext *SEL_CONTEXT_ID;

/******************************************************************************
 * selectLibInit - Initialize select library
 *
 * RETURNS: OK or ERROR
 */

STATUS selectLibInit(
    void
    );

/******************************************************************************
 * selTaskDeleteHookAdd - Initialize select library delete hook
 *
 * RETURNS: OK or ERROR
 */

STATUS selTaskDeleteHookAdd(
    void
    );

/******************************************************************************
 * select - Select on a set of file descriptors
 *
 * RETURNS: Number of file descritors, zero if timeout, otherwise ERROR
 */

int select(
    int width,
    fd_set *pReadFds,
    fd_set *pWriteFds,
    fd_set *pExceptFds,
    struct timeval *pTimeOut
    );

/******************************************************************************
 * selWakeupListInit - Initialize wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selWakeupListInit(
    SEL_WAKEUP_LIST *pList
    );

/******************************************************************************
 * selWakeupListTerminate - Terminate wakeup list
 *
 * RETURNS: N/A
 */

void selWakeupListTerminate(
    SEL_WAKEUP_LIST *pList
    );

/******************************************************************************
 * selWakeupListLen - Get wakeup list size
 *
 * RETURNS: Number of nodes
 */

int selWakeupListLen(
    SEL_WAKEUP_LIST *pList
    );

/******************************************************************************
 * selWakeupType - Get select type
 *
 * RETURNS: Select type
 */

SELECT_TYPE selWakeupType(
    SEL_WAKEUP_NODE *pNode
    );

/******************************************************************************
 * selNodeAdd - Add to wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selNodeAdd(
    SEL_WAKEUP_LIST *pList,
    SEL_WAKEUP_NODE *pNode
    );

/******************************************************************************
 * selNodeDelete - Delete from wakeup list
 *
 * RETURNS: OK or ERROR
 */

STATUS selNodeDelete(
    SEL_WAKEUP_LIST *pList,
    SEL_WAKEUP_NODE *pNode
    );

/******************************************************************************
 * selWakeup - Wakeup a task sleeping on file descriptor
 *
 * RETURNS: N/A
 */

void selWakeup(
    SEL_WAKEUP_NODE *pNode
    );

/******************************************************************************
 * selWakeupAll - Wakeup all tasks sleeping in wakeup list of specified type
 *
 * RETURNS: N/A
 */

void selWakeupAll(
    SEL_WAKEUP_LIST *pList,
    SELECT_TYPE type
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _selectLib_h */

