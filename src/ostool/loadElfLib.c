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

/* loadElfLib.c - ELF load library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <sys/types.h>
#include <vmx.h>
#include <arch/elfArchLib.h>
#include <os/errnoLib.h>
#include <os/symbol.h>
#include <os/symLib.h>
#include <os/ioLib.h>
#include <ostool/moduleLib.h>
#include <ostool/loadLib.h>
#include <ostool/private/loadElfLibP.h>
#include <ostool/loadElfLib.h>

/* Defines */

/* Imports */
IMPORT SYMTAB_ID sysSymTable;

/* Locals */
LOCAL FUNCPTR pElfVerifyFunc   = NULL;
LOCAL FUNCPTR pElfSegRelFunc   = NULL;
LOCAL int     loadElfModuleNum = 0;

LOCAL STATUS loadElfHeaderCheck(
    Elf32_Ehdr *pHeader
    );

LOCAL BOOL loadElfArchVerify(
    Elf32_Ehdr *pHeader
    );

LOCAL STATUS loadElfTablesAlloc(
    Elf32_Ehdr    *pHeader,
    Elf32_Phdr   **ppProgHeader,
    Elf32_Shdr   **ppSectHeader,
    INDEX_TABLES  *pIndex
    );

LOCAL STATUS loadElfTablesFree(
    Elf32_Phdr   **ppProgHeader,
    Elf32_Shdr   **ppSectHeader,
    INDEX_TABLES  *pIndex
    );

LOCAL STATUS loadElfProgHeaderCheck(
    Elf32_Phdr *pHeader,
    int         num
    );

LOCAL STATUS loadElfSectHeaderCheck(
    Elf32_Shdr *pHeader,
    int         num
    );

LOCAL STATUS loadElfSectHeaderIndex(
    Elf32_Shdr   *pTable,
    int           currIndex,
    INDEX_TABLES *pIndex
    );

LOCAL STATUS loadElfHeaderRead(
    int         fd,
    Elf32_Ehdr *pHeader
    );

LOCAL STATUS loadElfProgHeaderRead(
    int         fd,
    int         off,
    Elf32_Phdr *pTable,
    int         num
    );

LOCAL STATUS loadElfSectHeaderRead(
    int           fd,
    int           off,
    Elf32_Shdr   *pTable,
    int           num,
    INDEX_TABLES *pIndex
    );

LOCAL char* loadElfSectStrTableRead(
    int         fd,
    Elf32_Shdr *pSectHeaderTable,
    Elf32_Ehdr *pHeader
    );

LOCAL u_int32_t loadElfAlignGet(
    u_int32_t align,
    ARG       addrOrSize
    );

LOCAL void loadElfSegSizeGet(
    char       *pNameTable,
    u_int32_t  *pLoadSectHeaderIndex,
    Elf32_Shdr *pSectHeaderTable,
    SEG_INFO   *pSegInfo
    );

LOCAL int loadElfSymEntryRead(
    int        fd,
    int        symEntry,
    Elf32_Sym *pSym
    );

LOCAL STATUS loadElfSymTableRead(
    int        fd,
    int        nextSym,
    int        nSyms,
    Elf32_Sym *pSymTable
    );

LOCAL int loadElfSymTablesRead(
    u_int32_t     *pSymTableSectHeaderIndex,
    Elf32_Shdr    *pSectHeaderTable,
    int            fd,
    SYMTABLE_REFS *pSymTableRefs,
    SYMINFO_REFS  *pSymAddrRefs
    );

LOCAL STATUS loadElfSectRead(
    int              fd,
    char            *pSectStrTable,
    u_int32_t       *pLoadSectHeaderIndex,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE  sectAddrTable,
    SEG_INFO        *pSegInfo
    );

LOCAL STATUS loadElfRelocMod(
    SEG_INFO        *pSegInfo,
    int              fd,
    char            *pSectStrTable,
    INDEX_TABLES    *pIndex,
    Elf32_Ehdr      *pHeader,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE *pSectAddrTable
    );

LOCAL STATUS loadElfSegStore(
    SEG_INFO        *pSegInfo,
    int              loadFlag,
    int              fd,
    char            *pSectStrTable,
    INDEX_TABLES    *pIndex,
    Elf32_Ehdr      *pHeader,
    Elf32_Shdr      *pSectHeaderTable,
    Elf32_Phdr      *pProgHeaderTable,
    SECT_ADDR_TABLE *pSectAddrTable
    );

LOCAL SYM_TYPE loadElfSymTypeGet(
    Elf32_Sym  *pSym,
    Elf32_Shdr *pSectHeaderTable,
    char       *pSectStrTable
    );

LOCAL BOOL loadElfSymVisible(
    u_int32_t symAssoc,
    u_int32_t symBind,
    int loadFlag
    );

LOCAL STATUS loadElfSymTableProcess(
    MODULE_ID        moduleId,
    int              loadFlag,
    Elf32_Sym       *pSymArray,
    SECT_ADDR_TABLE  sectAddrTable,
    SYM_INFO_TABLE   symAddrTable,
    char            *pStrTable,
    SYMTAB_ID        symTable,
    u_int32_t        symNum,
    Elf32_Shdr      *pSectHeaderTable,
    char            *pSectStrTable,
    SEG_INFO        *pSegInfo
    );

LOCAL STATUS loadElfSymTableBuild(
    MODULE_ID        moduleId,
    int              loadFlag,
    SYMTABLE_REFS    symTableRefs,
    SECT_ADDR_TABLE  sectAddrTable,
    SYMINFO_REFS     symAddrRefs,
    INDEX_TABLES    *pIndex,
    SYMTAB_ID        symTable,
    int              fd,
    Elf32_Shdr      *pSectHeaderTable,
    char            *pSectStrTable,
    SEG_INFO        *pSegInfo
    );

LOCAL STATUS loadElfSegReloc(
    int              fd,
    int              loadFlag,
    MODULE_ID        moduleId,
    Elf32_Ehdr      *pHeader,
    INDEX_TABLES    *pIndex,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE  sectAddrTable,
    SYMTABLE_REFS    symTableRefs,
    SYMINFO_REFS     symAddrRefs,
    SYMTAB_ID        symTable,
    SEG_INFO        *pSegInfo
    );

LOCAL MODULE_ID loadElf(
    int         fd,
    int         loadFlag,
    void      **ppText,
    void      **ppData,
    void      **ppBss,
    SYMTAB_ID   symTable
    );

/* Globals */

/* Functions */

/******************************************************************************
 * loadElfLibInit - Initialize elf load library
 *
 * RETURNS: OK or ERROR
 */

STATUS loadElfLibInit(
    void
    )
{
    STATUS status;

    if (elfArchLibInit(&pElfVerifyFunc, &pElfSegRelFunc) != OK)
    {
        status = ERROR;
    }
    else
    {
        /* Set load function */
        loadFuncSet((FUNCPTR) loadElf, NULL);
        loadElfModuleNum = 1;
        status = OK;
    }

    return status;
}

/******************************************************************************
 * loadElfHeaderCheck - Check elf module header
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfHeaderCheck(
    Elf32_Ehdr *pHeader
    )
{
    STATUS status;

    if (pHeader->e_ehsize != EHDRSZ)
    {
        fprintf(
            stderr,
            "Incorrect ELF header size: %d\n",
            pHeader->e_ehsize
            );
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else if ((pHeader->e_type != ET_REL) &&
             (pHeader->e_type != ET_EXEC))
    {
        fprintf(
            stderr,
            "Incorrect module type: %d\n",
            pHeader->e_type
            );
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else if ((pHeader->e_phnum != 0) &&
             (pHeader->e_phoff == 0))
    {
        fprintf(stderr, "Null offset to program header table.\n");
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else if ((pHeader->e_type == ET_EXEC) &&
             (pHeader->e_phentsize != PHDRSZ))
    {
        fprintf(
            stderr,
            "Incorrect program header size: %d\n",
            pHeader->e_phentsize
            );
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else if (pHeader->e_shentsize != SHDRSZ)
    {
        fprintf(
            stderr,
            "Incorrect section header size: %d\n",
            pHeader->e_shentsize
            );
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else if ((pHeader->e_shnum != 0) &&
             (pHeader->e_shoff == 0))
    {
        fprintf(stderr, "Null offset to section header table.\n");
        errnoSet(S_loadElfLib_HDR_ERROR);
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * loadElfArchVerify - Verify elf module target architecture
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL loadElfArchVerify(
    Elf32_Ehdr *pHeader
    )
{
    BOOL      result;
    BOOL      sdaUsed;
    u_int16_t target;

    /* Check magic number */
    if (strncmp(pHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        fprintf(stderr, "Unknown object module format\n");
        result = FALSE;
    }
    else
    {
        /* Get arch type */
        target = pHeader->e_machine;

        /* If check function exists */
        if (pElfVerifyFunc != NULL)
        {
            result = (*pElfVerifyFunc)((u_int32_t) target, &sdaUsed);
        }
        else
        {
            result = FALSE;
        }
    }

    return result;
}

/******************************************************************************
 * loadElfTablesAlloc - Allocate memory for elf tables
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfTablesAlloc(
    Elf32_Ehdr    *pHeader,
    Elf32_Phdr   **ppProgHeader,
    Elf32_Shdr   **ppSectHeader,
    INDEX_TABLES  *pIndex
    )
{
    STATUS status;
    int    num;

    /* Get number of program headers */
    num = max(1, pHeader->e_phnum);

    /* Allocate program header table if needed */
    *ppProgHeader = (Elf32_Phdr *) malloc(num * PHDRSZ);
    if (*ppProgHeader == NULL)
    {
        errnoSet(S_loadElfLib_PHDR_MALLOC);
        status = ERROR;
    }
    else
    {
        /* Get number of section headers */
        num = max(1, pHeader->e_shnum);

        /* Allocate section header table */
        *ppSectHeader = (Elf32_Shdr *) malloc(num * SHDRSZ);
        if (*ppSectHeader == NULL)
        {
            errnoSet(S_loadElfLib_SHDR_MALLOC);
            free(*ppProgHeader);
            status = ERROR;
        }
        else
        {
            pIndex->pLoadSectHeaderIndex = malloc(num * sizeof(u_int32_t));
            if (pIndex->pLoadSectHeaderIndex == NULL)
            {
                free(*ppSectHeader);
                free(*ppProgHeader);
                status = ERROR;
            }
            else
            {
                /* Clear */
                memset(
                    pIndex->pLoadSectHeaderIndex,
                    0,
                    num * sizeof(u_int32_t)
                    );

                pIndex->pSymTableSectHeaderIndex =
                    malloc(num * sizeof(u_int32_t));
                if (pIndex->pSymTableSectHeaderIndex == NULL)
                {
                    free(pIndex->pLoadSectHeaderIndex);
                    free(*ppSectHeader);
                    free(*ppProgHeader);
                    status = ERROR;
                }
                else
                {
                    /* Clear */
                    memset(
                        pIndex->pSymTableSectHeaderIndex,
                        0,
                        num * sizeof(u_int32_t)
                        );
                    pIndex->pRelSectHeaderIndex =
                        malloc(num * sizeof(u_int32_t));
                    if (pIndex->pRelSectHeaderIndex == NULL)
                    {
                        free(pIndex->pSymTableSectHeaderIndex);
                        free(pIndex->pLoadSectHeaderIndex);
                        free(*ppSectHeader);
                        free(*ppProgHeader);
                        status = ERROR;
                    }
                    else
                    {
                        /* Clear */
                        memset(
                            pIndex->pRelSectHeaderIndex,
                            0,
                            num * sizeof(u_int32_t)
                            );

                        pIndex->pStrTableSectHeaderIndex =
                            malloc(num * sizeof(u_int32_t));
                        if (pIndex->pStrTableSectHeaderIndex == NULL)
                        {
                            free(pIndex->pRelSectHeaderIndex);
                            free(pIndex->pSymTableSectHeaderIndex);
                            free(pIndex->pLoadSectHeaderIndex);
                            free(*ppSectHeader);
                            free(*ppProgHeader);
                            status = ERROR;
                        }
                        else
                        {
                            /* Clear */
                            memset(
                                pIndex->pStrTableSectHeaderIndex,
                                0,
                                num * sizeof(u_int32_t)
                                );
                            status = OK;
                        }
                    }
                }
            }
        }
    }

    return status;
}

/******************************************************************************
 * loadElfTablesFree - Free memory for elf tables
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfTablesFree(
    Elf32_Phdr   **ppProgHeader,
    Elf32_Shdr   **ppSectHeader,
    INDEX_TABLES  *pIndex
    )
{
    free(*ppProgHeader);
    free(*ppSectHeader);
    free(pIndex->pLoadSectHeaderIndex);
    free(pIndex->pSymTableSectHeaderIndex);
    free(pIndex->pRelSectHeaderIndex);
    free(pIndex->pStrTableSectHeaderIndex);
}

/******************************************************************************
 * loadElfProgHeaderCheck - Check elf program header
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfProgHeaderCheck(
    Elf32_Phdr *pHeader,
    int         num
    )
{
    STATUS status;

    /* Check alignment */
    if (!CHECK_2_ALIGN(pHeader->p_align))
    {
        errnoSet(S_loadElfLib_PHDR_READ);
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * loadElfSectHeaderCheck - Check elf section header
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSectHeaderCheck(
    Elf32_Shdr *pHeader,
    int         num
    )
{
    STATUS status;

    /* Check alignment */
    if (!CHECK_2_ALIGN(pHeader->sh_addralign))
    {
        errnoSet(S_loadElfLib_SHDR_READ);
        status = ERROR;
    }
    else
    {
        status = OK;
    }

    return status;
}

/******************************************************************************
 * loadElfSectHeaderIndex - Get elf section header indexes
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSectHeaderIndex(
    Elf32_Shdr   *pTable,
    int           currIndex,
    INDEX_TABLES *pIndex
    )
{
    static int loadSect     = 0;
    static int symTableSect = 0;
    static int relocSect    = 0;
    static int strTableSect = 0;

    STATUS      status;
    Elf32_Shdr *pHeader;
    Elf32_Shdr *pAllocHeader;

    /* If reset counters */
    if (pTable == NULL)
    {
        loadSect     = 0;
        symTableSect = 0;
        relocSect    = 0;
        strTableSect = 0;

        status = OK;
    }
    else
    {
        /* Get current section header */
        pHeader = &pTable[currIndex];

        /* Select type */
        switch (pHeader->sh_type)
        {
            case SHT_PROGBITS:
            case SHT_NOBITS:
                if (pHeader->sh_flags & SHF_ALLOC)
                {
                    pIndex->pLoadSectHeaderIndex[loadSect++] = currIndex;
                }
                status = OK;
                break;

            case SHT_SYMTAB:
                pIndex->pSymTableSectHeaderIndex[symTableSect++] = currIndex;
                status = OK;
                break;

            case SHT_DYNSYM:
                fprintf(stderr, "Section type SHT_DYNSYM not supported.\n");
                status = ERROR;
                break;

            case SHT_REL:
            case SHT_RELA:
                pAllocHeader = &pTable[pHeader->sh_info];
                if ((pAllocHeader->sh_flags & SHF_ALLOC) &&
                    (pAllocHeader->sh_type == SHT_PROGBITS))
                {
                    pIndex->pRelSectHeaderIndex[relocSect++] = currIndex;
                }
                status = OK;
                break;

            case SHT_STRTAB:
                pIndex->pStrTableSectHeaderIndex[strTableSect++] = currIndex;
                status = OK;
                break;

            default:
                status = ERROR;
                break;
        }
    }

    return status;
}

/******************************************************************************
 * loadElfHeaderRead - Read elf header
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfHeaderRead(
    int         fd,
    Elf32_Ehdr *pHeader
    )
{
    STATUS status;
    int    len;

    /* Go to start of file */
    ioctl(fd, FIOSEEK, 0);

    /* Read from file */
    len = read(fd, pHeader, EHDRSZ);
    if (len != EHDRSZ)
    {
        errnoSet(S_loadElfLib_HDR_READ);
        status = ERROR;
    }
    else
    {
        /* Check module header */
        status = loadElfHeaderCheck(pHeader);
    }

    return status;
}

/******************************************************************************
 * loadElfProgHeaderRead - Read elf program headers
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfProgHeaderRead(
    int         fd,
    int         off,
    Elf32_Phdr *pTable,
    int         num
    )
{
    STATUS status;
    int    i;
    int    len;

    /* Move to correct position */
    ioctl(fd, FIOSEEK, off);

    /* Read from file */
    len = read(fd, pTable, num * PHDRSZ);
    if (len != num * PHDRSZ)
    {
        errnoSet(S_loadElfLib_PHDR_READ);
        status = ERROR;
    }
    else
    {
        status = OK;

        /* For all program headers */
        for (i = 0; i < num; i++)
        {
            if (loadElfProgHeaderCheck(&pTable[i], i) != OK)
            {
                status = ERROR;
                break;
            }
        }
    }

    return status;
}

/*******************************************************************************
 * loadElfSectHeadesRead - Read elf section headers
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL STATUS loadElfSectHeaderRead(
    int           fd,
    int           off,
    Elf32_Shdr   *pTable,
    int           num,
    INDEX_TABLES *pIndex
    )
{
    STATUS status;
    int    i;
    int    len;

    /* Set counters */
    loadElfSectHeaderIndex(NULL, 0, NULL);

    /* Move to correct position */
    ioctl(fd, FIOSEEK, off);

    /* Read from file */
    len = read(fd, pTable, num * SHDRSZ);
    if (len != num * SHDRSZ)
    {
        errnoSet(S_loadElfLib_SHDR_READ);
        status = ERROR;
    }
    else
    {
        status = OK;

        /* For all section headers */
        for (i = 0; i < num; i++)
        {
            if (loadElfSectHeaderCheck(&pTable[i], i) != OK)
            {
                status = ERROR;
                break;
            }

            loadElfSectHeaderIndex(pTable, i, pIndex);
        }
    }

    return status;
}

/******************************************************************************
 * loadElfSectStrTableRead - Read section name string table
 *
 * RETURNS: N/A
 */

LOCAL char* loadElfSectStrTableRead(
    int         fd,
    Elf32_Shdr *pSectHeaderTable,
    Elf32_Ehdr *pHeader
    )
{
    int   len;
    char *pStrTable;

     /* Allocate */
    pStrTable = (char *) malloc(pSectHeaderTable[pHeader->e_shstrndx].sh_size);
    if (pStrTable == NULL)
    {
        pStrTable = NULL;
    }
    else
    {
        /* Seek to */
        ioctl(fd, FIOSEEK, pSectHeaderTable[pHeader->e_shstrndx].sh_offset);

        /* Read from file */
        len = read(
                  fd,
                  pStrTable,
                  pSectHeaderTable[pHeader->e_shstrndx].sh_size
                  );
        if (len != pSectHeaderTable[pHeader->e_shstrndx].sh_size)
        {
            pStrTable = NULL;
        }
    }

    return pStrTable;
}

/******************************************************************************
 * loadElfAlignGet - Get required alignement for address or size
 *
 * RETURNS: Nuber of bytes to be added to address or size
 */

LOCAL u_int32_t loadElfAlignGet(
    u_int32_t align,
    ARG       addrOrSize
    )
{
    u_int32_t nBytes;

    if (align > 0)
    {
        nBytes = (u_int32_t) addrOrSize % align;
    }
    else
    {
        nBytes = 0;
    }

    if (nBytes != 0)
    {
        nBytes = align - nBytes;
    }

    return nBytes;
}

/******************************************************************************
 * loadElfSegSizeGet - Get segment sizes for section headers
 *
 * RETURNS: N/A
 */

LOCAL void loadElfSegSizeGet(
    char       *pNameTable,
    u_int32_t  *pLoadSectHeaderIndex,
    Elf32_Shdr *pSectHeaderTable,
    SEG_INFO   *pSegInfo
    )
{
    Elf32_Shdr     *pHeader;
    int             i;
    int             nBytes;
    SDA_SECT_TYPE   sda;
    SDA_INFO       *pSdaInfo;
    int             textAlign = _ALLOC_ALIGN_SIZE;
    int             dataAlign = _ALLOC_ALIGN_SIZE;
    int             bssAlign  = _ALLOC_ALIGN_SIZE;

    /* Inititalize arguments */
    pSegInfo->sizeText = 0;
    pSegInfo->sizeData = 0;
    pSegInfo->sizeBss  = 0;

    /* For all section headers */
    for (i = 0; pLoadSectHeaderIndex[i] != 0; i++)
    {
        pHeader = &pSectHeaderTable[pLoadSectHeaderIndex[i]];
        sda = NOT_SDA_SECT;

        /* Select header type */
        switch(pHeader->sh_type)
        {
            case SHT_PROGBITS:
                /* If data section */
                if ((pHeader->sh_flags & SHF_ALLOC) &&
                    (pHeader->sh_flags & SHF_WRITE) &&
                    ((pHeader->sh_flags & SHF_EXECINSTR) == 0))
                {
                    /* Select sda */
                    switch(sda)
                    {
                        case NOT_SDA_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pSegInfo->sizeData
                                         );
                            pSegInfo->sizeData += pHeader->sh_size + nBytes;
                            if (pHeader->sh_addralign > dataAlign)
                            {
                                dataAlign = pHeader->sh_addralign;
                            }
                            break;

                        case SDA_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pHeader->sh_size
                                         );
                            pSdaInfo->sizeSdata = pHeader->sh_size + nBytes;
                            break;

                        case SDA2_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pHeader->sh_size
                                         );
                            pSdaInfo->sizeSdata2 = pHeader->sh_size + nBytes;
                            break;
                    }
                }
                else if((pHeader->sh_flags & SHF_ALLOC) &&
                        (pHeader->sh_flags & SHF_EXECINSTR))
                {
                    nBytes = loadElfAlignGet(
                                 pHeader->sh_addralign,
                                 (ARG) pSegInfo->sizeText
                                 );
                    pSegInfo->sizeText += pHeader->sh_size + nBytes;
                    if (pHeader->sh_addralign > textAlign)
                    {
                        textAlign = pHeader->sh_addralign;
                    }
                }
                else if (pHeader->sh_flags & SHF_ALLOC)
                {
                    /* If sdata2 section */
                    if (sda == SDA2_SECT)
                    {
                        nBytes = loadElfAlignGet(
                                     pHeader->sh_addralign,
                                     (ARG) pHeader->sh_size
                                     );
                        pSdaInfo->sizeSdata2 = pHeader->sh_size + nBytes;
                    }
                    else
                    {
                        /* Treat as text */
                        nBytes = loadElfAlignGet(
                                     pHeader->sh_addralign,
                                     (ARG) pSegInfo->sizeText
                                     );
                        pSegInfo->sizeText += pHeader->sh_size + nBytes;
                        if (pHeader->sh_addralign > textAlign)
                        {
                            textAlign = pHeader->sh_addralign;
                        }
                    }
                }
                break;

            case SHT_NOBITS:
                /* If bss section */
                if ((pHeader->sh_flags & SHF_ALLOC) &&
                    (pHeader->sh_flags & SHF_WRITE))
                {
                    /* Select sda */
                    switch(sda)
                    {
                        case NOT_SDA_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pSegInfo->sizeBss
                                         );
                            pSegInfo->sizeBss += pHeader->sh_size + nBytes;
                            if (pHeader->sh_addralign > bssAlign)
                            {
                                bssAlign = pHeader->sh_addralign;
                            }
                            break;

                        case SDA_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pHeader->sh_size
                                         );
                            pSdaInfo->sizeSbss = pHeader->sh_size + nBytes;
                            break;

                        case SDA2_SECT:
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         (ARG) pHeader->sh_size
                                         );
                            pSdaInfo->sizeSbss2 = pHeader->sh_size + nBytes;
                            break;
                    }
                }
                else
                {
                    fprintf(
                        stderr,
                        "Section SHT_NOBITS ignored (wrong flags).\n"
                        );
                }
                break;

            default:
                fprintf(
                    stderr,
                    "Section of type %d ignored.\n",
                    pHeader->sh_type
                    );
                break;
        }
    }

    /* STore align in flags */
    pSegInfo->flagsText = textAlign;
    pSegInfo->flagsData = dataAlign;
    pSegInfo->flagsBss  = bssAlign;

    /* Align text segment size */
    if ((textAlign > _ALLOC_ALIGN_SIZE) &&
        (pSegInfo->sizeText > 0))
    {
        pSegInfo->sizeText += textAlign - _ALLOC_ALIGN_SIZE;
    }

    /* Align data segment size */
    if ((dataAlign > _ALLOC_ALIGN_SIZE) &&
        (pSegInfo->sizeData > 0))
    {
        pSegInfo->sizeData += dataAlign - _ALLOC_ALIGN_SIZE;
    }

    /* Align bss segment size */
    if ((bssAlign > _ALLOC_ALIGN_SIZE) &&
        (pSegInfo->sizeBss > 0))
    {
        pSegInfo->sizeBss += bssAlign - _ALLOC_ALIGN_SIZE;
    }

    /* If only one memory area for all segments */
    if ((pSegInfo->addrText == LD_NO_ADDRESS) &&
        (pSegInfo->addrData == LD_NO_ADDRESS) &&
        (pSegInfo->addrBss == LD_NO_ADDRESS))
    {
        /* If data segment size */
        if (pSegInfo->sizeData > 0)
        {
            pSegInfo->sizeText += loadElfAlignGet(
                                      dataAlign,
                                      (ARG) pSegInfo->sizeText
                                      );
        }

        if (pSegInfo->sizeBss > 0)
        {
            if (pSegInfo->sizeData > 0)
            {
                pSegInfo->sizeData += loadElfAlignGet(
                                          bssAlign,
                                          (ARG) (pSegInfo->sizeText +
                                                 pSegInfo->sizeData)
                                          );
            }
            else
            {
                pSegInfo->sizeText += loadElfAlignGet(
                                          bssAlign,
                                          (ARG) pSegInfo->sizeText
                                          );
            }
        }
    }
}

/******************************************************************************
 * loadElfSymEntryRead - Read entry in symbol table
 *
 * RETURNS: Next symbol entry offset or ERROR
 */

LOCAL int loadElfSymEntryRead(
    int        fd,
    int        symEntry,
    Elf32_Sym *pSym
    )
{
    STATUS ret;
    int    nBytes = SYMSZ;

    /* Goto position in file */
    ioctl(fd, FIOSEEK, symEntry);

    if (read(fd, pSym, nBytes) != nBytes)
    {
        ret = ERROR;
    }
    else
    {
        ret = symEntry + nBytes;
    }

    return ret;
}

/******************************************************************************
 * loadElfSymTableRead - Read symbol table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSymTableRead(
    int        fd,
    int        nextSym,
    int        nSyms,
    Elf32_Sym *pSymTable
    )
{
    int    i;
    STATUS status = OK;

    /* For all symbols */
    for (i = 0; i < nSyms; i++)
    {
        nextSym = loadElfSymEntryRead(fd, nextSym, &pSymTable[i]);
        if (nextSym == ERROR)
        {
            status = ERROR;
            break;
        }
    }

    return status;
}

/******************************************************************************
 * loadElfSymTablesRead - Real symbol tables in module
 *
 * RETURNS: Number of symbol tables or ERROR
 */

LOCAL int loadElfSymTablesRead(
    u_int32_t     *pSymTableSectHeaderIndex,
    Elf32_Shdr    *pSectHeaderTable,
    int            fd,
    SYMTABLE_REFS *pSymTableRefs,
    SYMINFO_REFS  *pSymAddrRefs
    )
{
    int         ret;
    STATUS      status;
    u_int32_t   i;
    u_int32_t   nSyms;
    int         symTableNum;
    Elf32_Shdr *pHeader;

    /* Get number of symbol tables */
    for (symTableNum = 0;
         pSymTableSectHeaderIndex[symTableNum] != 0;
         symTableNum++);

    /* Allocate memory for symbol table references */
    *pSymTableRefs = (SYMTABLE_REFS) malloc(symTableNum * SYMSZ);
    if (*pSymTableRefs == NULL)
    {
        status = ERROR;
    }
    else
    {
        /* Allocate memory for symbol table info references */
        *pSymAddrRefs =
            (SYMINFO_REFS) malloc(symTableNum * sizeof(SYM_INFO_TABLE));
        if (*pSymAddrRefs == NULL)
        {
            free(*pSymTableRefs);
            status = ERROR;
        }
        else
        {
            status = OK;

            /* For all symbol tables */
            for (i = 0; pSymTableSectHeaderIndex[i] != 0; i++)
            {
                pHeader = &pSectHeaderTable[pSymTableSectHeaderIndex[i]];
                if (pHeader->sh_entsize != SYMSZ)
                {
                    free(*pSymTableRefs);
                    errnoSet(S_loadElfLib_SYMTAB);
                    status = ERROR;
                    break;
                }

                /* Get number of entries */
                nSyms = pHeader->sh_size / pHeader->sh_entsize;

                /* Allocate memory for entries */
                *pSymTableRefs[i] = (Elf32_Sym *) malloc(pHeader->sh_size);
                if (*pSymTableRefs[i] == NULL)
                {
                    free(*pSymTableRefs);
                    status = ERROR;
                    break;
                }

                /* Read symbols */
                if (loadElfSymTableRead(
                        fd,
                        pHeader->sh_offset,
                        nSyms,
                        *pSymTableRefs[i]) != OK)
                {
                    free(*pSymTableRefs);
                    status = ERROR;
                    break;
                }

                /* Allocate undefined symbol address */
                *pSymAddrRefs[i] =
                    (SYM_INFO_TABLE) malloc(nSyms * sizeof(SYM_INFO));
                if (*pSymAddrRefs[i] == NULL)
                {
                    free(*pSymTableRefs);
                    status = ERROR;
                    break;
                }
            }
        }
    }

    if (status != OK)
    {
        ret = ERROR;
    }
    else
    {
        ret = symTableNum;
    }

    return ret;
}

/******************************************************************************
 * loadElfSectRead - Read section from elf file
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSectRead(
    int              fd,
    char            *pSectStrTable,
    u_int32_t       *pLoadSectHeaderIndex,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE  sectAddrTable,
    SEG_INFO        *pSegInfo
    )

{
    STATUS         status;
    Elf32_Shdr    *pHeader;
    SECT_ADDR     *pAddr;
    int32_t        sectSize;
    int            i;
    int            nBytes;
    void          *pTargetLoadAddr;
    void          *pTargetSdaLoadAddr;
    SDA_SECT_TYPE  sda;
    SDA_INFO      *pSdaInfo;
    void          *pTextAddr = pSegInfo->addrText;
    void          *pDataAddr = pSegInfo->addrData;
    void          *pBssAddr  = pSegInfo->addrBss;

    /* For all loadable sections */
    for (i = 0; pLoadSectHeaderIndex[i] != 0; i++)
    {
        pHeader            = &pSectHeaderTable[pLoadSectHeaderIndex[i]];
        pAddr              = &sectAddrTable[pLoadSectHeaderIndex[i]];
        pTargetLoadAddr    = NULL;
        sectSize           = pHeader->sh_size;
        pTargetSdaLoadAddr = NULL;
        sda                = NOT_SDA_SECT;

        /* Select section type */
        switch (pHeader->sh_type)
        {
            case SHT_PROGBITS:
                /* If text section */
                if ((pHeader->sh_flags & SHF_ALLOC) &&
                    (pHeader->sh_flags & SHF_EXECINSTR))
                {
                    pTargetLoadAddr = pTextAddr;
                    nBytes = loadElfAlignGet(
                                 pHeader->sh_addralign,
                                 pTargetLoadAddr
                                 );
                    pTargetLoadAddr = (u_int8_t *) pTargetLoadAddr + nBytes;
                    pTextAddr       = (u_int8_t *) pTargetLoadAddr + sectSize;
                }
                else if ((pHeader->sh_flags & SHF_ALLOC) &&
                         (pHeader->sh_flags & SHF_WRITE))
                {
                    switch (sda)
                    {
                        case NOT_SDA_SECT:
                            pTargetLoadAddr = pDataAddr;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetLoadAddr
                                         );
                            pTargetLoadAddr =
                                (u_int8_t *) pTargetLoadAddr + nBytes;
                            pDataAddr = (u_int8_t *) pTargetLoadAddr + sectSize;
                            break;

                        case SDA_SECT:
                            pTargetLoadAddr = pSdaInfo->pAddrSdata;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetSdaLoadAddr
                                         );
                            pTargetSdaLoadAddr =
                                (u_int8_t *) pTargetSdaLoadAddr + nBytes;
                            break;

                        case SDA2_SECT:
                            pTargetLoadAddr = pSdaInfo->pAddrSdata2;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetSdaLoadAddr
                                         );
                            pTargetSdaLoadAddr =
                                (u_int8_t *) pTargetSdaLoadAddr + nBytes;
                            break;
                    }
                }
                else if (pHeader->sh_flags & SHF_ALLOC)
                {
                    /* If sdata2 section */
                    if (sda == SDA2_SECT)
                    {
                        pTargetSdaLoadAddr = pSdaInfo->pAddrSdata2;
                        nBytes = loadElfAlignGet(
                                     pHeader->sh_addralign,
                                     pTargetSdaLoadAddr
                                     );
                        pTargetSdaLoadAddr =
                            (u_int8_t *) pTargetSdaLoadAddr + nBytes;
                    }
                    else
                    {
                        /* Treat as text */
                        pTargetLoadAddr = pTextAddr;
                        nBytes = loadElfAlignGet(
                                     pHeader->sh_addralign,
                                     pTargetLoadAddr
                                     );
                        pTargetLoadAddr = (u_int8_t *) pTargetLoadAddr + nBytes;
                        pTextAddr = (u_int8_t *) pTargetLoadAddr + sectSize;
                    }
                }
                else
                {
                    fprintf(
                        stderr,
                        "Section SHT_PROGBITS ignored (flags: %d).\n",
                        pHeader->sh_flags
                        );
                }
                status = OK;
                break;

            case SHT_NOBITS:
                /* If bss section */
                if ((pHeader->sh_flags & SHF_ALLOC) &&
                    (pHeader->sh_flags & SHF_WRITE))
                {
                    switch (sda)
                    {
                        case NOT_SDA_SECT:
                            pTargetLoadAddr = pBssAddr;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetLoadAddr
                                         );
                            pTargetLoadAddr =
                                (u_int8_t *) pTargetLoadAddr + nBytes;
                            pBssAddr = (u_int8_t *) pTargetLoadAddr + sectSize;
                            break;

                        case SDA_SECT:
                            pTargetSdaLoadAddr = pSdaInfo->pAddrSbss;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetSdaLoadAddr
                                         );
                            pTargetSdaLoadAddr =
                                (u_int8_t *) pTargetSdaLoadAddr + nBytes;
                            break;

                        case SDA2_SECT:
                            pTargetSdaLoadAddr = pSdaInfo->pAddrSbss2;
                            nBytes = loadElfAlignGet(
                                         pHeader->sh_addralign,
                                         pTargetSdaLoadAddr
                                         );
                            pTargetSdaLoadAddr =
                                (u_int8_t *) pTargetSdaLoadAddr + nBytes;
                            break;
                    }
                }
                else
                {
                    fprintf(
                        stderr,
                        "Section SHT_NOBITS ignored (flags: %d).\n",
                        pHeader->sh_flags
                        );
                }
                status = OK;
                break;

            default:
                fprintf(
                    stderr,
                    "Section of type %d ignored.\n",
                    pHeader->sh_type
                    );
                status = OK;
                break;
        }

        if (pHeader->sh_type != SHT_NOBITS)
        {
            ioctl(fd, FIOSEEK, pHeader->sh_offset);

            if (sda == NOT_SDA_SECT)
            {
                /* Read from file */
                if (read(fd, pTargetLoadAddr, sectSize) != sectSize)
                {
                    errnoSet(S_loadElfLib_SCN_READ);
                    status = ERROR;
                    break;
                }
            }
            else
            {
                if (read(fd, pTargetSdaLoadAddr, sectSize) != sectSize)
                {
                    errnoSet(S_loadElfLib_SCN_READ);
                    status = ERROR;
                    break;
                }
            }
        }

        /* Store address */
        if (sda == NOT_SDA_SECT)
        {
            *pAddr = pTargetLoadAddr;
        }
        else
        {
            *pAddr = pTargetSdaLoadAddr;
        }
    }

    return status;
}

/******************************************************************************
 * loadElfRelocMod - Realocate module
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfRelocMod(
    SEG_INFO        *pSegInfo,
    int              fd,
    char            *pSectStrTable,
    INDEX_TABLES    *pIndex,
    Elf32_Ehdr      *pHeader,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE *pSectAddrTable
    )
{
    STATUS status;

    /* Allocate memory */
    *pSectAddrTable =
        (SECT_ADDR_TABLE) malloc(pHeader->e_shnum * sizeof(SECT_ADDR));
    if (*pSectAddrTable == NULL)
    {
        status = ERROR;
    }
    else
    {
        status = loadElfSectRead(
                     fd,
                     pSectStrTable,
                     pIndex->pLoadSectHeaderIndex,
                     pSectHeaderTable,
                     *pSectAddrTable,
                     pSegInfo
                     );
    }

    return status;
}

/******************************************************************************
 * loadElfSegStore - Store elf segment in memory
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSegStore(
    SEG_INFO        *pSegInfo,
    int              loadFlag,
    int              fd,
    char            *pSectStrTable,
    INDEX_TABLES    *pIndex,
    Elf32_Ehdr      *pHeader,
    Elf32_Shdr      *pSectHeaderTable,
    Elf32_Phdr      *pProgHeaderTable,
    SECT_ADDR_TABLE *pSectAddrTable
    )
{
    return loadElfRelocMod(
               pSegInfo,
               fd,
               pSectStrTable,
               pIndex,
               pHeader,
               pSectHeaderTable,
               pSectAddrTable
               );
}

/******************************************************************************
 * loadElfSymTypeGet - Get symbol type
 *
 * RETURNS: SYM_TYPE
 */

LOCAL SYM_TYPE loadElfSymTypeGet(
    Elf32_Sym  *pSym,
    Elf32_Shdr *pSectHeaderTable,
    char       *pSectStrTable
    )
{
    SYM_TYPE    symType = SYM_UNDF;
    Elf32_Shdr *pHeader = &pSectHeaderTable[pSym->st_shndx];

    if (pSym->st_shndx == SHN_ABS)
    {
        symType = SYM_ABS;
    }
    else
    {
        /* Select type */
        switch (pHeader->sh_type)
        {
            case SHT_PROGBITS:
                /* If text section */
                if ((pHeader->sh_flags & SHF_ALLOC) &&
                    (pHeader->sh_flags & SHF_EXECINSTR))
                {
                    symType |= SYM_TEXT;
                }
                else if (pHeader->sh_flags & SHF_ALLOC)
                {
                    symType |= SYM_DATA;
                }
                break;

            case SHT_NOBITS:
                /* If bss section */
                if (pHeader->sh_flags & SHF_ALLOC)
                {
                    symType |= SYM_BSS;
                }
                break;

            default:
                symType = SYM_UNDF;
                break;
        }
    }

    return symType;
}

/******************************************************************************
 * loadElfSymVisible - Check if symbol should be added to symbol table
 *
 * RETURNS: TRUE or FALSE
 */

LOCAL BOOL loadElfSymVisible(
    u_int32_t symAssoc,
    u_int32_t symBind,
    int       loadFlag
    )
{
    BOOL ret;

    if (!((symAssoc == STT_OBJECT) ||
          (symAssoc == STT_FUNC)))
    {
        ret = FALSE;
    }
    else if (!(((loadFlag & LOAD_LOCAL_SYMBOLS) && (symBind == STB_LOCAL)) ||
               ((loadFlag & LOAD_GLOBAL_SYMBOLS) &&
              ((symBind == STB_GLOBAL) || (symBind == STB_WEAK)))))
    {
        ret = FALSE;
    }
    else
    {
        ret = TRUE;
    }

    return ret;
}

/******************************************************************************
 * loadElfSymTableProcess - Process elf module symbol table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSymTableProcess(
    MODULE_ID        moduleId,
    int              loadFlag,
    Elf32_Sym       *pSymArray,
    SECT_ADDR_TABLE  sectAddrTable,
    SYM_INFO_TABLE   symAddrTable,
    char            *pStrTable,
    SYMTAB_ID        symTable,
    u_int32_t        symNum,
    Elf32_Shdr      *pSectHeaderTable,
    char            *pSectStrTable,
    SEG_INFO        *pSegInfo
    )
{
    static char exportName[255];

    char      *name;
    Elf32_Sym *pSym;
    SYM_TYPE   symType;
    u_int32_t  i;
    u_int32_t  symBind;
    u_int32_t  symAccos;
    u_int32_t  commonSize;
    u_int32_t  commonAlign;
    SYM_ADDR   addr      = NULL;
    SYM_ADDR   bias      = NULL;
    SYM_ADDR  *pSectAddr = NULL;
    STATUS     status    = OK;

    /* For all symbols */
    for (i = 0; i < symNum; i++)
    {
        /* Get current symbol */
        pSym = &pSymArray[i];

        /* Get bindings and accosiation */
        symBind = ELF32_ST_BIND(pSym->st_info);
        symAccos = ELF32_ST_TYPE(pSym->st_info);

        /* Get symbol name in string table */
        name = &pStrTable[pSym->st_name];

        /* Export name with preceding underscore */
        strcpy(exportName, "_");
        strcat(exportName, name);

        /* If export symbol */
        if ((pSym->st_shndx != SHN_UNDEF) &&
            (pSym->st_shndx != SHN_COMMON))
        {
            /* If no bias */
            if (pSym->st_shndx == SHN_ABS)
            {
                bias = NULL;
            }
            else
            {
                pSectAddr = &sectAddrTable[pSym->st_shndx];
                bias      = *pSectAddr;
            }

            /* Get symbol type */
            symType = loadElfSymTypeGet(pSym, pSectHeaderTable, pSectStrTable);

            /* If symbol is visible */
            if (loadElfSymVisible(symAccos, symBind, loadFlag) == TRUE)
            {
                /* If symbol is global */
                if ((symBind == STB_GLOBAL) || (symBind == STB_WEAK))
                {
                    symType |= SYM_GLOBAL;
                }

                /* Add symbol to symbol table */
                if (symAdd(
                        symTable,
                        exportName,
                        (char *) (pSym->st_value + (u_int32_t) bias),
                        symType,
                        moduleId->group
                        ) != OK)
                {
                    fprintf(stderr, "Can't add '%s' to symbol table\n", name);
                    status = ERROR;
                }
            }

            symAddrTable[i].pAddr =
                (SYM_ADDR) (pSym->st_value + (u_int32_t) bias);
            symAddrTable[i].type = symType;
        }
        else
        {
            /* If global symbol */
            if (pSym->st_shndx == SHN_COMMON)
            {
                /* Load common symbol */
                commonSize  = pSym->st_size;
                commonAlign = pSym->st_value;
                if (loadCommonManage(
                        commonSize,
                        commonAlign,
                        name,
                        symTable,
                        &addr,
                        &symType,
                        loadFlag,
                        pSegInfo,
                        moduleId->group
                        ) != OK)
                {
                    status = ERROR;
                }
            }
            else
            {
                /* If local and notype */
                if ((symBind == STB_LOCAL) && (symAccos == STT_NOTYPE))
                {
                    continue;
                }

                if (symFindByNameAndType(
                        symTable,
                        exportName,
                        (ARG *) &addr,
                        &symType,
                        SYM_GLOBAL,
                        SYM_GLOBAL
                        ) != OK)
                {
                    addr = NULL;
                    fprintf(
                        stderr,
                        "Undefined symbol: %s (binding %d type %d)\n",
                        name,
                        symBind,
                        symAccos
                        );
                    /* NEW: Error on unresolved symbols */
                    status = ERROR;
                }
            }

            symAddrTable[i].pAddr = addr;
            symAddrTable[i].type  = symType;
        }
    }

    return status;
}

/******************************************************************************
 * loadElfSymTableBuild - Build symbol table
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSymTableBuild(
    MODULE_ID        moduleId,
    int              loadFlag,
    SYMTABLE_REFS    symTableRefs,
    SECT_ADDR_TABLE  sectAddrTable,
    SYMINFO_REFS     symAddrRefs,
    INDEX_TABLES    *pIndex,
    SYMTAB_ID        symTable,
    int              fd,
    Elf32_Shdr      *pSectHeaderTable,
    char            *pSectStrTable,
    SEG_INFO        *pSegInfo
    )
{
    u_int32_t   i;
    u_int32_t   nSyms;
    Elf32_Shdr *pSymHeader;
    Elf32_Shdr *pStrHeader;
    char       *pStrTable;
    STATUS      status = OK;

    /* For all symbol tables */
    for (i = 0; pIndex->pSymTableSectHeaderIndex[i] != 0; i++)
    {
        /* Get symbol table header */
        pSymHeader = &pSectHeaderTable[pIndex->pSymTableSectHeaderIndex[i]];

        /* Get number of symbols */
        nSyms = pSymHeader->sh_size / pSymHeader->sh_entsize;

        /* Get string table header */
        pStrHeader = &pSectHeaderTable[pSymHeader->sh_link];

        /* Allocate string table */
        pStrTable = (char *) malloc(pStrHeader->sh_size);
        if (pStrTable == NULL)
        {
            status = ERROR;
            break;
        }

        if ((ioctl(fd, FIOSEEK, pStrHeader->sh_offset) == ERROR) ||
            (read(fd, pStrTable, pStrHeader->sh_size) != pStrHeader->sh_size))
        {
            errnoSet(S_loadElfLib_READ_SECTIONS);
            free(pStrTable);
            status = ERROR;
            break;
        }

        /* Process symbols */
        if (loadElfSymTableProcess(
                moduleId,
                loadFlag,
                symTableRefs[i],
                sectAddrTable,
                symAddrRefs[i],
                pStrTable,
                symTable,
                nSyms,
                pSectHeaderTable,
                pSectStrTable,
                pSegInfo
                ) != OK)
        {
            free(pStrTable);
            status = ERROR;
            break;
        }
    }

    return status;
}

/******************************************************************************
 * loadElfRelocRelEntryRead - Real elf relocation entry
 *
 * RETURNS: Address of the next relocation entry or ERROR
 */

int loadElfRelocRelEntryRead(
    int        fd,
    int        pos,
    Elf32_Rel *pRel
    )
{
    int ret;
    int nBytes = RELSZ;

    /* Goto position in file */
    if (ioctl(fd, FIOSEEK, pos) == ERROR)
    {
        ret = ERROR;
    }
    else
    {
        if (read(fd, pRel, nBytes) != nBytes)
        {
            ret = ERROR;
        }
        else
        {
            ret = pos + nBytes;
        }
    }

    return ret;
}

/******************************************************************************
 * loadElfSegReloc - Relocate code
 *
 * RETURNS: OK or ERROR
 */

LOCAL STATUS loadElfSegReloc(
    int              fd,
    int              loadFlag,
    MODULE_ID        moduleId,
    Elf32_Ehdr      *pHeader,
    INDEX_TABLES    *pIndex,
    Elf32_Shdr      *pSectHeaderTable,
    SECT_ADDR_TABLE  sectAddrTable,
    SYMTABLE_REFS    symTableRefs,
    SYMINFO_REFS     symAddrRefs,
    SYMTAB_ID        symTable,
    SEG_INFO        *pSegInfo
    )
{
    Elf32_Shdr *pSymHeader;
    Elf32_Shdr *pStrHeader;
    Elf32_Shdr *pRelHeader;
    u_int32_t   i;
    u_int32_t   j;
    STATUS      status = OK;

    /* For all relocation headers */
    for (i = 0; pIndex->pRelSectHeaderIndex[i] != 0; i++)
    {
        /* Get relocation header */
        pRelHeader = &pSectHeaderTable[pIndex->pRelSectHeaderIndex[i]];

        /* Goto current index */
        for (j = 0;
             pIndex->pSymTableSectHeaderIndex[j] != pRelHeader->sh_link;
             j++);

        /* Relocate entry */
        if ((*pElfSegRelFunc)(
                 fd,
                 moduleId,
                 loadFlag,
                 pRelHeader->sh_offset,
                 pSectHeaderTable,
                 pRelHeader,
                 sectAddrTable + pRelHeader->sh_info,
                 symAddrRefs[j],
                 symTableRefs[i],
                 symTable,
                 pSegInfo
                 ) != OK)
        {
            status = ERROR;
            break;
        }
    }

    return status;
}

/******************************************************************************
 * loadElf - Load elf module
 *
 * RETURNS: MODULE_ID or NULL
 */

LOCAL MODULE_ID loadElf(
    int         fd,
    int         loadFlag,
    void      **ppText,
    void      **ppData,
    void      **ppBss,
    SYMTAB_ID   symTable
    )
{
    char             name[NAME_MAX];
    MODULE_ID        modId;
    Elf32_Ehdr       header;
    INDEX_TABLES     indexTables;
    SEG_INFO         segInfo;
    BOOL             segAddStatus;
    u_int32_t        i;
    Elf32_Phdr      *pProgHeaderTable = NULL;
    Elf32_Shdr      *pSectHeaderTable = NULL;
    SYMTABLE_REFS    symTableRefs = NULL;
    SECT_ADDR_TABLE  sectAddrTable = NULL;
    SYMINFO_REFS     symAddrRefs = NULL;
    char            *pSectStrTable = NULL;
    int              symTableNum = 0;

    /* Inititalize locals */
    memset(&indexTables, 0, sizeof(INDEX_TABLES));
    memset(&segInfo, 0, sizeof(SEG_INFO));

    /* Get module name */
    if (ioctl(fd, FIOGETNAME, (int) name) == ERROR)
    {
        sprintf(name, "module%d\n", loadElfModuleNum++);
    }

    /* Create module */
    modId = loadModuleGet(name, MODULE_ELF, &loadFlag);
    if (modId == NULL)
    {
        return NULL;
    }

    /* Read header */
    if (loadElfHeaderRead(fd, &header) != OK)
    {
        errnoSet(S_loadElfLib_HDR_READ);
        goto loadElfError;
    }

    /* Verify target */
    if (loadElfArchVerify(&header) != TRUE)
    {
        errnoSet(S_loadElfLib_HDR_READ);
        goto loadElfError;
    }

    /* Allocate index tables */
    if (loadElfTablesAlloc(
            &header,
            &pProgHeaderTable,
            &pSectHeaderTable,
            &indexTables
            ) != OK)
    {
        goto loadElfError;
    }

    /* Read program header table */
    if (header.e_phnum)
    {
        if (loadElfProgHeaderRead(
                fd,
                header.e_phoff,
                pProgHeaderTable, 
                header.e_phnum
                ) != OK)
        {
            goto loadElfError;
        }
    }

    /* Read section header table */
    if (header.e_shnum)
    {
        if (loadElfSectHeaderRead(
                fd,
                header.e_shoff,
                pSectHeaderTable,
                header.e_shnum,
                &indexTables
                ) != OK)
        {
            goto loadElfError;
        }
    }

    pSectStrTable = loadElfSectStrTableRead(fd, pSectHeaderTable, &header);
    if (pSectStrTable == NULL)
    {
        goto loadElfError;
    }

    /* Setup segment allocation hints */
    if (ppText == NULL)
    {
        segInfo.addrText = LD_NO_ADDRESS;
    }
    else
    {
        segInfo.addrText = (char *) *ppText;
    }

    if (ppData == NULL)
    {
        segInfo.addrData = LD_NO_ADDRESS;
    }
    else
    {
        segInfo.addrData = (char *) *ppData;
    }

    if (ppBss == NULL)
    {
        segInfo.addrBss = LD_NO_ADDRESS;
    }
    else
    {
        segInfo.addrBss = (char *) *ppBss;
    }

    /* Get segment sizes */
    loadElfSegSizeGet(
        pSectStrTable,
        indexTables.pLoadSectHeaderIndex,
        pSectHeaderTable,
        &segInfo
        );

    /* Calculate number of symbol tables */
    symTableNum = loadElfSymTablesRead(
                      indexTables.pSymTableSectHeaderIndex,
                      pSectHeaderTable,
                      fd,
                      &symTableRefs,
                      &symAddrRefs
                      );
    if (symTableNum == ERROR)
    {
        goto loadElfError;
    }

    /* Allocate segments */
    if (loadSegmentsAllocate(&segInfo) != OK)
    {
        fprintf(stderr, "could not allocate text and data segments\n");
        goto loadElfError;
    }

    /* Store segments */
    if (loadElfSegStore(
            &segInfo,
            loadFlag,
            fd,
            pSectStrTable,
            &indexTables,
            &header,
            pSectHeaderTable,
            pProgHeaderTable,
            &sectAddrTable
            ) != OK)
    {
        goto loadElfError;
    }

    /* Add symbols to symbol table */
    if (loadElfSymTableBuild(
            modId,
            loadFlag,
            symTableRefs,
            sectAddrTable,
            symAddrRefs,
            &indexTables,
            sysSymTable,
            fd,
            pSectHeaderTable,
            pSectStrTable,
            &segInfo
            ) != OK)
    {
        goto loadElfError;
    }

    if (loadElfSegReloc(
            fd,
            loadFlag,
            modId,
            &header,
            &indexTables,
            pSectHeaderTable,
            sectAddrTable,
            symTableRefs,
            symAddrRefs,
            sysSymTable,
            &segInfo
            ) != OK)
    {
        goto loadElfError;
    }

    /* Clear temporary buffers */
    if (symTableRefs != NULL)
    {
        for (i = 0; i < symTableNum; i++)
        {
            free(symTableRefs[i]);
        }

        free(symTableRefs);
    }

    if (symAddrRefs != NULL)
    {
        for (i = 0; i < symTableNum; i++)
        {
            free(symAddrRefs[i]);
        }

        free(symAddrRefs);
    }

    loadElfTablesFree(&pProgHeaderTable, &pSectHeaderTable, &indexTables);
    free(sectAddrTable);
    free(pSectStrTable);

    /* Store load addresses */
    if (ppText != NULL)
    {
        *ppText = segInfo.addrText;
    }

    if (ppData != NULL)
    {
        *ppData = segInfo.addrData;
    }

    if (ppBss != NULL)
    {
        *ppBss = segInfo.addrBss;
    }

    /* Inititalize bss segment */
    if (segInfo.sizeBss != 0)
    {
        memset(segInfo.addrBss, 0, segInfo.sizeBss);
    }

    /* Check if loaded */
    if ((segInfo.addrText == LD_NO_ADDRESS) &&
        (segInfo.addrData == LD_NO_ADDRESS) &&
        (segInfo.addrBss == LD_NO_ADDRESS))
    {
        fprintf(stderr, "Object module not loaded\n");
        goto loadElfError;
    }

    /* Add segments to module */
    segAddStatus = (moduleSegAdd(
                        modId,
                        SEGMENT_TEXT,
                        segInfo.addrText,
                        segInfo.sizeText,
                        segInfo.flagsText
                        ) == OK) ? TRUE : FALSE;

    segAddStatus = (moduleSegAdd(
                        modId,
                        SEGMENT_DATA,
                        segInfo.addrData,
                        segInfo.sizeData,
                        segInfo.flagsData
                        ) == OK) ? TRUE : FALSE;

    segAddStatus = (moduleSegAdd(
                        modId,
                        SEGMENT_BSS,
                        segInfo.addrBss,
                        segInfo.sizeBss,
                        segInfo.flagsBss
                        ) == OK) ? TRUE : FALSE;

    /* If segment add failed */
    if (segAddStatus != TRUE)
    {
        fprintf(stderr, "Object module load failed\n");
        goto loadElfError;
    }

    return modId;

loadElfError:
    moduleDelete(modId);

    return NULL;
}

