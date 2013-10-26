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

/* typesI386.h - System types */

#ifndef _typesI386_h
#define _typesI386_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Signed types */
typedef char                    int8_t;
typedef short                   int16_t;
typedef int                     int32_t;
typedef long long               int64_t;

/* Unsigned types */
typedef unsigned char           u_int8_t;
typedef unsigned short          u_int16_t;
typedef unsigned int            u_int32_t;
typedef unsigned long long      u_int64_t;

/* Pointer types */
typedef char                   *addr_t;

/* Misc types */
typedef unsigned int            size_t;
typedef int                     ssize_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _typesI386_h */

