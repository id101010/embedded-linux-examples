# Embedded-Linux (BTE5446)
# Project: Basic framebuffer Exercise 1.0
# Version: 1.0
# File:    Makefile
# Date:    04.10.2013
# Author   Martin Aebersold (AOM1)
#
# Last Modifications: V1.0, AOM1, 04.10.2013
# Initial release

SHELL = /bin/bash

# Include the ARCH (host or target) enviroments variables
# make HOST=1 
ifdef HOST
 include make_env_host
else
 include make_env_target
endif

# Tool names
TARGET_ARCH	= ${TARGET}-
AS		= $(TARGET_ARCH)as
AR 		= $(TARGET_ARCH)ar
CC 		= $(TARGET_ARCH)gcc
CPP 		= $(TARGET_ARCH)g++
LD 		= $(TARGET_ARCH)ld
NM 		= $(TARGET_ARCH)nm
OBJCOPY 	= $(TARGET_ARCH)objcopy
OBJDUMP 	= $(TARGET_ARCH)objdump
RANLIB 		= $(TARGET_ARCH)ranlib
READELF 	= $(TARGET_ARCH)readelf
SIZE 		= $(TARGET_ARCH)size
STRINGS 	= $(TARGET_ARCH)strings
STRIP 		= $(TARGET_ARCH)strip
export	AS AR CC CPP LD NM OBJCOPY OBJDUMP RANLIB READELF SIZE STRINGS STRIP

# Build settings
CFLAGS		= ${EXTRA_CFLAGS} -g -gdwarf-2 -Wall
HEADER		= -I${LOCAL_INC} -I${SYSTEM_INC}
LIBS		= -lm -lrt
LDFLAGS 	= $(LIBS) -Wl,-Map=${EXEC_NAME}.map -L${LOCAL_LIB} -L${SYSTEM_LIB}

# Name of Executable
EXEC_NAME	= examlib

# Installation variables like scripts images etc.
SHELL_SCRIPT	= 
IMAGES		=
INSTALL		= install
INSTALL_DIR	= ${TARGET_ROOTFS}/usr/local/bin

# Files needed for the build
OBJS 		= ${EXEC_NAME}.o

# Make rules
all:		${EXEC_NAME}

${EXEC_NAME}:	$(OBJS)
		$(CC) -o $(EXEC_NAME) ${OBJS} $(LDFLAGS)

%.o: %.c
		$(CC) -c $(HEADER) $(CFLAGS) $<

install:	${EXEC_NAME}
		test -d $(INSTALL_DIR) || $(INSTALL) -d -m 755 $(INSTALL_DIR)
		$(INSTALL) -m 755 $(EXEC_NAME) $(SHELL_SCRIPT) $(IMAGES) $(INSTALL_DIR)

asm:		
		@if [ -a $(EXEC_NAME) ]; then ${OBJDUMP} -C -D -S -l $(EXEC_NAME) > $(EXEC_NAME).S; \
		else echo "You must first run make!"; fi;
doc:
		doxygen

clean:
		rm -f *.o 
		rm -f $(EXEC_NAME)
		rm -f *.map
		rm -rf doc
distclean:
		rm -f *~
		rm -f *.S
		rm -f *.map
		rm -f *.o $(EXEC_NAME) 
		rm -r doc

