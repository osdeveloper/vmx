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

/* msgQShow.c - Message queue show facilities */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <arch/intArchLib.h>
#include <util/qLib.h>
#include <vmx/classLib.h>
#include <vmx/taskLib.h>
#include <vmx/taskInfo.h>
#include <vmx/private/kernelLibP.h>
#include <vmx/msgQLib.h>
#include <vmx/msgQInfo.h>

/* Defines */

/* Locals */
LOCAL BOOL msgQInfoEach(
    MSG_NODE *pNode,
    MSG_Q_INFO *pInfo
    );

/* Globals */

/* Functions */

/******************************************************************************
 * msgQInfoGet - Get information about message queue
 *
 * RETURNS: OK or ERROR
 */

STATUS msgQInfoGet(
    MSG_Q_ID msgQId,
    MSG_Q_INFO *pInfo
    )
{
    STATUS status;
    int level;
    Q_HEAD *pendQ;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Verify object class */
    if (OBJ_VERIFY(msgQId, msgQClassId) != OK)
    {
        INT_UNLOCK(level);
        status = ERROR;
    }
    else
    {
        /* If no messages */
        if (msgQId->msgQ.count == 0)
        {
            pendQ = &msgQId->msgQ.pendQ;
        }
        else
        {
            pendQ = &msgQId->freeQ.pendQ;
        }

        /* If task id list requested */
        if (pInfo->taskIdList != NULL)
        {
            Q_INFO(pendQ, pInfo->taskIdList, pInfo->taskIdListMax);
        }

        /* If message list or number requested */
        if ((pInfo->msgPtrList != NULL) || (pInfo->msgLengthList != NULL))
        {
            pInfo->numMsg = 0;
            if (pInfo->msgListMax > 0)
            {
                Q_EACH(&msgQId->msgQ, msgQInfoEach, pInfo);
            }
        }

        /* Setup info structure */
        pInfo->numMsg         = msgQId->msgQ.count;
        pInfo->numTask        = Q_INFO(pendQ, NULL, 0);
        pInfo->options        = msgQId->options;
        pInfo->maxMsg         = msgQId->maxMsg;
        pInfo->maxMsgLength   = msgQId->maxMsgLength;
        pInfo->sendTimeouts   = msgQId->sendTimeouts;
        pInfo->reciveTimeouts = msgQId->reciveTimeouts;

        /* Unlock interrupts */
        INT_UNLOCK(level);
        status = OK;
    }

    return status;
}

/******************************************************************************
 * msgQInfoEach - Run for each message in message queue list
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL msgQInfoEach(
    MSG_NODE *pNode,
    MSG_Q_INFO *pInfo
    )
{
    BOOL ret;

    /* If message list requested */
    if (pInfo->msgPtrList != NULL)
    {
        pInfo->msgPtrList[pInfo->numMsg] = MSG_NODE_DATA(pNode);
    }

    /* If message length requested */
    if (pInfo->msgLengthList != NULL)
    {
        pInfo->msgLengthList[pInfo->numMsg] = pNode->msgLength;
    }

    /* In more info to get */
    if (++pInfo->numMsg < pInfo->msgListMax)
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

