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

/* uglMsg.h - Universal graphics library messages header */

#ifndef _uglMsg_h
#define _uglMsg_h

#include "uglMsgTypes.h"

/* Pointer buttons */
#define UGL_PTR_BUTTON1        0x00000001
#define UGL_PTR_BUTTON2        0x00000002
#define UGL_PTR_BUTTON3        0x00000004
#define UGL_PTR_BUTTON4        0x00000008
#define UGL_PTR_BUTTON5        0x00000010
#define UGL_PTR_BUTTON_MASK    0x000000ff
#define UGL_PTR_MOD_MASK       0x000000ff

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

struct ugl_input_dev;

typedef struct ugl_raw_ptr_data {
    struct ugl_input_dev  *deviceId;
    UGL_UINT16             buttonState;
    UGL_BOOL               isAbsolute;

    union {
        UGL_POINT          absolute;
        UGL_VECTOR         relative;
    } pos;
} UGL_RAW_PTR_DATA;

typedef struct ugl_ptr_data {
    UGL_RAW_PTR_DATA    rawPtr;
    UGL_TIMESTAMP       timeStamp;
    UGL_POINT           position;
    UGL_VECTOR          delta;
    UGL_UINT32          buttonState;
    UGL_UINT32          buttonChange;
} UGL_POINTER_DATA;

/* Message data */
typedef UGL_UINT32 UGL_MSG_TYPE;

typedef union ugl_msg_data {
    UGL_POINTER_DATA    pointer;
    UGL_RAW_PTR_DATA    rawPtr;
} UGL_MSG_DATA;

/* Message */
typedef struct ugl_msg {
    UGL_MSG_TYPE    type;
    UGL_ID          objectId;
    UGL_MSG_DATA    data;
} UGL_MSG;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglMsg_h */

