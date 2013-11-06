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

/* symbolP.h - Symbol private header */

#ifndef _symbolP_h
#define _symbolP_h

#include <vmx.h>
#include <util/sllLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef unsigned char SYM_TYPE;         /* Symbol type */

typedef struct
{
    SL_NODE   nameHashNode;             /* Hash node for symbol name */
    char     *name;                     /* Pointer to symbol name string */
    ARG       value;                    /* Symbol value */
    unsigned  short group;              /* Symbol group */
    SYM_TYPE  type;                     /* Symbol type */
} SYMBOL;

typedef SYMBOL *SYMBOL_ID;              /* Symbol id */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _symbolP_h */

