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

#include "server_init.h"				/* vd server initialization library	*/
#include "server_funct.h"				/* vd server execution functions		*/
#include "command.h"						/* command structure								*/

/* Prints on standard output a list of FreeItem
   PRE: none
   POST: void function (no return)
   PARAMS: fli or flb (more generally a list of FreeItem) */
void print_fl(FreeList);

/* Prints on standard output the filesystem status
   PRE: none
   POST: void function (no return)
   PARAMS: fli and flb (more generally a list of FreeItem) */
void vdfs_status(FreeList,FreeList,int,int);

/* Main VD function
   PRE: none
   POST: n.p. (per il momento)
   PARAM: none */
int main(int argc, char *argv[]) 
{
	/*Socket connection declarations */
	struct sockaddr *server_socket_addr_ptr;																				/* Ptr to ptr to server address 		*/
  struct sockaddr *client_socket_addr_ptr;																				/* Ptr to ptr to client address 		*/
	struct sockaddr_un server_unix_address;																					/* Server Unix address 							*/
  struct sockaddr_un client_unix_address;																					/* Client Unix address 							*/
  int client_address_size;																												/* client_unix_address struct size	*/
	
	/* other declarations */
	char request[CMD_MAX_SIZE+(MAX_TOKENS-1)*(((MAX_FILENAME_SIZE+1)*INODE_NO)+1)];	/* command buffer 									*/
	char* tokens[MAX_TOKENS+1];																											/* array of tokens									*/
	int token_no;																																		/* number of the tokens 						*/
	int exit_status;																																/* return value of execute function	*/
	
	/* WARNING: if you add n functions, you MUST increment the array size of 	*/
	/* n in server_funct.h 																										*/
	command commands[CMD_NO];																												/* array of commands								*/

	/* installs the new SIGINT handler */
	signal(SIGINT,sigint_handler);
	/* Ignores SIGCHLD signal to exclude zombie processes generation */
  signal(SIGCHLD, SIG_IGN);
	
	/* Initialization of the file system, if not successfully done exit with an error */
	if (vdfs_init(&fli, &flb, &freeInodes_no, &freeBlocks_no)==-1) 
	{
		perror("Virtual Disk initialization failed!");
		return EXIT_FAILURE;
	}
	
	/* Initialization of the socket, if not successfully done exit with an error */
	socket_fd=socket_init(&server_socket_addr_ptr,&client_socket_addr_ptr,&server_unix_address,&client_unix_address);
	if (socket_fd==-1) 
	{
		perror("Server connection to socket failed!");
		return EXIT_FAILURE;
	}
	
	/* Initialization of the commands array */
	commands_init(commands);
	
	/* Prints on standard output fli and flb */
	vdfs_status(fli,flb,freeInodes_no,freeBlocks_no);
	
	client_address_size=sizeof(client_unix_address);
	
	/* Alive server loop */
	while (1) 
	{
		/* Waits a connection */
		client_fd=accept(socket_fd,client_socket_addr_ptr,&client_address_size);
		
		if(read_ln(client_fd,request)<0)
			perror("Request receiving failed!");
		
		printf("vd_server: I received this command: %s\n",request);
		
		/* explode the request string in a string array */
		token_no=explode(request,tokens," ");
		if(token_no<1)
		{
			close (client_fd); 
			perror("Fatal error analyzing the request");
			
		}
		
		/* call requsted command with a function pointer */
		exit_status=execute(token_no,tokens,commands);
		if(exit_status==-1)
		{
			close (client_fd); 
			perror("Fatal error executing the request");
			
		}
		else if(exit_status==-2)
		{
			/* write output to client */
			sprintf(output_buffer,"%cvd_server: %s is not a valid command!\n",err_char,tokens[0]);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			
			close (client_fd); 
			perror("Fatal error executing the request");
		}
			
		/* Close socket */
		close (client_fd); 
		vdfs_status(fli,flb,freeInodes_no,freeBlocks_no);
	}
}

/* Prints on standard output a list of FreeItem
   PRE: none
   POST: void function (no return)
   PARAMS: fli or flb (more generally a list of FreeItem) */
void print_fl(FreeItem* fl)
{
	if (fl==NULL)
		return;
	printf("%d ", fl->addr);
	print_fl(fl->next);
}

void vdfs_status(FreeList fli,FreeList flb,int freeInodes_no,int freeBlocks_no)
{
	printf("List of free i-nodes:\n");
	print_fl(fli);
	printf("\n\n");
	printf("List of free data blocks:\n");
	print_fl(flb);
	printf("\n\n");
	printf("Number of free i-nodes: %d\nNumber of free data blocks: %d\n\n", freeInodes_no, freeBlocks_no);
	printf("\nI-node pointer size= %d\n",inode_ptr_size);
}

