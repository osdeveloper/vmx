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

/* logLib.h - Logging library header */

#ifndef _logLib_h
#define _logLib_h

/* Defines */
#define MAX_LOGARGS             6       /* Max arguments to log job */
#define MAX_LOGFDS              5       /* Max number of log file(s) */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
typedef struct
{
    int   id;                   /* Id of sender task */
    char *fmt;                  /* Pointer to format string */
    ARG   arg[MAX_LOGARGS];     /* Arguments */
} LOG_MSG;

/* Functions */

/******************************************************************************
 * logLibInit - Inititalize logging library
 *
 * RETURNS: OK or ERROR
 */

STATUS logLibInit(
    int fd,
    int maxMsgs
    );

/******************************************************************************
 * logMsg - Send job to log task
 *
 * RETURNS: Size of message or EOF
 */

int logMsg(
    char *fmt,
    ARG arg0,
    ARG arg1,
    ARG arg2,
    ARG arg3,
    ARG arg4,
    ARG arg5
    );

/******************************************************************************
 * logFdAdd - Add logging file descriptor
 *
 * RETURNS: OK or ERROR
 */

STATUS logFdAdd(
    int fd
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _logLib_h */

