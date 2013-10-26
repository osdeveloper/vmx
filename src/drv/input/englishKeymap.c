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

/* englishKeymap.c - English keymap */

#include <drv/input/i8042Kbd.h>

u_int8_t kbdEnhanced[] =
{
    0x1c,                       /* num enter */
    0x1d,                       /* right ctrl */
    0x35,                       /* num slash */
    0x37,                       /* print screen */
    0x38,                       /* right alt */
    0x46,                       /* break */
    0x47,                       /* home */
    0x48,                       /* up */
    0x79,                       /* page up */
    0x4b,                       /* left */
    0x4d,                       /* right */
    0x4f,                       /* end */
    0x50,                       /* down */
    0x51,                       /* page down */
    0x52,                       /* insert */
    0x53                        /* delete */
};

u_int8_t kbdAction[] =
{
          0, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_CTL, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_SHI, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_NOR, KBA_SHI, KBA_NOR,
    KBA_NOR, KBA_NOR, KBA_CAP,       0,       0,       0,       0,       0,
          0,       0,       0,       0,       0, KBA_NUM, KBA_STP, KBA_ESC,
    KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC,
    KBA_ESC, KBA_ESC, KBA_ESC, KBA_ESC,       0,       0,       0,       0,
          0,       0,       0,       0,       0,       0,       0,       0,
          0,       0,       0,       0,       0,       0,       0,       0,
          0,       0,       0,       0,       0,       0,       0,       0,
    KBA_NOR,       0,       0, KBA_NOR,       0,       0, KBA_NOR,       0,
          0, KBA_NOR,       0,       0,       0, KBA_NOR,      0,        0,
   KBA_NOR, KBA_CTL, KBA_NOR, KBA_NOR, KBA_NOR, KBA_STP, KBA_EXT, KBA_EXT,
    KBA_NOR, KBA_EXT, KBA_EXT, KBA_NOR, KBA_EXT, KBA_NOR, KBA_EXT, KBA_EXT
};

u_int8_t kbdMap[4][144] =
{
    /* Unshift code */
    {
          0,   0x1b,    '1',    '2',    '3',    '4',    '5',    '6',
        '7',    '8',    '9',    '0',    '-',    '=',   0x08,   '\t',
        'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
        'o',    'p',    '[',    ']',   '\r',KBA_CTL,    'a',    's',
        'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
       '\'',    '`',KBA_SHI,   '\\',    'z',    'x',    'c',    'v',
        'b',    'n',    'm',    ',',    '.',    '/',KBA_SHI,    '*',
        ' ',    ' ',KBA_CAP,      0,      0,      0,      0,      0,
          0,      0,      0,      0,KBA_NUM,KBA_NUM,KBA_STP,    '7',
        '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
        '2',    '3',    '0',    '.',      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
       '\r',KBA_CTL,    '/',    '*',    ' ',KBA_STP,    'F',    'A',
          0,    'D',    'C',      0,    'B',      0,    '@',    'P'
    },

    /* Shift code */
    {
          0,   0x1b,    '!',    '@',    '#',    '$',    '%',    '^',
        '&',    '*',    '(',    ')',    '_',    '+',   0x08,   '\t',
        'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
        'O',    'P',    '{',    '}',   '\r',KBA_CTL,    'A',    'S',
        'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
        '"',    '~',KBA_SHI,    '|',    'Z',    'X',    'C',    'V',
        'B',    'N',    'M',    '<',    '>',    '?',KBA_SHI,    '*',
        ' ',    ' ',KBA_CAP,      0,      0,      0,      0,      0,
          0,      0,      0,      0,KBA_NUM,KBA_NUM,KBA_STP,    '7',
        '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
        '2',    '3',    '0',    '.',      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
       '\r',KBA_CTL,    '/',    '*',    ' ',KBA_STP,    'F',    'A',
          0,    'D',    'C',      0,    'B',      0,    '@',    'P'
    },

    /* Control code */
    {
          0xff,   0x1b,   0xff,   0x00,   0xff,   0xff,   0xff,   0xff,
          0x1e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   '\t',
          0x11,   0x17,   0x05,   0x12,   0x14,   0x19,   0x15,   0x09,
          0x0f,   0x10,   0x1b,   0x1d,   '\r',KBA_CTL,   0x01,   0x13,
          0x04,   0x06,   0x07,   0x08,   0x0a,   0x0b,   0x0c,   0xff,
          0xff,   0x1c,KBA_SHI,   0xff,   0x1a,   0x18,   0x03,   0x16,
          0x02,   0x0e,   0x0d,   0xff,   0xff,   0xff,KBA_SHI,   0xff,
          0xff,   0xff,KBA_CAP,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,KBA_NUM,KBA_STP,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,
          '\r',KBA_CTL,    '/',    '*',    ' ',KBA_STP,   0xff,   0xff,
          0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
    },

    /* Non-numeric code */
    {
          0,   0x1b,    '1',    '2',    '3',    '4',    '5',    '6',
        '7',    '8',    '9',    '0',    '-',    '=',   0x08,   '\t',
        'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
        'o',    'p',    '[',    ']',   '\r',KBA_CTL,    'a',    's',
        'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
       '\'',    '`',KBA_SHI,   '\\',    'z',    'x',    'c',    'v',
        'b',    'n',    'm',    ',',    '.',    '/',KBA_SHI,    '*',
        ' ',    ' ',KBA_CAP,      0,      0,      0,      0,      0,
          0,      0,      0,      0,KBA_NUM,KBA_NUM,KBA_STP,    'w',
        'r',    'y',    'l',    't',    'u',    'v',    'm',    'q',
        'r',    's',    'p',    'n',      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
          0,      0,      0,      0,      0,      0,      0,      0,
       '\r',KBA_CTL,    '/',    '*',    ' ',KBA_STP,    'F',    'A',
          0,    'D',    'C',      0,    'B',      0,    '@',    'P'
    }
};

