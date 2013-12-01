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

/* elf.h - Loadable file format header */

#ifndef _ELF_H
#define _ELF_H

#define EI_NIDENT       16

/* Values for e_ident */
#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_PAD          7

#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define ELFMAG          "\177ELF"
#define SELFMAG         4

/* EI_CLASS */
#define ELFCLASSNONE    0
#define ELFCLASS32      1
#define ELFCLASS64      2

/* EI_DATA */
#define ELFDATANONE     0
#define ELFDATA2LSB     1
#define ELFDATA2MSB     2

/* e_type */
#define ET_NONE         0
#define ET_REL          1
#define ET_EXEC         2
#define ET_DYN          3
#define ET_CORE         4
#define ET_LOPROC       0xff00
#define ET_HIPROC       0xffff

/* e_machine */
#define EM_NONE         0
#define EM_M32          1
#define EM_SPARC        2
#define EM_386          3
#define EM_68K          4
#define EM_88K          5
#define EM_486          6
#define EM_860          7
#define EM_MIPS         8
#define EM_PPC          20
#define EM_MCORE        39
#define EM_SH           42
#define EM_SC           58
#define EM_ARM          40

/* e_flags */

/* e_version */
#define EV_NONE         0
#define EV_CURRENT      1

/* Section indexes */
#define SHN_UNDEF       0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff
#define SHN_GHCOMMON    0xff00

/* sh_type */
#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_COMDAT      12
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

/* sh_flags */
#define SHF_WRITE       0x1
#define SHF_ALLOC       0x2
#define SHF_EXECINSTR   0x4
#define SHF_MASKPROC    0xf0000000

/* Symbol table */
#define STN_UNDEF       0

#define STB_LOCAL       0
#define STB_GLOBAL      1
#define STB_WEAK        2
#define STB_LOPROC      13
#define STB_HIPROC      15

#define STT_NOTYPE      0
#define STT_OBJECT      1
#define STT_FUNC        2
#define STT_SECTION     3
#define STT_FILE        4
#define STT_LOPROC      13
#define STT_HIPROC      15

/* p_type */
#define PT_NULL         0
#define PT_LOAD         1
#define PT_DYNAMIC      2
#define PT_INTERP       3
#define PT_NOTE         4
#define PT_SHLIB        5
#define PHDR            6
#define PT_LOPROC       0x70000000
#define PT_HIPROC       0x7fffffff

/* p_flags */
#define PF_X            0x1
#define PF_W            0x2
#define PF_R            0x4
#define PF_MASKPROC     0xf0000000

/* Dynamic */
#define DT_NULL         0
#define DT_NEEDED       1
#define DT_PLTRELSZ     2
#define DT_PLTGOT       3
#define DT_HASH         4
#define DT_STRTAB       5
#define DT_SYMTAB       6
#define DT_RELA         7
#define DT_RELASZ       8
#define DT_RELAENT      9
#define DT_STRSZ        10
#define DT_SYMENT       11
#define DT_INIT         12
#define DT_FINI         13
#define DT_SONAME       14
#define DT_RPATH        15
#define DT_SYMBOLIC     16
#define DT_REL          17
#define DT_RELSZ        18
#define DT_RELENT       19
#define DT_PLTREL       20
#define DT_DEBUG        21
#define DT_TEXTREL      22
#define DT_JMPREL       23
#define DT_LOPROC       0x70000000
#define DT_HIPROC       0x7fffffff

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */

typedef unsigned long   Elf32_Addr;
typedef unsigned short  Elf32_Half;
typedef unsigned long   Elf32_Off;
typedef long            Elf32_Sword;
typedef unsigned long   Elf32_Word;

/* Header */
typedef struct
{
    unsigned char         e_ident[EI_NIDENT];
    Elf32_Half            e_type;
    Elf32_Half            e_machine;
    Elf32_Word            e_version;
    Elf32_Addr            e_entry;
    Elf32_Off             e_phoff;
    Elf32_Off             e_shoff;
    Elf32_Word            e_flags;
    Elf32_Half            e_ehsize;
    Elf32_Half            e_phentsize;
    Elf32_Half            e_phnum;
    Elf32_Half            e_shentsize;
    Elf32_Half            e_shnum;
    Elf32_Half            e_shstrndx;
} Elf32_Ehdr;

/* Section header */
typedef struct
{
    Elf32_Word            sh_name;
    Elf32_Word            sh_type;
    Elf32_Word            sh_flags;
    Elf32_Addr            sh_addr;
    Elf32_Off             sh_offset;
    Elf32_Word            sh_size;
    Elf32_Word            sh_link;
    Elf32_Word            sh_info;
    Elf32_Word            sh_addralign;
    Elf32_Word            sh_entsize;
} Elf32_Shdr;

/* Symbol table */
typedef struct
{
    Elf32_Word            st_name;
    Elf32_Addr            st_value;
    Elf32_Word            st_size;
    unsigned char         st_info;
    unsigned char         st_other;
    Elf32_Half            st_shndx;
} Elf32_Sym;

/* Relocation */
typedef struct
{
    Elf32_Addr            r_offset;
    Elf32_Word            r_info;
} Elf32_Rel;

typedef struct
{
    Elf32_Addr            r_offset;
    Elf32_Word            r_info;
    Elf32_Sword           r_addend;
} ELF32_Rela;

/* Program header */
typedef struct
{
    Elf32_Word            p_type;
    Elf32_Off             p_offset;
    Elf32_Addr            p_vaddr;
    Elf32_Addr            p_paddr;
    Elf32_Word            p_filesz;
    Elf32_Word            p_memsz;
    Elf32_Word            p_flags;
    Elf32_Word            p_align;
} Elf32_Phdr;

/* Dynamic */
typedef struct
{
    Elf32_Sword           d_tag;
    union
    {
        Elf32_Word            d_val;
        Elf32_Addr            d_ptr;
    } d_un;
} Elf32_Dyn;

/* Macros */

#define EHDRSZ                          sizeof(Elf32_Ehdr)
#define SHDRSZ                          sizeof(Elf32_Shdr)
#define PHDRSZ                          sizeof(Elf32_Phdr)
#define SYMSZ                           sizeof(Elf32_Sym)
#define RELSZ                           sizeof(Elf32_Rel)

#define ELF32_ST_BIND(info)             ((info) >> 4)
#define ELF32_ST_TYPE(info)             ((info) & 0xf)
#define ELF32_ST_INFO(bind, type)       (((bind) << 4) + ((type) & 0xf))

#define ELF32_R_SYM(info)               ((info) >> 8)
#define ELF32_R_TYPE(info)              ((unsigned char) (info))
#define ELF32_R_INFO(sym, type)         (((sym) << 8) + (unsigned char) (type))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _ELF_H */

