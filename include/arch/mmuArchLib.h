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

/* mmuLib.h - Memory mapping unit */

#ifndef _mmuArchLib_h
#define _mmuArchLib_h

#include <ostool/moduleNumber.h>

#include <types/vmxCpu.h>

#if      CPU_FAMILY==I386
#include <arch/i386/mmuI386Lib.h>
#endif

#define S_mmuLib_NOT_INSTALLED           (M_mmuLib | 0x0001)
#define S_mmuLib_UNSUPPORTED_PAGE_SIZE   (M_mmuLib | 0x0002)
#define S_mmuLib_UNABLE_TO_GET_PTE       (M_mmuLib | 0x0003)
#define S_mmuLib_PAGE_NOT_PRESENT        (M_mmuLib | 0x0004)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUANGE */

#endif /* _mmuArchLib_h */

