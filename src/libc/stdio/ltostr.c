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

/* ltostr.c - */

/*
 * Copyright (C) 2000 Manuel Novoa III
 *
 * Note: buf is a pointer to the END of the buffer passed.
 * Call like this:
 *     char buf[SIZE], *p;
 *     p = __ltostr(buf + sizeof(buf) - 1, ...)
 *
 * For longs of 32 bits, appropriate buffer sizes are:
 *     base =  2      34  = 1 (possible -) sign + 32 digits + 1 nul
 *     base = 10      12  = 1 (possible -) sign + 10 digits + 1 nul
 *     base = 16      10  = 1 (possible -) sign + 8 hex digits + 1 nul
 */

#include <vmx.h>

extern char *__ultostr(char *buf, unsigned long uval, int base, int uppercase);

char *__ltostr(char *buf, long val, int base, int uppercase)
{
        unsigned long uval;
        char *pos;
    int negative;

        negative = 0;
    if (val < 0) {
                negative = 1;
                uval = ((unsigned long)(-(1+val))) + 1;
    } else {
                uval = val;
        }


    pos = __ultostr(buf, uval, base, uppercase);

    if (pos && negative) {
                *--pos = '-';
    }

    return pos;
}
