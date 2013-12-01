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

/* moduleLib.h - Module library header */

#ifndef _moduleLib_h
#define _moduleLib_h

#include <ostool/moduleNumber.h>

#define S_moduleLib_MODULE_NOT_FOUND            (M_moduleLib | 0x0001)
#define S_moduleLib_HOOK_NOT_FOUND              (M_moduleLib | 0x0002)
#define S_moduleLib_BAD_CHECKSUM                (M_moduleLib | 0x0003)
#define S_moduleLib_MAX_MODULES_LOADED          (M_moduleLib | 0x0004)

#include <vmx.h>
#include <os/classLib.h>
#include <util/dllLib.h>

#define MODULE_GROUP_MAX        65535

/* Module types */
#define MODULE_A_OUT            0
#define MODULE_B_OUT            1
#define MODULE_ECOFF            2
#define MODULE_ELF              3

#define MODULE_REPLACED         0x00010000      /* Flags if replaced */
#define SEG_FREE_MEMORY         1               /* Free segment */

#define NAME_MAX                32

#define SEGMENT_TEXT            2               /* Text segment */
#define SEGMENT_DATA            4               /* Data segment */
#define SEGMENT_BSS             8               /* Bss segment */

#define MODCHECK_NOPRINT        1               /* No printout */
#define MODCHECK_TEXT           SEGMENT_TEXT
#define MODCHECK_DATA           SEGMENT_DATA
#define MODCHECK_BSS            SEGMENT_BSS

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef struct
{
    OBJ_CORE                     objCore;           /* Object class */
    DL_NODE                      moduleNode;        /* List node */
    char                         name[NAME_MAX];    /* Module name */
    char                         path[NAME_MAX];    /* Module path */
    int                          flags;             /* Flags for loader */
    DL_LIST                      segmentList;       /* List of segments */
    int                          format;            /* Object module format */
    unsigned short               group;             /* Group number */
    DL_LIST                      dependencies;      /* List of depedencies */
    DL_LIST                      referents;         /* List of referents */
    VOIDFUNCPTR                 *ctors;             /* List of constructors */
    VOIDFUNCPTR                 *dtors;             /* List of destructors */
    void                        *user1;             /* User defined */
    void                        *reserved1;
    void                        *reserved2;
    void                        *reserved3;
    void                        *reserved4;
} MODULE;

typedef MODULE *MODULE_ID;

typedef struct
{
    DL_NODE                      segmentNode;       /* List node */
    void                        *address;           /* Segment address */
    int                          size;              /* Segment size */
    int                          type;              /* Segment type */
    int                          flags;             /* Segment flags */
    unsigned short               checksum;          /* Segment checksum */
} SEGMENT;

typedef SEGMENT *SEGMENT_ID;

typedef struct
{
    void                        *textAddr;          /* Text segment address */
    void                        *dataAddr;          /* Data segment address */
    void                        *bssAddr;           /* Bss segment address */
    int                          textSize;          /* Text segment size */
    int                          dataSize;          /* Data segment size */
    int                          bssSize;           /* Bss segment size */
} MODULE_SEG_INFO;

typedef struct
{
    char                         name[NAME_MAX];    /* Module name */
    int                          format;            /* Module format */
    int                          group;             /* Module group */
    MODULE_SEG_INFO              segInfo;           /* Module segment info */
} MODULE_INFO;

/* Exports */

IMPORT CLASS_ID moduleClassId;

/* Functions */

/******************************************************************************
 * moduleLibInit - Inititalize module library
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleLibInit(
    void
    );

/******************************************************************************
 * moduleCreate - Create module
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleCreate(
    char *name,
    int   format,
    int   flags
    );

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
    );

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
    );

/******************************************************************************
 * moduleDestroy - Destroy module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleDestroy(
    MODULE_ID modId,
    BOOL      dealloc
    );

/******************************************************************************
 * moduleTerminate - Terminate module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleTerminate(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleDelete - Delete module
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleDelete(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleIdFigure - Get module id from module name or id
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleIdFigure(
    ARG modNameOrId
    );

/******************************************************************************
 * moduleIdListGet - Get a list of modules
 *
 * RETURNS: Number of items
 */

int moduleIdListGet(
    MODULE_ID idList[],
    int       max
    );

/******************************************************************************
 * moduleNameGet - Get module name
 *
 * RETURNS: Pointer to module name or NULL
 */

char* moduleNameGet(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleGroupGet - Get module group
 * 
 * RETURNS: Group number or zero
 */

int moduleGroupGet(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleFlagsGet - Get module flags
 *
 * RETURNS: Flags or zero
 */

int moduleFlagsGet(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleFormatGet - Get module format
 *
 * RETURNS: Format or zero
 */

int moduleFormatGet(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleFindByName - Find module by name
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByName(
    char *name
    );

/******************************************************************************
 * moduleFindByAndPath - Find module by name and path
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByNameAndPath(
    char *name,
    char *path
    );

/******************************************************************************
 * moduleFindByGroup - Find module by group
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID moduleFindByGroup(
    int group
    );

/******************************************************************************
 * moduleEach - Run function for each module
 *
 * RETURNS: MODULE_ID where it ended or NULL
 */

MODULE_ID moduleEach(
    FUNCPTR func,
    ARG     arg
    );

/******************************************************************************
 * moduleInfoGet - Get module information
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleInfoGet(
    MODULE_ID    modId,
    MODULE_INFO *pModInfo
    );

/******************************************************************************
 * moduleCheck - Check all modules
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCheck(
    int options
    );

/******************************************************************************
 * moduleCreateHookAdd - Add module create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCreateHookAdd(
    FUNCPTR func
    );

/******************************************************************************
 * moduleCreateHookDelete - Delete module create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCreateHookDelete(
    FUNCPTR func
    );

/******************************************************************************
 * moduleCreateHookDelete - Delete module create hook
 *
 * RETURNS: OK or ERROR
 */

STATUS moduleCreateHookDelete(
    FUNCPTR func
    );

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
    );

/******************************************************************************
 * moduleSegGet - Get segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegGet(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleSegFirst - Get first segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegFirst(
    MODULE_ID modId
    );

/******************************************************************************
 * moduleSegNext - Get next segment in module
 *
 * RETURNS: SEGMENT_ID or NULL
 */

SEGMENT_ID moduleSegNext(
    SEGMENT_ID segId
    );

/******************************************************************************
 * moduleSegEach - Execute function on all segments in module
 *
 * RETURNS: SEGMENT_ID where it ended or NULL
 */

SEGMENT_ID moduleSegEach(
    MODULE_ID modId,
    FUNCPTR   func,
    ARG       arg
    );

/******************************************************************************
 * moduleSegInfoGet - Get module segment info
 *
 * RETURNS: OK
 */

STATUS moduleSegInfoGet(
    MODULE_ID        modId,
    MODULE_SEG_INFO *pSegInfo
    );

/******************************************************************************
 * moduleSegVerify - Verify module segment checksum
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL moduleSegVerify(
    SEGMENT_ID segId
    );

/******************************************************************************
 * moduleVerify - Verify module
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL moduleVerify(
    MODULE_ID modId,
    int       options
    );

/******************************************************************************
 * moduleInsert - Insert module
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS moduleInsert(
    MODULE_ID modId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _moduleLib_h */

