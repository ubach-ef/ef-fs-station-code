#
CFLAGS= 
LIBES = ../../fs/poclb/poclb.a ../../st/stlib/stlib.a ../../fs/clib/clib.a\
../../fs/rtelb/rtelb.a -lm -lnsl
#
OBJECTS = antcn.o rad2str.o  # antcn_rpc_client.o antcn_rpc_clnt.o
#
../bin/antcn: $(OBJECTS)
	cc  -o ../bin/antcn $(OBJECTS) $(LIBES)
	strip ../bin/antcn
#
antcn:
	rm -f ../bin/antcn 
	make


