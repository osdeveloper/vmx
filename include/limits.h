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

/* limits.h - Limits header */

#ifndef _LIMITS_H
#define _LIMITS_H

#define _POSIX_AIO_LISTIO_MAX	2
#define _POSIX_AIO_MAX		1
#define _POSIX_ARG_MAX		4096
#define _POSIX_CHILD_MAX	6
#define	_POSIX_CLOCKRES_MIN	20
#define	_POSIX_DELAYTIMER_MAX	32
#define _POSIX_LINK_MAX		8
#define _POSIX_MAX_CANON	255
#define _POSIX_MAX_INPUT	255
#define _POSIX_NAME_MAX		14
#define _POSIX_NGROUPS_MAX	0
#define _POSIX_OPEN_MAX		16
#define _POSIX_PATH_MAX		255
#define _POSIX_PIPE_BUF		512
#define _POSIX_SSIZE_MAX	32767
#define _POSIX_STREAM_MAX	8
#define	_POSIX_TIMER_MAX	32
#define _POSIX_TZNAME_MAX	3
#define _POSIX_DATAKEYS_MAX	16

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <types/vmxCpu.h>

#if CPU_FAMILY==I386
#include <arch/i386/limitsI386.h>
#endif /* I386 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _LIMITS_H */

