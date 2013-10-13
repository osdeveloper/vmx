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

/* vmxCpu.h - Define CPU family from type */

#ifndef _vmxCpu_h
#define _vmxCpu_h

#ifdef __cplusplus
extern "C" {
#endif

/* CPU types */
#define	I386                   80                /* CPU family */
#define I80386                 81
#define I80486                 82
#define PENTIUM                83
#define PENTIUM2               84
#define PENTIUM3               85
#define PENTIUM4               86

/* Define CPU_FAMILY */
#if     (CPU==I80386 || \
	 CPU==I80486 || \
	 CPU==PENTIUM || \
	 CPU==PENTIUM2 || \
	 CPU==PENTIUM3 || \
	 CPU==PENTIUM4)
#define CPU_FAMILY             I386
#endif /* CPU_FAMILY==I386 */

/* Check if CPU family was define correctly */
#if !defined(CPU) || !defined(CPU_FAMILY)
#error CPU not defined correctly
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _vmxCpu_h */

