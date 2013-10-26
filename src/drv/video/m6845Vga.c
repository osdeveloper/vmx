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

/* m6845Vga.c - VGA terminal */

#include <string.h>
#include <sys/types.h>
#include <vmx.h>
#include <vmx/semLib.h>
#include <util/rngLib.h>
#include <io/ioLib.h>
#include <io/ttyLib.h>
#include <drv/serial/pcConsole.h>

/* IMPORTS */
IMPORT PC_CON_DEV pcConDev[];

/* LOCALS */
LOCAL VGA_CON_DEV vgaConDev[N_VIRTUAL_CONSOLES];
LOCAL u_int8_t    curSt;
LOCAL u_int8_t    curEd;
LOCAL int         vgaWriteTreshold = 20;

LOCAL void vgaConBeep(
    BOOL mode
    );

LOCAL void vgaStatInit(
    void
    );

LOCAL void vgaHook(
    VGA_CON_DEV *vs,
    ARG arg,
    int opCode
    );

LOCAL void vgaClear(
    VGA_CON_DEV *vs,
    int pos,
    u_int8_t c
    );

LOCAL void vgaSreenReverse(
    VGA_CON_DEV *vs
    );

LOCAL void vgaScroll(
    VGA_CON_DEV *vs,
    int pos,
    int lines,
    BOOL upDir,
    u_int8_t attrib
    );

LOCAL void vgaInsertChar(
    VGA_CON_DEV *vs,
    int n
    );

LOCAL void vgaDelLeftChar(
    VGA_CON_DEV *vs
    );

LOCAL void vgaCarriageReturn(
    VGA_CON_DEV *vs
    );

LOCAL void vgaBackSpace(
    VGA_CON_DEV *vs
    );

LOCAL void vgaTab(
    VGA_CON_DEV *vs
    );

LOCAL void vgaLineFeed(
    VGA_CON_DEV *vs
    );

LOCAL void vgaCursorPos(
    u_int16_t pos
    );

LOCAL void vgaCursorOn(
    void
    );

LOCAL void vgaCursorOff(
    void
    );

/******************************************************************************
 * vgaConBeep - Sound beep
 *
 * RETURNS: N/A
 */

LOCAL void vgaConBeep(
    BOOL mode
    )
{
    int beepTime, beepPitch;
    int level;

    if (mode)
    {
        beepPitch = BEEP_PITCH_L;
        beepTime  = BEEP_TIME_L;
    }
    else
    {
        beepPitch = BEEP_PITCH_S;
        beepTime  = BEEP_TIME_S;
    }

    /* Lock interrupts */
    INT_LOCK(level);

    /* Set command counter */
    sysOutByte(PIT_BASE_ADR + 3, 0xb6);
    sysOutByte(PIT_BASE_ADR + 2, (beepPitch & 0xff));
    sysOutByte(PIT_BASE_ADR + 2, (beepPitch >> 8));

    /* Enable counter */
    sysOutByte(DIAG_CTRL, sysInByte(DIAG_CTRL) | 0x03);

    /* Sleep for a while */
    taskDelay(beepTime);

    /* Disable counter */
    sysOutByte(DIAG_CTRL, sysInByte(DIAG_CTRL) & ~0x03);

    /* Unlock Interrupts */
    INT_UNLOCK(level);
}

/******************************************************************************
 * vgaHrdInit - Initialize the VGA display
 *
 * RETURNS: N/A
 */

void vgaHrdInit(
    void
    )
{
    /* Get cursor */
    sysOutByte((int) CTRL_SEL_REG, 0x0a);
    curSt = sysInByte((int) CTRL_VAL_REG);
    sysOutByte((int) CTRL_SEL_REG, 0x0b);
    curEd = sysInByte((int) CTRL_VAL_REG);

    vgaStatInit();

    /* Clear screen */
    vgaClear(pcConDev [PC_CONSOLE].vs, 2, ' ');
    vgaCursorOn();
}

/******************************************************************************
 * vgaStatInit - Initialize the VGA display state
 *
 * RETURNS: N/A
 */

LOCAL void vgaStatInit(
    void
    )
{
    int i;
    VGA_CON_DEV *vs;

    for (i = 0; i < N_VIRTUAL_CONSOLES; i++)
    {
        vs = &vgaConDev[i];
        pcConDev[i].vs = vs;

        /* Init state */
        vs->memBase         = CTRL_MEM_BASE;
        vs->colorMode       = COLOR;
        vs->savedCol        = 0;
        vs->col             = 0;
        vs->savedRow        = 0;
        vs->row             = 0;
        vs->defaultAttrib   = DEFAULT_ATR;
        vs->currAttrib      = DEFAULT_ATR;
        vs->savedCurrAttrib = DEFAULT_ATR;
        vs->currCharPos     = vs->memBase;
        vs->savedReverse    = vs->reverse;
        vs->nCol            = 80;
        vs->nRow            = 25;
        vs->scrollStart     = 0;
        vs->scrollEnd       = 24;
        vs->autoWrap        = TRUE;
        vs->scrollCheck     = FALSE;
        vs->charSet         = charTable[TEXT_SET];
        vs->vgaMode         = TEXT_MODE;
        vs->insertMode      = INSERT_MODE_OFF;
        vs->escFlags        = ESC_NORMAL;
        vs->escParamCount   = 0;
        vs->escQuestion     = FALSE;
        memset((char *) vs->escParam, 0, sizeof(vs->escParam));
        memset((char *) vs->tabStop, 0, sizeof(vs->tabStop));
        vs->tabStop[0]      = 1;
        vs->tabStop[8]      = 1;
        vs->tabStop[16]     = 1;
        vs->tabStop[24]     = 1;
        vs->tabStop[32]     = 1;
        vs->tabStop[40]     = 1;
        vs->tabStop[48]     = 1;
        vs->tabStop[56]     = 1;
        vs->tabStop[64]     = 1;
        vs->tabStop[72]     = 1;
        vs->vgaHook         = (FUNCPTR) vgaHook;
    }
}

/******************************************************************************
 * vgaHook - Call for each vga screen
 *
 * RETURNS: N/A
 */

LOCAL void vgaHook(
    VGA_CON_DEV *vs,
    ARG arg,
    int opCode
    )
{
}

/******************************************************************************
 * vgaClear - Clear vga screen
 *
 * RETURNS: N/A
 */

LOCAL void vgaClear(
    VGA_CON_DEV *vs,
    int pos,
    u_int8_t c
    )
{
    u_int16_t *cp, *end, erase;

    erase = (vs->defaultAttrib << 8) + c;
    if (pos == 2)
    {
        cp              = (u_int16_t *) vs->memBase;
        end             = (u_int16_t *) (vs->memBase + 2048 * CHR);
        vs->col         = 0;
        vs->row         = 0;
        vs->currCharPos = (u_int8_t *) vs->memBase;
    }
    else if (pos == 0)
    {
        cp              = (u_int16_t *) vs->currCharPos;
        end             = (u_int16_t *) (vs->memBase + 2048 * CHR);
    }
    else
    {
        cp              = (u_int16_t *) vs->currCharPos;
        end             = (u_int16_t *) (vs->memBase + CHR);
    }

    for (; cp < end; cp++)
    {
        *cp = erase;
    }
}

/******************************************************************************
 * vgaScreenReverse - Reverse screen
 *
 * RETURNS: N/A
 */

LOCAL void vgaSreenReverse(
    VGA_CON_DEV *vs
    )
{
    u_int8_t *cp, attrib;

    for (cp = vs->memBase;
         cp < vs->memBase + 2000 * CHR;
         cp += CHR)
    {
        attrib     = *(cp + 1);
        *(cp + 1)  = attrib &INT_BLINK_MASK;
        *(cp + 1) |= (attrib << 4) & BG_ATTR_MASK;
        *(cp + 1) |= (attrib >> 4) & FG_ATTR_MASK;
    }
}

/******************************************************************************
 * vgaScroll - Scroll vga screen
 *
 * RETURNS: N/A
 */

LOCAL void vgaScroll(
    VGA_CON_DEV *vs,
    int pos,
    int lines,
    BOOL upDir,
    u_int8_t attrib
    )
{
    u_int8_t *dest, *src;

    if ((pos >= vs->scrollStart) && (pos <= vs->scrollEnd))
    {
        if (upDir == FORWARD)
        {
            if (vs->scrollStart + lines > vs->scrollEnd + 1)
            {
                lines = vs->scrollEnd - vs->scrollStart + 1;
            }

            dest = vs->memBase + vs->nCol * CHR * vs->scrollStart;
            src  = vs->memBase + vs->nCol * CHR * (vs->scrollStart + lines);

            for (;
                 src < vs->memBase + vs->nCol * CHR * (pos + 1);
                 *dest++ = *src++);

            dest = vs->memBase + vs->nCol * CHR * (pos - (lines - 1));

            for (;
                 dest < vs->memBase + vs->nCol * CHR * (pos + 1);
                 dest += CHR)
            {
                *dest       = ' ';
                *(dest + 1) = attrib;
            }
        }
        else
        {
            if (vs->scrollStart + lines > vs->scrollEnd + 1)
            {
                lines = vs->scrollEnd - vs->scrollEnd + 1;
            }

            dest = vs->memBase + vs->nCol * CHR * (vs->scrollEnd + 1) - 1;
            src  = vs->memBase + vs->nCol * CHR *
                (vs->scrollEnd - (lines - 1)) - 1;

            for (;
                 src > vs->memBase + vs->nCol * CHR * pos - 1;
                 *dest-- = *src--);

            dest = vs->memBase + vs->nCol * CHR * (pos + lines) - 1;

            for (;
                 dest > vs->memBase + vs->nCol * CHR * pos -1;
                 dest -= CHR)
            {
                *dest = attrib;
                *(dest - 1) = ' ';
            }
        }
    }
}

/******************************************************************************
 * vgaInsertChar - Insert character at current cursor location
 *
 * RETURNS: N/A
 */

LOCAL void vgaInsertChar(
    VGA_CON_DEV *vs,
    int n
    )
{
    int xPos;
    u_int16_t erase, swap, *charPos;

    if (n > vs->nCol)
    {
        n = vs->nCol;
    }
    else if (n == 0)
    {
        n = 1;
    }

    while (n--)
    {
        xPos    = vs->col;
        charPos = (u_int16_t *) vs->currCharPos;
        erase   = (vs->defaultAttrib << 8) + ' ';

        while (xPos++ < vs->nCol)
        {
            swap     = *charPos;
            *charPos = erase;
            erase    = swap;
            charPos++;
        }
    }
}

/******************************************************************************
 * vgaDelLeftChar - Delete character left of current cursor location
 *
 * RETURNS: N/A
 */

LOCAL void vgaDelLeftChar(
    VGA_CON_DEV *vs
    )
{
    u_int16_t erase;

    erase = (vs->defaultAttrib << 8) + ' ';

    if ((vs->autoWrap == TRUE) || (vs->nCol > 0))
    {
        vs->col--;
        vs->currCharPos -= CHR;
    }

    *(u_int16_t *) vs->currCharPos = erase;
}

/******************************************************************************
 * vgaCarriageReturn - Do a carriage return
 *
 * RETURNS: N/A
 */

LOCAL void vgaCarriageReturn(
    VGA_CON_DEV *vs
    )
{
    vs->currCharPos -= vs->col * CHR;
    vs->col          = 0;
}

/******************************************************************************
 * vgaBackSpace - Do a back space
 *
 * RETURNS: N/A
 */

LOCAL void vgaBackSpace(
    VGA_CON_DEV *vs
    )
{
    if ((vs->autoWrap == TRUE) || (vs->nCol > 0))
    {
        vs->col--;
        vs->currCharPos -= CHR;
    }

    if (vs->col < 0)
    {
        vs->col = vs->nCol - 1;
        vs->row--;
        vs->scrollCheck = TRUE;
    }
}

/******************************************************************************
 * vgaTab - Do a tab
 *
 * RETURNS: N/A
 */

LOCAL void vgaTab(
    VGA_CON_DEV *vs
    )
{
    int i;

    for (i = vs->col + 1; i < vs->nCol; i++)
    {
        if (vs->tabStop[i])
        {
            vs->col = i;
            break;
        }
    }

    if ((vs->autoWrap == TRUE) && (i >= vs->nCol))
    {
        vs->col = 0;
        vs->row++;
        vs->scrollCheck = TRUE;
    }

    /* Calculate new pos */
    vs->currCharPos = vs->memBase + vs->row * vs->nCol * CHR + vs->col * CHR;
}

/******************************************************************************
 * vgaLineFeed - Feed a new line
 *
 * RETURNS: N/A
 */

LOCAL void vgaLineFeed(
    VGA_CON_DEV *vs
    )
{
    vs->currCharPos += vs->nCol * CHR;
    vs->row++;
    vs->scrollCheck = TRUE;
}

/******************************************************************************
 * vgaCursorPos - Set cursor position
 *
 * RETURNS: N/A
 */

LOCAL void vgaCursorPos(
    u_int16_t pos
    )
{
    sysOutByte((int) CTRL_SEL_REG, 0x0e);
    sysOutByte((int) CTRL_VAL_REG, (pos >> 8) & 0xff);
    sysOutByte((int) CTRL_SEL_REG, 0x0f);
    sysOutByte((int) CTRL_VAL_REG, pos & 0xff);
}

/******************************************************************************
 * vgaCursorOn - Turn cursor on
 *
 * RETURNS: N/A
 */

LOCAL void vgaCursorOn(
    void
    )
{
    sysOutByte((int) CTRL_SEL_REG, 0x0a);
    sysOutByte((int) CTRL_VAL_REG, curSt & ~0x20);
    sysOutByte((int) CTRL_SEL_REG, 0x0b);
    sysOutByte((int) CTRL_VAL_REG, curEd);
}

/******************************************************************************
 * vgaCursorOff - Turn cursor off
 *
 * RETURNS: N/A
 */

LOCAL void vgaCursorOff(
    void
    )
{
    sysOutByte((int) CTRL_SEL_REG, 0x0a);
    sysOutByte((int) CTRL_VAL_REG, 0x20);
    sysOutByte((int) CTRL_SEL_REG, 0x0b);
    sysOutByte((int) CTRL_VAL_REG, 0x00);
}

/******************************************************************************
 * vgaLoadFont - Upload a font
 *
 * RETURNS: N/A
 */

void vgaLoadFont(
    u_int8_t *fontArray,
    int height
    )
{
    int i, j;
    u_int8_t *fbAddr = (u_int8_t *) 0xa0000;

    /* Enable font upload */
    sysOutWord(0x3c4, 0x0402);            /* Display planes: (0,1,0,0) */
    sysOutWord(0x3c4, 0x0704);            /* Deselect odd/even */
    sysOutWord(0x3ce, 0x0005);            /* Deselect odd/even */
    sysOutWord(0x3ce, 0x0406);            /* 64k a0000 - affff */

    /* For all characters */
    for (i = 0; i < 256; i++)
    {
        /* For character height */
        for (j = 0; j < height; j++)
        {
            /* Upload byte */
            *fbAddr++ = fontArray[(i * height) + j];

        }

        /* Advance one character */
        fbAddr += height;

    }

    /* Restore to normal text operation */
    sysOutWord(0x3c4, 0x0302);            /* Display planes: (0,0,1,1) */
    sysOutWord(0x3c4, 0x0204);            /* Select odd/even */
    sysOutWord(0x3ce, 0x1005);            /* Select odd/even */
    sysOutWord(0x3ce, 0x0e06);            /* 32k a0000 - affff */
}

/******************************************************************************
 * vgaWriteString - Write string to vga display
 *
 * RETURNS: Bytes written
 */

int vgaWriteString(
    PC_CON_DEV *pc
    )
{
    u_int8_t c, attrib;
    int dummy, nBytes;
    BOOL hdlSpecial;
    RING_ID ringId;
    VGA_CON_DEV *vs;

    ringId                     = pc->ttyDev.writeBuffer;
    vs                         = pc->vs;
    pc->ttyDev.writeState.busy = TRUE;
    attrib                     = vs->currAttrib;
    nBytes                     = 0;

    /* Check for xon/xoff */
    if ((pc->ttyDev.writeState.xoff == TRUE) ||
        (pc->ttyDev.writeState.flushingWriteBuffer == TRUE))
    {
        pc->ttyDev.writeState.busy = FALSE;
    }
    else
    {
        /* Walk trough write buffer */
        while (RNG_ELEM_GET(ringId, &c, dummy) != FALSE)
        {
            nBytes++;
            hdlSpecial = TRUE;

            /* Check if printable */
            if ((vs->escFlags == ESC_NORMAL) && (vs->charSet[c]))
            {
                /* Write character */
                *(u_int16_t *) vs->currCharPos = (attrib << 8) + vs->charSet[c];

                /* Check for wrap */
                if (vs->col == (vs->nCol - 1))
                {
                    if (vs->autoWrap == TRUE)
                    {
                        vgaCarriageReturn(vs);
                        vgaLineFeed(vs);
                        hdlSpecial = FALSE;
                    }
                }
                else
                {
                    vs->col++;
                    vs->currCharPos += CHR;
                    continue;
                }
            }

            if (hdlSpecial == TRUE)
            {
                /* Handle special character */
                switch (c)
                {
                    /* Bell */
                    case 0x07:
                        vgaConBeep(FALSE);
                        continue;

                    /* Backspace */
                    case 0x08:
                        vgaBackSpace(vs);
                        continue;

                    /* Tab */
                    case '\t':
                        vgaTab(vs);
                        continue;

                    /* Newline */
                    case '\n':
                        if ((pc->ttyDev.options & OPT_CRMOD) == OPT_CRMOD)
                        {
                            vgaCarriageReturn(vs);
                        }

                        vgaLineFeed(vs);
                        break;

                    /* Linefeed */
                    case 0x0b:
                        vgaLineFeed(vs);
                        continue;

                    /* clear */
                    case 0x0c:
                        vgaClear(vs, 2, ' ');
                        continue;

                    /* Carriage-return */
                    case 0x0d:
                        vgaCarriageReturn(vs);
                        continue;

                    /* vt100 char set */
                    case 0x0e:
                        vs->charSet = charTable[VT100_SET];
                        continue;

                    /* normal char set */
                    case 0x0f:
                        vs->charSet = charTable[TEXT_SET];
                        continue;

                    /* del-left */
                    case 0x7f:
                        vgaDelLeftChar(vs);
                        continue;
                }
            }

            /* Check forward scroll */
            if ((vs->scrollCheck == TRUE) &&
                (vs->currCharPos >=
                 (vs->memBase + vs->nCol * CHR * vs->scrollEnd + 1)))
            {
                vs->row = vs->scrollEnd;
                vgaScroll(
                    vs,
                    vs->row,
                    vs->scrollCheck,
                    FORWARD,
                    vs->defaultAttrib
                    );

                while (vs->currCharPos >=
                       (vs->memBase + vs->nCol * CHR * (vs->scrollEnd + 1)))
                {
                    vs->currCharPos -= vs->nCol * CHR;
                }
            }
            else if ((vs->scrollCheck == TRUE) &&
                     (vs->currCharPos <
                      (vs->memBase + vs->col * CHR * vs->scrollStart)))
            {
                vs->row = vs->scrollStart;
                vgaScroll(
                    vs,
                    vs->row,
                    vs->scrollCheck,
                    BACKWARD,
                    vs->defaultAttrib
                    );

                while (vs->currCharPos <
                       (vs->memBase + vs->col * CHR * vs->scrollStart))
                {
                    vs->currCharPos += vs->nCol * CHR;
                }
            }
            else if (vs->currCharPos >
                     (vs->memBase + vs->nCol * CHR * vs->nRow))
            {
                while (vs->currCharPos >
                       (vs->memBase + vs->nCol * CHR * vs->nRow))
                {
                    vs->currCharPos -= vs->nCol * CHR;
                    vs->row--;
                }
            }
            else if (vs->currCharPos < vs->memBase)
            {
                while (vs->currCharPos < vs->memBase)
                {
                    vs->currCharPos += vs->nCol * CHR;
                    vs->row++;
                }
            }
        }

        vgaCursorPos((vs->currCharPos - vs->memBase) / CHR);
        pc->ttyDev.writeState.busy = FALSE;

        /* Release lock */
        if (rngFreeBytes(ringId) >= vgaWriteTreshold)
        {
            semGive(&pc->ttyDev.writeSync);
        }
    }

    return nBytes;
}

