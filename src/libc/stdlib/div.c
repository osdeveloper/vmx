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

/* div.c - Division functions */

/* Includes */
#include <stdlib.h>
#include <vmx.h>

/* Functions */

/******************************************************************************
 * div - Calculate quotient and reminder
 *
 * RETURNS: Result in a div_t structure
 */

div_t div(
    int numer,
    int denom
    )
{
    static div_t result;

    div_r(numer, denom, &result);

    return result;
}

/******************************************************************************
 * div_r - Calculate quotient and reminder
 *
 * RETURNS: N/A
 */

void div_r(
    int numer,
    int denom,
    div_t *result
    )
{
    result->quot = numer / denom;
    result->rem = numer - (denom * result->quot);

    if ((numer < 0) &&
         (result->rem > 0))
    {
        result->quot++;
        result->rem -= denom;
    }
}

