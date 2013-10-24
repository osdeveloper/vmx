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

/* fcntlcom.h - Internal control types */

#ifndef _SYS_FCNTLCOM_H
#define _SYS_FCNTLCOM_H

#include <vmx.h>

#define _FREAD                  0x0001          /* Read enable */
#define _FWRITE                 0x0002          /* Write enable */
#define _FNDELAY                0x0004          /* Non-blocking I/O */
#define _FAPPEND                0x0008          /* Append to file */
#define _FMARK                  0x0010          /* Mark */
#define _FDEFER                 0x0020          /* Defer */
#define _FASYNC                 0x0040          /* Asynchoric I/O */
#define _FSHLOCK                0x0080          /* Shared lock */
#define _FEXLOCK                0x0100          /* Excluseive lock */
#define _FCREAT                 0x0200          /* Create file */
#define _FTRUNC                 0x0400          /* Truncate file */
#define _FEXCL                  0x0800          /* Error if file exists */
#define _FNBIO                  0x1000          /* Non-blocking I/O */
#define _FSYNC                  0x2000          /* Syncronize writes */
#define _FNONBLOCK              0x4000          /* Non-blocking I/O */
#define _FNOCTTY                0x8000          /* No assignment of ctty */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _SYS_FCNTLCOM_H */

