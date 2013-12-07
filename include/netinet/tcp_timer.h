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

/* tcp_timer.h - Tcp timer */

#ifndef _tcp_timer_h
#define _tcp_timer_h

#include <vmx.h>
#include <net/protosw.h>

/* Defines */
#define TCPT_NTIMERS			4		/* Number of timers */

#define TCPT_REXMT			0		/* Retransit */
#define TCPT_PERSIST			1		/* Ret. persistance */
#define TCPT_KEEP			2		/* Keep alive */
#define TCPT_2MSL			3		/* 2 * MSL time */

#define TCP_TTL				30		/* Time to live */

#define TCPTV_MSL			(30 * PR_SLOWHZ)
#define TCPTV_SRTTBASE			0

#define TCPTV_SRTTDFLT			(3 * PR_SLOWHZ)

#define TCPTV_PERSMIN			(5 * PR_SLOWHZ)
#define TCPTV_PERSMAX			(60 * PR_SLOWHZ)

#define TCPTV_KEEP_INIT			(75 * PR_SLOWHZ)
#define TCPTV_KEEP_IDLE			(120 * PR_SLOWHZ)
#define TCPTV_KEEPINTVL			(75 * PR_SLOWHZ)
#define TCPTV_KEEPCNT			8

#define TCPTV_MIN			(1 * PR_SLOWHZ)
#define TCPTV_REXMTMAX			(64 * PR_SLOWHZ)

#define TCP_LINGERTIME			120

#define TCP_MAXRXTSHIFT			12

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Macros */

/*******************************************************************************
 * TCPT_RANGESET - For a time value to a certain range
 *
 * RETURNS: N/A
 ******************************************************************************/

#define TCPT_RANGESET(tv, value, tvmin, tvmax)				       \
{									       \
  (tv) = (value);							       \
  if ( (tv) < (tvmin) )							       \
    (tv) = (tvmin);							       \
  else if ( (tv) > (tvmax) )						       \
    (tv) = (tvmax);							       \
}

/*******************************************************************************
 * TSTMP_LT - Time stamp less than
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define TSTMP_LT(a, b)			((int) ((a) - (b)) < 0)

/*******************************************************************************
 * TSTMP_LT - Time stamp less than
 *
 * RETURNS: TRUE or FALSE
 ******************************************************************************/

#define TSTMP_GEQ(a, b)			((int) ((a) - (b)) >= 0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _tcp_timer_h */

