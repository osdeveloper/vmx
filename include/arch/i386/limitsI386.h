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

/* limitsI386.h - Limits header */

#ifndef _limitsI386_h
#define _limitsI386_h

#define CHAR_BIT        8

#define SCHAR_MAX       127
#define SCHAR_MIN       (-128)

#define UCHAR_MAX       255
#define UCHAR_MIN       0

#define CHAR_MAX        UCHAR_MAX
#define CHAR_MIN        0

#define SHRT_MAX        32767
#define SHRT_MIN        (-32768)

#define USHRT_MAX       65536
#define USHRT_MIN       0

#define INT_MAX         2147483647
#define INT_MIN         (-INT_MAX - 1)

#define UINT_MAX        4294967295U
#define UINT_MIN        0

#define LONG_MAX        2147483647L
#define LONG_MIN        (-LONG_MAX - 1L)

#define ULONG_MAX       4294967295UL
#define ULONG_MIN       0

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _limitsI386_h */

