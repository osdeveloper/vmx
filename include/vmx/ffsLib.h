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

/* ffsLib.h - find first set bit library */


#ifndef __FFS_LIB_H
#define __FFS_LIB_H

/***************************************************************************
 * ffsMsb - find first set most significant bit
 *
 * 1 indicates the least significant bit, 32 the most.
 *
 * RETURNS: position of most-significant bit, 0 if no bits set
 */

int ffsMsb(
    unsigned  value
    );

/***************************************************************************
 * ffsLsb - find first set least significant bit
 *
 * 1 indicates the least significant bit, 32 the most.
 *
 * RETURNS: position of least-significant bit, 0 if no bits set
 */

int ffsLsb(
    unsigned value
    );

#endif

