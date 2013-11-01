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

/* ctype.h - Character types for the C library */

#ifndef _CTYPE_H
#define _CTYPE_H

#include <vmx.h>

/* Defines */
#define _C_UPPER                0x01
#define _C_LOWER                0x02
#define _C_NUMBER               0x04
#define _C_WHITE_SPACE          0x08
#define _C_PUNCT                0x10
#define _C_CONTROL              0x20
#define _C_HEX_NUMBER           0x40
#define _C_B                    0x80

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Imports */
IMPORT const unsigned char *__ctype;

/* Macros */
#define __isascii(c)            ((unsigned) (c) <= 177)
#define __toascii(c)            ((c) & 177)
#define __isalpha(c)            (__ctype[c] & (_C_UPPER | _C_LOWER))
#define __isupper(c)            (__ctype[c] & (_C_UPPER))
#define __islower(c)            (__ctype[c] & (_C_LOWER))
#define __isdigit(c)            (__ctype[c] & (_C_NUMBER))
#define __isxdigit(c)           (__ctype[c] & (_C_HEX_NUMBER))
#define __isspace(c)            (__ctype[c] & (_C_WHITE_SPACE | _C_CONTROL))
#define __ispunct(c)            (__ctype[c] & (_C_PUNCT))
#define __isalnum(c)            (__ctype[c] & (_C_UPPER | _C_LOWER | _C_NUMBER))
#define __isprint(c)            (__ctype[c] & (_C_PUNCT | _C_UPPER | _C_LOWER |\
                                        _C_WHITE_SPACE | _C_NUMBER))
#define __isgraph(c)            (__ctype[c] & (_C_PUNCT | _C_UPPER | _C_LOWER |\
                                        _C_NUMBER))
#define __iscntrl(c)            (__ctype[c] & (_C_CONTROL | _C_B))
#define __toupper(c)            (                                              \
                                 (('a' <= (c)) && ((c) <= 'z')) ?              \
                                  ((c) - 'a' + 'A') :                          \
                                  (c)                                          \
                                )
#define __tolower(c)            (                                              \
                                 (('A' <= (c)) && ((c) <= 'Z')) ?              \
                                  ((c) - 'A' + 'a') :                          \
                                  (c)                                          \
                                )

/* Functions */

/******************************************************************************
 * isalnum - Test if character is alpha numeric
 *
 * RETURNS: Non-zero in alphanum
 */

int isalnum(
    int c
    );

/******************************************************************************
 * isalnum - Test if character is an alphabetical letter
 *
 * RETURNS: Non-zero if alphabetical
 */

int isalpha(
    int c
    );

/******************************************************************************
 * iscntrl - Test if character is a control signal
 *
 * RETURNS: Non-zero if control signal
 */

int iscntrl(
    int c
    );

/******************************************************************************
 * isdigit - Test if character is a digit
 *
 * RETURNS: Non-zero if digit
 */

int isdigit(
    int c
    );

/******************************************************************************
 * isgraph - Test if character is graphical
 *
 * RETURNS: Non-zero if graphical
 */

int isgraph(
    int c
    );

/******************************************************************************
 * islower - Test if character is lower case
 *
 * RETURNS: Non-zero if lower case
 */

int islower(
    int c
    );

/******************************************************************************
 * isprint - Test if character is printable
 *
 * RETURNS: Non-zero if printable
 */

int isprint(
    int c
    );

/******************************************************************************
 * ispunct - Test if character is punctation
 *
 * RETURNS: Non-zero if putctation
 */

int ispunct(
    int c
    );

/******************************************************************************
 * isspace - Test if character is a blank space
 *
 * RETURNS: Non-zero if blank space
 */

int isspace(
    int c
    );

/*******************************************************************************
 * isupper - Test if character is upper case
 *
 * RETURNS: Non-zero if upper case
 */

int isupper(
    int c
    );

/******************************************************************************
 * isxdigit - Test if character is a hexadecimal digit
 *
 * RETURNS: Non-zero if hexadecimal digit
 */

int isxdigit(
    int c
    );

/******************************************************************************
 * tolower - Convert character to lower case
 *
 * RETURNS: Lowercase equivalent or unchanged
 */

int tolower(
    int c
    );

/******************************************************************************
 * toupper - Convert character to upper case
 *
 * RETURNS: Uppercase equivalent or unchanged
 */

int toupper(
    int c
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _CTYPE_H */

