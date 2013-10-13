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

/* config/x86/config.h - arch configuration settings */

#ifndef _config_h_
#define _config_h_

/* Include arch independant config */
//#include "../all/configAll.h"

#define VM_PAGE_SIZE		4096
#define LOCAL_MEM_LOCAL_ADRS	0x00100000
#define VEC_BASE_ADRS		LOCAL_MEM_LOCAL_ADRS
#define GDT_BASE_OFFSET		(1 * VM_PAGE_SIZE)

#define INCLUDE_VGA
#define INCLUDE_ATKBD
#define INCLUDE_PIC
#define INCLUDE_LIBC

#endif

