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

/* qPriBmpLib.h - Priority bitmapped queue */

#ifndef _qPriBmp_h
#define _qPriBmp_h

#include <vmx.h>
#include <util/qLib.h>
#include <util/dllLib.h>
#include <util/qPrioLib.h>

#ifndef _ASMLANGUAGE

typedef union                   /* Explicitly show that */
{
    Q_HEAD qNode;               /* Q_PRI_BMP_HEAD overlays */
    struct                      /* Q_HEAD. */
    {
        Q_PRIO_NODE *pFirst;
        unsigned *bmp;
        DL_LIST *head;
        Q_CLASS *pQClass;
    } qPriBmp;
} Q_PRI_BMP_HEAD;

extern Q_CLASS_ID qPriBmpClassId;

#endif  /* _ASMLANGUAGE */

#endif  /* _qPriBmp_h */

