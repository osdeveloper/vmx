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

/* classShow.h - Class show utilities header */

#ifndef _classShow_h
#define _classShow_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * classShowInit - Initialize class show utilities
 *
 * RETURNS: OK or ERROR
 */

STATUS classShowInit(
    void
    );

/******************************************************************************
 * classShow - Show class status
 *
 * RETURNS: OK or ERROR
 */

STATUS classShow(
    CLASS_ID classId,
    int mode
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _classShow_h */

