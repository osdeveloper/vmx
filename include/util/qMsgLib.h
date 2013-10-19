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

/* qMsgLib.h - Used with message queues */

#ifndef _qMsgLib_h
#define _qMsgLib_h

#define Q_MSG_PRI_TAIL          0
#define Q_MSG_PRI_HEAD          1
#define Q_MSG_PRI_ANY           1

#include <vmx.h>
#include <util/qLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qMsgNode
{
    struct qMsgNode *next;
} Q_MSG_NODE;

#include <util/qClass.h>

typedef struct
{
    Q_MSG_NODE *first;
    Q_MSG_NODE *last;
    int count;
    Q_CLASS pQClass;
    Q_HEAD pendQ;
} Q_MSG_HEAD;

IMPORT Q_CLASS_ID qMsgClassId;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _qMsgLib_h */

