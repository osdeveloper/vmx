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

/* msgQInfo.h - Message queue info header */

#ifndef _msgQInfo_h
#define _msgQInfo_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int    numMsg;              /* Output: Number of messages */
    int    numTask;             /* Output: Number of tasks waiting */
    int    sendTimeouts;        /* Output: Number of send timeouts */
    int    reciveTimeouts;      /* Output: Number of receive timeouts */
    int    options;             /* Output: Options */
    int    maxMsg;              /* Output: Maximum messages */
    int    maxMsgLength;        /* Output: Max byte length of each message */
    int    taskIdListMax;       /* Input: Max tasks for taskIdList */
    int   *taskIdList;          /* Input: Pointer to task id list */
    int    msgListMax;          /* Input: Max messages for message list */
    char **msgPtrList;          /* Input: Pointer to message list */
    int   *msgLengthList;       /* Input: Pointer to num messages in list */
} MSG_Q_INFO;

/******************************************************************************
 * msgQInfoGet - Get information about message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQInfoGet(
    MSG_Q_ID msgQId,
    MSG_Q_INFO *pInfo
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _msgQInfo_h */

