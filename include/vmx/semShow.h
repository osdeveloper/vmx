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

/* semShow.h - Semaphore info show header */

#ifndef _semShow_h
#define _semShow_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * semShowInit - Initialize semaphore info facilities
 *
 * RETURNS: OK or ERROR
 */

STATUS semShowInit(
    void
    );

/******************************************************************************
 * semInfo - Get list of task blocking on a semaphore
 *
 * RETURNS: Number of tasks
 */

int semInfo(
    SEM_ID semId,
    int idList[],
    int max
    );

/******************************************************************************
 * semShow - Show semaphore info
 *
 * RETURNS: OK or ERROR
 */

STATUS semShow(
    SEM_ID semId,
    int mode
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _semShow_h */

