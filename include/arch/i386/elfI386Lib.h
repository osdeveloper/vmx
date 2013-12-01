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

/* elfI386Lib.h - Elf relocation header */

#ifndef _elfI386Lib_h
#define _elfI386Lib_h

#include <vmx.h>
#include <elf.h>

/* Machine type */
#define EM_ARCH_MACHINE         EM_386  /* Machine type */

/* Relocation options */
#define R_386_NONE              0       /* None */
#define R_386_32                1       /* Direct 32-bit */
#define R_386_PC32              2       /* Pc relative address */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * elfArchLibInit - Initialize elf arch library
 *
 * RETURNS: OK
 */

STATUS elfArchLibInit(
    FUNCPTR *pElfVerifyFunc,
    FUNCPTR *pElfSegRelFunc
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _elfI386Lib_h */

