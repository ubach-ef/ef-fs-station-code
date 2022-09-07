#
CFLAGS= 
LIBES = ../stlib/stlib.a ../../fs/poclb/poclb.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm
#
OBJECTS = antrcv_multi.o 
#
../bin/antrcv_multi: $(OBJECTS)
	cc  -o ../bin/antrcv_multi $(OBJECTS) $(LIBES)
	strip ../bin/antrcv_multi
#
antrcv_multi:
	rm -f ../bin/antrcv_multi 
	make
