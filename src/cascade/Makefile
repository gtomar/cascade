#
#    Automatically Generated Makefile Include
#    Sat Nov 14 17:19:04 EST 2015
#
#
#    Configured for Darwin

CC=cc
MACHDEP_CFLAGS=-Ofast

#	CMU Cascade Neural Network Simulator (CNNS)
#
#	v1.0
#	Matt White (mwhite+@cmu.edu)
#	May 28, 1995
#
#	This is the make file for the CMU Cascade Neural Network
#	Simulator.  It is designed to be used with an ANSI and IEEE
#	compliant system.  If possible, use your system's native
#	compiler (cc) since this is likely to give the best
#	optimizations.  This is not always possible because,
#	especially in older systems, some system compilers are not 
#	ANSI compliant.  In these cases, the recommended compiler is
#	'gcc'.



CFLAGS = $(MACHDEP_CFLAGS) -O -I$(INSTALL_DIR)/include 
LFLAGS = $(MACHDEP_LFLAGS) -L$(INSTALL_DIR)/lib -lparse -ltoolkit -lm

OBJS = cascade.o cascor.o cascade2.o util.o cache.o init.o \
interface.o display.o query.o

cascade:	$(OBJS)
	$(CC) $(CFLAGS) -o cascade $(OBJS) $(LFLAGS)

cascade.o:	cascade.c cascade.h
cascor.o:	cascor.c cascade.h
cascade2.o:	cascade2.c cascade.h
util.o:		util.c cascade.h
cache.o:	cache.c cascade.h
init.o:		init.c cascade.h
interface.o:	interface.c cascade.h
display.o:	display.c cascade.h
query.o:	query.c cascade.h

install:	cascade
		cp cascade $(INSTALL_DIR)/bin

clean:
	'rm' -f core *.o *~ #* *.u
