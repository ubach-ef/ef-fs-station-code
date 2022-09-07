CFLAGS=-g
LIBS = /usr2/fs/port/port.a /usr2/st/stlib/stlib.a  -lfl
OBJECTS = hpcounter.o
#
../bin/hpcounter: hpcounter.o $(OBJECTS)
	cc -g -o ../bin/hpcounter $(OBJECTS) $(LIBS)
	strip ../bin/hpcounter
#
hpcounter:
	rm -f ../bin/hpcounter
	make
