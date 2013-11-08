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

/* taskLib.h - Task context switching stuff */

#ifndef _taskArchLib_h
#define _taskArchLib_h

#include <types/vmxCpu.h>
#include <vmx.h>
#include <arch/regs.h>

#if      CPU_FAMILY==I386
#include <arch/i386/taskI386Lib.h>
#endif

#ifndef _ASMLANGUAGE

IMPORT REG_INDEX taskRegName[];

#endif /* _ASMLANGUANGE */

#endif /* _taskArchLib_h */

