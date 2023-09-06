
# where to put everything

BIN_DIR      = ../bin

# where to get everything

UPLIB = ../Uplib/lib/libuplib6.a
UPINC = ../Uplib

#XXL="/usr/X11R6"
XXL=/usr/lib/i386-linux-gnu

#XXI="/usr/X11R6"
XXI=/usr/include/X11

CC       = gcc
CFLAGS   = -O -Wall -pedantic -I$(UPINC) -g
XINCS    = -I$(XXI)/include
XLINKS   = -L$(XXL)/lib -lXmu
INST_DIR = $(HOME)/bin/hp


####################################################
# Common Section
# (edits are unlikely to be necessary below this line)
#
#
#

TARGETS = \
	$(BIN_DIR)/contoursom2unipen

all: $(TARGETS)

contoursom2unipen: $(BIN_DIR)/contoursom2unipen
	@(echo made contoursom2unipen)
$(BIN_DIR)/contoursom2unipen: contoursom2unipen.o 
	$(CC) -o $@ contoursom2unipen.o \
		$(UPLIB) $(VHSLIB) $(XLINKS) -lm -static -static-libgcc

clean:
	\rm -f *.o $(TARGETS)
