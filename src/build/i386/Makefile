###############################################################################
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
#
#   This file is part of Real VMX.
#   Copyright (C) 2013 Surplus Users Ham Society
#
#   Real VMX is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Real VMX is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
#

################################################################################
# COMPILE PART
#

CC = gcc
LD = ld
RM = rm -f
OUTPUT = vmx

CPU = PENTIUM2
CFLAGS = -nostdinc -fno-builtin -m32 -fno-stack-protector
ASMFLAGS = -nostdinc -fno-builtin -m32
DFLAGS = -DCPU=$(CPU) \
	 -DCOMPILE_DATE=\"`date +%d_%B_%Y.%H:%M`\"
LIBS = -lvmx -larch -los -lstdc -lutil -lfs -lostool -lusr -lgcc \
       -lnet -lnetinet -lugl-2d -lugl-os -lugl-driver -lugl-fonts
LDFLAGS = -T $(VMX_BASE)/include/make/i386/link.ld -m elf_i386
INCFLAGS = -I./ -I../all -I$(VMX_BASE)/include -I$(VMX_BASE)/src -I$(VMX_BASE)/include/ugl
LD_LIBRARY_PATH = -L$(VMX_BASE)/lib/$(CPU)/

.SUFFIXES: .c

.c.o:
	$(CC) $(CFLAGS) $(INCFLAGS) $(DFLAGS) -o $*.o -c $*.c
.s.o:
	$(CC) $(ASMFLAGS) $(INCFLAGS) $(DFLAGS) -o $*.o -x assembler-with-cpp -c $*.s

###############################################################################
# LINK PART
#

OBJECTS = sysALib.o sysLib.o \
	  ../all/usrConfig.o ../all/usrKernel.o ../all/usrMmuInit.o \
          ../all/usrNetwork.o ../all/standTable.o \
          ../all/usrTest.o ../all/fsDemo.o ../all/netDemo.o ../all/uglDemo.o

###############################################################################
# BUILD PART
#

all: image

image:	$(OBJECTS)
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(OBJECTS) $(LD_LIBRARY_PATH) $(LIBS)

clean:
	$(RM) $(OBJECTS) $(OUTPUT)

