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

/* sigArchLib.h - Support for software signals */

#ifndef _sigArchLib_h
#define _sigArchLib_h

#include <vmx.h>
#include <arch/regs.h>
#include <os/private/sigLibP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Imports */

IMPORT struct sigfaulttable _sigfaulttable[];

/* Functions */

/*****************************************************************************
 * _sigCtxSave - Save current task context
 *
 * RETURNS: Zero
 */

int _sigCtxSave(
    REG_SET *pRegSet
    );

/*****************************************************************************
 * _sigCtxLoad - Load task context
 *
 * RETURNS: N/A
 */

void _sigCtxLoad(
    REG_SET *pRegSet
    );

/****************************************************************************
 * _sigCtxRetValueSet - Set return value of context
 *
 * RETURNS: N/A
 */

void _sigCtxRetValueSet(
    REG_SET *pRegSet,
    int      val
    );

/*****************************************************************************
 * _sigCtxStackEnd - Get end of stack for context
 *
 * RETURNS: Pointer to stack end
 */

void* _sigCtxStackEnd(
    const REG_SET *pRegSet
    );

/*****************************************************************************
 * _sigCtxSetup - Setup context
 *
 * RETURNS: N/A
 */

void _sigCtxSetup(
    REG_SET *pRegSet,
    void    *pStackBase,
    void   (*taskEntry)(),
    int     *pArgs
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _sigArchLib_h */

