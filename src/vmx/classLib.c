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

/* classLib.c - Class library */

#include <stdio.h>
#include <vmx.h>
#include <stdlib.h>

#include <vmx/objLib.h>
#include <vmx/classLib.h>
#include <vmx/errnoLib.h>
#include <os/memPartLib.h>

/* Imports */

/* Locals */
LOCAL OBJ_CLASS rootClass;

/* Globals */
CLASS_ID rootClassId = &rootClass;

/******************************************************************************
 * classLibInit - Initialize root class
 *
 * RETURNS: OK or ERROR
 */

STATUS classLibInit(
    void
    )
{
    STATUS status;

    /* Initialize root class */
    if (classInit(
            rootClassId,
            sizeof(OBJ_CLASS),
            OFFSET(OBJ_CLASS, objCore),
            memSysPartId,
            (FUNCPTR) classCreate,
            (FUNCPTR) classInit,
            (FUNCPTR) classDestroy
            ) != OK)
    {
        /* errno set by classInit() */
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * classCreate - Allocate and initialize class
 *
 * RETURNS: Pointer to CLASS_ID, or NULL
 */

CLASS_ID classCreate(
    unsigned         objSize,
    int              coreOffset,
    struct mem_part *partId,
    FUNCPTR          createMethod,
    FUNCPTR          initMethod,
    FUNCPTR          destroyMethod
    )
{
    CLASS_ID classId;

    classId = (CLASS_ID) objAlloc(rootClassId);
    if (classId != NULL)
    {
        if (classInit(
                classId,
                objSize,
                coreOffset,
                partId,
                createMethod,
                initMethod,
                destroyMethod
                ) != OK)
        {
            /* errno set by classInit() */
            objFree(classId, rootClassId);
            classId = NULL;
        }
    }

    return classId;
}

/******************************************************************************
 * classInit - Initialize an object class
 *
 * RETURNS: OK
 */

STATUS classInit(
    OBJ_CLASS       *pObjClass,
    unsigned         objSize,
    int              coreOffset,
    struct mem_part *partId,
    FUNCPTR          createMethod,
    FUNCPTR          initMethod,
    FUNCPTR          destroyMethod
    )
{
    /* Default memory partition */
    pObjClass->objPartId = memSysPartId;

    /* Record size */
    pObjClass->objSize = objSize;

    /* Set counters to zero */
    pObjClass->objAllocCount              = 0;
    pObjClass->objFreeCount               = 0;
    pObjClass->objInitCount               = 0;
    pObjClass->objTerminateCount          = 0;

    /* Record core offset */
    pObjClass->coreOffset                 = coreOffset;

    /* Set partition */
    pObjClass->objPartId                  = partId;

    /* Record methods */
    pObjClass->createMethod               = createMethod;
    pObjClass->initMethod                 = initMethod;
    pObjClass->destroyMethod              = destroyMethod;
    pObjClass->showMethod                 = NULL;

    /* Make class a valid object */
    objCoreInit(&pObjClass->objCore, rootClassId);

    return OK;
}

/******************************************************************************
 * classShowConnect - Connect a show function to a specified object class
 *
 * RETURNS: OK or ERROR
 */

STATUS classShowConnect(
    CLASS_ID classId,
    FUNCPTR  showMethod
    )
{
    STATUS status;

    /* Verify object class */
    if (OBJ_VERIFY(classId, rootClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Insatll show method */
        classId->showMethod = showMethod;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * classDestroy - Remove class
 *
 * RETURNS: ERROR
 */

STATUS classDestroy(
    CLASS_ID classId
    )
{
    errnoSet(S_classLib_NO_CLASS_DESTROY);

    return ERROR;
}

