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

/* hashLib.h - Hash table private library header */

#ifndef _hashLibP_h
#define _hashLibP_h

#include <vmx.h>
#include <util/sllLib.h>
#include <util/classLib.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct hashtable
{
    OBJ_CORE objCore;          /* Object class */
    int numElements;           /* Number of elements */
    FUNCPTR cmpFunc;           /* Compare function */
    FUNCPTR keyFunc;           /* Key function */
    int keyArg;                /* Argument */
    SL_LIST *pHashTable;       /* List of buckets */
} HASH_TABLE;

typedef SL_NODE HASH_NODE;
typedef HASH_TABLE *HASH_ID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _hashLibP_h */

