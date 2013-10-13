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

/* logLib.h - Logging library */

#ifndef _logLib_h
#define _logLib_h

/* Log flags */
#define LOG_ALL                 0xffffffff

/* Log flags */
#define LOG_ALL                 0xffffffff
#define LOG_ARCH_LIB            1
#define LOG_CLASS_LIB           2
#define LOG_OBJECT_LIB          4
#define LOG_MEM_PARTITION_LIB   8
#define LOG_MEM_LIB             16
#define LOG_MMU_LIB             32
#define LOG_VM_LIB              64
#define LOG_VMX_LIB             128
#define LOG_TASK_LIB            256
#define LOG_SEM_LIB             512
#define LOG_KERN_HOOK_LIB       1024
#define LOG_NONE                0x00000000

#define LOG_LEVEL_NONE          0x00000000
#define LOG_LEVEL_ERROR         1
#define LOG_LEVEL_EXCEPTION     2
#define LOG_LEVEL_WARNING       4
#define LOG_LEVEL_INFO          8
#define LOG_LEVEL_CALLS         16
#define LOG_LEVEL_FULL          0xffffffff

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * setLogFlags - Setup  logging flags
 *
 * RETURNS: N/A
 */

void setLogFlags(
    unsigned flags
    );

/******************************************************************************
 * setLogLevel - Setup  logging level
 *
 * RETURNS: N/A
 */

void setLogLevel(
    int level
    );

/******************************************************************************
 * logString - Print log string
 *
 * RETURNS: N/A
 */

void logString(
    const char *str,
    unsigned flags,
    int level
    );

/******************************************************************************
 * logStringAndInteger - Print log string followed by integer
 *
 * RETURNS: N/A
 */

void logStringAndInteger(
    const char *str,     
    int value,
    unsigned flags,
    int level
    );

/******************************************************************************
 * logStringAndAddress - Print log string followed by an address
 *
 * RETURNS: N/A
 */

void logStringAndAddress(
    const char *str,     
    unsigned value,
    unsigned flags,
    int level
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _logLib_h */

