CFLAGS=-Wall


all: vd_client

install:
	make -C ../ install
	
clean:
	rm -f *.o
	
uninstall:
	make -C ../ uninstall

vd_client:	client_init.o vd_client.o aux_client_funct.o
vd_client.o:	client_init.h aux_client_funct.h
client_init.o:	client_conf.h


