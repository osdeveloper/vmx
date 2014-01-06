/******************************************************************************
 *   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 *   This file is part of Real VMX.
 *   Copyright (C) 2014 Surplus Users Ham Society
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* TODO: Replace with ugl.h */
typedef struct ugl_ugi_driver {
    void * fbAddress;
} UGL_UGI_DRIVER;
typedef unsigned int UGL_UINT32;
#define UGL_NULL NULL

/******************************************************************************
 *
 * uglFbDevCreate - Create framebuffer device driver
 *
 * RETURNS: Pointer to driver or UGL_NULL
 */

UGL_UGI_DRIVER * uglFbDevCreate (
    UGL_UINT32  arg0,
    UGL_UINT32  arg1,
    UGL_UINT32  arg2
    ) {
    UGL_UGI_DRIVER            drv;
    int                       fbfd;
    struct fb_var_screeninfo  vinfo;
    struct fb_fix_screeninfo  finfo;
    long int                  screensize;

    /* Open the file for reading and writing */
    fbfd = open ("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        return (UGL_NULL);
    }

    /* Get fixed screen information */
    if (ioctl (fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        close (fbfd);
        return (UGL_NULL);
    }

    /* Get variable screen information */
    if (ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        close (fbfd);
        return (UGL_NULL);
    }

    /* Figure out the size of the screen in bytes */
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    /* Map the device to memory */
    drv.fbAddress = (char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int) drv.fbAddress == -1) {
        close (fbfd);
        return (UGL_NULL);
    }

    /* Temporary for test */
    char *   pDest = (char *) drv.fbAddress;
    int      x = 100;
    int      y = 100;
    long int location;

    /* Figure out where in memory to put the pixel */
    for (y = 100; y < 300; y++)
        for (x = 100; x < 300; x++) {

            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                       (y+vinfo.yoffset) * finfo.line_length;

            if (vinfo.bits_per_pixel == 32) {
                *(pDest + location) = 100; 
                *(pDest + location + 1) = 15 + (x - 100) / 2; 
                *(pDest + location + 2) = 200 - (y - 100) / 5; 
                *(pDest + location + 3) = 0;
                /* location += 4; */
            } else  { /* assume 16bpp */
                int b = 10;
                int g = (x - 100) / 6;
                int r = 31 - (y - 100) / 16;
                unsigned short int t = r << 11 | g << 5 | b;
                *((unsigned short int *) (pDest + location)) = t;
            }

        }

    munmap (drv.fbAddress, screensize);
    close (fbfd);
    /* End temporary for test */

    /* TODO: Return actual driver */
    return (UGL_UGI_DRIVER *) 1;
}

/* Temporary for test */
int main (void)
{
    UGL_UGI_DRIVER * pDrv;

    pDrv = uglFbDevCreate (0, 0, 0);
    if (pDrv == UGL_NULL) {
        printf("Error - Unable to open device.\n");
        return (1);
    }

    return (0);
}

/* End temporary for test */

