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

/* domainLib.c - Network domain library */

/* Includes */
#include <stdlib.h>
#include <sys/socket.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/netLib.h>
#include <vmx.h>
#include <vmx/wdLib.h>
#include <os/unixLib.h>

/* Defines */

/* Imports */

/* Locals */
LOCAL WDOG_ID pffasttimoWd;
LOCAL WDOG_ID pfslowtimoWd;

LOCAL void pffasttimo(void);
LOCAL void pfslowtimo(void);

/* Globals */
struct domain *domains = NULL;		/* List of domains */
int max_linkhdr = 0;			/* Max link level headers */
int max_protohdr = 0;			/* Max protocol headers */
int max_hdr = 0;			/* Max link and protocol headers */

/* Functions */

/*******************************************************************************
 * addDomain - Add a domain to domain list
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS addDomain(struct domain *pDom)
{
  /* Check domain pointer */
  if (pDom == NULL)
    return ERROR;

  /* Set next pointer in struct */
  pDom->dom_next = domains;

  /* Add current domain to list */
  domains = pDom;

  return OK;
}

/*******************************************************************************
 * domaininit - Initialize domain
 *
 * RETURNS: N/A
 ******************************************************************************/

void domaininit(void)
{
  struct domain *dp;
  struct protosw *pr;

  /* For all domains in global list */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* Call comain init func if set */
    if (dp->dom_init != NULL)
      ( *dp->dom_init) ();

    /* For all protocol switches */
    for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      if (pr->pr_init != NULL)
        ( *pr->pr_init) ();

    } /* End for all protocol switches */

  } /* End for all domains in global list */

  /* Initialize global max counters */
  if (max_linkhdr < 16)
    max_linkhdr = 16;

  max_hdr = max_linkhdr + max_protohdr;

  /* Inititalize timeout watchdogs */
  pffasttimoWd = wdCreate();
  pfslowtimoWd = wdCreate();

  pffasttimo();
  pfslowtimo();
}

/*******************************************************************************
 * pffindtype - Find family and type
 *
 * RETURNS: Pointer to protocol struct or NULL
 ******************************************************************************/

struct protosw* pffindtype(int family, int type)
{
  struct domain *dp;
  struct protosw *pr;

  /* For all domains in global list */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* If domain family found */
    if (dp->dom_family == family) {

      /* For all protocols */
      for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

        /* If type found */
        if ( (pr->pr_type) && (pr->pr_type == type) )
          return pr;

      } /* End for all protocols */

    } /* End if domain family found */

  } /* End for all domains in global list */

  return NULL;
}

/*******************************************************************************
 * pffindproto - Find protocol family and type
 *
 * RETURNS: Pointer to protocol struct or NULL
 ******************************************************************************/

struct protosw* pffindproto(int family, int protocol, int type)
{
  struct domain *dp;
  struct protosw *pr;
  struct protosw *chk;

  /* Check family */
  if (family == 0)
    return NULL;

  /* Initialize locals */
  chk = NULL;

  /* For all domains in global list */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* If domain family found */
    if (dp->dom_family == family) {

      /* For all protocols */
      for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

        /* If type found */
        if ( (pr->pr_protocol == protocol) && (pr->pr_type == type) )
          return pr;

        /* Check for raw socket type */
        if ( (type == SOCK_RAW) && (pr->pr_type == SOCK_RAW) &&
	     (pr->pr_protocol == 0) && (chk == NULL) )
          chk = pr;

      } /* End for all protocols */

    } /* End if domain family found */

  } /* End for all domains in global list */

  return chk;
}

/*******************************************************************************
 * pfctlinput - Call all protocols ctlinput
 *
 * RETURNS: N/A
 ******************************************************************************/

void pfctlinput(int cmd, struct sockaddr *sa)
{
  struct domain *dp;
  struct protosw *pr;

  /* For all domains */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* For all protocols */
    for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      /* Call ctlinput if it is set */
      if (pr->pr_ctlinput != NULL)
        ( *pr->pr_ctlinput) (cmd, sa, NULL);

    } /* End or all protocols */

  } /* End for all domains */

}

/*******************************************************************************
 * pffastimo - Fast timeout
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void pffasttimo(void)
{
  struct domain *dp;
  struct protosw *pr;
  int s;

  /* Get processor level */
  s = splnet();

  /* For all domains */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* For all protocols */
    for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      /* Call function */
      if (pr->pr_fasttimo != NULL)
        ( *pr->pr_fasttimo) ();

    } /* End or all protocols */

  } /* End for all domains */

  /* Restore processor level */
  splx(s);

  /* Start watchdog */
  wdStart(pffasttimoWd,
	  sysClockRateGet() / 5,
	  (FUNCPTR) netJobAdd,
	  (ARG) pffasttimo);
}

/*******************************************************************************
 * pfslowtimo - Slow timeout
 *
 * RETURNS: N/A
 ******************************************************************************/

LOCAL void pfslowtimo(void)
{
  struct domain *dp;
  struct protosw *pr;
  int s;

  /* Get processor level */
  s = splnet();

  /* For all domains */
  for (dp = domains; dp != NULL; dp = dp->dom_next) {

    /* For all protocols */
    for (pr = dp->dom_protosw; pr < dp->dom_nprotosw; pr++) {

      /* Call function */
      if (pr->pr_slowtimo != NULL)
        ( *pr->pr_slowtimo) ();

    } /* End or all protocols */

  } /* End for all domains */

  /* Restore processor level */
  splx(s);

  /* Start watchdog */
  wdStart(pfslowtimoWd,
	  sysClockRateGet() / 2,
	  (FUNCPTR) netJobAdd,
	  (ARG) pfslowtimo);
}

