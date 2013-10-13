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

/* config.h - arch configuration settings */

#ifndef _config_h_
#define _config_h_

/* Onchip interrupt levels */
#define INT_LVL_AUXCLK			15
#define INT_LVL_SYSCLK			12
#define INT_LVL_SCIF			8
#define INT_LVL_SCI			7
#define INT_LVL_TSTAMP			1

#define INCLUDE_LIBC
#define INCLUDE_VIDEO

#endif

