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

/* setjmp.h - Non-local goto */

#ifndef _SETJMP_H
#define _SETJMP_H

#include <arch/regs.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef struct _jmp_buf
{
    REG_SET reg;
    int     extra[3];
} jmp_buf[1];

typedef struct _sigjmp_buf
{
    REG_SET regs;
    int     extra[3];
} sigjmp_buf[1];

/* Functions */

/******************************************************************************
 * longjmp - Do a long goto
 *
 * RETURNS: N/A
 */

void longjmp(
    jmp_buf env,
    int     val
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _SETJMP_H */

