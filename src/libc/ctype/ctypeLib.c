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

/* ctypeLib.c - Character types for the C library */

#include <ctype.h>
#include <vmx.h>

/******************************************************************************
 * __ctype_table - Table of types
 */

LOCAL const unsigned char __ctype_table[] =
{
        0,                              /* 0            start of table  */
        _C_B,                           /* 80           nul             */
        _C_B,                           /* 80           soh             */
        _C_B,                           /* 80           stx             */
        _C_B,                           /* 80           etx             */
        _C_B,                           /* 80           eot             */
        _C_B,                           /* 80           enq             */
        _C_B,                           /* 80           ack             */
        _C_B,                           /* 80           bel             */
        _C_B,                           /* 80           bs              */
        _C_CONTROL,                     /* 20           ht              */
        _C_CONTROL,                     /* 20           nl              */
        _C_CONTROL,                     /* 20           vt              */
        _C_CONTROL,                     /* 20           np              */
        _C_CONTROL,                     /* 20           cr              */
        _C_B,                           /* 80           so              */
        _C_B,                           /* 80           si              */
        _C_B,                           /* 80           dle             */
        _C_B,                           /* 80           dc1             */
        _C_B,                           /* 80           dc2             */
        _C_B,                           /* 80           dc3             */
        _C_B,                           /* 80           dc4             */
        _C_B,                           /* 80           nak             */
        _C_B,                           /* 80           syn             */
        _C_B,                           /* 80           etb             */
        _C_B,                           /* 80           can             */
        _C_B,                           /* 80           em              */
        _C_B,                           /* 80           sub             */
        _C_B,                           /* 80           esc             */
        _C_B,                           /* 80           fs              */
        _C_B,                           /* 80           gs              */
        _C_B,                           /* 80           rs              */
        _C_B,                           /* 80           us              */
        _C_WHITE_SPACE,                 /* 08           space           */
        _C_PUNCT,                       /* 10           !               */
        _C_PUNCT,                       /* 10           "               */
        _C_PUNCT,                       /* 10           #               */
        _C_PUNCT,                       /* 10           $               */
        _C_PUNCT,                       /* 10           %               */
        _C_PUNCT,                       /* 10           &               */
        _C_PUNCT,                       /* 10           '               */
        _C_PUNCT,                       /* 10           (               */
        _C_PUNCT,                       /* 10           )               */
        _C_PUNCT,                       /* 10           *               */
        _C_PUNCT,                       /* 10           +               */
        _C_PUNCT,                       /* 10           ,               */
        _C_PUNCT,                       /* 10           -               */
        _C_PUNCT,                       /* 10           .               */
        _C_PUNCT,                       /* 10           /               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           0               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           1               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           2               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           3               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           4               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           5               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           6               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           7               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           8               */
        _C_NUMBER | _C_HEX_NUMBER,      /* 44           9               */
        _C_PUNCT,                       /* 10           :               */
        _C_PUNCT,                       /* 10           ;               */
        _C_PUNCT,                       /* 10           <               */
        _C_PUNCT,                       /* 10           =               */
        _C_PUNCT,                       /* 10           >               */
        _C_PUNCT,                       /* 10           ?               */
        _C_PUNCT,                       /* 10           @               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           A               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           B               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           C               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           D               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           E               */
        _C_UPPER | _C_HEX_NUMBER,       /* 41           F               */
        _C_UPPER,                       /* 01           G               */
        _C_UPPER,                       /* 01           H               */
        _C_UPPER,                       /* 01           I               */
        _C_UPPER,                       /* 01           J               */
        _C_UPPER,                       /* 01           K               */
        _C_UPPER,                       /* 01           L               */
        _C_UPPER,                       /* 01           M               */
        _C_UPPER,                       /* 01           N               */
        _C_UPPER,                       /* 01           O               */
        _C_UPPER,                       /* 01           P               */
        _C_UPPER,                       /* 01           Q               */
        _C_UPPER,                       /* 01           R               */
        _C_UPPER,                       /* 01           S               */
        _C_UPPER,                       /* 01           T               */
        _C_UPPER,                       /* 01           U               */
        _C_UPPER,                       /* 01           V               */
        _C_UPPER,                       /* 01           W               */
        _C_UPPER,                       /* 01           X               */
        _C_UPPER,                       /* 01           Y               */
        _C_UPPER,                       /* 01           Z               */
        _C_PUNCT,                       /* 10           [               */
        _C_PUNCT,                       /* 10           \               */
        _C_PUNCT,                       /* 10           ]               */
        _C_PUNCT,                       /* 10           ^               */
        _C_PUNCT,                       /* 10           _               */
        _C_PUNCT,                       /* 10           `               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           a               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           b               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           c               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           d               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           e               */
        _C_LOWER | _C_HEX_NUMBER,       /* 42           f               */
        _C_LOWER,                       /* 02           g               */
        _C_LOWER,                       /* 02           h               */
        _C_LOWER,                       /* 02           i               */
        _C_LOWER,                       /* 02           j               */
        _C_LOWER,                       /* 02           k               */
        _C_LOWER,                       /* 02           l               */
        _C_LOWER,                       /* 02           m               */
        _C_LOWER,                       /* 02           n               */
        _C_LOWER,                       /* 02           o               */
        _C_LOWER,                       /* 02           p               */
        _C_LOWER,                       /* 02           q               */
        _C_LOWER,                       /* 02           r               */
        _C_LOWER,                       /* 02           s               */
        _C_LOWER,                       /* 02           t               */
        _C_LOWER,                       /* 02           u               */
        _C_LOWER,                       /* 02           v               */
        _C_LOWER,                       /* 02           w               */
        _C_LOWER,                       /* 02           x               */
        _C_LOWER,                       /* 02           y               */
        _C_LOWER,                       /* 02           z               */
        _C_PUNCT,                       /* 10           {               */
        _C_PUNCT,                       /* 10           |               */
        _C_PUNCT,                       /* 10           }               */
        _C_PUNCT,                       /* 10           ~               */
        _C_B,                           /* 80           del             */
};

const unsigned char *__ctype = &__ctype_table[1];

