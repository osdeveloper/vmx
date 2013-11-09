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

/* usrPage.h - Termina page facilities header */

#ifndef _usrPage_h
#define _usrPage_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * usrPageInit - Initialize page display facilities
 *
 * RETURNS: N/A
 */

void usrPageInit(
    int numLines
    );

/******************************************************************************
 * usrPageReset - Reset line counter for page
 *
 * RETURNS: N/A
 */

void usrPageReset(
    void
    );

/******************************************************************************
 * usrPageNumLinesSet - Set number of lines for a page
 *
 * RETURNS: N/A
 */

void usrPageNumLinesSet(
    int numLines
    );

/******************************************************************************
 * usrPageCheck - Check if max page lines are reached and wait for input
 *
 * RETURNS: TRUE if continue or FALSE if quit
 */

BOOL usrPageCheck(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _usrPage_h */

