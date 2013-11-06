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

/* symLib.h - Symbol table library header */

#ifndef _symLib_h
#define _symLib_h

#include <tools/moduleNumber.h>

#define S_symLib_SYMBOL_NOT_FOUND               (M_symLib | 0x0001)

#include <os/private/symLibP.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Imports */
IMPORT CLASS_ID symTableClassId;
IMPORT unsigned short symGroupDefault;

/* Functions */
/******************************************************************************
 * symLibInit - Initialize symbol table libarary
 *
 * RETURNS: OK or ERROR
 */

STATUS symLibInit(
    void
    );

/******************************************************************************
 * symTableCreate - Create a symbol table
 *
 * RETURNS: Symbol table id or NULL
 */

SYMTAB_ID symTableCreate(
    int log2Size,
    BOOL sameNameOk,
    PART_ID partId
    );

/******************************************************************************
 * symTableInit - Initialize symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableInit(
    SYMTAB_ID symTableId,
    BOOL sameNameOk,
    PART_ID partId,
    HASH_ID nameHashId
    );

/******************************************************************************
 * symTableDestroy - Destroy symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableDestroy(
    SYMTAB_ID symTableId,
    BOOL dealloc
    );

/******************************************************************************
 * symTableDelete - Delete symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableDelete(
    SYMTAB_ID symTableId
    );

/******************************************************************************
 * symTableTerminate - Terminate symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableTerminate(
    SYMTAB_ID symTableId
    );

/******************************************************************************
 * symCreate - Create a new symbol entry for symbol table
 *
 * RETURNS: Symbol id or NULL
 */

SYMBOL_ID symCreate(
    SYMTAB_ID symTableId,
    char *name,
    ARG value,
    SYM_TYPE type, 
    unsigned short group
    );

/******************************************************************************
 * symInit - Initialize a symbol
 *
 * RETURNS: OK
 */

STATUS symInit(
    SYMBOL_ID symId,
    char *name,
    ARG value,
    SYM_TYPE type,
    unsigned short group
    );

/******************************************************************************
 * symDestroy - Destroy a symbol
 *
 * RETURNS: OK or ERROR
 */

STATUS symDestroy(
    SYMTAB_ID symTableId,
    SYMBOL_ID symId
    );

/******************************************************************************
 * symAdd - Create and add a symbol to symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symAdd(
    SYMTAB_ID symTableId,
    char *name,
    ARG value,
    SYM_TYPE type,
    unsigned short group
    );

/******************************************************************************
 * symTableAdd - Add a symbol to symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableAdd(
    SYMTAB_ID symTableId,
    SYMBOL_ID symId
    );

/******************************************************************************
 * symFindByName - Lookup a symbol in the symbol table by name
 *
 * RETURNS: OK or ERROR
 */

STATUS symFindByName(
    SYMTAB_ID symTableId,
    char *name,
    ARG *pValue,
    SYM_TYPE *pType
    );

/******************************************************************************
 * symFindByNameAndType - Lookup a symbol in the symbol table by name and type
 *
 * RETURNS: OK or ERROR
 */

STATUS symFindByNameAndType(
    SYMTAB_ID symTableId,
    char *name,
    ARG *pValue,
    SYM_TYPE *pType,
    SYM_TYPE sType,
    SYM_TYPE mask
    );

/******************************************************************************
 * symFindSymbol - Lookup a symbol in the symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symFindSymbol(
    SYMTAB_ID symTableId,
    char *name,
    ARG value,
    SYM_TYPE type,
    SYM_TYPE mask,
    SYMBOL_ID *pSymId
    );

/******************************************************************************
 * symNameGet - Get symbol name
 *
 * RETURNS: OK or ERROR
 */

STATUS symNameGet(
    SYMBOL_ID symId,
    char **pName
    );

/******************************************************************************
 * symValueGet - Get symbol value
 *
 * RETURNS: OK or ERROR
 */

STATUS symValueGet(
    SYMBOL_ID symId,
    ARG *pValue
    );

/******************************************************************************
 * symTypeGet - Get symbol type
 *
 * RETURNS: OK or ERROR
 */

STATUS symTypeGet(
    SYMBOL_ID symId,
    SYM_TYPE *pType
    );

/******************************************************************************
 * symEach - Call routine for each symbol in symbol table
 *
 * RETURNS: Symbol id where it stopped
 */

SYMBOL* symEach(
    SYMTAB_ID symTableId,
    FUNCPTR func,
    ARG arg
    );

/******************************************************************************
 * symRemove - Remove and destroy a symbol from symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symRemove(
    SYMTAB_ID symTableId,
    char *name,
    SYM_TYPE type
    );

/******************************************************************************
 * symTableRemove - Remove a symbol from symbol table
 *
 * RETURNS: OK or ERROR
 */

STATUS symTableRemove(
    SYMTAB_ID symTableId,
    SYMBOL_ID symId
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _symLib_h */

