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

/* intLib.h - Interrupt library */

#ifndef _intArchLib_h
#define _intArchLib_h

#include <vmx/moduleNumber.h>
#include <types/vmxCpu.h>

#if      CPU_FAMILY==I386
#include <arch/i386/intI386Lib.h>
#endif

#define S_intLib_NOT_ISR_CALLABLE      (M_intLib | 0x001)

/******************************************************************************
 * INT_CONTEXT - Check if in interrupt context
 *
 * RETURNS: Non-zero if in interrupt context
 */

#define INT_CONTEXT()           (intCnt > 0)

/******************************************************************************
 * INT_RESTRICT - Restrict usage from interrupt context
 *
 * RETURNS: ERROR if in interrupt contexte, otherwise OK
 */

#define INT_RESTRICT()          ((intCnt > 0) ? ERROR : OK)

#endif /* _intArchLib_h */

