/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 - 2009 Surplus Users Ham Society
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

/* socket.h - Socket header */

#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include <sys/types.h>
#include <sys/time.h>

/* Defines */
#define AF_UNSPEC		0		/* Unspecified */
#define AF_LOCAL		1		/* Local */
#define AF_UNIX			AF_LOCAL	/* Unix */
#define AF_INET			2		/* Internet */
#define AF_IMPLINK		3		/* Arpanet imp address */
#define AF_PUP			4		/* Pup protocols */
#define AF_CHAOS		5		/* Mit chaos protocols */
#define AF_NS			6		/* Xerox ns protocols */
#define AF_ISO			7		/* Iso protocols */
#define AF_OSI			AF_ISO
#define AF_ECMA			8		/* European */
#define AF_DATAKIT		9		/* Datakit protocols */
#define AF_CCITT		10		/* Citt protocols */
#define AF_SNA			11		/* Ibm sna */
#define AF_DECnet		12		/* Dec net protocols */
#define AF_DLI			13		/* Dec direct link */
#define AF_LAT			14		/* Lat protocols */
#define AF_HYLINK		15		/* Nsc hyperchannel */
#define AF_APPLETALK		16		/* Apple talk */
#define AF_ROUTE		17		/* Routing protocols */
#define AF_LINK			18		/* Link layer protocols */
#define AF_XTP			19		/* Express transfer */
#define AF_COIP			20		/* Connection oriented */
#define AF_CNT			21		/* Computer network tech */
#define AF_RTIP			22		/* Identify rtip packets */
#define AF_IPX			23		/* Novell internet protocols */
#define AF_SIP			24		/* Simple internet protocols */
#define AF_PIP			25		/* Identify pip packets */
#define AF_MAX			26

#define PF_UNSPEC		AF_UNSPEC
#define PF_LOCAL		AF_LOCAL
#define PF_UNIX			AF_UNIX
#define PF_INET			AF_INET
#define PF_IMPLINK		AF_IMPLINK
#define PF_PUP			AF_PUP
#define PF_CHAOS		AF_CHAOS
#define PF_NS			AF_NS
#define PF_ISO			AF_ISO
#define PF_OSI			AF_OSI
#define PF_ECMA			AF_ECMA
#define PF_DATAKIT		AF_DATAKIT
#define PF_CCITT		AF_CCITT
#define PF_SNA			AF_SNA
#define PF_DECnet		AF_DECnet
#define PF_DLI			AF_DLI
#define PF_LAT			AF_LAT
#define PF_HYLINK		AF_HYLINK
#define PF_APPLETALK		AF_APPLETALK
#define PF_ROUTE		AF_ROUTE
#define PF_LINK			AF_LINK
#define PF_XTP			AF_XTP
#define PF_COIP			AF_COIP
#define PF_CNT			AF_CNT
#define PF_SIP			AF_SIP
#define PF_IPX			AF_IPX
#define PF_RTIP			AF_RTIP
#define PR_PIP			AF_PIP
#define PF_MAX			AF_MAX

#define SOCK_UNSPEC		0		/* Unspecified */
#define SOCK_STREAM		1		/* Stream socket */
#define SOCK_DGRAM		2		/* Datagram socket */
#define SOCK_RAW		3		/* Raw socket */
#define SOCK_RDM		4		/* Reliability message */
#define SOCK_SEQPACKET		5		/* Sequenced packet stream */
#define SOCK_MAX		6

#define SO_DEBUG		0x0001		/* Turn on debugging */
#define SO_ACCEPTCONN		0x0002		/* Accepted connection */
#define SO_REUSEADDR		0x0004		/* Allow local address reuse */
#define SO_KEEPALIVE		0x0008		/* Keep connections alive */
#define SO_DONTROUTE		0x0010		/* No routing */
#define SO_BROADCAST		0x0020		/* Permit braodcasting */
#define SO_USELOOPBACK		0x0040		/* Bypass hardware */
#define SO_LINGER		0x0080		/* Linger on close */
#define SO_OOBINLINE		0x0100		/* Leave received oob data */
#define SO_REUSEPORT		0x0200		/* Allow local address port */
#define SO_USEPATHMTU		0x0400		/* Enable path mtu discover */
#define SO_VSID			0x0800		/* Virtual stack id */
#define SO_SNDBUF		0x1001		/* Send buffer size */
#define SO_RCVBUF		0x1002		/* Receive buffer size */
#define SO_SNDLOWAT		0x1003		/* Send buffer low watermark */
#define SO_RCVLOWAT		0x1004		/* Receive buf low watermake */
#define SO_SNDTIMEO		0x1005		/* Send timeout */
#define SO_RCVTIMEO		0x1006		/* Receive timeout */
#define SO_ERROR		0x1007		/* Get socket error */
#define SO_TYPE			0x1008		/* Get socket type */
#define SO_PROTOTYPE		0x1009		/* Get/set socket proto type */

#define SOL_SOCKET		0xffff		/* Options for socket level */

#define SCM_RIGHTS		0x01		/* Access rights */

#define MSG_OOB			0x01		/* Process out of band */
#define MSG_PEEK		0x02		/* Peek at incoming message */
#define MSG_DONTROUTE		0x04		/* Don't send routing tables */
#define MSG_EOR			0x08		/* Data completes record */
#define MSG_TRUNC		0x10		/* Data discarded */
#define MSG_CTRUNC		0x20		/* Control data lost */
#define MSG_WAITALL		0x40		/* Wait for full request */
#define MSG_DONTWAIT		0x80		/* Non blocking message */
#define MSG_MBUF		0x10000		/* Mbuf interface */
#define MSG_IGNORE		0x20000		/* Ignore socket */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct sockaddr {
  unsigned char			sa_len;			/* Length */
  unsigned char			sa_family;		/* Address family */
  char				sa_data[14];		/* Adress value */
};

struct sockproto {
  unsigned short		sp_family;		/* Protocol family */
  unsigned short		sp_protocol;		/* Protocol */
};

struct msghdr {
  char				*msg_name;		/* Optional address */
  unsigned int			msg_namelen;		/* Length of name */
  struct iovec			*msg_iov;		/* Scatter/gather arr */
  unsigned int			msg_iovlen;		/* Number of elements */
  char				*msg_control;		/* Data */
  unsigned int			msg_controllen;		/* Length of data */
  int				msg_flags;		/* Flags */
};

struct linger {
  int				l_onoff;		/* Option on-off */
  int				l_linger;		/* Linger time */
};

struct cmsghdr {
  unsigned int			cmsg_len;		/* Length */
  int				cmsg_level;		/* Origin proto */
  int				cmsg_type;		/* Type */
};

/* Types */
typedef int socklen_t;

/* Wrappers */
#define msg_accrights		msg_control
#define msg_accrightslen 	msg_controllen

/* Macros */
#define CMSG_DATA(cmsg)		((unsigned char *) ((cmsg) + 1))

/* Functions */
extern int socket(int domain, int type, int protocol);
extern int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
extern int listen(int sockfd, int backlog);
extern int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);
extern int connect(int sockfd, struct sockaddr *serv_addr,
		   socklen_t addrlen);
extern int connectWithTimeout(int sockfd, struct sockaddr *serv_addr,
			      socklen_t addrlen, struct timeval *timeout);
extern int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int sendto(int sockfd, void *buf, int buflen, int flags,
		  struct sockaddr *to_addr, int tolen);
extern int send(int sockfd, void *buf, int buflen, int flags);
extern int sendmsg(int sockfd, struct msghdr *msg, int flags);
extern int recvfrom(int sockfd, void *buf, int buflen, int flags,
		    struct sockaddr *from_addr, int *fromlen);
extern int recv(int sockfd, void *buf, int buflen, int flags);
extern int recvmsg(int sockfd, struct msghdr *msg, int flags);
extern int shutdown(int sockfd, int how);
extern int setsockopt(int sockfd, int level, int optname,
		      void *optval, int optlen);
extern int getsockopt(int sockfd, int level, int optname,
		      void *optval, int *optlen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _SYS_SOCKET_H */

