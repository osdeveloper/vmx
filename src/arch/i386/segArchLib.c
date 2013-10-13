/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* segArchLib.c - System dependet C segment intitiation code */

#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/arch.h>
#include <arch/sysArchLib.h>

/* Global variables to hold segment pointers */
u_int32_t sysCsSuper;
u_int32_t sysCsExc;
u_int32_t sysCsInt;

/* Extern sysGdt flush funtion in segALib.s */
IMPORT sysGDTSet(SEGDESC *baseAddr);

/* Local segment base holder */
LOCAL SEGDESC *segBase = 0;

/* Setup a GDT entry */
void segEntryInit(int num, u_int32_t base, u_int32_t limit,
		 	    u_int8_t access, u_int8_t gran)
{
  /* Setup the descriptor base address */
  segBase[num].baseLW = (base & 0xffff);
  segBase[num].baseMB = (base >> 16) & 0xff;
  segBase[num].baseUB = (base >> 24) & 0xff;

  /* Setup the descriptor limits */
  segBase[num].limitLW = (limit & 0xffff);
  segBase[num].limitUB = ((limit >> 16) & 0x0f);

  /* Finnaly, setup the granularity and access flags */
  segBase[num].limitUB |= (gran & 0xf0);
  segBase[num].type = access;
}

/* Setup special GDT pointer and Segments */
STATUS segBaseSet(SEGDESC *baseAddr)
{
  u_int8_t gdt[6];
  u_int8_t *p = gdt;

  /* Store pointer locally */
  segBase = baseAddr;

  /* Setup GDT pointer */
  *(u_int16_t *) p = (sizeof(SEGDESC) * GDT_ENTRIES) - 1;
  *(u_int32_t *) (p + 2) = (u_int32_t) baseAddr;

  /* NULL descriptor */
  /* 0x0000 */
  segEntryInit(0, 0, 0, 0, 0);

  /* Code descriptor, for the supervisor mode task */
  /* 0x0008 */
  segEntryInit(1, 0, 0xffffffff, 0x9a, 0xcf);
  sysCsSuper = 0x00000008;

  /* Data descriptor */
  /* 0x0010 */
  segEntryInit(2, 0, 0xffffffff, 0x92, 0xcf);

  /* Code descriptor, for the exception */
  /* 0x0018 */
  segEntryInit(3, 0, 0xffffffff, 0x9a, 0xcf);
  sysCsExc = 0x00000018;

  /* Code descriptor, for the interrupt */
  /* 0x0020 */
  segEntryInit(4, 0, 0xffffffff, 0x9a, 0xcf);
  sysCsInt = 0x00000020;

  /* Now flush the GDT */
  sysGDTSet((SEGDESC *) gdt);

  return(OK);
}

