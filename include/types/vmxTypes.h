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
 */

/* vmxTypes.h - Os specific types */

#ifndef _vmxTypes_h
#define _vmxTypes_h

/* Define boolean types */
#if     !defined(FALSE) || (FALSE!=0)
#define FALSE 0
#endif

#if     !defined(TRUE) || (TRUE!=1)
#define TRUE 1
#endif

/* Define return types */
#define OK                    ( 0)
#define ERROR                 (-1)

#ifndef _ASMLANGUAGE

#include <sys/types.h>

/* Define scope types */
#define FAST                   register
#define IMPORT                 extern
#define LOCAL                  static

/* Macros */
#define OFFSET(structure, member)        /* byte offset to member */\
            ((int) &(((structure *) 0 ) -> member))

#define ROUND_UP(x, align)     (((int)(x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)   ((int)(x) & ~(align - 1))
#define ALIGNED(x, align)      (((int)(x) & (align - 1)) == 0)

#ifdef __cplusplus
extern "C" {
#endif

typedef int                     BOOL;
typedef int                     STATUS;
typedef int                     ARGINT;

typedef void                    VOID;

typedef void                   *ARG;

#ifdef __cplusplus
typedef int                     (*FUNCPTR) (...);
typedef void                    (*VOIDFUNCPTR) (...);
typedef double                  (*DBLFUNCPTR) (...);
typedef float                   (*FLTFUNCPTR) (...);
#else
typedef int                     (*FUNCPTR) ();
typedef void                    (*VOIDFUNCPTR) ();
typedef double                  (*DBLFUNCPTR) ();
typedef float                   (*FLTFUNCPTR) ();
#endif

/* Architecture specific types */
#if CPU_FAMILY==I386
typedef u_int8_t INSTR;
#endif /* CPU_FAMILY==I80X86 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _vmxTypes_h */

