#
#    Automatically Generated Makefile Include
#    Sat Nov 14 17:19:04 EST 2015
#
#
#    Configured for Darwin

CC=cc
MACHDEP_CFLAGS=-Ofast
#	CMU Learning Benchmark Archive
#	Parse Library
#
#	v2.0
#	Matt White  (mwhite+@cmu.edu)
#	3/5/95
#
#	QUESTIONS/COMMENTS:  neural-bench@cs.cmu.edu
#
#  This is the makefile for compiling the parse library.  Replace the compiler
#  named in 'CC' with whatever ANSI compatible compiler you are most 
#  comfortable with and then type 'make' on the command line'.


CFLAGS = $(MACHDEP_CFLAGS) -O -I$(INSTALL_DIR)/include

libparse.a:	parse.o
	'rm' -f libparse.a
	ar r libparse.a parse.o
	ranlib libparse.a

parse.o:	parse.c parse.h
	$(CC) $(CFLAGS) -c parse.c

install:	libparse.a parse.h
		'rm' -f $(INSTALL_DIR)/lib/libparse.a
		'rm' -f $(INSTALL_DIR)/include/parse.h
		'cp' libparse.a $(INSTALL_DIR)/lib
		'cp' parse.h $(INSTALL_DIR)/include

clean:
	-'rm' -f *.o *.u *~ *.core core