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

/* cacheI386Lib.h - Architecture dependet cache library header */

#ifndef _cacheI386Lib_h
#define _cacheI386Lib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */

/******************************************************************************
 * cacheI386Reset - Reset cache
 *
 * RETURNS: N/A
 */

void cacheI386Reset(
    void
    );

/******************************************************************************
 * cacheI386Enable - Enable cache
 *
 * RETURNS: N/A
 */

void cacheI386Enable(
    void
    );

/******************************************************************************
 * cacheI386Unlock - Unlock cache
 *
 * RETURNS: N/A
 */

void cacheI386Unlock(
    void
    );

/******************************************************************************
 * cacheI386Disable - Disable cache
 *
 * RETURNS: N/A
 */

void cacheI386Disable(
    void
    );

/******************************************************************************
 * cacheI386Lock - Lock cache
 *
 * RETURNS: N/A
 */

void cacheI386Lock(
    void
    );

/******************************************************************************
 * cacheI386Clear - Clear cache
 *
 * RETURNS: N/A
 */

void cacheI386Clear(
    void
    );

/******************************************************************************
 * cacheI386Flush - Flush cache
 *
 * RETURNS: N/A
 */

void cacheI386Flush(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _cacheI386Lib_h */

