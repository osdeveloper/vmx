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

/* msgQLib.h - Header for message queues */

#ifndef _msgQLib_h
#define _msgQLib_h

#include <tools/moduleNumber.h>
#include <vmx.h>
#include <util/qLib.h>
#include <vmx/private/msgQLibP.h>

#define MSG_Q_TYPE_MASK                 0x01
#define MSG_Q_FIFO                      0x00
#define MSG_Q_PRIORITY                  0x01

#define MSG_PRI_NORMAL                  0
#define MSG_PRI_URGENT                  1

#define S_msgQLib_NOT_INSTALLED        (M_msgQLib | 0x0001)
#define S_msgQLib_INVALID_MSG_LENGTH   (M_msgQLib | 0x0002)
#define S_msgQLib_INVALID_TIMEOUT      (M_msgQLib | 0x0003)
#define S_msgQLib_INVALID_Q_TYPE       (M_msgQLib | 0x0004)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msg_q *MSG_Q_ID;
extern CLASS_ID msgQClassId;

/* Macros */

/******************************************************************************
 * MSG_NODE_SIZE - Get message node size
 *
 * RETURNS: Message node size
 */

#define MSG_NODE_SIZE(msgLength) \
    (ROUND_UP((sizeof(MSG_NODE) + msgLength), _ALLOC_ALIGN_SIZE))

/* Functions */

/******************************************************************************
 * msgQLibInit - Initialize message quque library
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQLibInit(
    void
    );

/******************************************************************************
 * msgQCreate - Create a message queue
 *
 * RETURNS: MSG_Q_ID or NULL
 */

MSG_Q_ID msgQCreate(
    int maxMsg,
    int maxMsgLength,
    int options
    );

/******************************************************************************
 * msgQInit - Initialize a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQInit(
    MSG_Q_ID msgQId,
    int maxMsg,
    int maxMsgLength,
    int options,
    void *pMsgPool
    );

/******************************************************************************
 * msgQTerminate - Terminate a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQTerminate(
    MSG_Q_ID msgQId
    );

/******************************************************************************
 * msgQDelete - Delete a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQDelete(
    MSG_Q_ID msgQId
    );

/******************************************************************************
 * msgQDestroy - Destroy a message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQDestroy(
    MSG_Q_ID msgQId,
    BOOL deallocate
    );

/******************************************************************************
 * msgQSend - Send a message on message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQSend(
    MSG_Q_ID msgQId,
    void *buffer,
    unsigned nBytes,
    unsigned timeout,
    unsigned priority
    );

/******************************************************************************
 * msgQReceive - Receive a message from message queue
 *
 * RETURNS: number of bytes read or ERROR
 */

int msgQReceive(
    MSG_Q_ID msgQId,
    void *buffer,
    unsigned maxBytes,
    unsigned timeout
    );

/******************************************************************************
 * msgQNumMsgs - Get number of messages on message queue
 *
 * RETURNS: Number of messages on queue or ERROR
 */

int msgQNumMsgs(
    MSG_Q_ID msgQId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _msgQLib_h */

