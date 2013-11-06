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

/* symbol.h - Symbols library header */

#ifndef _symbol_h
#define _symbol_h

#include <os/private/symbolP.h>

/* Defines */
/* Symbol type */
#define SYM_UNDF                0x00            /* Undefined */
#define SYM_LOCAL               0x00            /* Local */
#define SYM_GLOBAL              0x01            /* Global */
#define SYM_ABS                 0x02            /* Absolute */
#define SYM_TEXT                0x04            /* Text symbol */
#define SYM_DATA                0x06            /* Data symbol */
#define SYM_BSS                 0x08            /* Bss symbol */
#define SYM_COMM                0x12            /* Common symbol */

/* Symbol masks */
#define SYM_MASK_ALL            0xff            /* All bits set */
#define SYM_MASK_NONE           0x00            /* No bits set */
#define SYM_MASK_EXACT          0x1ff           /* Match symbol pointer exact */

#define SYM_MASK_ANY_TYPE       SYM_MASK_NONE   /* Ignore type */
#define SYM_MASK_EXACT_TYPE     SYM_MASK_ALL    /* Match all parameters */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _symbol_h */

