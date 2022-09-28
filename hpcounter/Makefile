CFLAGS=-g
LIBS = ../../fs/port/port.a ../stlib/stlib.a  -lfl
OBJECTS = hpcounter.o
#
../bin/hpcounter: hpcounter.o $(OBJECTS)
	cc -g -o ../bin/hpcounter $(OBJECTS) $(LIBS)
	strip ../bin/hpcounter
#
hpcounter:
	rm -f ../bin/hpcounter
	make
