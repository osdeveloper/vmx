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

/* loadElfLib.h - ELF loader header */

#ifndef _loadElfLib_h
#define _loadElfLib_h

#include <vmx.h>
#include <ostool/moduleNumber.h>

/* Error codes */
#define S_loadElfLib_HDR_READ                   (M_loadElfLib | 0x0001)
#define S_loadElfLib_HDR_ERROR                  (M_loadElfLib | 0x0002)
#define S_loadElfLib_PHDR_MALLOC                (M_loadElfLib | 0x0003)
#define S_loadElfLib_PHDR_READ                  (M_loadElfLib | 0x0004)
#define S_loadElfLib_SHDR_MALLOC                (M_loadElfLib | 0x0005)
#define S_loadElfLib_SHDR_READ                  (M_loadElfLib | 0x0006)
#define S_loadElfLib_SYMTAB                     (M_loadElfLib | 0x0007)
#define S_loadElfLib_SCN_READ                   (M_loadElfLib | 0x0008)
#define S_loadElfLib_READ_SECTIONS              (M_loadElfLib | 0x0009)
#define S_loadElfLib_RELA_SECTION               (M_loadElfLib | 0x000a)
#define S_loadElfLib_RELOC                      (M_loadElfLib | 0x000b)
#define S_loadElfLib_UNRECOGNIZED_RELOCENTRY    (M_loadElfLib | 0x000c)

#define MAX_SECT_LEN            255

#define NOT_SDA_SECT            0
#define SDA_SECT                1
#define SDA2_SECT               2

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef unsigned char SDA_SECT_TYPE;

/* Functions */

/******************************************************************************
 * loadElfLibInit - Initialize elf load library
 *
 * RETURNS: OK or ERROR
 */

STATUS loadElfLibInit(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _loadElfLib_h */

