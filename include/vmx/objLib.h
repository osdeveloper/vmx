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

/* objLib.h - Object Library header */

#ifndef _objLib_h
#define _objLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* includes */

#include <tools/moduleNumber.h>
#include <vmx.h>
#include <vmx/classLib.h>
#include <vmx/private/objLibP.h>
#include <vmx/errnoLib.h>

/* defines */

#define S_objLib_NOT_INSTALLED  (M_objLib | 0x0001)
#define S_objLib_ID_ERROR       (M_objLib | 0x0002)
#define S_objLib_UNAVAILABLE    (M_objLib | 0x0003)
#define S_objLib_DELETED        (M_objLib | 0x0004)
#define S_objLib_TIMEOUT        (M_objLib | 0x0005)

/* typedefs */

typedef struct obj_core *OBJ_ID;

/* macros */

/******************************************************************************
 * CLASS_TO_OBJ_ID - Get object core from class object
 *
 * RETURNS: Pointer to object core
 */

#define CLASS_TO_OBJ_ID(objId, classId)                                       \
  (OBJ_ID) ( (int) (objId) + (classId)->coreOffset )

/******************************************************************************
 * CLASS_RESOLVE - Resolve class
 *
 * RETURNS: Class id or pointer to object core
 */

#define CLASS_RESOLVE(objId, classId)                                         \
  ( (CLASS_TO_OBJ_ID(objId, classId))->pObjClass == (classId) )               \
    ? (classId)                                                               \
    : (objId)->pObjClass

/******************************************************************************
 * OBJ_VERIFY - Verify object class
 *
 * RETURNS: OK or ERROR
 */

#define OBJ_VERIFY(objId, classId)                                            \
 ( (CLASS_TO_OBJ_ID(objId, classId))->pObjClass == (classId) )                \
   ? OK                                                                       \
   : (errno = S_objLib_ID_ERROR, ERROR)

/* functions */

/******************************************************************************
 * objAllocPad - Allocated object with padding bytes
 *
 * RETURNS: Pointer to object, or NULL
 */

void* objAllocPad(
    CLASS_ID  classId,
    unsigned  nPadBytes,
    void    **ppPad
    );

/******************************************************************************
 * objAlloc - Allocated an object
 *
 * RETURNS: Pointer to object, or NULL
 */

void* objAlloc(
    CLASS_ID classId
    );

/******************************************************************************
 * objShow - Call objects show method
 *
 * RETURNS: OK or ERROR
 */

STATUS objShow(
    CLASS_ID classId,
    OBJ_ID   objId,
    int      mode
    );

/******************************************************************************
 * objFree - Free object memory
 *
 * RETURNS: OK or ERROR
 */

STATUS objFree(
    CLASS_ID classId,
    void    *pObj
    );

/******************************************************************************
 * objCoreInit - Initialize object core
 *
 * RETURNS: N/A
 */

void objCoreInit(
    OBJ_CORE *pObjCore,
    CLASS_ID  pObjClass
    );


/******************************************************************************
 * objCoreTerminate - Terminate object core
 *
 * RETURNS: N/A
 */

void objCoreTerminate(
    OBJ_CORE *pObjCore
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ASMLANGUAGE */

#endif /* _objLib_h */

