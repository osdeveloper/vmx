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

/* uglvmx.h - Universal graphics library os header */

#ifndef _uglvmx_h
#define _uglvmx_h

#include <vmx.h>
#include <vmx/taskLib.h>
#include <vmx/semLib.h>
#include <os/memPartLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

#define UGL_IN_BYTE(port)               sysInByte(port)
#define UGL_OUT_BYTE(port, val)         sysOutByte(port, val)

#define UGL_IN_WORD(port)               sysInWord(port)
#define UGL_OUT_WORD(port, val)         sysOutWord(port, val)

#define UGL_IN_LONG(port)               sysInLong(port)
#define UGL_OUT_LONG(port, val)         sysOutLong(port, val)

#define UGL_PART_MALLOC(pol, siz)       memPartAlloc(pol, siz)
#define UGL_PART_FREE(pol, buf)         memPartFree(pol, buf)

typedef SEM_ID UGL_LOCK_ID;
typedef PART_ID UGL_MEM_POOL_ID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _uglvmx_h */

