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

/* domain.h - Domain header file */

#ifndef _domain_h
#define _domain_h

#include <vmx.h>
#include <sys/socket.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

#include <net/protosw.h>

/* Structs */
struct domain {
    int              dom_family;        /* Domain family */
    char *           dom_name;          /* Domain name */

    FUNCPTR          dom_init;          /* Initialize domain */
    FUNCPTR          dom_externalize;   /* Externalize right */
    FUNCPTR          dom_dispose;       /* Dispose external right */

    struct protosw * dom_protosw;       /* Protocol switch table */
    struct protosw * dom_nprotosw;
    struct domain  * dom_next;          /* Next domain */

    FUNCPTR          dom_rtattach;      /* Initialize routing table */

    int              dom_rtoffset;      /* Arg to reattach */
    int              dom_maxrtkey;      /* For routing layer */
};

/* Functions */

/******************************************************************************
 *
 * addDomain - Add a domain to domain list
 *
 * RETURNS: OK or ERROR
 */

STATUS addDomain (
    struct domain * pDom
    );

/******************************************************************************
 *
 * domaininit - Initialize domain
 *
 * RETURNS: N/A
 */

void domaininit (
    void
    );

/******************************************************************************
 *
 * pfctlinput - Call all protocols ctlinput
 *
 * RETURNS: N/A
 */

void pfctlinput (
    int               cmd,
    struct sockaddr * sa
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _domain_h */

