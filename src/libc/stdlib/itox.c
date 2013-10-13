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

/*
 * itox -- converts nbr to hex string of length sz
 *             right adjusted and blank filled, returns str
 *
 *            if sz > 0 terminate with null byte
 *            if sz  =  0 find end of string
 *            if sz < 0 use last byte for data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vmx.h>

/******************************************************************************
 * itox_short - Short integer to hex-string
 *
 * RETURNS: Pointer to ascii string
 */

LOCAL char* itox_short(
    short nbr,
    char *str,
    short sz
    )
{
        int digit, offset ;

        if ( sz > 0 )
                str[--sz] = NULL ;
        else if ( sz < 0 )
                sz = -sz ;
        else
                while ( str[sz] != NULL )
                        ++sz ;
        while ( sz ) {
                digit = nbr & 15 ;
                nbr = ( nbr >> 4 ) & 0xfff ;
                if ( digit < 10 )
                        offset = 48 ;
                else
                        offset = 55 ;
                str[--sz] = digit + offset ;
                if ( nbr == 0 )
                        break ;
        }
        while ( sz )
                str[--sz] = ' ' ;
        return str ;
}

/******************************************************************************
 * itox - Integer to hex-string
 *
 * RETURNS: Pointer to ascii string
 */

char* itox(
    int val,
    char *str
    )
{
    int i;
    short x1, x2;
    char sx1[4], sx2[4];

    x1 = (short)((val & 0xffff0000) >> 16);
    x2 = (short)(val & 0x0000ffff);

    itox_short(x1, sx1, -4);
    itox_short(x2, sx2, -4);

    memcpy(str, sx1, 4);
    memcpy(str + 4, sx2, 4);

    /* Pad with zeros */
    for (i = 0; i < 8; i++)
    {
        if (str[i] == ' ')
        {
            str[i] = '0';
        }
    }

    str[8] = '\0';

    return str;
}

