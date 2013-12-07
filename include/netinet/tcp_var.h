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

/* tcp_var.h - Tcp header */

#ifndef _tcp_var_h
#define _tcp_var_h

#include <vmx.h>
#include <net/socketvar.h>
#include <net/netLib.h>
#include <net/mbuf.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_timer.h>

/* Defines */
#define TF_ACKNOW               0x0001                  /* Ack peer now */
#define TF_DELACK               0x0002                  /* Ack but delay it */
#define TF_NODELAY              0x0004                  /* No delay packets */
#define TF_NOOPT                0x0008                  /* No tcp options */
#define TF_SENTFIN              0x0010                  /* Sent fin */
#define TF_REQ_SCALE            0x0020                  /* Req. scale */
#define TF_RCVD_SCALE           0x0040                  /* Sent scale */
#define TF_REQ_TSTMP            0x0080                  /* Req. time stamp */
#define TF_RCVD_TSTMP           0x0100                  /* Sent time stamp */
#define TF_SACK_PERMIT          0x0200                  /* End side ack */
#define TF_EOB                  0x0400                  /* Last chunk */

#define TCP_RTT_SCALE           8                       /* Scale for rtt */
#define TCP_RTT_SHIFT           3                       /* Shift for rtt */
#define TCP_RTTVAR_SCALE        4                       /* Scale for rtt var */
#define TCP_RTTVAR_SHIFT        2                       /* Shift for rtt var */

#define TCPOOB_HAVEDATA         0x01
#define TCPOOB_HADDATA          0x02

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */

struct tcpcb {
  struct tcpiphdr       *seg_next;                      /* Next segment */
  struct tcpiphdr       *seg_prev;                      /* Previous segment */
  short                 t_state;                        /* State of conn */
  short                 t_timer[TCPT_NTIMERS];          /* Timers */
  short                 t_rxtshift;                     /* texmt exp. backoff */
  short                 t_rxtcur;                       /* Current transmit */
  short                 t_dupacks;                      /* Dup acks */
  unsigned short        t_maxseg;                       /* Max segment size */
  char                  t_force;                        /* For out byte */
  unsigned short        t_flags;                        /* Flags */

  struct mbuf           *t_template;                    /* Skeleton packet */
  struct inpcb          *t_inpcb;                       /* Pointer to inpcb */

  tcp_seq               snd_una;                        /* Send unack */
  tcp_seq               snd_nxt;                        /* Send next */
  tcp_seq               snd_up;                         /* Send urgent ptr */
  tcp_seq               snd_wl1;                        /* Window update seq */
  tcp_seq               snd_wl2;                        /* Window update ack */
  tcp_seq               iss;                            /* Initialize seq */
  unsigned long         snd_wnd;                        /* Send window */

  unsigned long         rcv_wnd;                        /* Receive window */
  tcp_seq               rcv_nxt;                        /* Receive next */
  tcp_seq               rcv_up;                         /* Receive urgent ptr */
  tcp_seq               irs;                            /* Initial recv seq n */

  tcp_seq               rcv_adv;                        /* Advertice window */
  tcp_seq               snd_max;                        /* Highest seq sent */

  unsigned long         snd_cwnd;                       /* Congnestion-ctrl w */
  unsigned long         snd_ssthresh;                   /* Thershold */

  unsigned short        t_idle;                         /* Inactivity time */
  short                 t_rtt;                          /* Round trip time */
  tcp_seq               t_rtseq;                        /* Seq. num timed */
  short                 t_srtt;                         /* Sm. round tip time */
  short                 t_rttvar;                       /* Variance in rtt */
  unsigned short        t_rttmin;                       /* Minumium rtt */
  unsigned long         max_sndwnd;                     /* Max window peer */

  char                  t_oobflags;                     /* Out-of-band flags */
  char                  t_iobc;                         /* Input char */
  short                 t_softerror;                    /* Error */

  unsigned char         snd_scale;                      /* Scale send win */
  unsigned char         rcv_scale;                      /* Scale receive win */
  unsigned char         request_r_scale;                /* Pend win scaling */
  unsigned char         requested_s_scale;
  unsigned long         ts_recent;                      /* Timestamp echo */
  unsigned long         ts_recent_age;
  tcp_seq               last_ack_sent;

  unsigned int          t_maxsize;                      /* Max seg. size */
};

struct tcpstat {
  unsigned long         tcps_connattempt;               /* Conn initiated */
  unsigned long         tcps_accepts;                   /* Conn accepted */
  unsigned long         tcps_connects;                  /* Conn established */
  unsigned long         tcps_drops;                     /* Conn dropped */
  unsigned long         tcps_conndrops;                 /* Init conn drops */
  unsigned long         tcps_closed;                    /* Conn closed */
  unsigned long         tcps_segstimed;                 /* Segs tried get rtt */
  unsigned long         tcps_rttupdated;                /* Times of success */
  unsigned long         tcps_delack;                    /* Delayed acks */
  unsigned long         tcps_timeoutdrop;               /* Conn timeout drop */
  unsigned long         tcps_rexmttimeo;                /* Retransmit timeos */
  unsigned long         tcps_persisttimeo;              /* Persist timeos */
  unsigned long         tcps_keeptimeo;                 /* Keepalive timeos */
  unsigned long         tcps_keepprobe;                 /* Keepalive probes */
  unsigned long         tcps_keepdrops;                 /* Droped keepalive */

  unsigned long         tcps_sndtotal;                  /* Total pkts sent */
  unsigned long         tcps_sndpack;                   /* Data pkts sent */
  unsigned long         tcps_sndbyte;                   /* Data bytes sent */
  unsigned long         tcps_sndrexmitpack;             /* Data pkts retrans */
  unsigned long         tcps_sndrexmitbyte;             /* Data bytes retrans */
  unsigned long         tcps_sndacks;                   /* Ack only pkts sent */
  unsigned long         tcps_sndprobe;                  /* Probes sent */
  unsigned long         tcps_sndurg;                    /* Ugent sent */
  unsigned long         tcps_sndwinup;                  /* Window update sent */
  unsigned long         tcps_sndctrl;                   /* Controls sent */

  unsigned long         tcps_rcvtotal;                  /* Total recvd */
  unsigned long         tcps_rcvpack;                   /* Pkts recvd */
  unsigned long         tcps_rcvbyte;                   /* Bytes recvd */
  unsigned long         tcps_rcvbadsum;                 /* Pkts chksum err */
  unsigned long         tcps_rcvbadoff;                 /* Pkts bad offset */
  unsigned long         tcps_rcvshort;                  /* Pkts to short */
  unsigned long         tcps_rcvduppack;                /* Duplicated pkts */
  unsigned long         tcps_rcvdupbyte;                /* Duplicated bytes */
  unsigned long         tcps_rcvoopack;                 /* Out-of-order pkts */
  unsigned long         tcps_rcvoobyte;                 /* Out-of-order bytes */
  unsigned long         tcps_rcvpackafterwin;           /* Pkts after win */
  unsigned long         tcps_rcvbyteafterwin;           /* Bytes after win */
  unsigned long         tcps_rcvafterclose;             /* Pkts after close */
  unsigned long         tcps_rcvwinprobe;               /* Win probe pkts */
  unsigned long         tcps_rcvdupack;                 /* Duplicate acks */
  unsigned long         tcps_rcvacktoomuch;             /* Acks for unsent */
  unsigned long         tcps_rcvackpack;                /* Recvd ack pkts */
  unsigned long         tcps_rcvackbyte;                /* Recvd ack bytes */
  unsigned long         tcps_rcvwinupd;                 /* Recvd win update */

  unsigned long         tcps_pawsdrop;                  /* Seg drops due paw */
  unsigned long         tcps_predack;                   /* Predict ok for ack */
  unsigned long         tcps_preddat;                   /* Predict ok data */
  unsigned long         tcps_pcbcachemiss;              /* Cache misses */
  unsigned long         tcps_persistdrop;               /* Timeos persist */
  unsigned long         tcps_badsyn;                    /* Premature ack */
};

/* Macros */

/*******************************************************************************
 * intotcpcb - Internet to control block
 *
 * RETURNS: Internet control block
 ******************************************************************************/

#define intotcpcb(ip)                   ((struct tcpcb *) (ip)->inp_ppcb)

/*******************************************************************************
 * sototcpcb - Socket to internet control block
 *
 * RETURNS: Internet control block
 ******************************************************************************/

#define sototcpcb(so)                   (intotcpcb(sotoinpcb(so)))

/*******************************************************************************
 * TCP_REXMTVAL - Get rexmt value
 *
 * RETURNS: Value
 ******************************************************************************/

#define TCP_REXMTVAL(tp)                                                       \
  (((tp)->t_srtt >> TCP_RTT_SHIFT) + (tp)->t_rttvar)

/*******************************************************************************
 * REASS_MBUF - Reassmeble buffer
 *
 * RETURNS: Buffer for tcp
 ******************************************************************************/

#define REASS_MBUF(ti)                  (*(struct mbuf **) &((ti)->ti_t))

/* Functions */
IMPORT STATUS tcpLibInit(TCP_CFG_PARAMS *params);
IMPORT int tcp_usrreq(struct socket *so,
                      int req,
                      struct mbuf *m,
                      struct mbuf *name,
                      struct mbuf *ctrl);
IMPORT int tcp_ctloutput(int op,
                         struct socket *so,
                         int level,
                         int optname,
                         struct mbuf **mp);
IMPORT int tcp_attach(struct socket *so);
IMPORT struct tcpcb* tcp_usrclosed(struct tcpcb *tp);
IMPORT struct tcpcb* tcp_disconnect(struct tcpcb *tp);

IMPORT struct tcpcb* tcp_newtcpcb(struct inpcb *inp);
IMPORT struct mbuf* tcp_template(struct tcpcb *tp);
IMPORT void tcp_respond(struct tcpcb *tp,
                        struct tcpiphdr *ti,
                        struct mbuf *m,
                        tcp_seq ack,
                        tcp_seq seq,
                        int flags);
IMPORT void tcp_notify(struct inpcb *inp, int error);
IMPORT void tcp_ctlinput(int cmd, struct sockaddr *sa, struct ip *ip);
IMPORT void tcp_quench(struct inpcb *tp, int err);
IMPORT void tcp_drain(void);
IMPORT struct tcpcb* tcp_close(struct tcpcb *tp);
IMPORT struct tcpcb* tcp_drop(struct tcpcb *tp, int error);

IMPORT int tcp_output(struct tcpcb *tp);
IMPORT void tcp_setpersist(struct tcpcb *tp);

IMPORT int tcp_reass(struct tcpcb *tp, struct tcpiphdr *ti, struct mbuf *m);
IMPORT void tcp_input(struct mbuf *m, int iphlen);
IMPORT void tcp_dooptions(struct tcpcb *tp, unsigned char *cp, int cnt,
                          struct tcpiphdr *ti, int *ts_present,
                          unsigned long *ts_val, unsigned long *ts_ecr);
IMPORT void tcp_pulloutofband(struct socket *so, struct tcpiphdr *ti,
                              struct mbuf *m);
IMPORT void tcp_xmit_timer(struct tcpcb *tp, short rtt);
IMPORT int tcp_mss(struct tcpcb *tp, int offer);

IMPORT void tcp_fasttimo(void);
IMPORT void tcp_slowtimo(void);
IMPORT void tcp_canceltimers(struct tcpcb *tp);
IMPORT struct tcpcb* tcp_timers(struct tcpcb *tp, int timer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcp_var_h */

