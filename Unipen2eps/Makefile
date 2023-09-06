# Template to make a Makefile using our 'configure' script
# Louis Vuurpijl & Lambert Schomaker, July '96

# where to put everything

BIN_DIR      = ../bin

# where to get everything

UPLIB = ../Uplib/lib/libuplib6.a
UPINC = ../Uplib

################### HP SETTINGS ###################
CC       = gcc
CFLAGS   = -O -Wall -pedantic -I$(UPINC) -g
XINCS    = -I/usr/X11R6/include
XLINKS   = -L/usr/X11R6/lib -lXmu
INST_DIR = $(HOME)/bin/hp

################### DEC SETTINGS ###################
#ULTRIX#CC       = gcc
#ULTRIX#CFLAGS   = -O -I$(UPINC)
#ULTRIX#INST_DIR = $(HOME)/bin/dec

################### SUN Solaris 5 SETTINGS ###################
#SunOS5#CC       = gcc
#SunOS5#CFLAGS   = -O -I$(UPINC) -DSUN_SOL
#SunOS5#INST_DIR = $(HOME)/bin/sun

################### SUN SunOs 4 SETTINGS ###################
#SunOS4#CC       = gcc
#SunOS4#CFLAGS   = -g -I$(UPINC)
#SunOS4#XINCS    = -I/usr/local/X11/include
#SunOS4#XLINKS   = -L/usr/local/X11/lib -lXmu
#SunOS4#INST_DIR = $(HOME)/bin/sun

################### SGI SETTINGS ###################
#IRIX#CC        = cc
#IRIX#CFLAGS    = -O -I$(UPINC)
#IRIX#INST_DIR  = $(HOME)/bin/sgi

################### AIX SETTINGS ###################
#AIX#CC         = cc -Aa
#AIX#CFLAGS     = -O -Daix -I$(UPINC)
#AIX#INST_DIR   = $(HOME)/bin/aix

################### LINUX SETTINGS ###################
#Linux#CC       = gcc
#Linux#CFLAGS   = -O -I$(UPINC)
#Linux#INST_DIR = $(HOME)/bin/linux

####################################################
# Common Section
# (edits are unlikely to be necessary below this line)
#
#
#

TARGETS = \
	$(BIN_DIR)/unipen2eps

all: $(TARGETS)

unipen2eps: $(BIN_DIR)/unipen2eps
	@(echo made unipen2eps)
$(BIN_DIR)/unipen2eps: unipen2eps.o $(DU_OBJECTS) $(IM_OBJ)
	$(CC) -o $@ unipen2eps.o \
		$(UPLIB) $(VHSLIB) $(XLINKS) -lm -static -static-libgcc
clean:
	\rm -f *.o $(TARGETS)
