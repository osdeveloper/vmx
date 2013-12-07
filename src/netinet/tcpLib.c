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

/* tcpLib.c - Tcp library */

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)tcp_output.c        8.4 (Berkeley) 5/24/95
 *      $Id: tcpLib.c,v 1.19 2010/06/04 07:41:58 alexuspol Exp $
 */

/* Includes */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vmx.h>
#include <util/listLib.h>
#include <os/unixLib.h>
#include <net/mbuf.h>
#include <net/protosw.h>
#include <net/socketvar.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_var.h>

/* Defines */
#define MAX_TCPOPTLEN                   32
#define TCP_PAWS_IDLE                   (24 * 24 * 60 * 60 * PR_SLOWHZ)

/* Imports */
IMPORT int max_protohdr;
IMPORT int max_linkhdr;
IMPORT int ip_defttl;
IMPORT int sb_max;
IMPORT struct in_addr zeroin_addr;
IMPORT unsigned char inetctlerrmap[];

/* Locals */
LOCAL BOOL tcpLibInstalled = FALSE;

/* Globals */

/* tcp_usrreq */
unsigned long tcp_sendspace = 1024 * 8;
unsigned long tcp_recvspace = 1024 * 8;

/* tcp_subr */
int tcp_mssdflt = TCP_MSS;
int tcp_rttdflt = TCPTV_SRTTDFLT / PR_SLOWHZ;
BOOL tcp_do_rfc1323 = TRUE;
unsigned short tcp_pcbhashsize = 128;

FUNCPTR tcpRandFunc = (FUNCPTR) rand;

/* tcp_output */
unsigned char tcp_outflags[TCP_NSTATES] = {
  TH_RST | TH_ACK, 0, TH_SYN, TH_SYN | TH_ACK,
  TH_ACK, TH_ACK,
  TH_FIN | TH_ACK, TH_FIN | TH_ACK, TH_FIN | TH_ACK, TH_ACK, TH_ACK
};

/* tcp_input */
long tcpOutRsts = 0;
tcp_seq tcp_iss = 0;
struct inpcbinfo tcbinfo;
struct tcpstat tcpstat;
int tcprexmtthresh = 3;
int tcp_msl = 0;
struct tcpiphdr tcp_saveti;
struct inpcb *tcp_last_inpcb = NULL;
unsigned long tcp_now = 0;

/* tcp_timer */
int tcp_keepidle = TCPTV_KEEP_IDLE;
int tcp_keepintvl = TCPTV_KEEPINTVL;
int tcp_keepcnt = TCPTV_KEEPCNT;
int tcp_keepinit = TCPTV_KEEP_INIT;
int tcp_maxpersistidle = TCPTV_KEEP_IDLE;       /* max idle time in persist */
int tcp_maxidle;

int tcp_backoff[TCP_MAXRXTSHIFT + 1] = {
  1, 2, 4, 8, 16, 32, 64, 64, 64, 64, 64, 64, 64
};

int tcp_totbackoff = 511;       /* sum of tcp_backoff[] */

/* Macros */
#define xchg(a, b, type) { type t; t = a; a = b; b = t; }

/*
 * Insert segment ti into reassembly queue of tcp with
 * control block tp.  Return TH_FIN if reassembly now includes
 * a segment with FIN.  The macro form does the common case inline
 * (segment is the next to be received on an established connection,
 * and the queue is empty), avoiding linkage into and removal
 * from the queue and repetition of various conversions.
 * Set DELACK for segments received in order, but ack immediately
 * when segments are out of order (so fast retransmit can work).
 */
#define TCP_REASS(tp, ti, m, so, flags) { \
        if ((ti)->ti_seq == (tp)->rcv_nxt && \
            (tp)->seg_next == (struct tcpiphdr *)(tp) && \
            (tp)->t_state == TCPS_ESTABLISHED) { \
                tp->t_flags |= TF_DELACK; \
                (tp)->rcv_nxt += (ti)->ti_len; \
                flags = (ti)->ti_flags & TH_FIN; \
                tcpstat.tcps_rcvpack++;\
                tcpstat.tcps_rcvbyte += (ti)->ti_len;\
                sbappend(&(so)->so_rcv, (m)); \
                sorwakeup(so); \
        } else { \
                (flags) = tcp_reass((tp), (ti), (m)); \
                tp->t_flags |= TF_ACKNOW; \
        } \
}

/* Functions */

/*******************************************************************************
 * tcpLibInit - Initialize tcp library
 *
 * RETURNS: OK or ERROR
 ******************************************************************************/

STATUS tcpLibInit(TCP_CFG_PARAMS *params)
{
  /* If already installed */
  if (tcpLibInstalled)
    return OK;

  /* If params set */
  if (params != NULL) {

    /* Set globals */
    if (params->tcpCfgFlags & TCP_DO_RFC1323)
      tcp_do_rfc1323 = TRUE;
    else
      tcp_do_rfc1323 = FALSE;

    tcp_sendspace = params->tcpSndSpace;
    tcp_recvspace = params->tcpRcvSpace;
    tcp_keepinit = params->tcpConnectTime;
    tcprexmtthresh = params->tcpReTxThresh;
    tcp_mssdflt = params->tcpMssDflt;
    tcp_rttdflt = params->tcpRttDflt;
    tcp_keepidle = params->tcpKeepIdle;
    tcp_keepcnt = params->tcpKeepCount;
    tcp_msl = params->tcpMsl * PR_SLOWHZ;
    tcpRandFunc = params->pTcpRandFunc;

  } /* End if params set */

  /* Get random iss */
  tcp_iss = ( *tcpRandFunc) ();

  /* Initialize list */
  tcbinfo.listhead = malloc( sizeof(LIST) );
  if (tcbinfo.listhead == NULL)
    return ERROR;

  listInit(tcbinfo.listhead);

  /* Initialize hash table */
  tcbinfo.hashbase = hashinit(tcp_pcbhashsize, MT_PCB, &tcbinfo.hashmask);
  if (tcbinfo.hashbase == NULL) {

    free(tcbinfo.listhead);
    return ERROR;

  }

  if (max_protohdr < sizeof(struct tcpiphdr))
    max_protohdr = sizeof(struct tcpiphdr);

  if (max_linkhdr + sizeof(struct tcpiphdr) > CL_SIZE_128)
    panic("tcpLibInit");

  /* Mark as installed */
  tcpLibInstalled = TRUE;

  return OK;
}

/*******************************************************************************
 * tcp_usrreq - Process protocol requests
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int tcp_usrreq(struct socket *so,
               int req,
               struct mbuf *m,
               struct mbuf *name,
               struct mbuf *ctrl)
{
struct sockaddr_in *sin;
  struct inpcb *inp;
  struct tcpcb *tp;
  int s, err, ostate;

  /* Initialize locals */
  tp = NULL;
  err = 0;

  /* If control request */
  if (req == PRU_CONTROL)
    return in_control(so, (unsigned long) m, name, (struct ifnet *) ctrl);

  /* If control gt. zero */
  if ( (ctrl != NULL) &&
       (ctrl->m_len) ) {

    if (m != NULL)
      m_freem(m);

    return EINVAL;

  } /* End if control gt. zero */

  /* Get processor level */
  s = splnet();

  /* Get control block */
  inp = sotoinpcb(so);

  /* If no control block and not attach request */
  if ( (inp == NULL) &&
       (req != PRU_ATTACH) ) {

    if ( (m != NULL) &&
         (req == PRU_SEND || req == PRU_SENDOOB) )
      m_freem(m);

    return EINVAL;

  } /* End if no control block and not attach request */

  /* If control block non-null */
  if (inp != NULL) {

    tp = intotcpcb(inp);

    /* Get old state */
    ostate = tp->t_state;

  } /* If control block non-null */

  /* Else control block null */
  else {

    ostate = 0;

  } /* End else control block null */

  /* Select request */
  switch(req) {

    case PRU_ATTACH:

      /* If control block non-null */
      if (inp != NULL) {

        err = EISCONN;
        break;

      } /* End if control block non-null */

      /* Attach control block */
      err = tcp_attach(so);
      if (err)
        break;

      /* Set options */
      so->so_options |= SO_USEPATHMTU;

      if ( (so->so_options & SO_LINGER) &&
           (so->so_linger == 0) )
        so->so_linger = TCP_LINGERTIME;

      /* Get tcp control block */
      tp = sototcpcb(so);

    break;

    case PRU_DETACH:

      /* Disconnect */
      tp = tcp_disconnect(tp);

    break;

    case PRU_BIND:

    /* Bind to given local address */
    err = in_pcbbind(inp, name);
    if (err)
      break;

    break;

    case PRU_LISTEN:

      /* Bind if zero local port */
      if (inp->inp_lport == 0)
        err = in_pcbbind(inp, NULL);

      /* If no error mark as listening */
      if (err == 0)
        tp->t_state = TCPS_LISTEN;

    break;

    case PRU_CONNECT:

      /* If zero local port */
      if (inp->inp_lport == 0) {

        /* Bind to random local port */
        err = in_pcbbind(inp, NULL);
        if (err)
          break;

        /* If same as connect port */
        if (inp->inp_lport == (mtod(name, struct sockaddr_in *))->sin_port) {

          inp->inp_lport = 0;
          err = in_pcbbind(inp, NULL);
          if (err)
            break;

        } /* End if same as connect port */

      } /* End if zero local port */

      /* Connect */
      err = in_pcbconnect(inp, name);
      if (err)
        break;

      /* Create template */
      tp->t_template = tcp_template(tp);
      if (tp->t_template == NULL) {

        in_pcbdisconnect(inp);
        err = ENOBUFS;
        break;

      }

      /* Calculate window scaling to request */
      while ( (tp->request_r_scale < TCP_MAX_WINSHIFT) &&
              ((TCP_MAXWIN << tp->request_r_scale) < so->so_rcv.sb_hiwat) )
        tp->request_r_scale++;

      /* Mark as connecting */
      soisconnecting(so);

      tcpstat.tcps_connattempt++;

      tp->t_state = TCPS_SYN_SENT;
      tp->t_timer[TCPT_KEEP] = tcp_keepinit;
      tp->iss = tcp_iss;
      tp->iss += TCP_ISSINCR / 4 + ((0x0000ffff) & ((* tcpRandFunc) () >> 16));

      tcp_sendseqinit(tp);

      err = tcp_output(tp);

    break;

    case PRU_CONNECT2:

      err = EOPNOTSUPP;

    break;

    case PRU_DISCONNECT:

      tp = tcp_disconnect(tp);

    break;

    case PRU_ACCEPT:

      /* Set peer address */
      in_setpeeraddr(inp, name);

    break;

    case PRU_SHUTDOWN:

      socantsendmore(so);
      tp = tcp_usrclosed(tp);

      if (tp != NULL)
        err = tcp_output(tp);

    break;

    case PRU_RCVD:

      tcp_output(tp);

    break;

    case PRU_SEND:

      /* If end-of-block */
      if (m->m_flags & M_EOB) {

        m->m_flags &= ~M_EOB;
        tp->t_flags |= TF_EOB;

      } /* End if end-of-block */

      sbappend(&so->so_snd, m);
      err = tcp_output(tp);
      tp->t_flags &= TF_EOB;

    break;

    case PRU_ABORT:

      tp = tcp_drop(tp, ECONNABORTED);

    break;

    case PRU_RCVOOB:

      /* If invalid */
      if ( ((so->so_oobmark == 0) && ((so->so_state & SS_RCVATMARK) == 0)) ||
           (so->so_options & SO_OOBINLINE) ||
           (tp->t_oobflags & TCPOOB_HADDATA) ) {

        err = EINVAL;
        break;

      } /* End if invalid */

      /* If would block */
      if ((tp->t_oobflags & TCPOOB_HAVEDATA) == 0) {

        err = EWOULDBLOCK;
        break;

      } /* End if would block */

      m->m_len = 1;
      *mtod(m, char *) = tp->t_iobc;

      if (((int) name & MSG_PEEK) == 0)
        tp->t_oobflags ^(TCPOOB_HAVEDATA | TCPOOB_HADDATA);

    break;

    case PRU_SENDOOB:

      /* If space lt. minus 512 */
      if (sbspace(&so->so_snd) < -512) {

        m_freem(m);
        err = ENOBUFS;
        break;

       } /* End if space lt. minus 512 */

       sbappend(&so->so_snd, m);
       tp->snd_up = tp->snd_una + so->so_snd.sb_cc;
       tp->t_force = 1;
       err = tcp_output(tp);
       tp->t_force = 0;

    break;

    case PRU_SOCKADDR:

      in_setsockaddr(inp, name);

    break;

    case PRU_PEERADDR:

      in_setpeeraddr(inp, name);

    break;

    case PRU_SLOWTIMO:

      tp = tcp_timers(tp, (int) name);
      req |= (int) name << 8;

    break;

    default:

      panic("tcp_usrreq");

    break;

  } /* End select request */

  /* Restore processor level */
  splx(s);

  return err;
}

/*******************************************************************************
 * tcp_ctloutput - Control output
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int tcp_ctloutput(int op, struct socket *so,
                  int level, int optname,
                  struct mbuf **mp)
{
        int error = 0, s;
        struct inpcb *inp;
        struct tcpcb *tp;
        struct mbuf *m;
        int i;

        s = splnet();
        inp = sotoinpcb(so);
        if (inp == NULL) {

                splx(s);
                if ( (op == PRCO_SETOPT) &&
                     (*mp != NULL) )
                        m_free(*mp);

                return (ECONNRESET);

        }

        if (level != IPPROTO_TCP) {

                error = ip_ctloutput(op, so, level, optname, mp);
                splx(s);
                return (error);

        }

        tp = intotcpcb(inp);

        switch (op) {

        case PRCO_SETOPT:

                m = *mp;
                switch (optname) {

                case TCP_NODELAY:
                        if ( (m == NULL) ||
                             (m->m_len < sizeof (int)) )
                                error = EINVAL;
                        else if (*mtod(m, int *))
                                tp->t_flags |= TF_NODELAY;
                        else
                                tp->t_flags &= ~TF_NODELAY;
                        break;

                case TCP_MAXSEG:
                        if ( (m != NULL) &&
                             ((i = *mtod(m, int *)) > 0) &&
                             (i <= tp->t_maxseg) )
                                tp->t_maxseg = i;
                        else
                                error = EINVAL;
                        break;

                default:
                        error = ENOPROTOOPT;
                        break;

                }

                if (m != NULL)
                        m_free(m);

                break;

        case PRCO_GETOPT:
                m = m_get(M_WAIT, MT_SOOPTS, CL_SIZE_128, TRUE);
                if (m == NULL) {

                        error = ENOBUFS;
                        break;

                }

                *mp = m;
                m->m_len = sizeof(int);

                switch (optname) {

                        case TCP_NODELAY:
                                *mtod(m, int *) = tp->t_flags & TF_NODELAY;
                                break;

                        case TCP_MAXSEG:
                                *mtod(m, int *) = tp->t_maxseg;
                                break;

                        default:
                                error = ENOPROTOOPT;
                                break;

                }

                break;

        }

        splx(s);
        return (error);
}

/*******************************************************************************
 * tcp_attach - Attach tcp control block
 *
 * RETURNS: Zero or error code
 ******************************************************************************/

int tcp_attach(struct socket *so)
{
  struct inpcb *inp;
  struct tcpcb *tp;
  int err, nofd;

  /* If zero high watermarks */
  if ( (so->so_snd.sb_hiwat == 0) ||
       (so->so_rcv.sb_hiwat == 0) ) {

    /* Reserve buffer */
    err = soreserve(so, tcp_sendspace, tcp_recvspace);
    if (err)
      return err;

  } /* End if zero high watermarks */

  /* Allocate inet pcb */
  err = in_pcballoc(so, &tcbinfo);
  if (err)
    return err;

  /* Get allocated pcb */
  inp = sotoinpcb(so);

  /* Allocate new tcp control block */
  tp = tcp_newtcpcb(inp);
  if (tp == NULL) {

    nofd = so->so_state & SS_NOFDREF;
    so->so_state &= ~SS_NOFDREF;
    in_pcbdetach(inp);
    so->so_state |= nofd;
    return ENOBUFS;

  }

  /* Set as closed */
  tp->t_state = TCPS_CLOSED;

  return 0;
}

/*******************************************************************************
 * tcp_usrclosed - User close
 *
 * RETURNS: Tcp control block
 ******************************************************************************/

struct tcpcb * tcp_usrclosed(struct tcpcb *tp)
{
  /* Select state */
  switch (tp->t_state) {

    case TCPS_CLOSED:
    case TCPS_LISTEN:
    case TCPS_SYN_SENT:

      tp->t_state = TCPS_CLOSED;
      tp = tcp_close(tp);

    break;

    case TCPS_SYN_RECEIVED:
    case TCPS_ESTABLISHED:

      tp->t_state = TCPS_FIN_WAIT_1;

    break;

    case TCPS_CLOSE_WAIT:

      tp->t_state = TCPS_LAST_ACK;

    break;

  } /* End select state */

  if ( (tp != NULL) &&
       (tp->t_state >= TCPS_FIN_WAIT_2) )
    soisdisconnected(tp->t_inpcb->inp_socket);

  return tp;
}

/*******************************************************************************
 * tcp_disconnect - Disconnect tcp connection
 *
 * RETURNS: NULL
 ******************************************************************************/

struct tcpcb* tcp_disconnect(struct tcpcb *tp)
{
  struct socket *so;

  /* Initialize locals */
  so = tp->t_inpcb->inp_socket;

  /* If connected */
  if (tp->t_state < TCPS_ESTABLISHED) {

    tp = tcp_close(tp);

  } /* End if connected */

  /* Else if linger time zero */
  else if ( (so->so_options & SO_LINGER) &&
            (so->so_linger == 0) ) {

    tp = tcp_drop(tp, 0);

  } /* End else if linger time zero */

  /* Else not connected */
  else {

    soisdisconnecting(so);
    sbflush(&so->so_rcv);

    tp = tcp_usrclosed(tp);
    if (tp != NULL)
      tcp_output(tp);

  } /* End else not connected */

  return tp;
}

/*******************************************************************************
 * tcp_newtcpcb - New tcp control block
 *
 * RETURNS: Pointer to control block or NULL
 ******************************************************************************/

struct tcpcb* tcp_newtcpcb(struct inpcb *inp)
{
  struct tcpcb *tp;

  /* Allocate buffer */
  tp = (struct tcpcb *) mb_alloc(sizeof(struct tcpcb), MT_PCB, M_DONTWAIT);
  if (tp == NULL)
    return NULL;

  /* Zero */
  memset( tp, 0, sizeof(struct tcpcb) );

  /* Setup struct */
  tp->seg_next = (struct tcpiphdr *) tp;
  tp->seg_prev = (struct tcpiphdr *) tp;
  tp->t_maxseg = tcp_mssdflt;
  tp->t_maxsize = tcp_mssdflt;

  if (tcp_do_rfc1323)
    tp->t_flags = TF_REQ_SCALE | TF_REQ_TSTMP;
  else
    tp->t_flags = 0;

  tp->t_inpcb = inp;

  tp->t_srtt = TCPTV_SRTTBASE;
  tp->t_rttvar = tcp_rttdflt * PR_SLOWHZ << 2;
  tp->t_rttmin = TCPTV_MIN;
  TCPT_RANGESET(tp->t_rxtcur,
                ((TCPTV_SRTTBASE >> 2) + (TCPTV_SRTTDFLT << 2)) >> 1,
                TCPTV_MIN,
                TCPTV_REXMTMAX);
  tp->snd_cwnd = TCP_MAXWIN << TCP_MAX_WINSHIFT;

  /* Setup inet control block */
  inp->inp_ip.ip_ttl = ip_defttl;
  inp->inp_ppcb = tp;

  return tp;
}

/*******************************************************************************
 * tcp_template - Create a tcp control block template
 *
 * RETURNS: Pointer to template or zero
 ******************************************************************************/

struct mbuf* tcp_template(struct tcpcb *tp)
{
  struct inpcb *inp;
  struct mbuf *m;
  struct tcpiphdr *n;

  /* Initialize locals */
  inp = tp->t_inpcb;
  m = tp->t_template;

  /* If not allocated */
  if (m == NULL) {

    m = m_get(M_DONTWAIT, MT_HEADER, sizeof(struct tcpiphdr), TRUE);
    if (m == NULL)
      return NULL;

    /* Set length */
    m->m_len = sizeof(struct tcpiphdr);

  } /* End if not allocated */

  /* Get pointer to data */
  n = mtod(m, struct tcpiphdr *);

  /* Setup struct */
  n->ti_next = NULL;
  n->ti_prev = NULL;
  n->ti_x1 = 0;
  n->ti_pr = IPPROTO_TCP;
  n->ti_len = htons(sizeof(struct tcpiphdr) - sizeof(struct ip));
  n->ti_src = inp->inp_laddr;
  n->ti_dst = inp->inp_faddr;
  n->ti_sport = inp->inp_lport;
  n->ti_dport = inp->inp_fport;
  n->ti_seq = 0;
  n->ti_ack = 0;
  n->ti_x2 = 0;
  n->ti_off = 5;
  n->ti_flags = 0;
  n->ti_win = 0;
  n->ti_sum = 0;
  n->ti_urp = 0;

#ifdef DEBUG_TCP
  printf("TCP_TEMPLATE: ");
  inLogAddr(n->ti_src);
  printf(":%d", ntohs(n->ti_sport));
  printf("->");
  inLogAddr(n->ti_dst);
  printf(":%d", ntohs(n->ti_dport));
  printf("\n");
#endif

  return m;
}

/*******************************************************************************
 * tcp_respond - Send single message to tcp at address
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_respond(struct tcpcb *tp,
                 struct tcpiphdr *ti,
                 struct mbuf *m,
                 tcp_seq ack,
                 tcp_seq seq,
                 int flags)
{
  int tlen, win;
  struct route *ro;

  /* Initialize locals */
  win = 0;
  ro = NULL;

  /* If control block non-null */
  if (tp != NULL) {

    win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
    ro = &tp->t_inpcb->inp_route;

  } /* End if control block non-null */

  /* If buffer null */
  if (m == NULL) {

    m = m_gethdr(M_DONTWAIT, MT_HEADER, CL_SIZE_128, TRUE);
    if (m == NULL)
      return;

    tlen = 0;
    m->m_data += max_linkhdr;
    *mtod(m, struct tcpiphdr *) = *ti;
    ti = mtod(m, struct tcpiphdr *);
    flags = TH_ACK;

  } /* If buffer null */

  /* Else buffer non-null */
  else {

    m_freem(m->m_next);
    m->m_next = NULL;
    m->m_data = (char *) ti;
    m->m_len = sizeof(struct tcpiphdr);
    tlen = 0;

    /* Exchange */
    xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, unsigned long);
    xchg(ti->ti_dport, ti->ti_sport, unsigned short);

  } /* End else buffer non-null */

  ti->ti_len = htons((unsigned short) (sizeof(struct tcphdr) + tlen));
  tlen += sizeof(struct tcpiphdr);

  m->m_len = tlen;
  m->m_pkthdr.len = tlen;
  m->m_pkthdr.rcvif = NULL;
  ti->ti_next = NULL;
  ti->ti_prev = NULL;
  ti->ti_x1 = 0;
  ti->ti_seq = htonl(seq);
  ti->ti_ack = htonl(ack);
  ti->ti_x2 = 0;
  ti->ti_off = sizeof(struct tcphdr) >> 2;
  ti->ti_flags = flags;

  if (tp != NULL)
    ti->ti_win = htons((unsigned short) (win >> tp->rcv_scale));
  else
    ti->ti_win = htons((unsigned short) win);

  ti->ti_urp = 0;
  ti->ti_sum = 0;
  ti->ti_sum = in_cksum(m, tlen);
  ((struct ip *) ti)->ip_len = tlen;
  ((struct ip *) ti)->ip_ttl = ip_defttl;

  if (tp != NULL)
    ((struct ip *) ti)->ip_tos = tp->t_inpcb->inp_ip.ip_tos;

  ip_output(m, NULL, ro, 0, NULL);
}

/*******************************************************************************
 * tcp_notify - Notify a tcp user of an asynchronous error;
 * store error as soft error, but wake up user
 * (for now, won't do anything until can select for soft error).
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_notify(struct inpcb *inp, int error)
{
        struct tcpcb *tp = (struct tcpcb *)inp->inp_ppcb;
        struct socket *so = inp->inp_socket;

        /*
         * Ignore some errors if we are hooked up.
         * If connection hasn't completed, has retransmitted several times,
         * and receives a second error, give up now.  This is better
         * than waiting a long time to establish a connection that
         * can never complete.
         */
        if (tp->t_state == TCPS_ESTABLISHED &&
             (error == EHOSTUNREACH || error == ENETUNREACH ||
              error == EHOSTDOWN)) {

                return;

        }

        else if (tp->t_state < TCPS_ESTABLISHED && tp->t_rxtshift > 3 &&
            tp->t_softerror) {

                so->so_error = error;

        }

        else {

                tp->t_softerror = error;

        }

        if (so->so_timeoSem != NULL)
                wakeup(so->so_timeoSem);

        sorwakeup(so);
        sowwakeup(so);
}

/*******************************************************************************
 * tcp_ctlinput - Control input
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_ctlinput(int cmd, struct sockaddr *sa, struct ip *ip)
{
        struct tcphdr *th;
        VOIDFUNCPTR notify = (VOIDFUNCPTR) tcp_notify;

        if (cmd == PRC_QUENCH)
                notify = (VOIDFUNCPTR) tcp_quench;

        else if (!PRC_IS_REDIRECT(cmd) &&
                 ((unsigned)cmd > PRC_NCMDS || inetctlerrmap[cmd] == 0))
                return;

        if (ip) {

                th = (struct tcphdr *)((caddr_t)ip + (ip->ip_hl << 2));
                in_pcbnotify(tcbinfo.listhead, sa,
                             th->th_dport, ip->ip_src, th->th_sport,
                             cmd, notify);
        }

        else {

                in_pcbnotify(tcbinfo.listhead, sa,
                             0, zeroin_addr, 0,
                             cmd, notify);

        }
}

/*******************************************************************************
 * tcp_quench - Handle source quench
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_quench(struct inpcb *inp, int err)
{
  struct tcpcb *tp;

  /* Initialize locals */
  tp = intotcpcb(inp);

  if (tp != NULL)
    tp->snd_cwnd = tp->t_maxseg;
}

/*******************************************************************************
 * tcp_drain - Empty
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_drain(void)
{
}

/*******************************************************************************
 * tcp_close - Close tcp control block
 *
 * RETURNS: NULL
 ******************************************************************************/

struct tcpcb* tcp_close(struct tcpcb *tp)
{
  struct tcpiphdr *t;
  struct inpcb *inp;
  struct socket *so;
  struct mbuf *m;
  struct rtentry *rt;
  unsigned long i;

  /* Initialize locals */
  inp = tp->t_inpcb;
  so = inp->inp_socket;

  /* If sequence less than send buffer max */
  if ( SEQ_LT(tp->iss + so->so_snd.sb_hiwat * 16, tp->snd_max) &&
       ((rt = inp->inp_route.ro_rt) != NULL) &&
       ((struct sockaddr_in *) rt_key(rt))->sin_addr.s_addr != INADDR_ANY) {

    i = 0;

    /* If rmx lock rtt */
    if ( (rt->rt_rmx.rmx_locks & RTV_RTT) == 0) {

      i = tp->t_srtt * (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
      if (rt->rt_rmx.rmx_rtt && i)
        rt->rt_rmx.rmx_rtt = (rt->rt_rmx.rmx_rtt + i) / 2;
      else
        rt->rt_rmx.rmx_rtt = i;

    } /* End if rmx lock rtt */

    if ( ( ((rt->rt_rmx.rmx_locks & RTV_SSTRESH) == 0) &&
           (i = tp->snd_ssthresh) && (rt->rt_rmx.rmx_ssthresh) ) ||
           (i < (rt->rt_rmx.rmx_sendpipe / 2) ) ) {

      i = (i + tp->t_maxseg / 2) / tp->t_maxseg;
      if (i < 2)
        i = 2;

      i *= (unsigned long) (tp->t_maxseg + sizeof(struct tcpiphdr));

      if (rt->rt_rmx.rmx_ssthresh)
        rt->rt_rmx.rmx_ssthresh = (rt->rt_rmx.rmx_ssthresh + i) / 2;
      else
        rt->rt_rmx.rmx_ssthresh = i;

    }

  } /* End if sequence less than send buffer max */

  /* Free reassembly queue */
  t = tp->seg_next;
  while ( t != (struct tcpiphdr *) tp) {

    t = (struct tcpiphdr *) t->ti_next;
    m = REASS_MBUF((struct tcpiphdr *) t->ti_next);

    /* Remove and free */
    remque(t->ti_prev);
    m_freem(m);

  }

  /* Free template */
  if (tp->t_template != NULL)
    m_free(tp->t_template);

  /* Free control block */
  mb_free(tp);

  /* Set to null */
  inp->inp_ppcb = NULL;

  /* Mark as disconnected */
  soisdisconnected(so);

  /* If last null last pointer */
  if (inp == tcp_last_inpcb)
    tcp_last_inpcb = NULL;

  /* Detach inet control block */
  in_pcbdetach(inp);

  /* Update status */
  tcpstat.tcps_closed++;

  return NULL;
}

/*******************************************************************************
 * tcp_drop - Drop tcp connection
 *
 * RETURNS: NULL
 ******************************************************************************/

struct tcpcb* tcp_drop(struct tcpcb *tp, int error)
{
  struct socket *so;

  /* Initialize locals */
  so = tp->t_inpcb->inp_socket;

  /* If normal disconnection */
  if (TCPS_HAVERCVDSYN(tp->t_state)) {

    tp->t_state = TCPS_CLOSED;
    tcp_output(tp);
    tcpstat.tcps_drops++;

  } /* End if normal disconnection */

  /* Else abnormal disconnection */
  else {

    tcpstat.tcps_conndrops++;

  } /* End else abnormal disconnection */

  /* If timeout and soft error */
  if ( (error == ETIMEDOUT) &&
       (tp->t_softerror) )
    error = tp->t_softerror;

  /* Set socket error */
  so->so_error = error;

  return tcp_close(tp);
}

/*******************************************************************************
 * tcp_output - Tcp output routine: figure out what should be sent and send it.
 *
 * RETURNS: Zero or error
 ******************************************************************************/

int tcp_output(struct tcpcb *tp)
{
        struct socket *so = tp->t_inpcb->inp_socket;
        struct rtentry *rt;
        long len, win;
        int off, flags, error;
        struct mbuf *m;
        struct tcpiphdr *ti;
        u_char opt[MAX_TCPOPTLEN];
        unsigned optlen, hdrlen;
        int idle, sendalot;
        long adv;
        unsigned short mss;
        unsigned long *lp;
        tcp_seq startseq;
        BOOL pktsent = FALSE;

        /*
         * Determine length of data that should be transmitted,
         * and flags that will be used.
         * If there is some data or critical controls (SYN, RST)
         * to send, then transmit; otherwise, investigate further.
         */
        idle = (tp->snd_max == tp->snd_una);
        if (idle && tp->t_idle >= tp->t_rxtcur)
                /*
                 * We have been idle for "a while" and no acks are
                 * expected to clock out any data we send --
                 * slow start to get ack "clock" running again.
                 */
                tp->snd_cwnd = tp->t_maxseg;
again:
        sendalot = 0;
        off = tp->snd_nxt - tp->snd_una;
        win = min(tp->snd_wnd, tp->snd_cwnd);

        flags = tcp_outflags[tp->t_state];

        /*
         * If in persist timeout with window of 0, send 1 byte.
         * Otherwise, if window is small but nonzero
         * and timer expired, we will send what we can
         * and go to transmit state.
         */
        if (tp->t_force) {

                if (win == 0) {

                        /*
                         * If we still have some data to send, then
                         * clear the FIN bit.  Usually this would
                         * happen below when it realizes that we
                         * aren't sending all the data.  However,
                         * if we have exactly 1 byte of unset data,
                         * then it won't clear the FIN bit below,
                         * and if we are in persist state, we wind
                         * up sending the packet without recording
                         * that we sent the FIN bit.
                         *
                         * We can't just blindly clear the FIN bit,
                         * because if we don't have any more data
                         * to send then the probe will be the FIN
                         * itself.
                         */
                        if (off < so->so_snd.sb_cc)
                                flags &= ~TH_FIN;
                        win = 1;

                }

                else {

                        tp->t_timer[TCPT_PERSIST] = 0;
                        tp->t_rxtshift = 0;

                }

        }

        len = min(so->so_snd.sb_cc, win) - off;

        if (len < 0) {

                /*
                 * If FIN has been sent but not acked,
                 * but we haven't been called to retransmit,
                 * len will be -1.  Otherwise, window shrank
                 * after we sent into it.  If window shrank to 0,
                 * cancel pending retransmit and pull snd_nxt
                 * back to (closed) window.  We will enter persist
                 * state below.  If the window didn't close completely,
                 * just wait for an ACK.
                 */
                len = 0;
                if (win == 0) {

                        tp->t_timer[TCPT_REXMT] = 0;
                        tp->snd_nxt = tp->snd_una;

                }

        }

        if (len > tp->t_maxseg) {

                len = tp->t_maxseg;
                sendalot = 1;

        }

        if (SEQ_LT(tp->snd_nxt + len, tp->snd_una + so->so_snd.sb_cc))
                flags &= ~TH_FIN;

        win = sbspace(&so->so_rcv);

        /*
         * Sender silly window avoidance.  If connection is idle
         * and can send all data, a maximum segment,
         * at least a maximum default-size segment do it,
         * or are forced, do it; otherwise don't bother.
         * If peer's buffer is tiny, then send
         * when window is at least half open.
         * If retransmitting (possibly after persist timer forced us
         * to send into a small window), then must resend.
         */
        if (len) {

                if (len == tp->t_maxseg)
                        goto send;

                if ((idle || tp->t_flags & TF_NODELAY) &&
                    (tp->t_flags & TF_EOB) &&
                    len + off >= so->so_snd.sb_cc)
                        goto send;

                if (tp->t_force)
                        goto send;

                if (len >= tp->max_sndwnd / 2)
                        goto send;

                if (SEQ_LT(tp->snd_nxt, tp->snd_max))
                        goto send;

        }

        /*
         * Compare available window to amount of window
         * known to peer (as advertised window less
         * next expected input).  If the difference is at least two
         * max size segments, or at least 50% of the maximum possible
         * window, then want to send a window update to peer.
         */
        if (win > 0) {

                /*
                 * "adv" is the amount we can increase the window,
                 * taking into account that we are limited by
                 * TCP_MAXWIN << tp->rcv_scale.
                 */
                adv = min(win, (long)TCP_MAXWIN << tp->rcv_scale) -
                        (tp->rcv_adv - tp->rcv_nxt);

                if (adv >= (long) (2 * tp->t_maxseg))
                        goto send;

                if (2 * adv >= (long) so->so_rcv.sb_hiwat)
                        goto send;

        }

        /*
         * Send if we owe peer an ACK.
         */
        if (tp->t_flags & TF_ACKNOW)
                goto send;

        if (flags & (TH_SYN | TH_RST))
                goto send;

        if (SEQ_GT(tp->snd_up, tp->snd_una))
                goto send;

        /*
         * If our state indicates that FIN should be sent
         * and we have not yet done so, or we're retransmitting the FIN,
         * then we need to send.
         */
        if (flags & TH_FIN &&
            ((tp->t_flags & TF_SENTFIN) == 0 || tp->snd_nxt == tp->snd_una))
                goto send;

        /*
         * TCP window updates are not reliable, rather a polling protocol
         * using ``persist'' packets is used to insure receipt of window
         * updates.  The three ``states'' for the output side are:
         *      idle                    not doing retransmits or persists
         *      persisting              to move a small or zero window
         *      (re)transmitting        and thereby not persisting
         *
         * tp->t_timer[TCPT_PERSIST]
         *      is set when we are in persist state.
         * tp->t_force
         *      is set when we are called to send a persist packet.
         * tp->t_timer[TCPT_REXMT]
         *      is set when we are retransmitting
         * The output side is idle when both timers are zero.
         *
         * If send window is too small, there is data to transmit, and no
         * retransmit or persist is pending, then go to persist state.
         * If nothing happens soon, send when timer expires:
         * if window is nonzero, transmit what we can,
         * otherwise force out a byte.
         */
        if (so->so_snd.sb_cc && tp->t_timer[TCPT_REXMT] == 0 &&
            tp->t_timer[TCPT_PERSIST] == 0) {

                tp->t_rxtshift = 0;
                tcp_setpersist(tp);

        }

        /*
         * No reason to send a segment, just return.
         */
        return (0);

send:
        /*
         * Before ESTABLISHED, force sending of initial options
         * unless TCP set not to do any options.
         * NOTE: we assume that the IP/TCP header plus TCP options
         * always fit in a single mbuf, leaving room for a maximum
         * link header, i.e.
         *      max_linkhdr + sizeof (struct tcpiphdr) + optlen <= MHLEN
         */
        optlen = 0;
        hdrlen = sizeof (struct tcpiphdr);

        if (flags & TH_SYN) {

                tp->snd_nxt = tp->iss;
                if ((tp->t_flags & TF_NOOPT) == 0) {

                        opt[0] = TCPOPT_MAXSEG;
                        opt[1] = TCPOLEN_MAXSEG;
                        mss = htons((u_short) tcp_mss(tp, 0));
                        memcpy(opt + 2, &mss, sizeof(mss));
                        optlen = TCPOLEN_MAXSEG;

                        if ((tp->t_flags & TF_REQ_SCALE) &&
                            ((flags & TH_ACK) == 0 ||
                            (tp->t_flags & TF_RCVD_SCALE))) {

                                *((u_long *) (opt + optlen)) = htonl(
                                        TCPOPT_NOP << 24 |
                                        TCPOPT_WINDOW << 16 |
                                        TCPOLEN_WINDOW << 8 |
                                        tp->request_r_scale);

                                optlen += TCPOLEN_MAXSEG;

                        }

                }

        }

        /*
         * Send a timestamp and echo-reply if this is a SYN and our side
         * wants to use timestamps (TF_REQ_TSTMP is set) or both our side
         * and our peer have sent timestamps in our SYN's.
         */
        if ((tp->t_flags & (TF_REQ_TSTMP|TF_NOOPT)) == TF_REQ_TSTMP &&
            (flags & TH_RST) == 0 &&
            ((flags & (TH_SYN | TH_ACK)) == TH_SYN ||
             (tp->t_flags & TF_RCVD_TSTMP))) {

                lp = (u_long *)(opt + optlen);

                /* Form timestamp option as shown in appendix A of RFC 1323. */
                *lp++ = htonl(TCPOPT_TSTAMP_HDR);
                *lp++ = htonl(tcp_now);
                *lp   = htonl(tp->ts_recent);
                optlen += TCPOLEN_TSTAMP_APPA;

        }

        hdrlen += optlen;

        /*
         * Adjust data length if insertion of options will
         * bump the packet length beyond the t_maxopd length.
         * Clear the FIN bit because we cut off the tail of
         * the segment.
         */
         if (len > tp->t_maxseg - optlen) {

                len = tp->t_maxseg - optlen;
                sendalot = 1;
                flags &= ~TH_FIN;

        }

#ifdef DIAGNOSTIC
        if (max_linkhdr + hdrlen > CL_SIZE_128)
                panic("tcphdr too big");
#endif

        /*
         * Grab a header mbuf, attaching a copy of data to
         * be transmitted, and initialize the header from
         * the template for sends on this connection.
         */
        if (len) {

                if (tp->t_force && len == 1) {

                        tcpstat.tcps_sndprobe++;

                }

                else if (SEQ_LT(tp->snd_nxt, tp->snd_max)) {

                        tcpstat.tcps_sndrexmitpack++;
                        tcpstat.tcps_sndrexmitbyte += len;


                }

                else {

                        tcpstat.tcps_sndpack++;
                        tcpstat.tcps_sndbyte += len;

                }

#ifdef notyet
                if ((m = m_copypack(so->so_snd.sb_mb, off,
                    (int)len, max_linkhdr + hdrlen)) == 0) {
                        error = ENOBUFS;
                        goto out;
                }
                /*
                 * m_copypack left space for our hdr; use it.
                 */
                m->m_len += hdrlen;
                m->m_data -= hdrlen;
#else
                m = m_gethdr(M_DONTWAIT, MT_HEADER, (max_linkhdr + hdrlen),
                             TRUE);
                if (m == NULL) {

                        m_free(m);
                        error = ENOBUFS;
                        goto out;

                }

                m->m_data += max_linkhdr;
                m->m_len = hdrlen;
                m->m_next = m_copy(so->so_snd.sb_mb, off, len);
                if (m->m_next == NULL) {

                        m_free(m);
                        error = ENOBUFS;
                        goto out;

                }
#endif
                /*
                 * If we're sending everything we've got, set PUSH.
                 * (This will keep happy those implementations which only
                 * give data to the user when a buffer fills or
                 * a PUSH comes in.)
                 */
                if (off + len == so->so_snd.sb_cc)
                        flags |= TH_PUSH;

        }

        else {

                if (tp->t_flags & TF_ACKNOW)
                        tcpstat.tcps_sndacks++;
                else if (flags & (TH_SYN|TH_FIN|TH_RST))
                        tcpstat.tcps_sndctrl++;
                else if (SEQ_GT(tp->snd_up, tp->snd_una))
                        tcpstat.tcps_sndurg++;
                else
                        tcpstat.tcps_sndwinup++;

                m = m_gethdr(M_DONTWAIT, MT_HEADER, (max_linkhdr + hdrlen),
                             TRUE);
                if (m == NULL) {

                        error = ENOBUFS;
                        goto out;

                }

                m->m_data += max_linkhdr;
                m->m_len = hdrlen;
        }

        m->m_pkthdr.rcvif = NULL;
        ti = mtod(m, struct tcpiphdr *);
        if (tp->t_template == NULL)
                panic("tcp_output");

        memcpy(ti,
               mtod(tp->t_template, struct tcpiphdr *),
               sizeof(struct tcpiphdr));

        /*
         * Fill in fields, remembering maximum advertised
         * window for use in delaying messages about window sizes.
         * If resending a FIN, be sure not to use a new sequence number.
         */
        if (flags & TH_FIN && tp->t_flags & TF_SENTFIN &&
            tp->snd_nxt == tp->snd_max)
                tp->snd_nxt--;
        /*
         * If we are doing retransmissions, then snd_nxt will
         * not reflect the first unsent octet.  For ACK only
         * packets, we do not want the sequence number of the
         * retransmitted packet, we want the sequence number
         * of the next unsent octet.  So, if there is no data
         * (and no SYN or FIN), use snd_max instead of snd_nxt
         * when filling in ti_seq.  But if we are in persist
         * state, snd_max might reflect one byte beyond the
         * right edge of the window, so use snd_nxt in that
         * case, since we know we aren't doing a retransmission.
         * (retransmit and persist are mutually exclusive...)
         */
        if (len || (flags & (TH_SYN|TH_FIN)) || tp->t_timer[TCPT_PERSIST])
                ti->ti_seq = htonl(tp->snd_nxt);
        else
                ti->ti_seq = htonl(tp->snd_max);

        ti->ti_ack = htonl(tp->rcv_nxt);
        if (optlen) {

                memcpy(ti + 1, opt, optlen);
                ti->ti_off = (sizeof (struct tcphdr) + optlen) >> 2;

        }

        ti->ti_flags = flags;

        /*
         * Calculate receive window.  Don't shrink window,
         * but avoid silly window syndrome.
         */
        if (win < (long)(so->so_rcv.sb_hiwat / 4) && win < (long)tp->t_maxseg)
                win = 0;

        if (win > (long)TCP_MAXWIN << tp->rcv_scale)
                win = (long)TCP_MAXWIN << tp->rcv_scale;

        if (win < (long)(tp->rcv_adv - tp->rcv_nxt))
                win = (long)(tp->rcv_adv - tp->rcv_nxt);

        ti->ti_win = htons((u_short) (win>>tp->rcv_scale));

        if (SEQ_GT(tp->snd_up, tp->snd_nxt)) {

                ti->ti_urp = htons((u_short)(tp->snd_up - tp->snd_nxt));
                ti->ti_flags |= TH_URG;

        }

        else {
                /*
                 * If no urgent pointer to send, then we pull
                 * the urgent pointer to the left edge of the send window
                 * so that it doesn't drift into the send window on sequence
                 * number wraparound.
                 */
                tp->snd_up = tp->snd_una;               /* drag it along */

        }

        /*
         * Put TCP length in extended header, and then
         * checksum extended header and data.
         */
        if (len + optlen)
                ti->ti_len = htons((u_short)(sizeof (struct tcphdr) +
                    optlen + len));
        ti->ti_sum = in_cksum(m, (int)(hdrlen + len));

        /*
         * In transmit state, time the transmission and arrange for
         * the retransmit.  In persist state, just set snd_max.
         */
        if (tp->t_force == 0 || tp->t_timer[TCPT_PERSIST] == 0) {

                startseq = tp->snd_nxt;

                /*
                 * Advance snd_nxt over sequence space of this segment.
                 */
                if (flags & (TH_SYN|TH_FIN)) {

                        if (flags & TH_SYN)
                                tp->snd_nxt++;

                        if (flags & TH_FIN) {

                                tp->snd_nxt++;
                                tp->t_flags |= TF_SENTFIN;

                        }

                }

                tp->snd_nxt += len;

                if (SEQ_GT(tp->snd_nxt, tp->snd_max)) {

                        tp->snd_max = tp->snd_nxt;
                        /*
                         * Time this transmission if not a retransmission and
                         * not currently timing anything.
                         */
                        if (tp->t_rtt == 0) {

                                tp->t_rtt = 1;
                                tp->t_rtseq = startseq;
                                tcpstat.tcps_segstimed++;

                        }

                }

                /*
                 * Set retransmit timer if not currently set,
                 * and not doing an ack or a keep-alive probe.
                 * Initial value for retransmit timer is smoothed
                 * round-trip time + 2 * round-trip time variance.
                 * Initialize shift counter which is used for backoff
                 * of retransmit time.
                 */
                if (tp->t_timer[TCPT_REXMT] == 0 &&
                    tp->snd_nxt != tp->snd_una) {

                        tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

                        if (tp->t_timer[TCPT_PERSIST]) {

                                tp->t_timer[TCPT_PERSIST] = 0;
                                tp->t_rxtshift = 0;

                        }

                }

        }

        else {

                if (SEQ_GT(tp->snd_nxt + len, tp->snd_max))
                        tp->snd_max = tp->snd_nxt + len;

        }

#ifdef TCPDEBUG
        /*
         * Trace.
         */
        if (so->so_options & SO_DEBUG)
                tcp_trace(TA_OUTPUT, tp->t_state, tp, ti, 0);
#endif

        /*
         * Fill in IP length and desired time to live and
         * send to IP level.  There should be a better way
         * to handle ttl and tos; we could keep them in
         * the template, but need a way to checksum without them.
         */
        m->m_pkthdr.len = hdrlen + len;

        ((struct ip *)ti)->ip_len = m->m_pkthdr.len;
        ((struct ip *)ti)->ip_ttl = tp->t_inpcb->inp_ip.ip_ttl; /* XXX */
        ((struct ip *)ti)->ip_tos = tp->t_inpcb->inp_ip.ip_tos; /* XXX */

        rt = tp->t_inpcb->inp_route.ro_rt;

        if ( (rt != NULL) &&
             (rt->rt_flags & RTF_UP) &&
             !(rt->rt_rmx.rmx_locks & RTV_MTU) ) {

                if (tp->t_inpcb->inp_socket->so_options & SO_USEPATHMTU)
                        ((struct ip *) ti)->ip_off |= IP_DF;

        }

        error = ip_output(m, NULL, &tp->t_inpcb->inp_route,
            so->so_options & SO_DONTROUTE, 0);

        pktsent = TRUE;

        if (error) {

out:

                if (!pktsent) {

                        if (!SEQ_GT(tp->snd_up, tp->snd_nxt))
                                tp->snd_up = tp->snd_una;

                        if (tp->t_force == 0 ||
                            tp->t_timer[TCPT_PERSIST] == 0) {

                                startseq = tp->snd_nxt;

                                if (flags & (TH_SYN | TH_FIN)) {

                                        if (flags & TH_FIN)
                                                tp->snd_nxt++;

                                        if (flags & TH_FIN) {

                                                tp->snd_nxt++;
                                                tp->t_flags |= TF_SENTFIN;

                                        }

                                }

                                tp->snd_nxt += len;

                                if (SEQ_GT(tp->snd_nxt, tp->snd_max)) {

                                        tp->snd_max = tp->snd_nxt;

                                        if (tp->t_rtt == 0) {

                                                tp->t_rtt = 1;
                                                tp->t_rtseq = startseq;
                                                tcpstat.tcps_segstimed++;

                                        }

                                }

                                if (tp->t_timer[TCPT_REXMT] == 0 &&
                                    tp->snd_nxt != tp->snd_una) {

                                        tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

                                        if (tp->t_timer[TCPT_PERSIST]) {

                                                tp->t_timer[TCPT_PERSIST] = 0;
                                                tp->t_rxtshift = 0;

                                        }

                                }

                        }

                        else {

                                if (SEQ_GT(tp->snd_nxt + len, tp->snd_max))
                                        tp->snd_max = tp->snd_nxt + len;

                        }

                }

                if (error == ENOBUFS) {

                        tcp_quench(tp->t_inpcb, 0);
                        return (0);

                }

                if (error == EMSGSIZE) {

                        /* NOTE: Fix me */
                        //tcp_updatemtu(tp->t_inpcb, 0);
                        return (0);

                }

                if ((error == EHOSTUNREACH || error == ENETDOWN)
                    && TCPS_HAVERCVDSYN(tp->t_state)) {

                        tp->t_softerror = error;
                        return (0);

                }

                return (error);

        }

        tcpstat.tcps_sndtotal++;

        /*
         * Data sent (as far as we can tell).
         * If this advertises a larger window than any other segment,
         * then remember the size of the advertised window.
         * Any pending ACK has now been sent.
         */
        if (win > 0 && SEQ_GT(tp->rcv_nxt+win, tp->rcv_adv))
                tp->rcv_adv = tp->rcv_nxt + win;

        tp->last_ack_sent = tp->rcv_nxt;
        tp->t_flags &= ~(TF_ACKNOW|TF_DELACK);

        if (sendalot)
                goto again;

        return (0);
}

/*******************************************************************************
 * tcp_setpersist
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_setpersist(struct tcpcb *tp)
{
        int t = ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1;

        if (tp->t_timer[TCPT_REXMT])
                panic("tcp_output REXMT");
        /*
         * Start/restart persistance timer.
         */
        TCPT_RANGESET(tp->t_timer[TCPT_PERSIST],
            t * tcp_backoff[tp->t_rxtshift],
            TCPTV_PERSMIN, TCPTV_PERSMAX);
        if (tp->t_rxtshift < TCP_MAXRXTSHIFT)
                tp->t_rxtshift++;
}

/*******************************************************************************
 * tcp_reass - Reasslble package
 *
 * RETURNS: Zero or flags
 ******************************************************************************/

int tcp_reass(struct tcpcb *tp, struct tcpiphdr *ti, struct mbuf *m)
{
        struct tcpiphdr *q;
        struct socket *so = tp->t_inpcb->inp_socket;
        int flags, i;

        /*
         * Call with ti==0 after become established to
         * force pre-ESTABLISHED data up to user socket.
         */
        if (ti == NULL)
                goto present;

        /*
         * Find a segment which begins after this one does.
         */
        for (q = tp->seg_next; q != (struct tcpiphdr *)tp;
            q = (struct tcpiphdr *)q->ti_next)
                if (SEQ_GT(q->ti_seq, ti->ti_seq))
                        break;

        /*
         * If there is a preceding segment, it may provide some of
         * our data already.  If so, drop the data from the incoming
         * segment.  If it provides all of our data, drop us.
         */
        if ((struct tcpiphdr *)q->ti_prev != (struct tcpiphdr *)tp) {

                q = (struct tcpiphdr *)q->ti_prev;

                /* conversion to int (in i) handles seq wraparound */
                i = q->ti_seq + q->ti_len - ti->ti_seq;

                if (i > 0) {

                        if (i >= ti->ti_len) {

                                tcpstat.tcps_rcvduppack++;
                                tcpstat.tcps_rcvdupbyte += ti->ti_len;
                                m_freem(m);
                                return (0);

                        }

                        m_adj(m, i);
                        ti->ti_len -= i;
                        ti->ti_seq += i;

                }

                q = (struct tcpiphdr *) (q->ti_next);
        }

        tcpstat.tcps_rcvoopack++;
        tcpstat.tcps_rcvoobyte += ti->ti_len;
        REASS_MBUF(ti) = m;             /* XXX */

        /*
         * While we overlap succeeding segments trim them or,
         * if they are completely covered, dequeue them.
         */
        while (q != (struct tcpiphdr *) tp) {

                i = (ti->ti_seq + ti->ti_len) - q->ti_seq;

                if (i <= 0)
                        break;

                if (i < q->ti_len) {

                        q->ti_seq += i;
                        q->ti_len -= i;
                        m_adj(REASS_MBUF(q), i);
                        break;

                }

                q = (struct tcpiphdr *) q->ti_next;
                m = REASS_MBUF((struct tcpiphdr *)q->ti_prev);
                remque(q->ti_prev);
                m_freem(m);
        }

        /*
         * Stick new segment in its place.
         */
        insque(ti, q->ti_prev);

present:

        /*
         * Present data to user, advancing rcv_nxt through
         * completed sequence space.
         */
        if (TCPS_HAVERCVDSYN(tp->t_state) == 0)
                return (0);

        ti = tp->seg_next;
        if (ti == (struct tcpiphdr *)tp || ti->ti_seq != tp->rcv_nxt)
                return (0);

        if (tp->t_state == TCPS_SYN_RECEIVED && ti->ti_len)
                return (0);

        do {

                tp->rcv_nxt += ti->ti_len;
                flags = ti->ti_flags & TH_FIN;
                remque(ti);
                m = REASS_MBUF(ti);
                ti = (struct tcpiphdr *)ti->ti_next;

                if (so->so_state & SS_CANTRCVMORE)
                        m_freem(m);
                else
                        sbappend(&so->so_rcv, m);

        } while (ti != (struct tcpiphdr *)tp && ti->ti_seq == tp->rcv_nxt);

        sorwakeup(so);

        return (flags);
}

/*******************************************************************************
 * tcp_input - TCP input routine, follows pages 65-76 of the
 * protocol specification dated September, 1981 very closely.
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_input(struct mbuf *m, int iphlen)
{
        struct tcpiphdr *ti;
        struct inpcb *inp;
        u_char *optp = NULL;
        int optlen = 0;
        int len, tlen, off;
        struct tcpcb *tp = NULL;
        int tiflags;
        struct socket *so = NULL;
        int todrop, acked, ourfinisacked, needoutput = 0;
        short ostate = 0;
        struct in_addr laddr;
        int dropsocket = 0;
        int iss = 0;
        unsigned long tiwin, ts_val, ts_ecr;
        int ts_present = 0;
        unsigned int win;
        struct mbuf *am;
        struct sockaddr_in *sin;
        tcp_seq onxt;
        unsigned int cw, incr;
        int i;

#ifdef DEBUG_TCP
        printf("tcp_input called:\n");
#endif

        tcpstat.tcps_rcvtotal++;

        /*
         * Get IP and TCP header together in first mbuf.
         * Note: IP leaves IP header in first mbuf.
         */
        ti = mtod(m, struct tcpiphdr *);
        if (iphlen > sizeof (struct ip))
                ip_stripoptions(m, (struct mbuf *)0);

        if (m->m_len < sizeof (struct tcpiphdr)) {

                if ((m = m_pullup(m, sizeof (struct tcpiphdr))) == NULL) {

                        tcpstat.tcps_rcvshort++;
                        return;

                }

                ti = mtod(m, struct tcpiphdr *);

        }

        /*
         * Checksum extended TCP header and data.
         */
        tlen = ((struct ip *)ti)->ip_len;
        len = sizeof (struct ip) + tlen;
        ti->ti_next = ti->ti_prev = NULL;
        ti->ti_x1 = 0;
        ti->ti_len = (u_short) tlen;
        HTONS(ti->ti_len);

        if ((ti->ti_sum = in_cksum(m, len))) {

                tcpstat.tcps_rcvbadsum++;
                goto drop;

        }

        /*
         * Check that TCP offset makes sense,
         * pull out TCP options and adjust length.              XXX
         */
        off = ti->ti_off << 2;
        if (off < sizeof (struct tcphdr) || off > tlen) {

                tcpstat.tcps_rcvbadoff++;
                goto drop;

        }

        tlen -= off;
        ti->ti_len = tlen;
        if (off > sizeof (struct tcphdr)) {

                if (m->m_len < sizeof(struct ip) + off) {

                        if ((m = m_pullup(m, sizeof (struct ip) + off)) == NULL) {


                                tcpstat.tcps_rcvshort++;
                                return;

                        }

                        ti = mtod(m, struct tcpiphdr *);

                }

                optlen = off - sizeof (struct tcphdr);
                optp = mtod(m, u_char *) + sizeof (struct tcpiphdr);

                /*
                 * Do quick retrieval of timestamp options ("options
                 * prediction?").  If timestamp is the only option and it's
                 * formatted as recommended in RFC 1323 appendix A, we
                 * quickly get the values now and not bother calling
                 * tcp_dooptions(), etc.
                 */
                if ((optlen == TCPOLEN_TSTAMP_APPA ||
                     (optlen > TCPOLEN_TSTAMP_APPA &&
                        optp[TCPOLEN_TSTAMP_APPA] == TCPOPT_EOL)) &&
                     *(u_long *)optp == htonl(TCPOPT_TSTAMP_HDR) &&
                     (ti->ti_flags & TH_SYN) == 0) {

                        ts_present = 1;
                        ts_val = ntohl(*(unsigned long *) (optp + 4));
                        ts_ecr = ntohl(*(unsigned long *) (optp + 8));
                        optp = NULL;    /* we've parsed the options */

                }

        }

        tiflags = ti->ti_flags;

        /*
         * Convert TCP protocol specific fields to host format.
         */
        NTOHL(ti->ti_seq);
        NTOHL(ti->ti_ack);
        NTOHS(ti->ti_win);
        NTOHS(ti->ti_urp);

        /*
         * Locate pcb for segment.
         */

findpcb:

        inp = tcp_last_inpcb;
        if ( (inp != NULL) &&
             (inp->inp_lport == ti->ti_dport) &&
             (inp->inp_fport == ti->ti_sport) &&
             (inp->inp_faddr.s_addr == ti->ti_src.s_addr) &&
             (inp->inp_laddr.s_addr == ti->ti_dst.s_addr) ) {

                goto pcbfound;

        }

        else {

                tcpstat.tcps_pcbcachemiss++;

                /*
                 * First look for an exact match.
                 */
                inp = in_pcblookuphash(&tcbinfo, ti->ti_src, ti->ti_sport,
                        ti->ti_dst, ti->ti_dport, 1);

                if (inp == NULL) {

                /*
                 * If the state is CLOSED (i.e., TCB does not exist) then
                 * all data in the incoming segment is discarded.
                 * If the TCB exists but is in CLOSED state, it is embryonic,
                 * but should either do a listen or a connect soon.
                 */
                        goto dropwithreset;

                }

        }

        /* Store last */
        tcp_last_inpcb = inp;

pcbfound:

        tp = intotcpcb(inp);
        if (tp == NULL)
                goto dropwithreset;

        if (tp->t_state == TCPS_CLOSED)
                goto drop;

        /* Unscale the window into a 32-bit value. */
        if ((tiflags & TH_SYN) == 0)
                tiwin = ti->ti_win << tp->snd_scale;
        else
                tiwin = ti->ti_win;

        so = inp->inp_socket;
        if (so->so_options & (SO_DEBUG|SO_ACCEPTCONN)) {

#ifdef TCPDEBUG
                if (so->so_options & SO_DEBUG) {

                        ostate = tp->t_state;
                        tcp_saveti = *ti;

                }
#endif

                if (so->so_options & SO_ACCEPTCONN) {

                        if ((tiflags & (TH_RST | TH_ACK | TH_SYN)) != TH_SYN) {

                                if (tiflags & TH_ACK) {

                                        tcpstat.tcps_badsyn++;
                                        goto dropwithreset;

                                }

                                goto drop;

                        }

                        so = sonewconn(so, 0);
                        if (so == NULL)
                                goto drop;

                        /*
                         * This is ugly, but ....
                         *
                         * Mark socket as temporary until we're
                         * committed to keeping it.  The code at
                         * ``drop'' and ``dropwithreset'' check the
                         * flag dropsocket to see if the temporary
                         * socket created here should be discarded.
                         * We mark the socket as discardable until
                         * we're committed to it below in TCPS_LISTEN.
                         */
                        dropsocket++;
                        inp = (struct inpcb *)so->so_pcb;
                        i = in_pcbhash(inp);
                        inp->inp_laddr = ti->ti_dst;
                        inp->inp_lport = ti->ti_dport;
                        in_pcbchghash(inp, i);

                        tp = intotcpcb(inp);
                        tp->t_state = TCPS_LISTEN;

                        /* Compute proper scaling value from buffer space */
                        while (tp->request_r_scale < TCP_MAX_WINSHIFT &&
                           TCP_MAXWIN << tp->request_r_scale < so->so_rcv.sb_hiwat)
                                tp->request_r_scale++;

                }

        }

        /*
         * Segment received on connection.
         * Reset idle time and keep-alive timer.
         */
        tp->t_idle = 0;
        tp->t_timer[TCPT_KEEP] = tcp_keepidle;

        /*
         * Process options if not in LISTEN state,
         * else do it below (after getting remote address).
         */
        if (optp && tp->t_state != TCPS_LISTEN)
                tcp_dooptions(tp, optp, optlen, ti,
                        &ts_present, &ts_val, &ts_ecr);

        /*
         * Header prediction: check for the two common cases
         * of a uni-directional data xfer.  If the packet has
         * no control flags, is in-sequence, the window didn't
         * change and we're not retransmitting, it's a
         * candidate.  If the length is zero and the ack moved
         * forward, we're the sender side of the xfer.  Just
         * free the data acked & wake any higher level process
         * that was blocked waiting for space.  If the length
         * is non-zero and the ack didn't move, we're the
         * receiver side.  If we're getting packets in-order
         * (the reassembly queue is empty), add the data to
         * the socket buffer and note that we need a delayed ack.
         * Make sure that the hidden state-flags are also off.
         * Since we check for TCPS_ESTABLISHED above, it can only
         * be TH_NEEDSYN.
         */
        if (tp->t_state == TCPS_ESTABLISHED &&
            (tiflags & (TH_SYN|TH_FIN|TH_RST|TH_URG|TH_ACK)) == TH_ACK &&
            (!ts_present || TSTMP_GEQ(ts_val, tp->ts_recent)) &&
            ti->ti_seq == tp->rcv_nxt &&
            tiwin && tiwin == tp->snd_wnd &&
            tp->snd_nxt == tp->snd_max) {

                /*
                 * If last ACK falls within this segment's sequence numbers,
                 * record the timestamp.
                 * NOTE that the test is modified according to the latest
                 * proposal of the tcplw@cray.com list (Braden 1993/04/26).
                 */
                if (ts_present && SEQ_LEQ(ti->ti_seq, tp->last_ack_sent)) {

                        tp->ts_recent_age = tcp_now;
                        tp->ts_recent = ts_val;

                }

                if (ti->ti_len == 0) {

                        if (SEQ_GT(ti->ti_ack, tp->snd_una) &&
                            SEQ_LEQ(ti->ti_ack, tp->snd_max) &&
                            tp->snd_cwnd >= tp->snd_wnd) {

                                /*
                                 * this is a pure ack for outstanding data.
                                 */
                                ++tcpstat.tcps_predack;

                                if (ts_present)
                                        tcp_xmit_timer(tp,
                                            tcp_now - ts_ecr + 1);
                                else if (tp->t_rtt &&
                                            SEQ_GT(ti->ti_ack, tp->t_rtseq))
                                        tcp_xmit_timer(tp, tp->t_rtt);

                                acked = ti->ti_ack - tp->snd_una;

                                tcpstat.tcps_rcvackpack++;
                                tcpstat.tcps_rcvackbyte += acked;
                                sbdrop(&so->so_snd, acked);
                                tp->snd_una = ti->ti_ack;
                                m_freem(m);

                                /*
                                 * If all outstanding data are acked, stop
                                 * retransmit timer, otherwise restart timer
                                 * using current (possibly backed-off) value.
                                 * If process is waiting for space,
                                 * wakeup/selwakeup/signal.  If data
                                 * are ready to send, let tcp_output
                                 * decide between more output or persist.
                                 */
                                if (tp->snd_una == tp->snd_max)
                                        tp->t_timer[TCPT_REXMT] = 0;
                                else if (tp->t_timer[TCPT_PERSIST] == 0)
                                        tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

                                if ( (so->so_snd.sb_want > 0) ||
                                     (so->so_snd.sb_sel != NULL) ) {

                                        if (sbspace(&so->so_snd) >=
                                            so->so_snd.sb_lowat)
                                                sowwakeup(so);

                                }

                                if (so->so_snd.sb_cc)
                                        tcp_output(tp);

                                return;
                        }

                }

                else if (ti->ti_ack == tp->snd_una &&
                    tp->seg_next == (struct tcpiphdr *)tp &&
                    ti->ti_len <= sbspace(&so->so_rcv)) {

                        /*
                         * this is a pure, in-sequence data packet
                         * with nothing on the reassembly queue and
                         * we have enough buffer space to take it.
                         */
                        ++tcpstat.tcps_preddat;
                        tp->rcv_nxt += ti->ti_len;
                        tcpstat.tcps_rcvpack++;
                        tcpstat.tcps_rcvbyte += ti->ti_len;

                        m->m_data += sizeof(struct tcpiphdr) +
                                off - sizeof(struct tcphdr);
                        m->m_len -= sizeof(struct tcpiphdr) +
                                off - sizeof(struct tcphdr);

                        /*
                         * Add data to socket buffer.
                         */
                        sbappend(&so->so_rcv, m);
                        sorwakeup(so);
                        if (tiflags & TH_PUSH)
                                tp->t_flags |= TF_ACKNOW;
                        else
                                tp->t_flags |= TF_DELACK;

                        return;

                }

        }

        /* Drop TCP, IP headers and TCP opts */
        m->m_data += sizeof(struct tcpiphdr) + off - sizeof(struct tcphdr);
        m->m_len -= sizeof(struct tcpiphdr) + off - sizeof(struct tcphdr);

        /*
         * Calculate amount of space in receive window,
         * and then do TCP input processing.
         * Receive window is amount of space in rcv queue,
         * but not less than advertised window.
         */
        {

        win = sbspace(&so->so_rcv);
        if (win < 0)
                win = 0;

        tp->rcv_wnd = max(win, (int)(tp->rcv_adv - tp->rcv_nxt));

        }

        switch (tp->t_state) {

        /*
         * If the state is LISTEN then ignore segment if it contains an RST.
         * If the segment contains an ACK then it is bad and send a RST.
         * If it does not contain a SYN then it is not interesting; drop it.
         * Don't bother responding if the destination was a broadcast.
         * Otherwise initialize tp->rcv_nxt, and tp->irs, select an initial
         * tp->iss, and send a segment:
         *     <SEQ=ISS><ACK=RCV_NXT><CTL=SYN,ACK>
         * Also initialize tp->snd_nxt to tp->iss+1 and tp->snd_una to tp->iss.
         * Fill in remote peer address fields if not previously specified.
         * Enter SYN_RECEIVED state, and process any other fields of this
         * segment in this state.
         */
        case TCPS_LISTEN: {

#ifdef REMOVE_ME
                if (tiflags & TH_RST)
                        goto drop;
                if (tiflags & TH_ACK)
                        goto dropwithreset;
                if ((tiflags & TH_SYN) == 0)
                        goto drop;
#endif
                /*
                 * RFC1122 4.2.3.10, p. 104: discard bcast/mcast SYN
                 * in_broadcast() should never return true on a received
                 * packet with M_BCAST not set.
                 */
                if (m->m_flags & (M_BCAST|M_MCAST) ||
                    IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
                        goto drop;

                am = m_get(M_DONTWAIT, MT_SONAME, CL_SIZE_128, TRUE);
                if (am == NULL)
                        goto drop;

                am->m_len = sizeof (struct sockaddr_in);
                sin = mtod(am, struct sockaddr_in *);
                sin->sin_family = AF_INET;
                sin->sin_len = sizeof(*sin);
                sin->sin_addr = ti->ti_src;
                sin->sin_port = ti->ti_sport;
                memset(sin->sin_zero, 0, sizeof(sin->sin_zero));
                laddr = inp->inp_laddr;

                if (inp->inp_laddr.s_addr == INADDR_ANY)
                        inp->inp_laddr = ti->ti_dst;

                if (in_pcbconnect(inp, am)) {

                        inp->inp_laddr = laddr;
                        m_free(am);
                        goto drop;

                }

                m_free(am);

                tp->t_template = tcp_template(tp);
                if (tp->t_template == NULL) {

                        tp = tcp_drop(tp, ENOBUFS);
                        dropsocket = 0;         /* socket is already gone */
                        goto drop;

                }

                if (optp)
                        tcp_dooptions(tp, optp, optlen, ti,
                                &ts_present, &ts_val, &ts_ecr);

                if (iss)
                        tp->iss = iss;
                else
                        tp->iss = tcp_iss;

                tcp_iss += TCP_ISSINCR / 4 +
                                ((0x0000ffff & ( *tcpRandFunc) () >> 16));

                tp->irs = ti->ti_seq;
                tcp_sendseqinit(tp);
                tcp_rcvseqinit(tp);
                tp->t_flags |= TF_ACKNOW;
                tp->t_state = TCPS_SYN_RECEIVED;
                tp->t_timer[TCPT_KEEP] = tcp_keepinit;
                dropsocket = 0;         /* committed to socket */
                tcpstat.tcps_accepts++;

                goto trimthenstep6;

                }

        /*
         * If the state is SYN_SENT:
         *      if seg contains an ACK, but not for our SYN, drop the input.
         *      if seg contains a RST, then drop the connection.
         *      if seg does not contain SYN, then drop it.
         * Otherwise this is an acceptable SYN segment
         *      initialize tp->rcv_nxt and tp->irs
         *      if seg contains ack then advance tp->snd_una
         *      if SYN has been acked change to ESTABLISHED else SYN_RCVD state
         *      arrange for segment to be acked (eventually)
         *      continue processing rest of data/controls, beginning with URG
         */
        case TCPS_SYN_SENT:

                if ((tiflags & TH_ACK) &&
                    (SEQ_LEQ(ti->ti_ack, tp->iss) ||
                     SEQ_GT(ti->ti_ack, tp->snd_max)))
                                goto dropwithreset;

                if (tiflags & TH_RST) {

                        if (tiflags & TH_ACK)
                                tp = tcp_drop(tp, ECONNREFUSED);

                        goto drop;

                }

                if ((tiflags & TH_SYN) == 0)
                        goto drop;

                if (tiflags & TH_ACK) {

                        tp->snd_una = ti->ti_ack;

                        if (SEQ_LT(tp->snd_nxt, tp->snd_una))
                                tp->snd_nxt = tp->snd_una;

                        tp->t_timer[TCPT_REXMT] = 0;

                }

                tp->irs = ti->ti_seq;
                tcp_rcvseqinit(tp);
                tp->t_flags |= TF_ACKNOW;

                if (tiflags & TH_ACK) {

                        tcpstat.tcps_connects++;
                        soisconnected(so);
                        tp->t_state = TCPS_ESTABLISHED;

                        /* Do window scaling on this connection? */
                        if ((tp->t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
                                (TF_RCVD_SCALE|TF_REQ_SCALE)) {

                                tp->snd_scale = tp->requested_s_scale;
                                tp->rcv_scale = tp->request_r_scale;

                        }

                        tcp_reass(tp, NULL, NULL);

                        if (tp->t_rtt)
                                tcp_xmit_timer(tp, tp->t_rtt);

                }

                else {

                        tp->t_state = TCPS_SYN_RECEIVED;

                }

trimthenstep6:

                /*
                 * Advance ti->ti_seq to correspond to first data byte.
                 * If data, trim to stay within window,
                 * dropping FIN if necessary.
                 */
                ti->ti_seq++;
                if (ti->ti_len > tp->rcv_wnd) {

                        todrop = ti->ti_len - tp->rcv_wnd;
                        m_adj(m, -todrop);
                        ti->ti_len = tp->rcv_wnd;
                        tiflags &= ~TH_FIN;
                        tcpstat.tcps_rcvpackafterwin++;
                        tcpstat.tcps_rcvbyteafterwin += todrop;

                }

                tp->snd_wl1 = ti->ti_seq - 1;
                tp->rcv_up = ti->ti_seq;

                goto step6;
        }

        /*
         * States other than LISTEN or SYN_SENT.
         * First check timestamp, if present.
         * Then check the connection count, if present.
         * Then check that at least some bytes of segment are within
         * receive window.  If segment begins before rcv_nxt,
         * drop leading data (and SYN); if nothing left, just ack.
         *
         * RFC 1323 PAWS: If we have a timestamp reply on this segment
         * and it's less than ts_recent, drop it.
         */
        if (ts_present && (tiflags & TH_RST) == 0 &&
            tp->ts_recent && TSTMP_LT(ts_val, tp->ts_recent)) {

                /* Check to see if ts_recent is over 24 days old.  */
                if ((int)(tcp_now - tp->ts_recent_age) > TCP_PAWS_IDLE) {

                        /*
                         * Invalidate ts_recent.  If this segment updates
                         * ts_recent, the age will be reset later and ts_recent
                         * will get a valid value.  If it does not, setting
                         * ts_recent to zero will at least satisfy the
                         * requirement that zero be placed in the timestamp
                         * echo reply when ts_recent isn't valid.  The
                         * age isn't reset until we get a valid ts_recent
                         * because we don't want out-of-order segments to be
                         * dropped when ts_recent is old.
                         */
                        tp->ts_recent = 0;

                }

                else {

                        tcpstat.tcps_rcvduppack++;
                        tcpstat.tcps_rcvdupbyte += ti->ti_len;
                        tcpstat.tcps_pawsdrop++;
                        goto dropafterack;

                }
        }

        todrop = tp->rcv_nxt - ti->ti_seq;
        if (todrop > 0) {

                if (tiflags & TH_SYN) {

                        tiflags &= ~TH_SYN;
                        ti->ti_seq++;

                        if (ti->ti_urp > 1)
                                ti->ti_urp--;
                        else
                                tiflags &= ~TH_URG;
                        todrop--;

                }

                /*
                 * Following if statement from Stevens, vol. 2, p. 960.
                 */
                if (todrop > ti->ti_len ||
                    (todrop == ti->ti_len &&
                    (tiflags & TH_FIN) == 0)) {

                        /*
                         * Any valid FIN must be to the left of the window.
                         * At this point the FIN must be a duplicate or out
                         * of sequence; drop it.
                         */
                        tiflags &= ~TH_FIN;

                        /*
                         * Send an ACK to resynchronize and drop any data.
                         * But keep on processing for RST or ACK.
                         */
                        tp->t_flags |= TF_ACKNOW;
                        todrop = ti->ti_len;

                }

                tcpstat.tcps_rcvduppack++;
                tcpstat.tcps_rcvdupbyte += todrop;

                m_adj(m, todrop);
                ti->ti_seq += todrop;
                ti->ti_len -= todrop;

                if (ti->ti_urp > todrop) {

                        ti->ti_urp -= todrop;

                }

                else {

                        tiflags &= ~TH_URG;
                        ti->ti_urp = 0;

                }
        }

        /*
         * If new data are received on a connection after the
         * user processes are gone, then RST the other end.
         */
        if ((so->so_state & SS_NOFDREF) &&
            tp->t_state > TCPS_CLOSE_WAIT && ti->ti_len) {

                tp = tcp_close(tp);
                tcpstat.tcps_rcvafterclose++;
                goto dropwithreset;

        }

        /*
         * If segment ends after window, drop trailing data
         * (and PUSH and FIN); if nothing left, just ACK.
         */
        todrop = (ti->ti_seq+ti->ti_len) - (tp->rcv_nxt+tp->rcv_wnd);

        if (todrop > 0) {

                tcpstat.tcps_rcvpackafterwin++;

                if (todrop >= ti->ti_len) {

                        tcpstat.tcps_rcvbyteafterwin += ti->ti_len;

                        /*
                         * If a new connection request is received
                         * while in TIME_WAIT, drop the old connection
                         * and start over if the sequence numbers
                         * are above the previous ones.
                         */
                        if (tiflags & TH_SYN &&
                            tp->t_state == TCPS_TIME_WAIT &&
                            SEQ_GT(ti->ti_seq, tp->rcv_nxt)) {

                                iss = tp->rcv_nxt + TCP_ISSINCR;
                                tp = tcp_close(tp);
                                goto findpcb;

                        }

                        /*
                         * If window is closed can only take segments at
                         * window edge, and have to drop data and PUSH from
                         * incoming segments.  Continue processing, but
                         * remember to ack.  Otherwise, drop segment
                         * and ack.
                         */
                        if (tp->rcv_wnd == 0 && ti->ti_seq == tp->rcv_nxt) {

                                tp->t_flags |= TF_ACKNOW;
                                tcpstat.tcps_rcvwinprobe++;

                        }

                        else {

                                goto dropafterack;

                        }

                }

                else {

                        tcpstat.tcps_rcvbyteafterwin += todrop;

                }

                m_adj(m, -todrop);
                ti->ti_len -= todrop;
                tiflags &= ~(TH_PUSH|TH_FIN);
        }

        /*
         * If last ACK falls within this segment's sequence numbers,
         * record its timestamp.
         * NOTE that the test is modified according to the latest
         * proposal of the tcplw@cray.com list (Braden 1993/04/26).
         */
        if (ts_present && TSTMP_GEQ(ts_val, tp->ts_recent) &&
            SEQ_LEQ(ti->ti_seq, tp->last_ack_sent)) {

                tp->ts_recent_age = tcp_now;
                tp->ts_recent = ts_val;

        }

        /*
         * If the RST bit is set examine the state:
         *    SYN_RECEIVED STATE:
         *      If passive open, return to LISTEN state.
         *      If active open, inform user that connection was refused.
         *    ESTABLISHED, FIN_WAIT_1, FIN_WAIT2, CLOSE_WAIT STATES:
         *      Inform user that connection was reset, and close tcb.
         *    CLOSING, LAST_ACK, TIME_WAIT STATES
         *      Close the tcb.
         */
        if (tiflags&TH_RST) {

                switch (tp->t_state) {

                case TCPS_SYN_RECEIVED:
                        so->so_error = ECONNREFUSED;
                        goto close;

                case TCPS_ESTABLISHED:
                case TCPS_FIN_WAIT_1:
                case TCPS_FIN_WAIT_2:
                case TCPS_CLOSE_WAIT:
                        so->so_error = ECONNRESET;
                close:
                        tp->t_state = TCPS_CLOSED;
                        tcpstat.tcps_drops++;
                        tp = tcp_close(tp);
                        goto drop;

                case TCPS_CLOSING:
                case TCPS_LAST_ACK:
                case TCPS_TIME_WAIT:
                        tp = tcp_close(tp);
                        goto drop;
                }

        }

        /*
         * If a SYN is in the window, then this is an
         * error and we send an RST and drop the connection.
         */
        if (tiflags & TH_SYN) {

                tp = tcp_drop(tp, ECONNRESET);
                goto dropwithreset;

        }

        /*
         * If the ACK bit is off:  if in SYN-RECEIVED state or SENDSYN
         * flag is on (half-synchronized state), then queue data for
         * later processing; else drop segment and return.
         */
        if ((tiflags & TH_ACK) == 0)
                goto drop;

        /*
         * Ack processing.
         */
        switch (tp->t_state) {

        /*
         * In SYN_RECEIVED state if the ack ACKs our SYN then enter
         * ESTABLISHED state and continue processing, otherwise
         * send an RST.
         */
        case TCPS_SYN_RECEIVED:
                if (SEQ_GT(tp->snd_una, ti->ti_ack) ||
                    SEQ_GT(ti->ti_ack, tp->snd_max))
                        goto dropwithreset;

                tcpstat.tcps_connects++;
                soisconnected(so);
                tp->t_state = TCPS_ESTABLISHED;

                /* Do window scaling? */
                if ((tp->t_flags & (TF_RCVD_SCALE|TF_REQ_SCALE)) ==
                        (TF_RCVD_SCALE|TF_REQ_SCALE)) {

                        tp->snd_scale = tp->requested_s_scale;
                        tp->rcv_scale = tp->request_r_scale;

                }

                tcp_reass(tp, NULL, NULL);
                tp->snd_wl1 = ti->ti_seq - 1;

                /* fall into ... */

        /*
         * In ESTABLISHED state: drop duplicate ACKs; ACK out of range
         * ACKs.  If the ack is in the range
         *      tp->snd_una < ti->ti_ack <= tp->snd_max
         * then advance tp->snd_una to ti->ti_ack and drop
         * data from the retransmission queue.  If this ACK reflects
         * more up to date window information we update our window information.
         */
        case TCPS_ESTABLISHED:
        case TCPS_FIN_WAIT_1:
        case TCPS_FIN_WAIT_2:
        case TCPS_CLOSE_WAIT:
        case TCPS_CLOSING:
        case TCPS_LAST_ACK:
        case TCPS_TIME_WAIT:

                if (SEQ_LEQ(ti->ti_ack, tp->snd_una)) {

                        if (ti->ti_len == 0 && tiwin == tp->snd_wnd) {

                                tcpstat.tcps_rcvdupack++;

                                /*
                                 * If we have outstanding data (other than
                                 * a window probe), this is a completely
                                 * duplicate ack (ie, window info didn't
                                 * change), the ack is the biggest we've
                                 * seen and we've seen exactly our rexmt
                                 * threshhold of them, assume a packet
                                 * has been dropped and retransmit it.
                                 * Kludge snd_nxt & the congestion
                                 * window so we send only this one
                                 * packet.
                                 *
                                 * We know we're losing at the current
                                 * window size so do congestion avoidance
                                 * (set ssthresh to half the current window
                                 * and pull our congestion window back to
                                 * the new ssthresh).
                                 *
                                 * Dup acks mean that packets have left the
                                 * network (they're now cached at the receiver)
                                 * so bump cwnd by the amount in the receiver
                                 * to keep a constant cwnd packets in the
                                 * network.
                                 */
                                if (tp->t_timer[TCPT_REXMT] == 0 ||
                                    ti->ti_ack != tp->snd_una) {

                                        tp->t_dupacks = 0;

                                }

                                else if (++tp->t_dupacks == tcprexmtthresh) {

                                        onxt = tp->snd_nxt;

                                        win =
                                          min(tp->snd_wnd, tp->snd_cwnd) / 2 /
                                                tp->t_maxseg;

                                        if (win < 2)
                                                win = 2;

                                        tp->snd_ssthresh = win * tp->t_maxseg;
                                        tp->t_timer[TCPT_REXMT] = 0;
                                        tp->t_rtt = 0;
                                        tp->snd_nxt = ti->ti_ack;
                                        tp->snd_cwnd = tp->t_maxseg;

                                        tcp_output(tp);

                                        tp->snd_cwnd = tp->snd_ssthresh +
                                               tp->t_maxseg * tp->t_dupacks;

                                        if (SEQ_GT(onxt, tp->snd_nxt))
                                                tp->snd_nxt = onxt;

                                        goto drop;

                                }

                                else if (tp->t_dupacks > tcprexmtthresh) {

                                        tp->snd_cwnd += tp->t_maxseg;

                                        tcp_output(tp);

                                        goto drop;

                                }

                        }

                        else {

                                tp->t_dupacks = 0;

                        }

                        break;
                }

                /*
                 * If the congestion window was inflated to account
                 * for the other side's cached packets, retract it.
                 */
                if (tp->t_dupacks > tcprexmtthresh &&
                    tp->snd_cwnd > tp->snd_ssthresh)
                        tp->snd_cwnd = tp->snd_ssthresh;

                tp->t_dupacks = 0;

                if (SEQ_GT(ti->ti_ack, tp->snd_max)) {

                        tcpstat.tcps_rcvacktoomuch++;
                        goto dropafterack;

                }

                acked = ti->ti_ack - tp->snd_una;
                tcpstat.tcps_rcvackpack++;
                tcpstat.tcps_rcvackbyte += acked;

                /*
                 * If we have a timestamp reply, update smoothed
                 * round trip time.  If no timestamp is present but
                 * transmit timer is running and timed sequence
                 * number was acked, update smoothed round trip time.
                 * Since we now have an rtt measurement, cancel the
                 * timer backoff (cf., Phil Karn's retransmit alg.).
                 * Recompute the initial retransmit timer.
                 */
                if (ts_present)
                        tcp_xmit_timer(tp, tcp_now - ts_ecr + 1);
                else if (tp->t_rtt && SEQ_GT(ti->ti_ack, tp->t_rtseq))
                        tcp_xmit_timer(tp,tp->t_rtt);

                /*
                 * If all outstanding data is acked, stop retransmit
                 * timer and remember to restart (more output or persist).
                 * If there is more data to be acked, restart retransmit
                 * timer, using current (possibly backed-off) value.
                 */
                if (ti->ti_ack == tp->snd_max) {

                        tp->t_timer[TCPT_REXMT] = 0;
                        needoutput = 1;

                }

                else if (tp->t_timer[TCPT_PERSIST] == 0) {

                        tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

                }

                /*
                 * When new data is acked, open the congestion window.
                 * If the window gives us less than ssthresh packets
                 * in flight, open exponentially (maxseg per packet).
                 * Otherwise open linearly: maxseg per window
                 * (maxseg^2 / cwnd per packet).
                 */
                {
                cw = tp->snd_cwnd;
                incr = tp->t_maxseg;

                if (cw > tp->snd_ssthresh)
                        incr = incr * incr / cw;

                tp->snd_cwnd = min(cw + incr, TCP_MAXWIN<<tp->snd_scale);
                }

                if (acked > so->so_snd.sb_cc) {

                        tp->snd_wnd -= so->so_snd.sb_cc;
                        sbdrop(&so->so_snd, (int)so->so_snd.sb_cc);
                        ourfinisacked = 1;

                }

                else {

                        sbdrop(&so->so_snd, acked);
                        tp->snd_wnd -= acked;
                        ourfinisacked = 0;

                }

                if ((so->so_snd.sb_want > 0) ||
                    (so->so_snd.sb_sel)) {

                        if (sbspace(&so->so_snd) >= so->so_snd.sb_lowat)
                                sowwakeup(so);

                }

                tp->snd_una = ti->ti_ack;
                if (SEQ_LT(tp->snd_nxt, tp->snd_una))
                        tp->snd_nxt = tp->snd_una;

                switch (tp->t_state) {

                /*
                 * In FIN_WAIT_1 STATE in addition to the processing
                 * for the ESTABLISHED state if our FIN is now acknowledged
                 * then enter FIN_WAIT_2.
                 */
                case TCPS_FIN_WAIT_1:

                        if (ourfinisacked) {

                                /*
                                 * If we can't receive any more
                                 * data, then closing user can proceed.
                                 * Starting the timer is contrary to the
                                 * specification, but if we don't get a FIN
                                 * we'll hang forever.
                                 */
                                if (so->so_state & SS_CANTRCVMORE) {

                                        soisdisconnected(so);
                                        tp->t_timer[TCPT_2MSL] = tcp_maxidle;

                                }

                                tp->t_state = TCPS_FIN_WAIT_2;

                        }

                        break;

                /*
                 * In CLOSING STATE in addition to the processing for
                 * the ESTABLISHED state if the ACK acknowledges our FIN
                 * then enter the TIME-WAIT state, otherwise ignore
                 * the segment.
                 */
                case TCPS_CLOSING:

                        if (ourfinisacked) {

                                tp->t_state = TCPS_TIME_WAIT;
                                tcp_canceltimers(tp);
                                tp->t_timer[TCPT_2MSL] = 2 * tcp_msl;
                                soisdisconnected(so);

                        }

                        break;

                /*
                 * In LAST_ACK, we may still be waiting for data to drain
                 * and/or to be acked, as well as for the ack of our FIN.
                 * If our FIN is now acknowledged, delete the TCB,
                 * enter the closed state and return.
                 */
                case TCPS_LAST_ACK:

                        if (ourfinisacked) {

                                tp = tcp_close(tp);
                                goto drop;

                        }

                        break;

                /*
                 * In TIME_WAIT state the only thing that should arrive
                 * is a retransmission of the remote FIN.  Acknowledge
                 * it and restart the finack timer.
                 */
                case TCPS_TIME_WAIT:

                        tp->t_timer[TCPT_2MSL] = 2 * tcp_msl;
                        goto dropafterack;

                }

        }

step6:
        /*
         * Update window information.
         * Don't look at window if no ACK: TAC's send garbage on first SYN.
         */
        if ((tiflags & TH_ACK) &&
            (SEQ_LT(tp->snd_wl1, ti->ti_seq) ||
               (tp->snd_wl1 == ti->ti_seq &&
                  (SEQ_LT(tp->snd_wl2, ti->ti_ack) ||
                  (tp->snd_wl2 == ti->ti_ack && tiwin > tp->snd_wnd))
                )
              )) {

                /* keep track of pure window updates */
                if (ti->ti_len == 0 &&
                    tp->snd_wl2 == ti->ti_ack && tiwin > tp->snd_wnd)
                        tcpstat.tcps_rcvwinupd++;

                tp->snd_wnd = tiwin;
                tp->snd_wl1 = ti->ti_seq;
                tp->snd_wl2 = ti->ti_ack;

                if (tp->snd_wnd > tp->max_sndwnd)
                        tp->max_sndwnd = tp->snd_wnd;

                needoutput = 1;

        }

        /*
         * Process segments with URG.
         */
        if ((tiflags & TH_URG) && ti->ti_urp &&
            TCPS_HAVERCVDFIN(tp->t_state) == 0) {

                /*
                 * This is a kludge, but if we receive and accept
                 * random urgent pointers, we'll crash in
                 * soreceive.  It's hard to imagine someone
                 * actually wanting to send this much urgent data.
                 */
                if (ti->ti_urp + so->so_rcv.sb_cc > sb_max) {

                        ti->ti_urp = 0;                 /* XXX */
                        tiflags &= ~TH_URG;             /* XXX */
                        goto dodata;                    /* XXX */

                }

                /*
                 * If this segment advances the known urgent pointer,
                 * then mark the data stream.  This should not happen
                 * in CLOSE_WAIT, CLOSING, LAST_ACK or TIME_WAIT STATES since
                 * a FIN has been received from the remote side.
                 * In these states we ignore the URG.
                 *
                 * According to RFC961 (Assigned Protocols),
                 * the urgent pointer points to the last octet
                 * of urgent data.  We continue, however,
                 * to consider it to indicate the first octet
                 * of data past the urgent section as the original
                 * spec states (in one of two places).
                 */
                if (SEQ_GT(ti->ti_seq+ti->ti_urp, tp->rcv_up)) {

                        tp->rcv_up = ti->ti_seq + ti->ti_urp;
                        so->so_oobmark = so->so_rcv.sb_cc +
                            (tp->rcv_up - tp->rcv_nxt) - 1;

                        if (so->so_oobmark == 0)
                                so->so_state |= SS_RCVATMARK;

                        sohasoutofband(so);
                        tp->t_oobflags &= ~(TCPOOB_HAVEDATA | TCPOOB_HADDATA);

                }

                /*
                 * Remove out of band data so doesn't get presented to user.
                 * This can happen independent of advancing the URG pointer,
                 * but if two URG's are pending at once, some out-of-band
                 * data may creep in... ick.
                 */
                if (ti->ti_urp <= (u_long)ti->ti_len &&
                    (so->so_options & SO_OOBINLINE) == 0)
                        tcp_pulloutofband(so, ti, m);
        }

        else {

                /*
                 * If no out of band data is expected,
                 * pull receive urgent pointer along
                 * with the receive window.
                 */
                if (SEQ_GT(tp->rcv_nxt, tp->rcv_up))
                        tp->rcv_up = tp->rcv_nxt;

        }

dodata:                                                 /* XXX */

        /*
         * Process the segment text, merging it into the TCP sequencing queue,
         * and arranging for acknowledgment of receipt if necessary.
         * This process logically involves adjusting tp->rcv_wnd as data
         * is presented to the user (this happens in tcp_usrreq.c,
         * case PRU_RCVD).  If a FIN has already been received on this
         * connection then we just ignore the text.
         */
        if ((ti->ti_len || (tiflags&TH_FIN)) &&
            TCPS_HAVERCVDFIN(tp->t_state) == 0) {

                TCP_REASS(tp, ti, m, so, tiflags);

        }

        else {

                m_freem(m);
                tiflags &= ~TH_FIN;

        }

        /*
         * If FIN is received ACK the FIN and let the user know
         * that the connection is closing.
         */
        if (tiflags & TH_FIN) {

                if (TCPS_HAVERCVDFIN(tp->t_state) == 0) {

                        socantrcvmore(so);
                        tp->t_flags |= TF_ACKNOW;
                        tp->rcv_nxt++;

                }

                switch (tp->t_state) {

                /*
                 * In SYN_RECEIVED and ESTABLISHED STATES
                 * enter the CLOSE_WAIT state.
                 */
                case TCPS_SYN_RECEIVED:
                        break;

                case TCPS_ESTABLISHED:
                        tp->t_state = TCPS_CLOSE_WAIT;
                        break;

                /*
                 * If still in FIN_WAIT_1 STATE FIN has not been acked so
                 * enter the CLOSING state.
                 */
                case TCPS_FIN_WAIT_1:
                        tp->t_state = TCPS_CLOSING;
                        break;

                /*
                 * In FIN_WAIT_2 state enter the TIME_WAIT state,
                 * starting the time-wait timer, turning off the other
                 * standard timers.
                 */
                case TCPS_FIN_WAIT_2:
                        tp->t_state = TCPS_TIME_WAIT;
                        tcp_canceltimers(tp);
                        tp->t_timer[TCPT_2MSL] = 2 * tcp_msl;
                        soisdisconnected(so);
                        break;

                /*
                 * In TIME_WAIT state restart the 2 MSL time_wait timer.
                 */
                case TCPS_TIME_WAIT:
                        tp->t_timer[TCPT_2MSL] = 2 * tcp_msl;
                        break;

                }

        }

#ifdef TCPDEBUG
        if (so->so_options & SO_DEBUG)
                tcp_trace(TA_INPUT, ostate, tp, &tcp_saveti, 0);
#endif

        /*
         * Return any desired output.
         */
        if (needoutput || (tp->t_flags & TF_ACKNOW))
                tcp_output(tp);

        return;

dropafterack:

        /*
         * Generate an ACK dropping incoming segment if it occupies
         * sequence space, where the ACK reflects our state.
         */
        if (tiflags & TH_RST)
                goto drop;

        m_freem(m);
        tp->t_flags |= TF_ACKNOW;
        tcp_output(tp);
        return;

dropwithreset:

        /*
         * Generate a RST, dropping incoming segment.
         * Make ACK acceptable to originator of segment.
         * Don't bother to respond if destination was broadcast/multicast.
         */
        if ((tiflags & TH_RST) || m->m_flags & (M_BCAST|M_MCAST) ||
            IN_MULTICAST(ntohl(ti->ti_dst.s_addr)))
                goto drop;

        if (tiflags & TH_ACK) {

                tcp_respond(tp, ti, m, (tcp_seq)0, ti->ti_ack, TH_RST);

        }

        else {

                if (tiflags & TH_SYN)
                        ti->ti_len++;

                tcp_respond(tp, ti, m, ti->ti_seq+ti->ti_len, (tcp_seq)0,
                    TH_RST|TH_ACK);

        }

        tcpOutRsts++;

        /* destroy temporarily created socket */
        if (dropsocket)
                soabort(so);

        return;

drop:
        /*
         * Drop space held by incoming segment and return.
         */
#ifdef TCPDEBUG
        if (tp != NULL || (tp->t_inpcb->inp_socket->so_options & SO_DEBUG))
                tcp_trace(TA_DROP, ostate, tp, &tcp_saveti, 0);
#endif

        m_freem(m);

        /* destroy temporarily created socket */
        if (dropsocket)
                soabort(so);

        return;
}

/******************************************************************************
 * tcp_dooptions - Do options
 *
 * RETURNS: N/A
 *****************************************************************************/

void tcp_dooptions(struct tcpcb *tp, unsigned char *cp, int cnt,
              struct tcpiphdr *ti, int *ts_present,
              unsigned long *ts_val, unsigned long *ts_ecr)
{
        unsigned short mss = 0;
        int opt, optlen;

        for (; cnt > 0; cnt -= optlen, cp += optlen) {

                opt = cp[0];
                if (opt == TCPOPT_EOL)
                        break;

                if (opt == TCPOPT_NOP) {

                        optlen = 1;

                }

                else {

                        optlen = cp[1];
                        if (optlen <= 0)
                                break;

                }

                switch (opt) {

                default:
                        continue;

                case TCPOPT_MAXSEG:
                        if (optlen != TCPOLEN_MAXSEG)
                                continue;

                        if (!(ti->ti_flags & TH_SYN))
                                continue;

                        memcpy(&mss, cp + 2, sizeof(mss));
                        NTOHS(mss);
                        tcp_mss(tp, mss);

                        break;

                case TCPOPT_WINDOW:
                        if (optlen != TCPOLEN_WINDOW)
                                continue;

                        if (!(ti->ti_flags & TH_SYN))
                                continue;

                        tp->t_flags |= TF_RCVD_SCALE;
                        tp->requested_s_scale = min(cp[2], TCP_MAX_WINSHIFT);

                        break;

                case TCPOPT_TIMESTAMP:
                        if (optlen != TCPOLEN_TIMESTAMP)
                                continue;

                        *ts_present = 1;

                        memcpy(ts_val, cp + 2, sizeof(*ts_val));
                        NTOHL(*ts_val);

                        memcpy(ts_ecr, cp + 6, sizeof(*ts_ecr));
                        NTOHL(*ts_ecr);

                        /*
                         * A timestamp received in a SYN makes
                         * it ok to send timestamp requests and replies.
                         */
                        if (ti->ti_flags & TH_SYN) {

                                tp->t_flags |= TF_RCVD_TSTMP;
                                tp->ts_recent = *ts_val;
                                tp->ts_recent_age = tcp_now;

                        }

                        break;
                }
        }

        if ((ti->ti_flags & TH_SYN) && !mss) {

                if (tp->t_inpcb->inp_route.ro_rt != NULL)
                        (tp->t_inpcb->inp_route.ro_rt)->rt_rmx.rmx_mss =
                                tcp_mssdflt;
        }

}

/*******************************************************************************
 * tcp_pulloutofband - Pull out of band byte out of a segment so
 * it doesn't appear in the user's data queue.
 * It is still reflected in the segment length for
 * sequencing purposes.
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_pulloutofband(struct socket *so, struct tcpiphdr *ti, struct mbuf *m)
{
        char *cp;
        struct tcpcb *tp;
        int cnt = ti->ti_urp - 1;

        while (cnt >= 0) {

                if (m->m_len > cnt) {

                        cp = mtod(m, char*) + cnt;
                        tp = sototcpcb(so);

                        tp->t_iobc = *cp;
                        tp->t_oobflags |= TCPOOB_HAVEDATA;
                        memcpy(cp, cp + 1, (unsigned) (m->m_len - cnt - 1));
                        m->m_len--;
                        return;

                }

                cnt -= m->m_len;
                m = m->m_next;
                if (m == 0)
                        break;

        }

        panic("tcp_pulloutofband");
}

/*******************************************************************************
 * tcp_xmit_timer - Collect new round-trip time estimate
 * and update averages and current timeout.
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_xmit_timer(struct tcpcb *tp, short rtt)
{
        short delta;

        tcpstat.tcps_rttupdated++;

        if (tp->t_srtt != 0) {

                /*
                 * srtt is stored as fixed point with 3 bits after the
                 * binary point (i.e., scaled by 8).  The following magic
                 * is equivalent to the smoothing algorithm in rfc793 with
                 * an alpha of .875 (srtt = rtt/8 + srtt*7/8 in fixed
                 * point).  Adjust rtt to origin 0.
                 */
                delta = rtt - 1 - (tp->t_srtt >> TCP_RTT_SHIFT);
                if ((tp->t_srtt += delta) <= 0)
                        tp->t_srtt = 1;

                /*
                 * We accumulate a smoothed rtt variance (actually, a
                 * smoothed mean difference), then set the retransmit
                 * timer to smoothed rtt + 4 times the smoothed variance.
                 * rttvar is stored as fixed point with 2 bits after the
                 * binary point (scaled by 4).  The following is
                 * equivalent to rfc793 smoothing with an alpha of .75
                 * (rttvar = rttvar*3/4 + |delta| / 4).  This replaces
                 * rfc793's wired-in beta.
                 */
                if (delta < 0)
                        delta = -delta;
                delta -= (tp->t_rttvar >> TCP_RTTVAR_SHIFT);
                if ((tp->t_rttvar += delta) <= 0)
                        tp->t_rttvar = 1;

        }

        else {

                /*
                 * No rtt measurement yet - use the unsmoothed rtt.
                 * Set the variance to half the rtt (so our first
                 * retransmit happens at 3*rtt).
                 */
                tp->t_srtt = rtt << TCP_RTT_SHIFT;
                tp->t_rttvar = rtt << (TCP_RTTVAR_SHIFT - 1);

        }

        tp->t_rtt = 0;
        tp->t_rxtshift = 0;

        /*
         * the retransmit should happen at rtt + 4 * rttvar.
         * Because of the way we do the smoothing, srtt and rttvar
         * will each average +1/2 tick of bias.  When we compute
         * the retransmit timer, we want 1/2 tick of rounding and
         * 1 extra tick because of +-1/2 tick uncertainty in the
         * firing of the timer.  The bias will give us exactly the
         * 1.5 tick we need.  But, because the bias is
         * statistical, we have to test that we don't drop below
         * the minimum feasible timer (which is 2 ticks).
         */
        TCPT_RANGESET(tp->t_rxtcur, TCP_REXMTVAL(tp),
            tp->t_rttmin, TCPTV_REXMTMAX);

        /*
         * We received an ack for a packet that wasn't retransmitted;
         * it is probably safe to discard any error indications we've
         * received recently.  This isn't quite right, but close enough
         * for now (a route might have failed after we sent a segment,
         * and the return path might not be symmetrical).
         */
        tp->t_softerror = 0;
}

/*******************************************************************************
 * tcp_mss - Determine a reasonable value for maxseg size.
 * If the route is known, check route for mtu.
 * If none, use an mss that can be handled on the outgoing
 * interface without forcing IP to fragment; if bigger than
 * an mbuf cluster (MCLBYTES), round down to nearest multiple of MCLBYTES
 * to utilize large mbufs.  If no route is found, route has no mtu,
 * or the destination isn't local, use a default, hopefully conservative
 * size (usually 512 or the default IP max size, but no more than the mtu
 * of the interface), as we can't discover anything about intervening
 * gateways or networks.  We also initialize the congestion/slow start
 * window to be a single segment if the destination isn't local.
 * While looking at the routing entry, we also initialize other path-dependent
 * parameters from pre-set or cached values in the routing entry.
 *
 * Also take into account the space needed for options that we
 * send regularly.  Make maxseg shorter by that amount to assure
 * that we can send maxseg amount of data even when the options
 * are present.  Store the upper limit of the length of options plus
 * data in maxopd.
 *
 * NOTE that this routine is only called when we process an incoming
 * segment, for outgoing segments only tcp_mssopt is called.
 *
 * In case of T/TCP, we call this routine during implicit connection
 * setup as well (offer = -1), to initialize maxseg from the cached
 * MSS of our peer.
 *
 * RETURNS: Mss
 ******************************************************************************/

int tcp_mss(struct tcpcb *tp, int offer)
{
        struct route *ro;
        struct rtentry *rt;
        struct ifnet *ifp;
        int rtt, mss;
        u_long bufsize;
        struct inpcb *inp;
        struct socket *so;

        inp = tp->t_inpcb;
        ro = &inp->inp_route;
        rt = ro->ro_rt;

        if (rt == NULL) {

                if (inp->inp_faddr.s_addr != INADDR_ANY) {

                        ro->ro_dst.sa_family = AF_INET;
                        ro->ro_dst.sa_len = sizeof(ro->ro_dst);
                        ((struct sockaddr_in *) &ro->ro_dst)->sin_addr=                                         inp->inp_faddr;
                        TOS_SET(&ro->ro_dst, inp->inp_ip.ip_tos);
                        rtalloc(ro);

                }

                rt = ro->ro_rt;
                if (rt == NULL)
                        return tcp_mssdflt;

        }

        ifp = rt->rt_ifp;
        so = inp->inp_socket;

        /*
         * While we're here, check if there's an initial rtt
         * or rttvar.  Convert from the route-table units
         * to scaled multiples of the slow timeout timer.
         */
        if (tp->t_srtt == 0 && (rtt = rt->rt_rmx.rmx_rtt)) {

                /*
                 * XXX the lock bit for RTT indicates that the value
                 * is also a minimum value; this is subject to time.
                 */
                if (rt->rt_rmx.rmx_locks & RTV_RTT)
                        tp->t_rttmin = rtt / (RTM_RTTUNIT / PR_SLOWHZ);

                tp->t_srtt = rtt / (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
                if (rt->rt_rmx.rmx_rttvar)
                        tp->t_rttvar = rt->rt_rmx.rmx_rttvar /
                            (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTTVAR_SCALE));
                else
                        /* default variation is +- 1 rtt */
                        tp->t_rttvar =
                            tp->t_srtt * TCP_RTTVAR_SCALE / TCP_RTT_SCALE;
                TCPT_RANGESET(tp->t_rxtcur,
                    ((tp->t_srtt >> 2) + tp->t_rttvar) >> 1,
                    tp->t_rttmin, TCPTV_REXMTMAX);

        }

        /*
         * if there's an mtu associated with the route, use it
         */
        if (rt->rt_rmx.rmx_mtu) {

                mss = rt->rt_rmx.rmx_mtu - sizeof(struct tcpiphdr);

        }

        else {

                mss = ifp->if_mtu - sizeof(struct tcpiphdr);
                if (!in_localaddr(inp->inp_faddr))
                        mss = min(mss, tcp_mssdflt);

        }

        if (offer) {

                rt->rt_rmx.rmx_mss = offer;
                mss = min(mss, offer);

        }

        mss = max(mss, 32);

        if (offer)
                tp->t_maxsize = mss;

        if (mss < tp->t_maxseg || offer != 0) {

                if ((bufsize = rt->rt_rmx.rmx_sendpipe) == 0)
                        bufsize = so->so_snd.sb_hiwat;

                if (bufsize < mss)
                        mss = bufsize;

                tp->t_maxseg = mss;

                if ((bufsize = rt->rt_rmx.rmx_recvpipe) == 0)
                        bufsize = so->so_rcv.sb_hiwat;

        }

        tp->snd_cwnd = mss;

        if (rt->rt_rmx.rmx_ssthresh)
                tp->snd_ssthresh = max(2 * mss, rt->rt_rmx.rmx_ssthresh);

        return mss;
}

/*******************************************************************************
 * tcp_fasttimo - Fast timeout routine for processing delayed acks
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_fasttimo(void)
{
        struct inpcb *inp;
        struct tcpcb *tp;
        int s;

        s = splnet();

        for (inp = (struct inpcb *) LIST_HEAD(tcbinfo.listhead);
             inp != NULL;
             inp = (struct inpcb *) LIST_NEXT(&inp->inp_list)) {

                if ( ((tp = (struct tcpcb *) inp->inp_ppcb) != NULL) &&
                     (tp->t_flags & TF_DELACK)) {

                        tp->t_flags &= ~TF_DELACK;
                        tp->t_flags |= TF_ACKNOW;
                        tcpstat.tcps_delack++;
                        tcp_output(tp);

                }

        }

        splx(s);
}

/*******************************************************************************
 * tcp_slowtimeo - Tcp protocol timeout routine called every 500 ms.
 * Updates the timers in all active tcb's and
 * causes finite state machine actions if timers expire.
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_slowtimo(void)
{
        struct inpcb *ip, *ipnxt;
        struct tcpcb *tp;
        int i, s;

        s = splnet();

        tcp_maxidle = tcp_keepcnt * tcp_keepintvl;

        ip = (struct inpcb *) LIST_HEAD(tcbinfo.listhead);
        if (ip == NULL) {

                splx(s);
                return;

        }

        /*
         * Search through tcb's and update active timers.
         */
        for (; ip != NULL; ip = ipnxt) {

                ipnxt = (struct inpcb *) LIST_NEXT(&ip->inp_list);
                tp = intotcpcb(ip);

                if ( (tp == NULL) ||
                     (tp->t_state == TCPS_LISTEN) )
                        continue;

                for (i = 0; i < TCPT_NTIMERS; i++) {

                        if ( (tp->t_timer[i]) &&
                             (--tp->t_timer[i] == 0) ) {

                                tp = tcp_timers(tp, i);
                                if (tp == NULL)
                                        goto tpgone;

                        }

                }

                tp->t_idle++;
                if (tp->t_rtt)
                        tp->t_rtt++;
tpgone:
                ;
        }

        tcp_iss += TCP_ISSINCR / PR_SLOWHZ +
                   ((0x0000ffff) & (( *tcpRandFunc) () >> 16));

#ifdef TCP_COMPAT_42
        if ((int)tcp_iss < 0)
                tcp_iss = TCP_ISSINCR;                  /* XXX */
#endif

        tcp_now++;                                      /* for timestamps */
        splx(s);
}

/*******************************************************************************
 * tcp_canceltimers - Cancel all timers for TCP tp.
 *
 * RETURNS: N/A
 ******************************************************************************/

void tcp_canceltimers(struct tcpcb *tp)
{
        int i;

        for (i = 0; i < TCPT_NTIMERS; i++)
                tp->t_timer[i] = 0;
}

/*******************************************************************************
 * tcp_timers - TCP timer processing.
 *
 * RETURNS: Pointer to tcbcb struct
 ******************************************************************************/

struct tcpcb* tcp_timers(struct tcpcb *tp, int timer)
{
        int rexmt;
        unsigned int win;

        switch (timer) {

        /*
         * 2 MSL timeout in shutdown went off.  If we're closed but
         * still waiting for peer to close and connection has been idle
         * too long, or if 2MSL time is up from TIME_WAIT, delete connection
         * control block.  Otherwise, check again in a bit.
         */
        case TCPT_2MSL:

#ifdef DEBUG_TCP
                printf("TCPT_2MSL timer\n");
#endif

                if ( (tp->t_state != TCPS_TIME_WAIT) &&
                     (tp->t_idle <= tcp_maxidle) )
                        tp->t_timer[TCPT_2MSL] = tcp_keepintvl;
                else
                        tp = tcp_close(tp);
                break;

        /*
         * Retransmission timer went off.  Message has not
         * been acked within retransmit interval.  Back off
         * to a longer retransmit interval and retransmit one segment.
         */
        case TCPT_REXMT:

#ifdef DEBUG_TCP
                printf("TCPT_REXMT timer\n");
#endif

                if (++tp->t_rxtshift > TCP_MAXRXTSHIFT) {

                        tp->t_rxtshift = TCP_MAXRXTSHIFT;
                        tcpstat.tcps_timeoutdrop++;
                        tp = tcp_drop(tp, tp->t_softerror ?
                            tp->t_softerror : ETIMEDOUT);
                        break;

                }

                tcpstat.tcps_rexmttimeo++;
                rexmt = TCP_REXMTVAL(tp) * tcp_backoff[tp->t_rxtshift];
                TCPT_RANGESET(tp->t_rxtcur, rexmt,
                    tp->t_rttmin, TCPTV_REXMTMAX);
                tp->t_timer[TCPT_REXMT] = tp->t_rxtcur;

                /*
                 * If losing, let the lower level know and try for
                 * a better route.  Also, if we backed off this far,
                 * our srtt estimate is probably bogus.  Clobber it
                 * so we'll take the next rtt measurement as our srtt;
                 * move the current srtt into rttvar to keep the current
                 * retransmit times until then.
                 */
                if (tp->t_rxtshift > TCP_MAXRXTSHIFT / 4) {

                        in_losing(tp->t_inpcb);
                        tp->t_rttvar += (tp->t_srtt >> TCP_RTT_SHIFT);
                        tp->t_srtt = 0;
                }

                tp->snd_nxt = tp->snd_una;

                /*
                 * If timing a segment in this window, stop the timer.
                 */
                tp->t_rtt = 0;

                /*
                 * Close the congestion window down to one segment
                 * (we'll open it by one segment for each ack we get).
                 * Since we probably have a window's worth of unacked
                 * data accumulated, this "slow start" keeps us from
                 * dumping all that data as back-to-back packets (which
                 * might overwhelm an intermediate gateway).
                 *
                 * There are two phases to the opening: Initially we
                 * open by one mss on each ack.  This makes the window
                 * size increase exponentially with time.  If the
                 * window is larger than the path can handle, this
                 * exponential growth results in dropped packet(s)
                 * almost immediately.  To get more time between
                 * drops but still "push" the network to take advantage
                 * of improving conditions, we switch from exponential
                 * to linear window opening at some threshhold size.
                 * For a threshhold, we use half the current window
                 * size, truncated to a multiple of the mss.
                 *
                 * (the minimum cwnd that will give us exponential
                 * growth is 2 mss.  We don't allow the threshhold
                 * to go below this.)
                 */
                {
                win = min(tp->snd_wnd, tp->snd_cwnd) / 2 / tp->t_maxseg;
                if (win < 2)
                        win = 2;
                tp->snd_cwnd = tp->t_maxseg;
                tp->snd_ssthresh = win * tp->t_maxseg;
                tp->t_dupacks = 0;
                }

                tcp_output(tp);

                break;

        /*
         * Persistance timer into zero window.
         * Force a byte to be output, if possible.
         */
        case TCPT_PERSIST:

#ifdef DEBUG_TCP
                printf("TCPT_PERSIST timer\n");
#endif

                tcpstat.tcps_persisttimeo++;

                /*
                 * Hack: if the peer is dead/unreachable, we do not
                 * time out if the window is closed.  After a full
                 * backoff, drop the connection if the idle time
                 * (no responses to probes) reaches the maximum
                 * backoff that we would use if retransmitting.
                 */
                if ((tp->t_rxtshift == TCP_MAXRXTSHIFT) &&
                    ((tp->t_idle >= tcp_maxpersistidle) ||
                    (tp->t_idle >= TCP_REXMTVAL(tp) * tcp_totbackoff)) ) {

                        tcpstat.tcps_persistdrop++;
                        tp = tcp_drop(tp, ETIMEDOUT);
                        break;

                }

                tcp_setpersist(tp);
                tp->t_force = 1;
                tcp_output(tp);
                tp->t_force = 0;
                break;

        /*
         * Keep-alive timer went off; send something
         * or drop connection if idle for too long.
         */
        case TCPT_KEEP:

#ifdef DEBUG_TCP
                printf("TCPT_KEEP timer\n");
#endif

                tcpstat.tcps_keeptimeo++;

                if (tp->t_state < TCPS_ESTABLISHED)
                        goto dropit;

                if ( (tp->t_inpcb->inp_socket->so_options & SO_KEEPALIVE) &&
                     (tp->t_state <= TCPS_CLOSE_WAIT) ) {

                        if (tp->t_idle >= (tcp_keepidle + tcp_maxidle) )
                                goto dropit;

                        /*
                         * Send a packet designed to force a response
                         * if the peer is up and reachable:
                         * either an ACK if the connection is still alive,
                         * or an RST if the peer has closed the connection
                         * due to timeout or reboot.
                         * Using sequence number tp->snd_una-1
                         * causes the transmitted zero-length segment
                         * to lie outside the receive window;
                         * by the protocol spec, this requires the
                         * correspondent TCP to respond.
                         */
                        tcpstat.tcps_keepprobe++;
#ifdef TCP_COMPAT_42
                        /*
                         * The keepalive packet must have nonzero length
                         * to get a 4.2 host to respond.
                         */
                        tcp_respond(tp,
                                    mtod(tp->t_template, struct tcpiphdr *),
                                    (struct mbuf *)NULL,
                                    tp->rcv_nxt - 1, tp->snd_una - 1, 0);
#else
                        tcp_respond(tp,
                                    mtod(tp->t_template, struct tcpiphdr *),
                                    (struct mbuf *)NULL,
                                    tp->rcv_nxt, tp->snd_una - 1, 0);
#endif
                        tp->t_timer[TCPT_KEEP] = tcp_keepintvl;

                }

                else {

                        tp->t_timer[TCPT_KEEP] = tcp_keepidle;

                }

                break;

        dropit:
                tcpstat.tcps_keepdrops++;
                tp = tcp_drop(tp, ETIMEDOUT);
                break;
        }

        return (tp);
}

