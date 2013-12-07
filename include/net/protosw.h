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

/* protosw.h - Protocol switch table header */

#ifndef _protosw_h
#define _protosw_h

#include <vmx.h>

/* Defines */
#define PRU_ATTACH		0		/* Attach protocol */
#define PRU_DETACH		1		/* Detach protocol */
#define PRU_BIND		2		/* Bind socket to address */
#define PRU_LISTEN		3		/* Listen for connections */
#define PRU_CONNECT		4		/* Connect */
#define PRU_ACCEPT		5		/* Accept connection */
#define PRU_DISCONNECT		6		/* Disconnect */
#define PRU_SHUTDOWN		7		/* Shutdown */
#define PRU_RCVD		8		/* Received data */
#define PRU_SEND		9		/* Send data */
#define PRU_ABORT		10		/* Abort */
#define PRU_CONTROL		11		/* Control operations */
#define PRU_SENSE		12		/* Return status */
#define PRU_RCVOOB		13		/* Retrive out of band data */
#define PRU_SENDOOB		14		/* Send out of band data */
#define PRU_SOCKADDR		15		/* Fetch socket address */
#define PRU_PEERADDR		16		/* Fetch peer address */
#define PRU_CONNECT2		17		/* Connect two sockets */
#define PRU_FASTTIMO		18		/* Fast timeout */
#define PRU_SLOWTIMO		19		/* Slow timeout */
#define PRU_PROTORCV		20		/* Receive */
#define PRU_PROTOSEND		21		/* Send */
#define PRU_NREQ		21		/* Number of types */

#define PRC_IFDOWN		0		/* Interface transition */
#define PRC_ROUTEDEAD		1		/* Select new route */
#define PRC_QUENCH2		3		/* DEC slow down */
#define PRC_QUENCH		4		/* Said slow down */
#define PRC_MSGSIZE		5		/* Message size force drop */
#define PRC_HOSTDEAD		6		/* Host down */
#define PRC_HOSTUNREACH		7		/* Host unreachable */
#define PRC_UNREACH_NET		8		/* Net down */
#define PRC_UNREACH_HOST	9		/* Host down */
#define PRC_UNREACH_PROTOCOL	10		/* Bad protocol */
#define PRC_UNREACH_PORT	11		/* Bad port */
#define PRC_UNREACH_SRCFAIL	13		/* Bad source */
#define PRC_REDIRECT_NET	14		/* Redirect net */
#define PRC_REDIRECT_HOST	15		/* Redirect host */
#define PRC_REDIRECT_TOSNET	16		/* Redirect tos & net */
#define PRC_REDIRECT_TOSHOST	17		/* Redirect tos & host */
#define PRC_TIMXCEED_INTRANS	18		/* Packet life expired */
#define PRC_TIMXCEED_REASS	19		/* Packet life reass */
#define PRC_PARAMPROB		20		/* Header bad */
#define PRC_NCMDS		21

#define PR_ATOMIC		0x01		/* Atomic message only */
#define PR_ADDR			0x02		/* Addresses with msg */
#define PR_CONNREQUIRED		0x04		/* Connection required */
#define PR_WANTRCVD		0x08		/* Want rcvd calls */
#define PR_RIGHTS		0x10		/* Pass rights */

#define PRCO_SETOPT		0		/* Set option */
#define PRCO_GETOPT		1		/* Get option */

#define PRCO_NCMDS		2

#define PR_SLOWHZ		2		/* Slow timeouts per sec */
#define PR_FASTHZ		5		/* Fast timeouts per sec */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct domain;

struct protosw {
  short			pr_type;		/* Protocol type */
  struct domain		*pr_domain;		/* Domain protocol */
  short			pr_protocol;		/* Protocol number */
  short			pr_flags;		/* Flags */

  FUNCPTR		pr_input;		/* Input protocol */
  FUNCPTR		pr_output;		/* Output protocol */
  FUNCPTR		pr_ctlinput;		/* Control input */
  FUNCPTR		pr_ctloutput;		/* Control output */
  FUNCPTR		pr_usrreq;		/* User request */
  FUNCPTR		pr_init;		/* Initilization hook */
  FUNCPTR		pr_fasttimo;		/* Fast timeout */
  FUNCPTR		pr_slowtimo;		/* Slow timeout */
  FUNCPTR		pr_drain;		/* Flush excess space */
  FUNCPTR		pr_sysctl;		/* Sysctl protocol */
};

/* Macros */
#define PRC_IS_REDIRECT(cmd)						       \
  ((cmd) >= PRC_REDIRECT_NET && (cmd) <= PRC_REDIRECT_TOSHOST)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _protosw_h */

