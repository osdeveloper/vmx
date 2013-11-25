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

/* i8237Dma.c - Intel DMA 8237 driver */

/* Includes */
#include <stdlib.h>
#include <fcntl.h>
#include <vmx.h>
#include <arch/sysArchLib.h>
#include <os/cacheLib.h>
#include <drv/dma/i8237Dma.h>

/* Defines */

/* Imports */
void sysDelay(
    void
    );

/* Locals */
LOCAL int dma1PageReg[] =
{
    0x87,
    0x83,
    0x81,
    0x82
};

LOCAL int dma2PageReg[] =
{
    0x8b,
    0x89,
    0x8a,
    0x8f
};

/* Globals */

/* Functions */

/******************************************************************************
 * dmaSetup - Setup dma controller
 *
 * RETURNS: OK or ERROR
 */

STATUS dmaSetup(
    int           dir,
    void         *buf,
    unsigned int  nBytes,
    unsigned int  c
    )
{
    STATUS status;
    char   reg;

    if ((c > 7) || (nBytes > (1 << 16)))
    {
        status = ERROR;
    }
    else
    {
        if (dir == O_RDONLY)
        {
            reg = DMA_MODE_SINGLE | DMA_MODE_READ | (c & 0x03);
        }
        else
        {
            reg = DMA_MODE_SINGLE | DMA_MODE_WRITE | (c & 0x03);
        }

        if ((c & 4) == 0)
        {
            cacheInvalidate(DATA_CACHE, buf, nBytes);
            sysOutByte(DMA1_MASK_SINGLE, DMA_MASK_SET | (c & 0x03));
            sysDelay();
            sysOutByte(DMA1_MODE, reg);
            sysDelay();
            sysOutByte(DMA1_CLEAR_FF, 0);
            sysDelay();
            sysOutByte(DMA1_ADDR(c), (int) buf & 0x000000ff);
            sysDelay();
            sysOutByte(DMA1_ADDR(c), ((int) buf & 0x0000ff00) >> 8);
            sysDelay();
            sysOutByte(dma1PageReg[c & 0x03], ((int) buf & 0x00ff0000) >> 16);
            sysDelay();
            sysOutByte(DMA1_COUNT(c), --nBytes & 0x000000ff);
            sysDelay();
            sysOutByte(DMA1_COUNT(c), (nBytes & 0x0000ff00) >> 8);
            sysDelay();
            sysOutByte(DMA1_MASK_SINGLE, DMA_MASK_RESET | (c & 0x03));
            sysDelay();

            status = OK;
        }
        else
        {
            cacheInvalidate(DATA_CACHE, buf, nBytes);
            sysOutByte(DMA2_MASK_SINGLE, DMA_MASK_SET | (c & 0x03));
            sysDelay();
            sysOutByte(DMA2_MODE, reg);
            sysDelay();
            sysOutByte(DMA2_CLEAR_FF, 0);
            sysDelay();
            sysOutByte(DMA2_ADDR(c), ((int) buf & 0x000001fe) >> 1);
            sysDelay();
            sysOutByte(DMA2_ADDR(c), ((int) buf & 0x0001fe00) >> 9);
            sysDelay();
            sysOutByte(dma2PageReg[c & 0x03], ((int) buf & 0x00ff0000) >> 16);
            sysDelay();
            nBytes <<= 1;
            sysOutByte(DMA2_COUNT(c), --nBytes & 0x000000ff);
            sysDelay();
            sysOutByte(DMA2_COUNT(c), (nBytes & 0x0000ff00) >> 8);
            sysDelay();
            sysOutByte(DMA2_MASK_SINGLE, DMA_MASK_RESET | (c & 0x03));
            sysDelay();

            status = OK;
        }
    }

    return status;
}

