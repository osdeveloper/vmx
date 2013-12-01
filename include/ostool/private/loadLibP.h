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

/* loadLibP.h - Load library private header*/

#ifndef _loadLibP_h
#define _loadLibP_h

#include <vmx.h>
#include <os/symbol.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef void *SYM_ADDR;

typedef struct
{
    char          *symName;               /* Symbol name */
    BOOL           symMatched;            /* TRUE if syms are included */
    void          *pSymAddrBss;           /* Bss symbol address */
    SYM_TYPE       bssSymType;            /* Bss symbol type */
    void          *pSymAddrData;          /* Data symbol address */
    SYM_TYPE       dataSymType;           /* Data symbol type */
} COMMON_INFO;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _loadLibP_h */

