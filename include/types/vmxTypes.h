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

/* Define I/O type */
#define NONE                   (-1)
#define EOS                    '\0'
#define EOF                    EOS

/* Define return types */
#define OK                     ( 0)
#define ERROR                  (-1)

/* Define wait type */
#define WAIT_NONE              0x00000000
#define WAIT_FOREVER           0xffffffff

#ifndef _ASMLANGUAGE

#include <sys/types.h>

/* Define scope types */
#define FAST                   register
#define IMPORT                 extern
#define LOCAL                  static

/* Macros */
/******************************************************************************
 * OFFSET - Get byte offset to member in structure
 *
 * RETURNS: Offset to datastruct member
 */

#define OFFSET(structure, member)                                             \
    ((int) &(((structure *) 0) -> member))

/******************************************************************************
 * NELEMENTS - Get number of elements in an array
 *
 * RETURNS: Number of element is array
 */

#define NELEMENTS(array)                                                      \
    (sizeof (array) / sizeof ((array) [0]))

#define ROUND_UP(x, align)     (((int)(x) + (align - 1)) & ~(align - 1))
#define ROUND_DOWN(x, align)   ((int)(x) & ~(align - 1))
#define ALIGNED(x, align)      (((int)(x) & (align - 1)) == 0)

#define MEM_ROUND_UP(x)        ROUND_UP(x, _ALLOC_ALIGN_SIZE)
#define MEM_ROUND_DOWN(x)      ROUND_DOWN(x, _ALLOC_ALIGN_SIZE)
#define STACK_ROUND_UP(x)      ROUND_UP(x, _STACK_ALIGN_SIZE)
#define STACK_ROUND_DOWN(x)    ROUND_DOWN(x, _ALLOC_ALIGN_SIZE)
#define MEM_ALIGNED(x)         ALIGNED(x, _ALLOC_ALIGN_SIZE)

#define MSB(x)                 (((x) >> 8) & 0xff)
#define LSB(x)                 ((x) & 0xff)

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

