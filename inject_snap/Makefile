# fs/oprin/Makefile  --  of UFS 'oprin'
# $Id$

# Copyright (C) 1992 NASA GSFC, Ed Himwich. (weh@vega.gsfc.nasa.gov)
# Copyright (C) 1995 Ari Mujunen. (amn@nfra.nl, Ari.Mujunen@hut.fi)

# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License.
# See the file 'COPYING' for details.

# $Log$

# We want the built-in default GNU Make actions to use 'gcc' as our C compiler.
CC = gcc -g

# In all default compilation commands involving the C preprocessor,
# use these preprocessor control flags (include dirs, defines).
CPPFLAGS = 

# Flags for default $(CC) compilations.
CFLAGS = -Wall -pipe -O2 -fomit-frame-pointer
# debugging CFLAGS =  -Wall -pipe -g

# What objects and libraries are linked together.
OBJECTS = inject_snap.o 
LIBS = ../../fs/flib/flib.a ../../fs/fclib/fclib.a ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a
# (Note: ":=" evaluates '$(xxx)'s on right-hand side immediately whereas
#        "=" stores the '$(xxx)' var references and evaluates them again
#            whenever the left-hand-side variable is used.)

# The main target, 'oprin'.  Both new/updated object files and libraries
# cause relinking; if libraries are missing, this makefile cannot
# create them on its own, it says "don't know how to make $(LIBS)".
../bin/inject_snap : $(OBJECTS) $(LIBS)
	gcc -o ../bin/inject_snap $(OBJECTS) $(LIBS) 
	strip ../bin/inject_snap

inject_snap.o: inject_snap.c

clean:
	rm -rf *~ *.o ../bin/inject_snap
