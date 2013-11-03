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

/* config/x86/config.h - arch configuration settings */

#ifndef _config_h_
#define _config_h_

/* Include arch independant config */
#include "../all/configAll.h"

#define VM_PAGE_SIZE		4096
#define LOCAL_MEM_LOCAL_ADRS	0x00100000
#define GDT_BASE_OFFSET		(1 * VM_PAGE_SIZE)

/* PC interrupt controller related */
#define PIC_REG_ADDR_INTERVAL   1

/* PC timer related */
#define PIT0_INT_LVL            0x00
#define PIT_BASE_ADR            0x40
#define PIT_REG_ADDR_INTERVAL   1
#define PIT_CLOCK               1193180

#define SYS_CLOCK_RATE_MIN      19
#define SYS_CLOCK_RATE_MAX      8192

/* PC vga related */
#define DIAG_CTRL               0x61

#define BEEP_PITCH_L            1280
#define BEEP_TIME_L             18
#define BEEP_PITCH_S            1280
#define BEEP_TIME_S             9

#define COLOR                   1
#define VGA_MEM_BASE            (u_int8_t *) 0xb8000
#define VGA_SEL_REG             (u_int8_t *) 0x3d4
#define VGA_VAL_REG             (u_int8_t *) 0x3d5

#define DEFAULT_FG              ATRB_FG_BRIGHTWHITE
#define DEFAULT_BG              ATRB_BG_BLUE
#define DEFAULT_ATR             DEFAULT_FG | DEFAULT_BG
#define COLOR_MODE              COLOR
#define CTRL_MEM_BASE           VGA_MEM_BASE
#define CTRL_SEL_REG            VGA_SEL_REG
#define CTRL_VAL_REG            VGA_VAL_REG
#define CHR                     2

/* PC keyboard related */
#define KBD_INT_LVL             0x01
#define COMMAND_8042            0x64
#define DATA_8042               0x60
#define STATUS_8042             COMMAND_8042
#define COMMAND_8048            0x61
#define DATA_8048               0x60
#define STATUS_8048             COMMAND_8048

#define PC_CONSOLE              0
#define N_VIRTUAL_CONSOLES      2

#define INCLUDE_EXC_HANDELING
#define INCLUDE_TASK_HOOKS
#define INCLUDE_SEM_BINARY
#define INCLUDE_SEM_MUTEX
#define INCLUDE_SEM_COUNTING
#define INCLUDE_MSG_Q
#define INCLUDE_CONSTANT_RDY_Q
#define INCLUDE_PC_CONSOLE

#endif

