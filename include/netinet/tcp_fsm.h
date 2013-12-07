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

/* tcp_fsm.h - Tcp states */

#ifndef _tcp_fsm_h
#define _tcp_fsm_h

/* Defines */
#define TCPS_CLOSED                     0               /* Closed */
#define TCPS_LISTEN                     1               /* Listening for conn */
#define TCPS_SYN_SENT                   2               /* Sent syn */
#define TCPS_SYN_RECEIVED               3               /* Received syn */
#define TCPS_ESTABLISHED                4               /* Established conn */
#define TCPS_CLOSE_WAIT                 5               /* Waiting for close */
#define TCPS_FIN_WAIT_1                 6               /* Closed, sent fin */
#define TCPS_CLOSING                    7               /* Closing */
#define TCPS_LAST_ACK                   8               /* Final ack */
#define TCPS_FIN_WAIT_2                 9               /* Closed, find acked */
#define TCPS_TIME_WAIT                  10              /* Wait for timer */

#define TCP_NSTATES                     11

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

/*******************************************************************************
 * TCPS_HAVERCVDSYN - If tcp have received sync
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define TCPS_HAVERCVDSYN(s)             ((s) >= TCPS_SYN_RECEIVED)

/*******************************************************************************
 * TCPS_HAVEESTABLISHED - If tcp connection has been established
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define TCPS_HAVEESTABLISHED(s)         ((s) >= TCPS_ESTABLISHED)

/*******************************************************************************
 * TCPS_HAVERCVDFIN - If tcp connection have received fin
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define TCPS_HAVERCVDFIN(s)             ((s) < TCPS_ESTABLISHED &&             \
                                         (s) != TCPS_FIN_WAIT_1 &&             \
                                         (s) != TCPS_FIN_WAIT_2)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcp_fsm_h */

