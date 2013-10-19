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

/* msgQLibP.h - Private header for message queues */

#ifndef _msgQLibP_h
#define _msgQLibP_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx/objLib.h>
#include <vmx/classLib.h>
#include <util/qMsgLib.h>

typedef struct msg_q
{
    OBJ_CORE objCore;
    Q_MSG_HEAD msgQ;
    Q_MSG_HEAD freeQ;
    int options;
    int maxMsg;
    int maxMsgLength;
    int sendTimeouts;
    int reciveTimeouts;
} MSG_Q;

typedef struct
{
    Q_MSG_NODE node;
    int msgLength;
} MSG_NODE;

/******************************************************************************
 * MSG_NODE_DATA - Get message node data
 *
 * RETURNS: Pointer to data
 */

#define MSG_NODE_DATA(pNode) \
    (((char *)pNode) + sizeof(MSG_NODE))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _msgQLibP_h */

