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

/* i8259a.h - Programable interrupt controller */

#ifndef _i8259a_h
#define _i8259a_h

#ifdef __cplusplus
extern "C" {
#endif

/* Default definitions */
#define PIC1_BASE_ADR          0x20
#define PIC2_BASE_ADR          0xa0
#define PIC_ADRS(base,reg)     (base + reg)

/* Register definitions */
#define PIC_Port1(base)        PIC_ADRS(base,0x00)
#define PIC_Port2(base)        PIC_ADRS(base,0x01)

/* Alias */
#define PIC_IMASK(base)        PIC_Port2(base)

/******************************************************************************
 * sysIntInitPIC - Initialize PIC
 *
 * RETURNS: N/A
 */

void sysIntInitPIC(
    void
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _i8259a_h */

