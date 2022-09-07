#
CFLAGS= -g
LIBES = ../stlib/stlib.a  \
  ../../fs/port/port.a ../../fs/poclb/poclb.a \
  ../../fs/clib/clib.a ../../fs/rtelb/rtelb.a /usr/lib/x86_64-linux-gnu/libm.a ../../fs/poclb/poclb.a
#
OBJECTS =\
stqkr.o getgps.o effcal.o effpcal.o effrx.o effwx.o efflo.o 
#
../bin/stqkr: $(OBJECTS)
	cc -g -o ../bin/stqkr $(OBJECTS) $(LIBES) -lm
	strip ../bin/stqkr
#
stqkr:
	rm -f ../bin/stqkr
	make
