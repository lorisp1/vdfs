CFLAGS=-Wall


all: vd_server

install:
	make -C ../ install
	
clean:
	rm -f *.o
	
uninstall:
	make -C ../ uninstall

vd_server:		server_init.o mytime.o list.o vd_server.o server_funct.o aux_server_funct.o
vd_server.o:		server_init.h command.h server_funct.h
server_init.o:		server_conf.h mytime.h command.h server_funct.h list.h
server_funct.o:		server_conf.h command.h server_init.h aux_server_funct.h
aux_server_funct.o:	server_conf.h command.h server_init.h 
mytime.o:		server_conf.h
list.o:			server_conf.h

