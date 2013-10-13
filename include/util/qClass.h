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

/* qClass.h - Queue class */

#ifndef _qClass_h
#define _qClass_h

#include <vmx.h>

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef struct qClass
{
    FUNCPTR createMethod;
    FUNCPTR initMethod;
    FUNCPTR destroyMethod;
    FUNCPTR terminateMethod;
    FUNCPTR putMethod;
    FUNCPTR getMethod;
    FUNCPTR removeMethod;
    FUNCPTR moveMethod;
    FUNCPTR advanceMethod;
    FUNCPTR expiredMethod;
    FUNCPTR keyMethod;
    FUNCPTR offsetMethod;
    FUNCPTR infoMethod;
    FUNCPTR eachMethod;
    struct  qClass *obj;
} Q_CLASS;

typedef Q_CLASS *Q_CLASS_ID;

/******************************************************************************
 * Q_CLASS_VERIFY - Validate class
 *
 * RETURNS: OK or ERROR
 */

#define Q_CLASS_VERIFY(pQClass)                                               \
  (                                                                           \
    ( ((Q_CLASS *)(pQClass))->obj == (Q_CLASS *) pQClass) ?                   \
      OK                                                                      \
    :                                                                         \
      ERROR                                                                   \
  )

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _qClass_h */

