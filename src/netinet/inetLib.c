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

/* inetLib.c - Internet library */

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <vmx.h>
#include <os/errnoLib.h>
#include <net/protosw.h>
#include <net/domain.h>
#include <net/radix.h>
#include <netinet/in.h>
#include <netinet/inetLib.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/icmp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

/* Defines */
#define MAX_PARTS               4

/* Imports */

/* Locals */
LOCAL STATUS ifIoctlSet(char *ifname, int cmd, int val);
LOCAL STATUS ifIoctlGet(char *ifname, int cmd, int *val);
LOCAL STATUS ifIoctlCall(int cmd, struct ifreq *ifrp);

/* Globals */
struct protosw inetsw[] = {
   /* Type              Domain          Protocol        Flags */
  {0,                   &inetdomain,    0,              0,
   (FUNCPTR) NULL,
   (FUNCPTR) ip_output,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) ip_slowtimo,
   (FUNCPTR) ip_drain,
   (FUNCPTR) NULL},

  /* UDP socket */
  {SOCK_DGRAM,          &inetdomain,    IPPROTO_UDP,    PR_ATOMIC |
                                                        PR_ADDR,
   (FUNCPTR) udp_input,
   (FUNCPTR) NULL,
   (FUNCPTR) udp_ctlinput,
   (FUNCPTR) ip_ctloutput,
   (FUNCPTR) udp_usrreq,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL},

  /* TCP socket */
  {SOCK_STREAM,         &inetdomain,    IPPROTO_TCP,    PR_CONNREQUIRED |
                                                        PR_WANTRCVD,
   (FUNCPTR) tcp_input,
   (FUNCPTR) NULL,
   (FUNCPTR) tcp_ctlinput,
   (FUNCPTR) tcp_ctloutput,
   (FUNCPTR) tcp_usrreq,
   (FUNCPTR) NULL,
   (FUNCPTR) tcp_fasttimo,
   (FUNCPTR) tcp_slowtimo,
   (FUNCPTR) tcp_drain,
   (FUNCPTR) NULL},

  /* Raw IP socket */
  {SOCK_RAW,            &inetdomain,    IPPROTO_RAW,    PR_ATOMIC |
                                                        PR_ADDR,
   (FUNCPTR) rip_input,
   (FUNCPTR) rip_output,
   (FUNCPTR) NULL,
   (FUNCPTR) rip_ctloutput,
   (FUNCPTR) rip_usrreq,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL},

  /* ICMP IP socket */
  {SOCK_RAW,            &inetdomain,    IPPROTO_ICMP,   PR_ATOMIC |
                                                        PR_ADDR,
   (FUNCPTR) icmp_input,
   (FUNCPTR) rip_output,
   (FUNCPTR) NULL,
   (FUNCPTR) rip_ctloutput,
   (FUNCPTR) rip_usrreq,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL,
   (FUNCPTR) NULL}
};

struct domain inetdomain = {
  AF_INET, "internet",
  (FUNCPTR) NULL,
  (FUNCPTR) NULL,
  (FUNCPTR) NULL,
  inetsw,
  &inetsw[sizeof(inetsw) / sizeof(inetsw[0])],
  NULL,
  (FUNCPTR) rn_inithead,
  27,
  sizeof(struct sockaddr_in)
};

/* Macros */
#define UC(b)                   (((int) b) & 0xff)

/* Functions */

/*******************************************************************************
 * inetLibInit - Initialize internet library
 *
 * RETURNS: N/A
 ******************************************************************************/

void inetLibInit(void)
{
  addDomain(&inetdomain);
}

/*******************************************************************************
 * inet_addr - Get integer internet address from address string
 *
 * RETURNS: Interger inet address
 ******************************************************************************/

unsigned long inet_addr(char *inetString)
{
  unsigned long val, base, n;
  char c;
  unsigned long parts[MAX_PARTS];
  unsigned long *pp;

  /* If null inet string */
  if (inetString == NULL) {

    errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
    return ERROR;

  } /* End if null inet string */

  /* Initialize locals */
  pp = parts;

again:

  val = 0;
  base = 10;

  /* If zero prefix */
  if (*inetString == '0') {

    base = 8;
    inetString++;

    /* If hexadecimal prefix */
    if ( (*inetString == 'x') || (*inetString == 'X') ) {

      base = 16;
      inetString++;

    } /* End if hexadecimal prefix */

  } /* End if zero prefix */

  /* While not string terminator */
  while ( (c = *inetString) ) {

    /* If digit */
    if ( __isdigit((int) c) ) {

      val = (val * base) + (c - '0');
      inetString++;
      continue;

    } /* End if digit */

    /* If hexadecimal digit */
    if ( __isxdigit((int) c) ) {

      val = (val << 4) + (c + 10 - (__islower((int) c) ? 'a' : 'A'));
      inetString++;
      continue;

    } /* End if hexadecimal digit */

    /* If here break loop */
    break;

  } /* End while not string terminator */

  /* If dot found */
  if (*inetString == '.') {

    /* If string to large or value to large */
    if ( (pp >= parts + (MAX_PARTS - 1)) || (val > 0xff) ) {

      errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
      return ERROR;

    } /* End if string to large or value to large */

    /* Store value */
    *pp++ = val;

    /* Advance */
    inetString++;

    /* Find next value */
    goto again;

  } /* End if dot found */

  /* If trailing character */
  if ( (*inetString) && !(__isspace((int) *inetString)) ) {

    errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
    return ERROR;

  } /* End if trailing character */

  /* Store value */
  *pp++ = val;

  n = pp - parts;

  /* Select parts number */
  switch (n) {

    /* 32 bit address */
    case 1:
 
      val = parts[0];

    break;

    /* 8.24 bits address */
    case 2:

      /* If value to large */
      if (val > 0xffffff) {

        errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
        return ERROR;

      } /* End if value to large */

      /* Store value */
      val = (parts[0] << 24) | parts[1];

    break;

    /* 8.8.16 bits address */
    case 3:

      /* If value to large */
      if (val > 0xffff) {

        errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
        return ERROR;

      } /* End if value to large */

      /* Store value */
      val = (parts[0] << 24) | (parts[1] << 16) | parts[2];

    break;

    /* 8.8.8.8 bits */
    case 4:

      /* If value to large */
      if (val > 0xff) {

        errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
        return ERROR;

      } /* End if value to large */

      /* Store value */
      val = (parts[0] << 24) | (parts[1] << 16) |
            (parts[2] << 8) | parts[3];

    break;

    default:

      errnoSet(S_inetLib_ILLEGAL_INTERNET_ADDRESS);
      return ERROR;

  } /* End select part number */

  return htonl(val);
}

/*******************************************************************************
 * inet_ntoa_b - Get internet address string
 *
 * RETURNS: N/A
 ******************************************************************************/

void inet_ntoa_b(struct in_addr addr, char *str)
{
  char *p;

  /* Setup locals */
  p = (char *) &addr;

  sprintf(str, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
}

/*******************************************************************************
 * ifAddrAdd - Add internet address to interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS ifAddrAdd(char *ifName, char *ifAddr, char *broadAddr, int subnetMask)
{
  struct ifaliasreq ifa;
  struct sockaddr_in *addr, *baddr, *mask;
  int fd, err;

  /* Initialize locals */
  addr = (struct sockaddr_in *) &ifa.ifra_addr;
  baddr = (struct sockaddr_in *) &ifa.ifra_broadaddr;
  mask = (struct sockaddr_in *) &ifa.ifra_mask;
  err = 0;

  /* Clear struct */
  memset(&ifa, 0, sizeof(struct ifaliasreq));

  /* Get internet address */
  addr->sin_addr.s_addr = inet_addr(ifAddr);
  if (addr->sin_addr.s_addr == ERROR)
    return ERROR;

  /* If broadcast address */
  if (broadAddr != NULL) {

    /* Get broadcast address */
    baddr->sin_addr.s_addr = inet_addr(broadAddr);
    if (baddr->sin_addr.s_addr == ERROR)
      return ERROR;

    ifa.ifra_broadaddr.sa_len = sizeof(struct sockaddr_in);
    ifa.ifra_broadaddr.sa_family = AF_INET;

  } /* End if broadcast address */

  /* If subnet mask */
  if (subnetMask) {

    ifa.ifra_mask.sa_len = sizeof(struct sockaddr_in);
    ifa.ifra_mask.sa_family = AF_INET;
    mask->sin_addr.s_addr = htonl(subnetMask);

  } /* End if subnet mask */

  /* Copy interface name */
  strncpy(ifa.ifra_name, ifName, IFNAMSIZ);

  /* Setup as internet address */
  ifa.ifra_addr.sa_len = sizeof(struct sockaddr_in);
  ifa.ifra_addr.sa_family = AF_INET;

  /* Create a raw socket */
  fd = socket(AF_INET, SOCK_RAW, 0);
  if (fd == ERROR)
    return ERROR;

  /* Perform address set tru ioctl call */
  err = ioctl(fd, SIOCAIFADDR, (int) &ifa);

  /* Close socket */
  close(fd);

  /* If error */
  if (err) {

    if (err != ERROR)
      errnoSet(err);

    return ERROR;

  } /* End if error */

  return OK;
}

/*******************************************************************************
 * ifIoctlSet - Perform ioctl set operation on interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL STATUS ifIoctlSet(char *ifname, int cmd, int val)
{
  struct ifreq ifr;

  /* Copy string */
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

  /* Select command */
  switch (cmd) {

    case SIOCSIFFLAGS:

      ifr.ifr_flags = (short) val;

    break;

    case SIOCSIFMETRIC:

      ifr.ifr_metric = val;

    break;

    default:

      /* Set address */
      memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
      ifr.ifr_addr.sa_family = AF_INET;
      ifr.ifr_addr.sa_len = sizeof(struct sockaddr_in);
      ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = val;

    break;

  } /* End select command */

  return ifIoctlCall(cmd, &ifr);
}

/*******************************************************************************
 * ifIoctlGet - Perform ioctl get operation on interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL STATUS ifIoctlGet(char *ifname, int cmd, int *val)
{
  struct ifreq ifr;

  /* Copy string */
  strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

  if ( ifIoctlCall(cmd, &ifr) == ERROR)
    return ERROR;

  /* Select command */
  switch (cmd) {

    case SIOCGIFFLAGS:

      *val = ifr.ifr_flags;

    break;

    case SIOCGIFMETRIC:

      *val = ifr.ifr_metric;

    break;

    default:

      /* Get address */
      *val = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;

    break;

  } /* End select command */

  return OK;
}

/*******************************************************************************
 * ifIoctlCall - Perform ioctl on interface
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

LOCAL STATUS ifIoctlCall(int cmd, struct ifreq *ifrp)
{
  int err, fd;

  /* Create a raw socket */
  fd = socket(AF_INET, SOCK_RAW, 0);
  if (fd < 0)
    return ERROR;

  /* Perform ioctl on socket */
  err = ioctl(fd, cmd, (int) ifrp);

  /* Close raw socket */
  close(fd);

  /* If error */
  if (err) {

    if (err != ERROR)
      errnoSet(err);

  } /* End if error */

  return OK;
}

