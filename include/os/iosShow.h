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

/* iosShow.h - Show I/O devices and drivers header */

#ifndef _iosShow_h
#define _iosShow_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * iosShowInit - Initialize I/O show
 *
 * RETURNS: N/A
 */

void iosShowInit(
    void
    );

/******************************************************************************
 * iosDevShow - Show I/O devices
 *
 * RETURNS: N/A
 */

void iosDevShow(
    void
    );

/******************************************************************************
 * iosFdShow - Show filedesciptors
 *
 * RETURNS: N/A
 */

void iosFdShow(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _iosShow_h */

