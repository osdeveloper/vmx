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

/* uglps2ptr - PS/2 mouse pointer driver for universal graphics library */

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ugl.h"
#include "uglinput.h"
#include "uglMsg.h"

typedef struct ugl_ps2_ptr_device {
    UGL_INPUT_DEV    inputDev;
    UGL_INT8         data[3];
    UGL_UINT8        state;
    int              buttonState;
} UGL_PS2_PTR_DEVICE;

/* Locals */
UGL_LOCAL UGL_INPUT_DEV_ID uglPs2PtrOpen (
    char          *name,
    UGL_INPUT_DRV *pDriver
    );

UGL_LOCAL UGL_STATUS uglPs2PtrClose (
    UGL_INPUT_DEV *pDevice
    );

UGL_LOCAL UGL_STATUS uglPs2PtrControl (
    UGL_INPUT_DEV   *pDevice,
    UGL_DEVICE_REQ   request,
    void            *pArg
    );

UGL_LOCAL UGL_STATUS uglPs2PtrReadMsg (
    UGL_PS2_PTR_DEVICE *pDevice
    );

UGL_LOCAL void uglPs2PtrDecodeMsgPacket (
    UGL_INT8   data[3],
    UGL_MSG   *pMsg
    );

UGL_INPUT_DRV uglPsPtrDriver = {
    uglPs2PtrOpen,
    uglPs2PtrClose,
    uglPs2PtrControl
};

/******************************************************************************
 *
 * uglPs2PtrOpen - Open pointer device
 *
 * RETURNS: UGL_INPUT_DEV_ID or UGL_NULL
 */

UGL_LOCAL UGL_INPUT_DEV_ID uglPs2PtrOpen (
    char          *name,
    UGL_INPUT_DRV *pDriver
    ) {
    UGL_PS2_PTR_DEVICE *ps2Dev;

    ps2Dev = (UGL_PS2_PTR_DEVICE *) UGL_CALLOC(1, sizeof(UGL_PS2_PTR_DEVICE));
    if (ps2Dev != UGL_NULL) {
        ps2Dev->inputDev.fd = open(name, 0, 0);
        if (ps2Dev->inputDev.fd >= 0) {

            /* Setup device structure */
            ps2Dev->inputDev.deviceType = UGL_DEVICE_POINTER_RELATIVE;
            ps2Dev->inputDev.pDriver = pDriver;

            /* Setup device mode */
            ioctl(ps2Dev->inputDev.fd, FIOSETOPTIONS, OPT_RAW);
            ioctl(ps2Dev->inputDev.fd, FIOFLUSH, 0);
        }
        else {
            UGL_FREE(ps2Dev);
        }
    }

    return &ps2Dev->inputDev;
}

/******************************************************************************
 *
 * uglPs2PtrClose - Close pointer device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglPs2PtrClose (
    UGL_INPUT_DEV *pDevice
    ) {
    UGL_STATUS status;

    UGL_PS2_PTR_DEVICE *ps2Dev = (UGL_PS2_PTR_DEVICE *) pDevice;

    if (ps2Dev == UGL_NULL) {
        status = UGL_STATUS_ERROR;
    }
    else {
        close(ps2Dev->inputDev.fd);
        UGL_FREE(pDevice);
        status = UGL_STATUS_OK;
    }

    return status;
}

/******************************************************************************
 *
 * uglPs2PtrControl - Control commands to pointer device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglPs2PtrControl (
    UGL_INPUT_DEV   *pDevice,
    UGL_DEVICE_REQ   request,
    void            *pArg
    ) {
    UGL_STATUS status;

    UGL_PS2_PTR_DEVICE *ps2Dev = (UGL_PS2_PTR_DEVICE *) pDevice;
    if (ps2Dev == UGL_NULL) {
        status = UGL_STATUS_ERROR;
    }
    else {
        switch(request) {
            case ICR_READ:
                status = uglPs2PtrReadMsg(ps2Dev);
                break;

            case ICR_GET_PTR_TYPE:
                if (pArg == UGL_NULL) {
                    status = UGL_STATUS_ERROR;
                }
                else {
                    *(int *) pArg = UGL_PTR_TYPE_MOUSE;
                    status = UGL_STATUS_OK;
                }
                break;

            default:
                status = UGL_STATUS_ERROR;
                break;
        }
    }

    return status;
}

/******************************************************************************
 *
 * uglPs2PtrReadMsg - Read messages from pointer device
 *
 * RETURNS: UGL_STATUS_OK or UGL_STATUS_ERROR
 */

UGL_LOCAL UGL_STATUS uglPs2PtrReadMsg (
    UGL_PS2_PTR_DEVICE *pDevice
    ) {
    UGL_MSG    msg;
    UGL_INT8   value;
    UGL_INT8   chunk[3];
    UGL_POS    dx;
    UGL_POS    dy;
    UGL_STATUS status = UGL_STATUS_OK;
    int        nread = 0;

    /* Setup message */
    memset(&msg, 0, sizeof(UGL_MSG));
    msg.type                   = MSG_RAW_PTR;
    msg.data.rawPtr.deviceId   = &pDevice->inputDev;
    msg.data.rawPtr.isAbsolute = UGL_FALSE;

    /* Read data */
    ioctl(pDevice->inputDev.fd, FIONREAD, (int) &nread);
    while (nread > 0) {
        if (read(pDevice->inputDev.fd, &value, 1) != 1) {
            status = UGL_STATUS_ERROR;
            break;
        }

        nread--;

        /* Check sync */
        if ((pDevice->state == 0) && ((value & 0x08) == 0x00)) {
            continue;
        }

        pDevice->data[pDevice->state] = value;

        /* Check if more in packet */
        if (pDevice->state < 2) {
            pDevice->state++;
            continue;
        }

        /* Decode message packet */
        uglPs2PtrDecodeMsgPacket(pDevice->data, &msg);

        /* Compress messages */
        while (nread >= 3) {
            read(pDevice->inputDev.fd, chunk, 3);
            nread -= 3;

            if (msg.data.rawPtr.buttonState == pDevice->buttonState) {
                dx = msg.data.rawPtr.pos.relative.x;
                dy = msg.data.rawPtr.pos.relative.y;

                uglPs2PtrDecodeMsgPacket(chunk, &msg);
                msg.data.rawPtr.pos.relative.x += dx;
                msg.data.rawPtr.pos.relative.y += dy;
            }
            else {
                pDevice->buttonState = msg.data.rawPtr.buttonState;

                /* TODO: uglInputMsgPost(pDevice->inputDev.inputServiceId, &msg); */
                uglPs2PtrDecodeMsgPacket(chunk, &msg);
            }
        }

        /* Post message */
        /* TODO: uglInputMsgPost(pDevice->inputDev.inputServiceId, &msg); */

        /* Reset state */
        pDevice->state = 0;
    }

    return status;
}

/******************************************************************************
 *
 * uglPs2PtrDecodeMsgPacket - Decode message packet from pointer device
 *
 * RETURNS: N/A
 */

UGL_LOCAL void uglPs2PtrDecodeMsgPacket (
    UGL_INT8   data[3],
    UGL_MSG   *pMsg
    ) {

    /* Get relative position */
    if ((data[0] & 0x10) != 0x00) {
        pMsg->data.rawPtr.pos.relative.x = (UGL_POS) (data[1] - 256);
    }
    else {
        pMsg->data.rawPtr.pos.relative.x = (UGL_POS) data[1];
    }

    if ((data[0] & 0x20) != 0x00) {
        pMsg->data.rawPtr.pos.relative.y = (UGL_POS) -(data[2] - 256);
    }
    else {
        pMsg->data.rawPtr.pos.relative.y = (UGL_POS) -data[2];
    }

    /* Get button state */
    pMsg->data.rawPtr.buttonState = 0;

    if ((data[0] & 0x01) != 0x00) {
        pMsg->data.rawPtr.buttonState |= UGL_PTR_BUTTON1;
    }

    if ((data[0] & 0x02) != 0x00) {
        pMsg->data.rawPtr.buttonState |= UGL_PTR_BUTTON2;
    }

    if ((data[0] & 0x04) != 0x00) {
        pMsg->data.rawPtr.buttonState |= UGL_PTR_BUTTON3;
    }
}

