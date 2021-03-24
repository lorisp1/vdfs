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


/* File system initialization function
   PRE: VDFS file writable by users
   POST: returns a non-negative file descriptor if successfully initialized, -1 otherwise
   PARAMS: fli*, flb*, freeItems*, freeBlocks* */
int vdfs_init (FreeList* fli,FreeList* flb, int* freeInodes, int* freeBlocks) 
{
	char buffer[INODE_SIZE];	/* inode buffer			*/
	char date[DATE_SIZE];			/* date string			*/
	int i;										/* generic counter	*/
	
	/* Free blocks list and Free i-nodes list definition */
	freeInodes_no=0;
	freeBlocks_no=0;
	*fli=NULL;
	*flb=NULL;
	
	/* inode pointer size(chars) (used by directories) */
	inode_ptr_size=number_sizeof(INODE_NO)+1;
	/* data block pointer size(chars) (used by directories) */
	block_ptr_size=number_sizeof(BLOCK_NO);
	/* max elements in a directory (used by directories) */
	max_files_per_dir=max_files_per_dir_calc();

	/* VDFS file creation */
	int file=open(VDFS, O_WRONLY|O_CREAT|O_TRUNC, 0755);
	
	/* Checks if the file has been successfully created */
	if (file==-1)
		return file;
	
	/* Root dir initialization */
	getdate(date);
	/* i-node entry creation																							*/
	/* sintax="<type>:<date>:<hard link count>:<first block ptr>:<size(BLOCKS)>:<ordered data block pointer list> */
	sprintf(buffer,"d:%s:%d:%d:%d:",date,1,0,1);
	
	/* Writes in VDFS file the root i-node */
	write(file,buffer,INODE_SIZE);
	
	/* I-nodes free list (fli) initialization */
	
	for (i=1;i<INODE_NO;i++)
	{
		if (!enqueue(fli,i,&freeInodes_no))
			return -1;
	}
	
	/* Free blocks list (flb) initialization */
	for (i=1;i<BLOCK_NO;i++)
	{
		if (!enqueue(flb,i,&freeBlocks_no))
			return -1;
	}
	
	/* Vdfs file size adjustment */
	lseek(file,((INODE_NO-1)*INODE_SIZE)+(BLOCK_NO*BLOCK_SIZE)-1,SEEK_CUR);
	write(file,&(buffer[strlen(buffer)+1]),sizeof(char));
	
	close(file);
	return file;
}

/* Socket initialization function
   PRE: SOCKET_NAME file writable by users
   POST: returns a non-negative socket-file descriptor if successfully initialized, -1 otherwise 
	 	and initializes server_socket_addr_ptr, client_socket_addr_ptr, server_unix_address and client_unix_address
   PARAMS: ptr to ptr to struct of server's and client's addresses,ptr to server's and client's sockaddr_un structures */
int socket_init (struct sockaddr** server_socket_addr_ptr,struct sockaddr** client_socket_addr_ptr,struct sockaddr_un *server_unix_address,struct sockaddr_un *client_unix_address)
{
	/* Server and client socket address pointers initialization */
	*server_socket_addr_ptr=(struct sockaddr*)server_unix_address;
  *client_socket_addr_ptr=(struct sockaddr*)client_unix_address;
  
	if((socket_fd=socket(PF_UNIX,SOCK_STREAM,DEFAULT_PROTOCOL))<0)
	{
		socket_fd=-1;
		return socket_fd;
	}
	
	/* PF_UNIX socket initialization */
	(*server_unix_address).sun_family=PF_UNIX;					/* Domain */
  strcpy((*server_unix_address).sun_path,SOCKET_NAME);/* Name */
  unlink(SOCKET_NAME); 																/* Delete previous socket if not deleted in the previous session */
  if((bind(socket_fd,*server_socket_addr_ptr,sizeof(*server_unix_address)))<0)
	{
		socket_fd=-1;
		return socket_fd;
	}
	listen (socket_fd,MAX_CONNECTIONS); /* Max pending connection number */
	
	return socket_fd;
}

/* Commands array initialization function
   PRE: the command array size MUST be big enough to contain all the commands! 
   POST: writes on the commands array every function
	 PARAMS: the command array (of type "command")*/
void commands_init (command *commands)
{
	/* string identifying an operation on VD initialization		*/
	strcpy(vd_string,"VD:/");
	
	/* char heading an error message to client initialization	*/
	/* MUST be as client's one																*/
	err_char='!';
	
	/* "ls" initialization */
	strcpy(commands[0].cmd_name,"ls");
	commands[0].funct=ls;
	
	/* "ln" initialization */
	strcpy(commands[1].cmd_name,"ln");
	commands[1].funct=ln;
	
	/* "mkdir" initialization */
	strcpy(commands[2].cmd_name,"mkdir");
	commands[2].funct=_mkdir;
	
	/* "cp" initialization */
	strcpy(commands[3].cmd_name,"cp");
	commands[3].funct=cp;
	
	/* "del" initialization */
	strcpy(commands[4].cmd_name,"del");
	commands[4].funct=del;
	
	/* "undel" initialization */
	strcpy(commands[5].cmd_name,"undel");
	commands[5].funct=undel;
	
	/* "purge" initialization */
	strcpy(commands[6].cmd_name,"purge");
	commands[6].funct=purge;
	
	/* "rm" initialization */
	strcpy(commands[7].cmd_name,"rm");
	commands[7].funct=rm;
	
	return;
}

/* Calculates the max number of elements in a dir
   PRE: none
   POST: returns the max number of elements in a dir
   PARAMS: none */
int max_files_per_dir_calc(void)
{
	return (BLOCK_SIZE / (MAX_FILENAME_SIZE + number_sizeof(INODE_NO)));
}

/* SIGINT signal handler
   PRE: none
   POST: none
	 PARAMS: signal number */
void sigint_handler(int signum) 
{
  /* close and delete socket */
	close(socket_fd);
  unlink(SOCKET_NAME);
  
	/* write output to client */
	sprintf(output_buffer,"Server down!\n");
	write(client_fd,output_buffer,strlen(output_buffer)+1);
	
	/* delete filesystem */
	unlink(VDFS);
	
	exit(EXIT_SUCCESS);
}

