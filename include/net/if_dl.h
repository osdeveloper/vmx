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

/* if_dl.h - Link level interface header */

#ifndef _if_dl_h
#define _if_dl_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct sockaddr_dl {
  unsigned char			sdl_len;		/* Length */
  unsigned char			sdl_family;		/* Address family */
  unsigned short		sdl_index;		/* Index to interface */
  unsigned char			sdl_type;		/* Interface type */
  unsigned char			sdl_nlen;		/* Name length */
  unsigned char			sdl_alen;		/* Link addr len */
  unsigned char			sdl_slen;		/* Link selector len */
  char				sdl_data[12];		/* Data */
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _if_dl_h */

