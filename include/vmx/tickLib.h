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

/* tickLib.h - Tick library header */

#ifndef _tickLib_h
#define _tickLib_h

#include <sys/types.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * tickLibInit - Initialize kernel tick library
 *
 * RETURNS: N/A
 */

void tickLibInit(
    void
    );

/***************************************************************************
 * tickAnnounce - timer tick handler
 *
 * RETURNS: N/A
 */

void tickAnnounce(
    void
    );

/******************************************************************************
 * tickSet - Set time
 *
 * RETURNS: N/A
 */

void tickSet(
    unsigned long ticks
    );

/******************************************************************************
 * tickGet - Get time
 *
 * RETURNS: N/A
 */

unsigned long tickGet(
    void
    );

/******************************************************************************
 * tick64Set - Set time
 *
 * RETURNS: N/A
 */

void tick64Set(
    u_int64_t ticks
    );

/******************************************************************************
 * tick64Get - Get time
 *
 * RETURNS: N/A
 */

u_int64_t tick64Get(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tickLib_h */

