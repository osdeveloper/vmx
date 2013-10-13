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

/* i8259Pic.c - Programable interrupt controller */

#include <sys/types.h>
#include <vmx.h>
#include <arch/regs.h>
#include <arch/iv.h>
#include <arch/esf.h>

#include <drv/intrCtl/i8259a.h>

/* Externals */
IMPORT u_int32_t               sysVectorIRQ0;

/******************************************************************************
 * sysIntInitPIC - Initialize PIC
 *
 * RETURNS: N/A
 */

void sysIntInitPIC(
    void
    )
{

    /* Remap hw interrupts above exception table */
    /* IRQ 0-7 Master */
    sysOutByte(PIC_Port1(PIC1_BASE_ADR), 0x11);
    sysOutByte(PIC_Port2(PIC1_BASE_ADR), sysVectorIRQ0);
    sysOutByte(PIC_Port2(PIC1_BASE_ADR), 0x04);
    sysOutByte(PIC_Port2(PIC1_BASE_ADR), 0x01);

    /* IRQ 8 - 15 Slave */
    sysOutByte(PIC_Port1(PIC2_BASE_ADR), 0x11);
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), sysVectorIRQ0+8);
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), 0x02);
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), 0x01);

#if 0
    /* Disable interrupts */
    sysOutByte(PIC_IMASK(PIC1_BASE_ADR), 0xfb);
    sysOutByte(PIC_IMASK(PIC2_BASE_ADR), 0xff);
#endif

    /* Enable interrupts */
    sysOutByte(PIC_IMASK(PIC1_BASE_ADR), 0x00);
    sysOutByte(PIC_IMASK(PIC2_BASE_ADR), 0x00);
}

