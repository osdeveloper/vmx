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

/* tcp_seq.h - Tcp sequence header */

#ifndef _tcp_seq_h
#define _tcp_seq_h

#include <vmx.h>

/* Defines */
#define TCP_ISSINCR			(250 * 1024)

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

/*******************************************************************************
 * SEQ_LT - Sequence less than
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define SEQ_LT(a, b)			((int) ((a) - (b)) < 0)

/*******************************************************************************
 * SEQ_LEQ - Sequence less than or equal
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define SEQ_LEQ(a, b)			((int) ((a) - (b)) <= 0)

/*******************************************************************************
 * SEQ_LEQ - Sequence greater than
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define SEQ_GT(a, b)			((int) ((a) - (b)) > 0)

/*******************************************************************************
 * SEQ_GEQ - Sequence greater than or equal
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define SEQ_GEQ(a, b)			((int) ((a) - (b)) >= 0)

/*******************************************************************************
 * tcp_rcvseqinit - Initialize receive sequence
 *
 * RETURNS: N/A
 ******************************************************************************/

#define tcp_rcvseqinit(tp)						       \
  (tp)->rcv_adv = (tp)->rcv_nxt = (tp)->irs + 1

/*******************************************************************************
 * tcp_sendseqinit - Initialize send sequence
 *
 * RETURNS: N/A
 ******************************************************************************/

#define tcp_sendseqinit(tp)						       \
  (tp)->snd_una = (tp)->snd_nxt = (tp)->snd_max = (tp)->snd_up = (tp)->iss

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcp_seq_h */

