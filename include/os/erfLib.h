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

#ifndef __ERFLIB_H
#define __ERFLIB_H

#include <ostool/moduleNumber.h>

#define S_erfLib_INVALID_PARAMETER   (M_erfLib | 0x0001)
#define S_erfLib_HANDLER_NOT_FOUND   (M_erfLib | 0x0002)
#define S_erfLib_TOO_MANY_USER_CATS  (M_erfLib | 0x0003)
#define S_erfLib_TOO_MANY_USER_TYPES (M_erfLib | 0x0004)

#define ERF_ASYNC_PROCESS    0x00000001
#define ERF_SYNC_PROCESS     0x00000002

/***************************************************************************
 *
 * erfLibInit - Initialize event reporting framework
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfLibInit (
    int  maxCategories,
    int  maxTypes
    );

/***************************************************************************
 *
 * erfHandlerRegister - Register event handler
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfHandlerRegister (
    int     eventCategory,
    int     eventType,
    void    (*func)(int, int, void *, void*),
    void *  pUserData,
    int     flags
    );

/***************************************************************************
 *
 * erfHandlerUnregister - Unregister event handler
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfHandlerUnregister (
    int     eventCategory,
    int     eventType,
    void    (*func)(int, int, void *, void*),
    void *  pUserData
    );

/***************************************************************************
 *
 * erfEventRaise - raise an event
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfEventRaise (
    int     eventCategory,
    int     eventType,
    int     processType,
    void *  pEventData,
    void    (*freeFunc)(void *)
    );

/***************************************************************************
 *
 * erfCategoryAllocate - Allocate event category
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfCategoryAllocate (
    int *  pCategory
    );

/***************************************************************************
 *
 * erfTypeAllocate - Allocate event type
 *
 * RETURNS: OK on success, ERROR otherwise
 */

STATUS erfTypeAllocate (
    int    category,
    int *  pType
    );

#endif

