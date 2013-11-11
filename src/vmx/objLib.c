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

/* objLib.c - Object Library */

#include <stdlib.h>
#include <vmx.h>
#include <vmx/errnoLib.h>
#include <vmx/classLib.h>
#include <os/memPartLib.h>
#include <vmx/objLib.h>

/******************************************************************************
 * objAllocPad - Allocated object with padding bytes
 *
 * RETURNS: Pointer to object, or NULL
 */

void* objAllocPad(
    CLASS_ID  classId,
    unsigned  nPadBytes,
    void    **ppPad
    )
{
    void *pObj;

    /* Verify object */
    if (OBJ_VERIFY(classId, rootClassId) != OK)
    {
        errnoSet (S_classLib_INVALID_INSTANCE);
        pObj = NULL;
    }
    else
    {
        /* Allocate memory for object */
        pObj = memPartAlloc(classId->objPartId, classId->objSize + nPadBytes);
        if (pObj != NULL)
        {
            classId->objAllocCount++;
            if (ppPad != NULL)
            {
                *ppPad = (void *) (((char *) pObj) + classId->objSize);
            }
        }
    }

    return pObj;
}

/******************************************************************************
 * objAlloc - Allocated an object
 *
 * RETURNS: Pointer to object, or NULL
 */

void* objAlloc(
    CLASS_ID classId
    )
{
    return objAllocPad(classId, 0, (void **) NULL);
}

/******************************************************************************
 * objShow - Call objects show method
 *
 * RETURNS: OK or ERROR
 */

STATUS objShow(
    CLASS_ID classId,
    OBJ_ID   objId,
    int      mode
    )
{
    STATUS   status;
    CLASS_ID pObjClass;

    /* Resolve object class */
    pObjClass = CLASS_RESOLVE(objId, classId);

    /* Verify object */
    if (OBJ_VERIFY(pObjClass, rootClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* If no show method specified */
        if (pObjClass->showMethod == NULL)
        {
            errnoSet(S_classLib_OBJ_NO_METHOD);
            status = ERROR;
        }
        else
        {
            /* Call function and return */
            status = (*pObjClass->showMethod)(objId, mode);
        }
    }

    return status;
}

/******************************************************************************
 * objFree - Free object memory
 *
 * RETURNS: OK or ERROR
 */

STATUS objFree(
    CLASS_ID classId,
    void    *pObj
    )
{
    STATUS status;

    /* Verify object */
    if (OBJ_VERIFY(classId, rootClassId) != OK)
    {
        /* errno set by OBJ_VERIFY() */
        status = ERROR;
    }
    else
    {
        memPartFree(classId->objPartId, pObj);
        classId->objFreeCount++;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * objCoreInit - Initialize object core
 *
 * RETURNS: N/A
 */

void objCoreInit(
    OBJ_CORE *pObjCore,
    CLASS_ID  pObjClass
    )
{
    /* Initialize object core */
    pObjCore->pObjClass = pObjClass;

    /* Increase intialized objects counter in used class */
    pObjClass->objInitCount++;
}

/******************************************************************************
 * objCoreTerminate - Terminate object core
 *
 * RETURNS: N/A
 */

void objCoreTerminate(
    OBJ_CORE *pObjCore
    )
{
    pObjCore->pObjClass->objTerminateCount++;
    pObjCore->pObjClass = NULL;
}

