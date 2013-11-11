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

/* symLibP.h - Symbol table library private header */

#ifndef _symLibP_h
#define _symLibP_h

#include <util/hashLib.h>
#include <vmx/semLib.h>
#include <os/classLib.h>
#include <os/memPartLib.h>
#include <os/private/symbolP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct symtab
{
    OBJ_CORE  objCore;                  /* Object class */
    HASH_ID   nameHashId;               /* Hash table for symbol names */
    SEMAPHORE symMutex;                 /* Symbol syncronization */
    PART_ID   partId;                   /* Symbol memory partition id */
    BOOL      sameNameOk;               /* Allow duplicate symbol names */
    int       numSymbols;               /* Number of symbols in table */
} SYMTAB;

typedef struct
{
  FUNCPTR func;
  ARG     arg;
} SYM_FUNC_DESC;

typedef SYMTAB *SYMTAB_ID;              /* Symbol table id */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _symLibP_h */

