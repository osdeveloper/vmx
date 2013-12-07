/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2010 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* un.h - Unix domain ipc header */

#ifndef _un_h
#define _un_h

#include <stdlib.h>
#include <string.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct sockaddr_un {
  unsigned char sun_len;		/* Length including null */
  unsigned char sun_family;		/* AF_UNIX */
  char sun_path[PATH_MAX];		/* Path for IPC file */
};

/* Macros */

/*******************************************************************************
 * SUN_LEN - Get length of socket address
 *
 * RETURNS: Number of bytes required for socket address
 */

#define SUN_LEN(su)							       \
  (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _un_h */

