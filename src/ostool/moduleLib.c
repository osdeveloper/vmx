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

/* moduleLib.c - Object code module library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <util/dllLib.h>
#include <util/cksumLib.h>
#include <vmx/semLib.h>
#include <os/classLib.h>
#include <os/errnoLib.h>
#include <os/memPartLib.h>
#include <os/pathLib.h>
#include <ostool/moduleLib.h>

/* Types */
typedef struct
 {
    DL_NODE node;
    FUNCPTR func;
} MODULE_HOOK;

/* Locals */
LOCAL BOOL      moduleLibInstalled = FALSE;
LOCAL OBJ_CLASS moduleClass;
LOCAL DL_LIST   moduleList;
LOCAL SEM_ID    moduleListSem;
LOCAL SEM_ID    moduleSegSem;

LOCAL BOOL      moduleCreateHookInstalled = FALSE;
LOCAL DL_LIST   moduleCreateHookList;
LOCAL SEM_ID    moduleCreateHookSem;

LOCAL BOOL moduleSegVerify(
    SEGMENT_ID segId
    );

LOCAL BOOL moduleVerify(
    MODULE_ID modId,
    int       options
    );

LOCAL STATUS moduleInsert(
    MODULE_ID modId
    );

/* Globals */
CLASS_ID moduleClassId         = &moduleClass;
int moduleListSemOptions       = SEM_DELETE_SAFE;
int moduleSegSemOptions        = SEM_DELETE_SAFE;
int moduleCreateHookSemOptions = SEM_DELETE_SAFE;

/* Functions */

/******************************************************************************
 * moduleLibInit - Inititalize module library
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleLibInit(
    void
    )
{
    STATUS status;

    /* Check if installed */
    if (moduleLibInstalled == TRUE)
    {
        status = OK;
    }
    else
    {
        /* Inititalize list */
        dllInit(&moduleList);

        /* Inititalize list semaphore */
        moduleListSem = semMCreate(moduleListSemOptions);
        if (moduleListSem == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Inititalize module segment semaphore */
            moduleSegSem = semMCreate(moduleSegSemOptions);
            if (moduleSegSem == NULL)
            {
                semDelete(moduleListSem);
                status = ERROR;
            }
            else
            {
                /* Initialize module create hook list */
                dllInit(&moduleCreateHookList);

                /* Initialize moudle create hook semaphore */
                moduleCreateHookSem = semMCreate(moduleCreateHookSemOptions);
                if (moduleCreateHookSem == NULL)
                {
                    semDelete(moduleSegSem);
                    semDelete(moduleListSem);
                    status = ERROR;
                }
                else
                {
                    /* Mark module create hooks as installed */
                    moduleCreateHookInstalled = TRUE;

                    /* Initialize object class */
                    if (classInit(
                            moduleClassId,
                            sizeof(MODULE),
                            OFFSET(MODULE, objCore),
                            memSysPartId,
                            (FUNCPTR) moduleCreate,
                            (FUNCPTR) moduleInit,
                            (FUNCPTR) moduleDestroy
                            ) != OK)
                    {
                        status = ERROR;
                    }
                    else
                    {
                        /* Mark module library as installed */
                        moduleLibInstalled = TRUE;
                        status = OK;
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * moduleCreate - Create module
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleCreate(
    char *name,
    int   format,
    int   flags
    )
{
    MODULE_ID modId;

    /* Allocate object */
    modId = objAlloc(moduleClassId);
    if (modId == NULL)
    {
        modId = NULL;
    }
    else
    {
        /* Inititalize module */
        if (moduleInit(modId, name, format, flags) != OK)
        {
            objFree(moduleClassId, modId);
            modId = NULL;
        }
    }

    return modId;
}

/******************************************************************************
 * moduleInit - Initialize module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleInit(
    MODULE_ID  modId,
    char      *name,
    int        format,
    int        flags
    )
{
    static unsigned int moduleGroupNum = 1;

    STATUS       status;
    DL_NODE     *pNode;
    MODULE_ID    oldMod;
    MODULE_ID    lastMod;
    MODULE_HOOK *createHook;

    /* Split path and filename */
    pathSplit(name, modId->path, modId->name);

    semTake(moduleListSem, WAIT_FOREVER);

    /* If module with same name */
    oldMod = moduleFindByName(modId->name);
    if (oldMod != NULL)
    {
        oldMod->flags |= MODULE_REPLACED;
    }

    /* Get last module */
    pNode = DLL_TAIL(&moduleList);
    if (pNode != NULL)
    {
        lastMod = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));
        moduleGroupNum = lastMod->group;
        moduleGroupNum++;
    }

    /* If group max not reached */
    if (moduleGroupNum < MODULE_GROUP_MAX)
    {
        modId->group = moduleGroupNum++;
        dllAdd(&moduleList, &modId->moduleNode);
        status = OK;
    }
    else
    {
        if (moduleInsert(modId) != OK)
        {
            fprintf(stderr, "No free group number. Abort load operation.\n");
            errnoSet(S_moduleLib_MAX_MODULES_LOADED);
            status = ERROR;
        }
        else
        {
            status = OK;
        }
    }

    semGive(moduleListSem);

    if (status == OK)
    {
        /* Setup struct */
        modId->flags  = flags;
        modId->format = format;
        modId->ctors  = NULL;
        modId->dtors  = NULL;

        /* Initialize segment list */
        dllInit(&modId->segmentList);

        /* Inititalize object class */
        objCoreInit(&modId->objCore, moduleClassId);

        semTake(moduleCreateHookSem, WAIT_FOREVER);

        /* For all module create hooks */
        for (createHook = (MODULE_HOOK *) DLL_HEAD(&moduleCreateHookList);
             createHook != NULL;
             createHook = (MODULE_HOOK *) DLL_NEXT((DL_NODE *) createHook))
        {
            (*createHook->func)(modId);
        }

        semGive(moduleCreateHookSem);
    }

    return status;
}

/******************************************************************************
 * moduleDestroy - Destroy module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleDestroy(
    MODULE_ID modId,
    BOOL      dealloc
    )
{
    STATUS     status;
    SEGMENT_ID segId;

    /* Verify object class */
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Terminate object */
        objCoreTerminate(&modId->objCore);

        /* Remove from list */
        semTake(moduleListSem, WAIT_FOREVER);
        dllRemove(&moduleList, &modId->moduleNode);
        semGive(moduleListSem);

        /* Deallocate if requested */
        if (dealloc == TRUE)
        {
            objFree(moduleClassId, modId);
        }

        status = OK;
    }

    return status;
}

/******************************************************************************
 * moduleTerminate - Terminate module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleTerminate(
    MODULE_ID modId
    )
{
    return moduleDestroy(modId, FALSE);
}

/******************************************************************************
 * moduleDelete - Delete module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleDelete(
    MODULE_ID modId
    )
{
    return moduleDestroy(modId, TRUE);
}

/******************************************************************************
 * moduleIdFigure - Get module id from module name or id
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleIdFigure(
    ARG modNameOrId
    )
{
    MODULE_ID modId;

    if (modNameOrId == (ARG) 0)
    {
        modId = NULL;
    }
    else
    {
        /* If not correct object class */
        if (OBJ_VERIFY(modNameOrId, moduleClassId) != OK)
        {
            /* Find by name */
            modId = moduleFindByName(modNameOrId);
            if (modId == NULL)
            {
                errnoSet(S_moduleLib_MODULE_NOT_FOUND);
                modId = NULL;
            }
        }
        else
        {
            modId = (MODULE_ID) modNameOrId;
        }
    }

    return modId;
}

/******************************************************************************
 * moduleIdListGet - Get a list of modules
 *
 * RETURNS: Number of items
 */

int moduleIdListGet(
    MODULE_ID idList[],
    int       max
    )
{
    DL_NODE   *pNode;
    MODULE_ID  modId;
    int        i;

    semTake(moduleListSem, WAIT_FOREVER);

    /* For all modules */
    for (pNode = DLL_HEAD(&moduleList), i = 0;
         (pNode != NULL) && (i < max);
         pNode = DLL_NEXT(pNode), i++)
    {
        /* Get module id */
        modId = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));
        idList[i] = modId;
    }

    semGive(moduleListSem);

    return i;
}

/******************************************************************************
 * moduleNameGet - Get module name
 *
 * RETURNS: Pointer to module name or NULL
 */

char* moduleNameGet(
    MODULE_ID modId
    )
{
    char *name;
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        name = NULL;
    }
    else
    {
        name = modId->name;
    }

    return name;
}

/******************************************************************************
 * moduleGroupGet - Get module group
 *
 * RETURNS: Group number or zero
 */

int moduleGroupGet(
    MODULE_ID modId
    )
{
    int group;

    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        group = 0;
    }
    else
    {
        group = ((int) modId->group);
    }

    return group;
}

/******************************************************************************
 * moduleFlagsGet - Get module flags
 *
 * RETURNS: Flags or zero
 */

int moduleFlagsGet(
    MODULE_ID modId
    )
{
    int flags;
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        flags = 0;
    }
    else
    {
        flags = modId->flags;
    }

    return flags;
}

/******************************************************************************
 * moduleFormatGet - Get module format
 *
 * RETURNS: Format or zero
 */

int moduleFormatGet(
    MODULE_ID modId
    )
{
    int format;
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        format = 0;
    }
    else
    {
        format =  modId->format;
    }

    return format;
}

/******************************************************************************
 * moduleFindByName - Find module by name
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByName(
    char *name
    )
{
    DL_NODE   *pNode;
    char       mPath[NAME_MAX];
    char       mName[PATH_MAX];
    MODULE_ID  modId = NULL;

    /* Split name into directory and name */
    pathSplit(name, mPath, mName);

    semTake(moduleListSem, WAIT_FOREVER);

    /* For all modules */
    for (pNode = DLL_TAIL(&moduleList);
         pNode != NULL;
         pNode = DLL_PREV(pNode))
    {
        /* Get module id */
        modId = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));

        /* If match */
        if (strcmp(modId->name, mName) == 0)
        {
            break;
        }
    }

    semGive(moduleListSem);

    return modId;
}

/******************************************************************************
 * moduleFindByAndPath - Find module by name and path
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByNameAndPath(
    char *name,
    char *path
    )
{
    DL_NODE   *pNode;
    MODULE_ID  modId = NULL;

    semTake(moduleListSem, WAIT_FOREVER);

    /* For all modules */
    for (pNode = DLL_TAIL(&moduleList);
         pNode != NULL;
         pNode = DLL_PREV(pNode))
    {
        /* Get module id */
        modId = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));

        /* If match */
        if ((strcmp(modId->name, name) == 0) &&
            (strcmp(modId->path, path) == 0))
        {
            break;
        }
    }

    semGive(moduleListSem);

    return modId;
}

/******************************************************************************
 * moduleFindByGroup - Find module by group
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByGroup(
    int group
    )
{
    DL_NODE   *pNode;
    MODULE_ID  modId = NULL;

    semTake(moduleListSem, WAIT_FOREVER);

    /* For all modules */
    for (pNode = DLL_TAIL(&moduleList);
         pNode != NULL;
         pNode = DLL_PREV(pNode))
    {
        /* Get module id */
        modId = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));

        /* If match */
        if (group == (int) modId->group)
        {
            break;
        }
    }

    semGive(moduleListSem);

    return modId;
}

/******************************************************************************
 * moduleEach - Run function for each module
 *
 * RETURNS: MODULE_ID where it ended or NULL
 */

MODULE_ID moduleEach(
    FUNCPTR func,
    ARG     arg
    )
{
    DL_NODE   *pNode;
    MODULE_ID  modId = NULL;

    semTake(moduleListSem, WAIT_FOREVER);

    /* For all modules */
    for (pNode = DLL_TAIL(&moduleList);
         pNode != NULL;
         pNode = DLL_PREV(pNode))
    {
        /* Get module id */
        modId = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));

        /* If function returns false */
        if (( *func)(modId, arg) == FALSE)
        {
            break;
        }
    }

    semGive(moduleListSem);

    return modId;
}

/******************************************************************************
 * moduleInfoGet - Get module information
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleInfoGet(
    MODULE_ID    modId,
    MODULE_INFO *pModInfo
    )
{
    STATUS status;

    /* Verify object class */
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Copy name */
        strcpy(pModInfo->name, modId->name);

        /* Copy group and format */
        pModInfo->format = modId->format;
        pModInfo->group  = (int) modId->group;

        /* Get segment info */
        status = moduleSegInfoGet(modId, &pModInfo->segInfo);
    }

    return status;
}

/******************************************************************************
 * moduleCheck - Check all modules
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCheck(
    int options
    )
{
    STATUS status;

    if (moduleEach((FUNCPTR) moduleVerify, (ARG) options) == NULL)
    {
        status = OK;
    }
    else
    {
        status = ERROR;
    }

    return status;
}

/******************************************************************************
 * moduleCreateHookAdd - Add module create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCreateHookAdd(
    FUNCPTR func
    )
{
    STATUS       status;
    MODULE_HOOK *pHook;

    if (moduleCreateHookInstalled != TRUE)
    {
        status = ERROR;
    }
    else
    {
        /* Allocate module hook struct */
        pHook = (MODULE_HOOK *) malloc(sizeof(MODULE_HOOK));
        if (pHook == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Setup struct */
            pHook->func = func;

            /* Insert hook in list */
            semTake(moduleCreateHookSem, WAIT_FOREVER);
            dllAdd(&moduleCreateHookList, &pHook->node);
            semGive(moduleCreateHookSem);

            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * moduleCreateHookDelete - Delete module create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCreateHookDelete(
    FUNCPTR func
    )
{
    MODULE_HOOK *pNode;
    STATUS       status = ERROR;

    /* For all hooks */
    for (pNode = (MODULE_HOOK *) DLL_HEAD(&moduleCreateHookList);
         pNode != NULL;
         pNode = (MODULE_HOOK *) DLL_NEXT((DL_NODE *) pNode))
    {
        /* If hook match */
        if (pNode->func == func)
        {
            dllRemove(&moduleCreateHookList, (DL_NODE *) pNode);
            free(pNode);
            status = OK;
            break;
        }
    }

    if (status != OK)
    {
        errnoSet(S_moduleLib_HOOK_NOT_FOUND);
    }

    return status;
}

/******************************************************************************
 * moduleSegAdd - Add segment to module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleSegAdd(
    MODULE_ID  modId,
    int        type,
    void      *location,
    int        length,
    int        flags
    )
{
    STATUS     status;
    SEGMENT_ID segId;

    /* Verify object class */
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Allocate segment */
        segId = (SEGMENT_ID) malloc(sizeof(SEGMENT));
        if (segId == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Setup struct */
            segId->address  = location;
            segId->size     = length;
            segId->type     = type;
            segId->flags    = flags;
            segId->checksum = checksum(location, length);

            /* Add segment to segment list */
            semTake(moduleSegSem, WAIT_FOREVER);
            dllAdd(&modId->segmentList, &segId->segmentNode);
            semGive(moduleSegSem);

            status = OK;
        }
    }

    return status;
}

/******************************************************************************
 * moduleSegGet - Get segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegGet(
    MODULE_ID modId
    )
{
    SEGMENT_ID segId;

    /* Verify object class */
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        segId = NULL;
    }
    else
    {
        /* Get segment */
        semTake(moduleSegSem, WAIT_FOREVER);
        segId = (SEGMENT_ID) dllGet(&modId->segmentList);
        semGive(moduleSegSem);
    }

    return segId;
}

/******************************************************************************
 * moduleSegFirst - Get first segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegFirst(
    MODULE_ID modId
    )
{
    SEGMENT_ID segId;

    /* Verify object class */
    if (OBJ_VERIFY(modId, moduleClassId) != OK)
    {
        segId = NULL;
    }
    else
    {
        /* Get segment */
        semTake(moduleSegSem, WAIT_FOREVER);
        segId = (SEGMENT_ID) DLL_HEAD(&modId->segmentList);
        semGive(moduleSegSem);
    }

    return segId;
}

/******************************************************************************
 * moduleSegNext - Get next segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegNext(
    SEGMENT_ID segId
    )
{
    SEGMENT_ID nextSeg;

    /* Get segment */
    semTake(moduleSegSem, WAIT_FOREVER);
    nextSeg = (SEGMENT_ID) DLL_NEXT((DL_NODE *) segId);
    semGive(moduleSegSem);

    return nextSeg;
}

/******************************************************************************
 * moduleSegEach - Execute function on all segments in module
 *
 * RETURNS: SEGMENT_ID where it ended or NULL
 */

SEGMENT_ID moduleSegEach(
    MODULE_ID modId,
    FUNCPTR   func,
    ARG       arg
    )
{
    SEGMENT_ID segId;

    semTake(moduleSegSem, WAIT_FOREVER);

    /* For all segments */
    for (segId = moduleSegFirst(modId);
         segId != NULL;
         segId = moduleSegNext(segId))
    {
        /* If function returns false */
        if ((*func)(segId, modId, arg) == FALSE)
        {
            break;
        }
    }

    semGive(moduleSegSem);

    return segId;
}

/******************************************************************************
 * moduleSegInfoGet - Get module segment info
 *
 * RETURNS: OK
 */

STATUS moduleSegInfoGet(
    MODULE_ID        modId,
    MODULE_SEG_INFO *pSegInfo
    )
{
    SEGMENT_ID segId;

    /* Clear */
    memset(pSegInfo, 0, sizeof(MODULE_SEG_INFO));

    semTake(moduleSegSem, WAIT_FOREVER);

    /* For all segments */
    for (segId = moduleSegFirst(modId);
         segId != NULL;
         segId = moduleSegNext(segId))
    {
        /* Select segment type */
        switch (segId->type)
        {
            case SEGMENT_TEXT:
                pSegInfo->textAddr = segId->address;
                pSegInfo->textSize = segId->size;
                break;

            case SEGMENT_DATA:
                pSegInfo->dataAddr = segId->address;
                pSegInfo->dataSize = segId->size;
                break;

            case SEGMENT_BSS:
                pSegInfo->bssAddr = segId->address;
                pSegInfo->bssSize = segId->size;
                break;

            default:
                break;
        }
    }

    semGive(moduleSegSem);

    return OK;
}

/******************************************************************************
 * moduleSegVerify - Verify module segment checksum
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL moduleSegVerify(
    SEGMENT_ID segId
    )
{
    BOOL           result;
    unsigned short sum;

    /* Calculate checksum */
    sum = checksum(segId->address, segId->size);

    /* Check sum */
    if (sum != segId->checksum)
    {
        errnoSet(S_moduleLib_BAD_CHECKSUM);
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    return result;
}

/******************************************************************************
 * moduleVerify - Verify module
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL moduleVerify(
    MODULE_ID modId,
    int       options
    )
{
    SEGMENT_ID segId;
    BOOL       result = TRUE;

    /* Default options */
    if (options == 0)
    {
        options = MODCHECK_TEXT;
    }

    /* For all segments */
    for (segId = moduleSegFirst(modId);
         segId != NULL;
         segId = moduleSegNext(segId))
    {
        /* If segment check in options */
        if (segId->type & options)
        {
            /* If check fails */
            if (moduleSegVerify(segId) != TRUE)
            {
                /* If print */
                if ((options & MODCHECK_NOPRINT) == 0)
                {
                    fprintf(
                        stderr,
                        "Checksum error in segment type %d, ",
                        segId->type
                        );
                    fprintf(
                        stderr,
                        "module %#x (%s)\n",
                        (int) modId,
                        modId->name
                        );
                }

                result = FALSE;
            }
        }
    }

    return result;
}

/******************************************************************************
 * moduleInsert - Insert module
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS moduleInsert(
    MODULE_ID modId
    )
{
    STATUS     status;
    DL_NODE   *pNode;
    MODULE_ID  currMod;
    MODULE_ID  nextMod;

    /* For all modules */
    for (pNode = DLL_TAIL(&moduleList);
         pNode != NULL;
         pNode = DLL_PREV(pNode))
    {
        /* Get module id */
        currMod = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));

        /* Get next module */
        pNode = DLL_NEXT(pNode);
        nextMod = (MODULE_ID) ((char *) pNode - OFFSET(MODULE, moduleNode));
        if (nextMod == NULL)
        {
            break;
        }

        /* If module group greater */
        if (nextMod->group > (currMod->group + 1))
        {
            /* Insert */
            dllInsert(&moduleList, &currMod->moduleNode, &modId->moduleNode);
            modId->group = currMod->group + 1;
            break;
        }
    }

    if (nextMod == NULL)
    {
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

