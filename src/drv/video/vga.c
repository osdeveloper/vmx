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

/* vga.c - VGA terminal */

#include <string.h>
#include <sys/types.h>
#include <drv/video/vga.h>

/* Textpointer, color and cursor position */
unsigned short *textmemptr = (unsigned short *) 0xb8000;
unsigned short attrib = 0x1f;
int csr_x = 0, csr_y = 0;

/******************************************************************************
 * vgaInit - Initialize frame buffer
 *
 * RETURNS: N/A
 */

void vgaInit(
    void
    )
{
    vgaClear();
}

/******************************************************************************
 * vgaClear - Clear screen
 *
 * RETURNS: N/A
 */

void vgaClear(
    void
    )
{
    u_int16_t *ptr;
    u_int16_t blank;
    int i;

    ptr = textmemptr;
    blank = 0x20 | (attrib << 8);

    /* Loop for all lines */
    for (i = 0; i < 25; i++, ptr+=80)
    {
        memsetw(ptr, blank, 80);
    }

    /* Reset position */
    csr_x = 0;
    csr_y = 0;
    vgaMoveCsr();
}

/******************************************************************************
 * vgaMoveCsr - Update the hardware cursor, blink, blink
 *
 * RETURNS: N/A
 */

void vgaMoveCsr(
    void
    )
{
    int temp;

    /* Equation to find index in linear chunk buffer */
    temp = csr_y * 80 + csr_x;

    sysOutByte(0x3d4, 14);
    sysOutByte(0x3d5, temp >> 8);
    sysOutByte(0x3d4, 15);
    sysOutByte(0x3d5, temp);
}

/******************************************************************************
 * vgaScroll - Scroll screen
 *
 * RETURNS: N/A
 */

void vgaScroll(
    void
    )
{
    u_int16_t blank;
    int temp;

    /* Blank is defined as space */
    blank = 0x20 | (attrib << 8);

    /* Row 25 is end, scroll up */
    if (csr_y >= 25)
    {
        /* Move text chunk back */
        temp = csr_y - 25 + 1;
        memcpy(textmemptr, textmemptr + temp * 80, (25 - temp) * 80 * 2);

        /* Finally make last line blank */
        memsetw(textmemptr + (25 - temp) * 80, blank, 80);
        csr_y = 25 - 1;
    }
}

/******************************************************************************
 * vgaPutch - Put a single character on screen
 *
 * RETURNS: N/A
 */

void vgaPutch(
    char c
    )
{
    u_int16_t *where;
    u_int16_t att = attrib << 8;

    /* Handle a backspace, by moving cursor back one space */
    if (c == 0x08)
    {
        if (csr_x != 0)
        {
            csr_x--;
        }
    }

    /* Handle a tab by incrementing the cursors x a multiple of 8 */
    else if (c == 0x09)
    {
        csr_x  = (csr_x + 8) & ~(8 - 1);
    }

    /* Handle a carrige return which brings back cursor to start of line */
    else if (c == '\r')
    {
        csr_x = 0;
    }

    /* Handle newlines like CR/LF was sent */
    else if (c == '\n')
    {
        csr_x = 0;
        csr_y++;
    }

    /* Any character greater than space is pritable */
    else if (c >= ' ')
    {
        where = (u_int16_t *) (textmemptr + (csr_y * 80 + csr_x));
        *where = c | att;                /* Char and color */
        csr_x++;
    }

    /* If end on line reached, advance to next line */
    if (csr_x >= 80)
    {
        csr_x = 0;
        csr_y++;
    }

    /* Scroll and move cursor if needed */
    vgaScroll();
    vgaMoveCsr();
}

/******************************************************************************
 * vgaPuts - Print a line on screen
 *
 * RETURNS: N/A
 */

void vgaPuts(
    const char *str
    )
{
    int i;

    for (i = 0; i < strlen(str); i++)
    {
        vgaPutch(str[i]);
    }
}

/******************************************************************************
 * vgaSetColor - Set bg/fg color
 *
 * RETURNS: N/A
 */

void vgaSetColor(
    u_int8_t fg,
    u_int8_t bg
    )
{
    /* Top 4 bytes are the background, bottom 4 foreground */
    attrib = (bg << 4) | (fg &0x0f);
}

