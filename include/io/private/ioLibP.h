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

#ifndef __IOLIBP_H
#define __IOLIBP_H

#include <stdarg.h>
#include <io/iosLib.h>

/******************************************************************************
 * openConnect - Generic open connection function
 *
 * RETURNS: Status from I/O open function
 */

int openConnect(
    DEV_HEADER *pDevHeader,
    char *filename,        
    va_list args
    );

/******************************************************************************
 * creatConnect - Generic create function
 *
 * RETURNS: Status from I/O creat function
 */

int creatConnect(
    DEV_HEADER *pDevHeader,
    char *filename,
    va_list args
    );

/******************************************************************************
 * removeConnect - Generic remove function
 *
 * RETURNS: Status from I/O delete function
 */

int removeConnect(
    DEV_HEADER *pDevHeader,
    char *filename,
    va_list args
    );

/******************************************************************************
 * ioConnect - Generic I/O connect function
 *
 * RETURNS: ERROR on failure, other on success
 */

int ioConnect(
    FUNCPTR funcInternal,
    const char *filename,
    ...
    );

#endif

