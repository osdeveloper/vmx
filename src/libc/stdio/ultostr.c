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

/* ultostr.c - */

/*
 * Copyright (C) 2000 Manuel Novoa III
 *
 * Note: buf is a pointer to the END of the buffer passed.
 * Call like this;
 *     char buf[SIZE], *p;
 *     p = __ultostr(buf + sizeof(buf) - 1, ...)
 *
 * For longs of 32 bits, appropriate buffer sizes are:
 *     base =  2      33  = 32 digits + 1 nul
 *     base = 10      11  = 10 digits + 1 nul
 *     base = 16       9  = 8 hex digits + 1 nul
 */

#include <vmx.h>

char *__ultostr(char *buf, unsigned long uval, int base, int uppercase)
{
    int digit;

    if ((base < 2) || (base > 36)) {
                return 0;
    }

    *buf = '\0';

    do {
                digit = uval % base;
                uval /= base;

                /* note: slightly slower but generates less code */
                *--buf = '0' + digit;
                if (digit > 9) {
                        *buf = (uppercase ? 'A' : 'a') + digit - 10;
                }
    } while (uval);

    return buf;
}

