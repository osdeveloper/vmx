/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2008 Surplus Users Ham Society
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
******************************************************************************/

/* kernQLib.h - Kernel work queue library */

#ifndef _kernQLib_h
#define _kernQLib_h

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Struct */
typedef struct {
  FUNCPTR func;
  int numArgs;
  ARG arg1;
  ARG arg2;
} KERN_JOB;

/* Globals */
extern volatile unsigned       kernQReadIndex;
extern volatile unsigned       kernQWriteIndex;
extern volatile BOOL           kernQEmpty;
extern KERN_JOB                kernJobs[];

/* Functions */
extern STATUS kernQLibInit(void);
extern void kernQAdd0(FUNCPTR func);
extern void kernQAdd1(FUNCPTR func, ARG arg1);
extern void kernQAdd2(FUNCPTR func, ARG arg1, ARG arg2);
extern void kernQDoWork(void);
extern void kernQPanic(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _kernQLib_h */

