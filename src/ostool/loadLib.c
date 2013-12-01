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

/* loadLib.c - Loader library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <util/sllLib.h>
#include <util/hashLib.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <ostool/moduleLib.h>
#include <ostool/private/loadLibP.h>
#include <ostool/loadLib.h>

/* Defines */
#define SYM_BASIC_MASK                  0x0f
#define SYM_BASIC_NOT_COMM_MASK         0x0d

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Locals */

LOCAL FUNCPTR loadFunc     = (FUNCPTR) NULL;
LOCAL FUNCPTR loadSyncFunc = (FUNCPTR) NULL;

/* Functions */

/******************************************************************************
 * loadLibInit - Inititalize loader library
 *
 * RETURNS: OK
 */

STATUS loadLibInit(
    void
    )
{
    return OK;
}

/******************************************************************************
 * loadFuncSet - Set load function
 *
 * RETURNS: N/A
 */

void loadFuncSet(
    FUNCPTR load,
    FUNCPTR sync
    )
{
    loadFunc = load;
    sync     = sync;
}

/******************************************************************************
 * loadModule - Load module
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID loadModule(
    int fd,
    int symFlags
    )
{
    return loadModuleAt(fd, symFlags, NULL, NULL, NULL);
}

/******************************************************************************
 * loadModuleAt - Load module at system symbol table
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID loadModuleAt(
    int    fd,
    int    symFlags,
    void **ppText,
    void **ppData,
    void **ppBss
    )
{
    return loadModuleAtSym(fd, symFlags, ppText, ppData, ppBss, sysSymTable);
}

/******************************************************************************
 * loadModuleAtSym - Load module at symbol table
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID loadModuleAtSym(
    int         fd,
    int         symFlags,
    void      **ppText,
    void      **ppData,
    void      **ppBss,
    SYMTAB_ID   symTable
    )
{
    MODULE_ID modId;

    /* If no load func */
    if (loadFunc == NULL)
    {
        errnoSet(S_loadLib_ROUTINE_NOT_INSTALLED);
        modId = NULL;
    }
    else
    {
        /* Load module */
        modId = (MODULE_ID) (*loadFunc)(
                                fd,
                                symFlags,
                                ppText,
                                ppData,
                                ppBss,
                                symTable
                                );
        if (modId != NULL)
        {
            /* Synchronize if function set */
            if (loadSyncFunc != NULL)
            {
                (*loadSyncFunc)(modId);
            }
        }
    }

    return modId;
}

/******************************************************************************
 * loadModuleGet - Create module for loadable code
 *
 * RETURNS: MODULE_ID or NULL
 */

MODULE_ID loadModuleGet(
    char *name,
    int   format,
    int  *symFlags
    )
{
    /* Select flags */
    switch(*symFlags)
    {
        case NO_SYMBOLS:
            *symFlags = LOAD_NO_SYMBOLS;
            break;

        case GLOBAL_SYMBOLS:
            *symFlags = LOAD_GLOBAL_SYMBOLS;
            break;

        case ALL_SYMBOLS:
            *symFlags = LOAD_ALL_SYMBOLS;
            break;

        default:
            *symFlags = LOAD_ALL_SYMBOLS;
            break;
    }

    /* Create module */
    return moduleCreate(name, format, *symFlags);
}

/******************************************************************************
 * loadSegmentsAllocate - Allocate text, data and bss segments
 *
 * RETURNS: OK or ERROR
 */

STATUS loadSegmentsAllocate(
    SEG_INFO *pSegInfo
    )
{
    STATUS status;
    int    align;
    int    textAlign = pSegInfo->flagsText;
    int    dataAlign = pSegInfo->flagsData;
    int    bssAlign  = pSegInfo->flagsBss;

    /* Clear flags */
    pSegInfo->flagsText = 0;
    pSegInfo->flagsData = 0;
    pSegInfo->flagsBss = 0;

    /* If all-in-one segment */
    if ((pSegInfo->addrText == LD_NO_ADDRESS) &&
        (pSegInfo->addrData == LD_NO_ADDRESS) &&
        (pSegInfo->addrBss == LD_NO_ADDRESS))
    {
        align = 0;
        if (textAlign > dataAlign)
        {
            align = max(textAlign, bssAlign);
        }
        else
        {
            align = max(dataAlign, bssAlign);
        }

        if (align != 0)
        {
            pSegInfo->addrText = (char *) memalign(
                                              align,
                                              pSegInfo->sizeText +
                                              pSegInfo->sizeData +
                                              pSegInfo->sizeBss
                                              );
        }
        else
        {
            pSegInfo->addrText = (char *) malloc(
                                              pSegInfo->sizeText +
                                              pSegInfo->sizeData +
                                              pSegInfo->sizeBss
                                              );
        }

        if (pSegInfo->addrText == NULL)
        {
            status = ERROR;
        }
        else
        {
            /* Setup segment pointers */
            pSegInfo->addrData = pSegInfo->addrText + pSegInfo->sizeText;
            pSegInfo->addrBss = pSegInfo->addrData + pSegInfo->sizeData;

            pSegInfo->flagsText |= SEG_FREE_MEMORY;
            status = OK;
        }
    }
    else
    {
        status = OK;

        /* If allocate memory for text segment */
        if ((pSegInfo->addrText == LD_NO_ADDRESS) &&
            (pSegInfo->sizeText != 0))
        {
            if (textAlign != 0)
            {
                pSegInfo->addrText = (char *) memalign(
                                                  textAlign,
                                                  pSegInfo->sizeText
                                                  );
                if (pSegInfo->addrText == NULL)
                {
                    status = ERROR;
                }
            }
            else
            {
                pSegInfo->addrText = (char *) malloc(pSegInfo->sizeText);
                if (pSegInfo->addrText == NULL)
                {
                    status = ERROR;
                }
            }

            if (status == OK)
            {
                pSegInfo->flagsText |= SEG_FREE_MEMORY;
            }
        }

        /* If allocate memory for data segment */
        if ((status == OK) &&
            (pSegInfo->addrData == LD_NO_ADDRESS) &&
            (pSegInfo->sizeData != 0))
        {
            if (dataAlign != 0)
            {
                pSegInfo->addrData = (char *) memalign(
                                                  dataAlign,
                                                  pSegInfo->sizeData
                                                  );
                if (pSegInfo->addrData == NULL)
                {
                    status = ERROR;
                }
            }
            else
            {
                pSegInfo->addrData = (char *) malloc(pSegInfo->sizeData);
                if (pSegInfo->addrData == NULL)
                {
                    status = ERROR;
                }
            }

            if (status == OK)
            {
                pSegInfo->flagsData |= SEG_FREE_MEMORY;
            }
        }

        /* If allocate memory for bss segment */
        if ((status == OK) &&
            (pSegInfo->addrBss == LD_NO_ADDRESS) &&
            (pSegInfo->sizeBss != 0))
        {
            if (bssAlign != 0)
            {
                pSegInfo->addrBss = (char *) memalign(
                                                 bssAlign,
                                                 pSegInfo->sizeBss
                                                 );
                if (pSegInfo->addrBss == NULL)
                {
                    status = ERROR;
                }
            }
            else
            {
                pSegInfo->addrBss = (char *) malloc(pSegInfo->sizeBss);
                if (pSegInfo->addrBss == NULL)
                {
                    status = ERROR;
                }
            }

            if (status == OK)
            {
                pSegInfo->flagsBss |= SEG_FREE_MEMORY;
            }
        }
    }

    return status;
}

/******************************************************************************
 * loadCommonMatch - Fill in common info
 *
 * RETURNS: OK
 */

STATUS loadCommonMatch(
    COMMON_INFO *pCommonInfo,
    SYMTAB_ID    symTable
    )
{
    SYMBOL    matchSym;
    SYM_TYPE  symType;
    int       i;
    int       mask;
    HASH_ID   hashId   = symTable->nameHashId;
    SYMBOL   *pSymNode = NULL;

    matchSym.name = pCommonInfo->symName;
    matchSym.type = SYM_MASK_NONE;

    /* Get index */
    i = (*hashId->keyFunc)(
            hashId->numElements,
            (HASH_NODE *) &matchSym,
            hashId->keyArg
            );

    /* Get first symbol in bucket */
    pSymNode = (SYMBOL *) SLL_HEAD(&hashId->pHashTable[i]);

    /* For all symbols in bucket */
    while (pSymNode != NULL)
    {
        mask = SYM_BASIC_MASK;

        /* If symbol match */
        if (!((pCommonInfo->symMatched == FALSE) &&
              (pSymNode->group == 0)) &&
              (strcmp(pCommonInfo->symName, pSymNode->name) == 0))
        {
            if ((pSymNode->type & SYM_COMM) == SYM_COMM)
            {
                mask = SYM_BASIC_NOT_COMM_MASK;
            }

            /* Get type */
            symType = pSymNode->type & mask;

            /* If bss symbol */
            if ((symType == (SYM_BSS | SYM_GLOBAL)) &&
                (pCommonInfo->pSymAddrBss == NULL))
            {
                pCommonInfo->pSymAddrBss = pSymNode->value;
                pCommonInfo->bssSymType  = pSymNode->type;
            }
            else if ((symType == (SYM_DATA | SYM_GLOBAL)) &&
                (pCommonInfo->pSymAddrData == NULL))
            {
                pCommonInfo->pSymAddrData = pSymNode->value;
                pCommonInfo->dataSymType  = pSymNode->type;
            }
        }

        /* Advance */
        pSymNode = (SYMBOL *) SLL_NEXT((HASH_NODE *) pSymNode);
    }

    return OK;
}

/******************************************************************************
 * loadCommonManage - Process common symbol
 *
 * RETURNS: OK or ERROR
 */

STATUS loadCommonManage(
    int             size,
    int             align,
    char           *symName,
    SYMTAB_ID       symTable,
    SYM_ADDR       *pSymAddr,
    SYM_TYPE       *pSymType,
    int             loadFlag,
    SEG_INFO       *pSegInfo,
    unsigned short  group
    )
{
    COMMON_INFO commonInfo;
    STATUS      status = ERROR;

    /* Fix flags */
    if (((loadFlag & LOAD_COMMON_MATCH_ALL) == 0) &&
        ((loadFlag & LOAD_COMMON_MATCH_USER) == 0) &&
        ((loadFlag & LOAD_COMMON_MATCH_NONE) == 0))
    {
        loadFlag |= LOAD_COMMON_MATCH_NONE;
    }

    /* If any match */
    if ((loadFlag & LOAD_COMMON_MATCH_NONE) == 0)
    {
        memset(&commonInfo, 0, sizeof(COMMON_INFO));
        commonInfo.symName = symName;

        if (loadFlag & LOAD_COMMON_MATCH_USER)
        {
            commonInfo.symMatched = FALSE;
        }
        else if (loadFlag & LOAD_COMMON_MATCH_ALL)
        {
            commonInfo.symMatched = TRUE;
        }

        /* Fill in common info struct */
        loadCommonMatch(&commonInfo, symTable);

        /* If bss symbol */
        if (commonInfo.pSymAddrBss != NULL)
        {
            *pSymAddr = commonInfo.pSymAddrBss;
            *pSymType = commonInfo.bssSymType;
        }
        else if (commonInfo.pSymAddrData != NULL)
        {
            *pSymAddr = commonInfo.pSymAddrData;
            *pSymType = commonInfo.dataSymType;
        }
        else
        {
            *pSymAddr = NULL;
        }

        /* If match found, return */
        if (*pSymAddr != NULL)
        {
            status = OK;
        }
    }

    if (status != OK)
    {
        /* Set symbol type */
        *pSymType = SYM_BSS | SYM_COMM | SYM_GLOBAL;

        /* Allocate memory */
        if (align != 0)
        {
            *pSymAddr = memalign(align, size);
        }
        else
        {
            *pSymAddr = malloc(size);
        }

        if (*pSymAddr != NULL)
        {
            /* Clear */
            memset((SYM_ADDR) *pSymAddr, 0, size);

            /* If load symbol */
            if (((loadFlag & LOAD_NO_SYMBOLS) == 0) &&
                 (loadFlag & LOAD_GLOBAL_SYMBOLS))
            {
                if (symAdd(
                        symTable,
                        symName,
                        *pSymAddr,
                        *pSymType,
                        group
                        ) != OK)
                {
                    fprintf(
                        stderr,
                        "Can't add '%s' to symbol table.\n",
                        symName
                        );
                    *pSymAddr = NULL;
                    status = ERROR;
                }
                else
                {
                    status = OK;
                }
            }
            else
            {
                status = OK;
            }
        }
    }

    return status;
}

