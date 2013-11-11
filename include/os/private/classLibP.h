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

/* classLibP.h - Private class header */

#ifndef _classLibP_h
#define _classLibP_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <vmx.h>
#include <os/classLib.h>
#include <os/private/objLibP.h>

struct mem_part;

typedef struct obj_class
{
    OBJ_CORE         objCore;
    struct mem_part *objPartId;
    unsigned         objSize;
    unsigned         objAllocCount;
    unsigned         objFreeCount;
    unsigned         objInitCount;
    unsigned         objTerminateCount;
    int              coreOffset;

    FUNCPTR          createMethod;
    FUNCPTR          initMethod;
    FUNCPTR          destroyMethod;
    FUNCPTR          showMethod;
} OBJ_CLASS;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _classLibP_h */

