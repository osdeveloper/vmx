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

/* memLib.h - Memory library header */

#ifndef _memLib_h
#define _memLib_h

#include <ostool/moduleNumber.h>

#include <vmx.h>
#include <os/memPartLib.h>

#define MEM_BLOCK_CHECK                  0x10
#define MEM_ALLOC_ERROR_LOG_FLAG         0x20
#define MEM_ALLOC_ERROR_SUSPEND_FLAG     0x40
#define MEM_BLOCK_ERROR_LOG_FLAG         0x80
#define MEM_BLOCK_ERROR_SUSPEND_FLAG    0x100

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * memLibInit - Initialize memory library
 *
 * RETURNS: OK
 */

STATUS memLibInit(
    void
    );

/******************************************************************************
 * memPartOptionsSet - Set memory options
 *
 * RETURNS: OK or ERROR
 */

STATUS memPartOptionsSet(
    PART_ID partId,
    unsigned options
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _memLib_h */

