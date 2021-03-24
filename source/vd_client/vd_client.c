/***************************************************************************
 *   Copyright (C) 2006 by Manzo Team                                      *
 *   mdarios@dsi.unive.it                                                  *
 *   lpozzobo@dsi.unive.it                                                 *
 *   jzambon@dsi.unive.it                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "client_init.h"				/* vd client initialization library	*/
#include "aux_client_funct.h"		/* auxiliary client functions				*/

/* Main VD client function
   PRE: none
   POST: returns EXIT_SUCCESS if vd_server answer the request, EXIT FAILURE otherwise
   PARAM: none (per il momento, poi saranno cose del tipo "start" o "stop") */
int main(int argc, char *argv[]) 
{
	/*Socket connection declarations */
	struct sockaddr *server_socket_addr_ptr;	/* Ptr to ptr to server address 		*/
  struct sockaddr_un server_unix_address;		/* Server Unix address 							*/
  int server_address_size;									/* client_unix_address struct size	*/
	int connection_result;										/* result of the connection try			*/
	int tries=1;															/* number of connection tries				*/
	char request[MAX_REQUEST_SIZE];						/* max size of the request					*/
	
	/* char heading a server error msg MUST be as server's one										*/
	err_char='!';
	
	/* if -h or --help option is passed prints help and exit with EXIT_SUCCESS */
	if(argc==1 || (!strcmp(argv[1],"-h")) || (!strcmp(argv[1],"--help")))
	{
		if(argc==2)
		{
			print_help();
			exit(EXIT_SUCCESS);
		}
		/* write sintax error on stderr end exit with EXIT_FAILURE*/
		sprintf(output_string,"vd_client: command sintax error.\nusage: vd_client [-v] {command} [argument1],[argument2]...\nOr try \"vd_client -h\" or \"vd_client --help\"\n");
		lseek(STDERR_FILENO,0,SEEK_END);
		write(STDERR_FILENO,output_string,strlen(output_string)+1);
		exit(EXIT_FAILURE);
	}
	
	/* Client file descriptor, *server_socket_addr_ptr and server_unix_address inizialization */
	client_fd=socket_init(&server_socket_addr_ptr,&server_unix_address);
	if(client_fd==-1)
	{
		perror("vd_client");
		return EXIT_FAILURE;
	}
	server_address_size=sizeof(server_unix_address);
	/* Server connection*/
	if(!strcmp(argv[1],"-v") || !strcmp(argv[1],"--verbose"))
		strcpy(verbose_string,"\nTrying to connect to VD Server System...\n");
	do 
	{
		connection_result=connect(client_fd,server_socket_addr_ptr,server_address_size);
		if(connection_result==-1)
			sleep(SLEEP_TIME); /* If the connection fails, waits then retry */
		tries++;
	}while(connection_result==-1 && tries<=MAX_CONN_TRIES);
	if(connection_result==-1)
	{
		perror("vd_client");
		return EXIT_FAILURE;
	}
	
	if(!strcmp(argv[1],"-v") || !strcmp(argv[1],"--verbose"))
	{
		/* verbose output */
		strcat(verbose_string,"Connection estabilished!\n");
		arg_cat(request,argv,2,argc);
		
		/* write verbose output on stout */
		lseek(STDOUT_FILENO,0,SEEK_END);
		write(STDOUT_FILENO,verbose_string,strlen(verbose_string)+1);
	}
	else
		arg_cat(request,argv,1,argc);
	
	
	/* Send the request to the server */
 	write(client_fd,request,strlen(request)+1);
  
  /* Waits for the server response and prints it on stdout or stderr */
	print_server_output();
	
	return EXIT_SUCCESS;	
}

