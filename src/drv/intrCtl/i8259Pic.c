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
#include <arch/intArchLib.h>

#include <drv/intrCtl/i8259a.h>

/* Imports */
IMPORT u_int32_t sysVectorIRQ0;

/* Locals */
LOCAL u_int8_t sysIntMask1;
LOCAL u_int8_t sysIntMask2;

LOCAL void sysIntEOI(
    void
    );

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
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), sysVectorIRQ0 + 8);
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), 0x02);
    sysOutByte(PIC_Port2(PIC2_BASE_ADR), 0x01);

    /* Disable interrupts */
    sysOutByte(PIC_IMASK(PIC1_BASE_ADR), 0xfb);
    sysOutByte(PIC_IMASK(PIC2_BASE_ADR), 0xff);

    /* Install end of interrupt handler */
    intEoi = sysIntEOI;
}

/******************************************************************************
 * sysIntEOI - Send end of interrupt signal
 *
 * RETURNS: N/A
 */

LOCAL void sysIntEOI(
    void
    )
{
    int level;

    /* Lock interrupts */
    INT_LOCK(level);

    /* Send acknowlegement to both interrput controller porst */
    sysOutByte(PIC_IACK(PIC1_BASE_ADR), 0x20);
    sysOutByte(PIC_IACK(PIC2_BASE_ADR), 0x20);

    /* Unlock interrputs */
    INT_UNLOCK(level);
}

/******************************************************************************
 * sysIntEnablePIC - Enable interrupt level
 *
 * RETURNS: OK
 */

STATUS sysIntEnablePIC(
    int level
    )
{
    /* If level lt. 8 */
    if (level < 8)
    {
        sysOutByte(
            PIC_IMASK(PIC1_BASE_ADR),
            sysInByte(PIC_IMASK(PIC1_BASE_ADR)) & ~(1 << level)
            );
    }
    else
    {
        sysOutByte(
            PIC_IMASK(PIC2_BASE_ADR),
            sysInByte(PIC_IMASK(PIC2_BASE_ADR)) & ~(1 << (level - 8))
            );
    }

    return OK;
}

/******************************************************************************
 * sysIntDisablePIC - Disable interrupt level
 *
 * RETURNS: OK
 */

STATUS sysIntDisablePIC(
    int level
    )
{
    /* If level lt. 8 */
    if (level < 8)
    {
        sysOutByte(
            PIC_IMASK(PIC1_BASE_ADR),
            sysInByte(PIC_IMASK(PIC1_BASE_ADR)) | (1 << level)
            );
    }
    else
    {
        sysOutByte(
            PIC_IMASK(PIC2_BASE_ADR),
            sysInByte(PIC_IMASK(PIC2_BASE_ADR)) | (1 << (level - 8))
            );
    }

    return OK;
}

/******************************************************************************
 * sysIntLock - Disable interrupts and store level
 *
 * RETURNS: N/A
 */

void sysIntLock(
    void
    )
{
    /* Store current levels */
    sysIntMask1 = sysInByte(PIC_IMASK(PIC1_BASE_ADR) );
    sysIntMask2 = sysInByte(PIC_IMASK(PIC2_BASE_ADR) );

    /* Lock out both */
    sysOutByte(PIC_IMASK(PIC1_BASE_ADR), 0xff);
    sysOutByte(PIC_IMASK(PIC2_BASE_ADR), 0xff);
}

/******************************************************************************
 * sysIntUnlock - Restore interrupt level
 *
 * RETURNS: N/A
 */

void sysIntUnlock(
    void
    )
{
    /* Restore interupt levels */
    sysOutByte(PIC_IMASK(PIC1_BASE_ADR), sysIntMask1);
    sysOutByte(PIC_IMASK(PIC2_BASE_ADR), sysIntMask2);
}

/******************************************************************************
 * sysIntLevel - Get interrupt level
 *
 * RETURNS: Interrupt level
 */

int sysIntLevel(
    void
    )
{
    int i, reg, level, oldLevel;

    /* Lock interrputs */
    INT_LOCK(oldLevel);

    for (i = 0; i < 1; i++)
    {
        /* Get in service register */
        sysOutByte(PIC_Port1(PIC1_BASE_ADR), 0x0b);
        reg = sysInByte(PIC_Port1(PIC1_BASE_ADR) );

        /* For all levels */
        for (level = 0; level < 8; level++)
        {
            /* If register bit zero set */
            if (reg & 0x1)
            {
                /* Unlock interrupts */
                INT_UNLOCK(oldLevel);
                break;
            }

            /* Advance in bitfield */
            reg >>= 1;
        }
    }

    /* Unlock interrupts */
    INT_UNLOCK(level);

    return level;
}

