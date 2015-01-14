/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2015 Surplus Users Ham Society
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

/* udvmxq.c - Universal graphics library message queue */

#include <os/errnoLib.h>
#include "uglos.h"

int sysClockRateGet(void);

/******************************************************************************
 *
 * uglOSMsgQCreate - Create message queue
 *
 * RETURNS: UGL_OS_MSG_Q_ID or UGL_NULL
 */

UGL_OS_MSG_Q_ID uglOSMsgQCreate (
    UGL_UINT32  maxMsgs,
    UGL_UINT32  maxMsgSize
    ) {

    return msgQCreate(maxMsgs, maxMsgSize, MSG_Q_FIFO);
}

/******************************************************************************
 *
 * uglOSMsgQDelete - Delete message queue
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMsgQDelete(
    UGL_OS_MSG_Q_ID  qId
    ) {
    UGL_STATUS  status;

    if (msgQDelete(qId) != OK) {
        status = UGL_STATUS_ERROR;
    }
    else {
        status = UGL_STATUS_OK;
    }

    return status;
}

/******************************************************************************
 *
 * uglOSMsgQPost - Post message on message queue
 *
 * RETURNS: UGL_STATUS_OK, UGL_STATUS_Q_FULL or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMsgQPost(
    UGL_OS_MSG_Q_ID  qId,
    UGL_TIMEOUT      timeout,
    void            *pMsg,
    UGL_UINT32       msgSize
    ) {
    UGL_STATUS  status;
    int         ern;

    if (timeout != UGL_WAIT_FOREVER) {
        timeout = timeout * sysClockRateGet() / 1000;
    }

    ern = errnoGet();
    if (msgQSend(qId, (char *) pMsg, msgSize, timeout, 0)) {
        switch(ern) {
            case S_objLib_UNAVAILABLE:
                errnoSet(ern);
                status = UGL_STATUS_Q_FULL;
                break;

            default:
                status = UGL_STATUS_ERROR;
                break;
        }
    }
    else {
        status = UGL_STATUS_OK;
    }

    return status;
}

/******************************************************************************
 *
 * uglOSMsgQGet - Get message from message queue
 *
 * RETURNS: UGL_STATUS_OK, UGL_STATUS_Q_EMPTY or UGL_STATUS_ERROR
 */

UGL_STATUS uglOSMsgQGet(
    UGL_OS_MSG_Q_ID  qId,
    UGL_TIMEOUT      timeout,
    void            *pMsg,
    UGL_UINT32       msgSize
    ) {
    UGL_STATUS  status;
    int         ern;
    int         nrecv;

    if (timeout != UGL_WAIT_FOREVER) {
        timeout = timeout * sysClockRateGet() / 1000;
    }

    ern = errnoGet();
    nrecv = msgQReceive(qId, (char *) pMsg, msgSize, timeout);
    if (nrecv == ERROR) {
        switch(ern) {
            case S_objLib_TIMEOUT:
            case S_objLib_UNAVAILABLE:
                errnoSet(ern);
                status = UGL_STATUS_Q_EMPTY;
                break;

            default:
                status = UGL_STATUS_ERROR;
                break;
        }
    }
    else {
        status = UGL_STATUS_OK;
    }

    return status;
}

