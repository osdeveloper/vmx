/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* ivI386.h - Interrupt vector */

#ifndef _ivI386_h
#define _ivI386_h

/* Exception numbers */
/* 0 - 19 */
#define IN_DIVIDE_ERROR		0
#define IN_DEBUG		1
#define IN_NON_MASKABLE		2
#define IN_BREAKPOINT		3
#define IN_OVERFLOW		4
#define IN_BOUND		5
#define IN_INVALID_OPCODE	6
#define IN_NO_DEVICE		7
#define IN_DOUBLE_FAULT		8
#define IN_CP_OVERRUN		9
#define IN_INVALID_TSS		10
#define IN_NO_SEGMENT		11
#define IN_STACK_FAULT		12
#define IN_PROTECTION_FAULT	13
#define IN_PAGE_FAULT		14
#define IN_RESERVED		15
#define IN_CP_ERROR		16
#define IN_ALIGNMENT		17
#define IN_MACHINE_CHECK	18
#define IN_SIMD			19

/* 20 - 31	unassigned, reserved exceptions */

#define IN_RESERVED_START	20
#define IN_RESERVED_END		31

/* 32 - 255	used defined interrupt vectors */

#define IN_USER_START		32
#define IN_USER_END		255

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <vmx.h>

/* Macros to convert between interrupt vectors and interrupt numbers */
#define IVEC_TO_INUM(intVec)	((int) (intVec) >> 3)
#define INUM_TO_IVEC(intNum)	((VOIDFUNCPTR *) ((intNum) << 3))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

#endif /* _ivI386_h */

