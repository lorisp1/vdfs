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

#ifndef _server_init_h
#define _server_init_h

#include <sys/file.h>				/* file operations functions								*/
#include <sys/socket.h>			/* socket communication functions 					*/
#include <sys/types.h>			/* socket types			 												*/
#include <sys/un.h>					/* unix type socket communication functions */
#include <unistd.h>  				/* general unix functions										*/
#include <stdio.h>					/* standard i/o															*/
#include <stdlib.h>					/* standard c																*/
#include <signal.h>					/* signal functions 												*/
#include <string.h>					/* string functions													*/

#include "server_conf.h"		/* server configuration defines							*/
#include "mytime.h"					/* time and date functions									*/
#include "command.h"				/* command structure												*/
#include "server_funct.h"		/* vd server initialization library					*/
#include "list.h"						/* list functions														*/

/* token number of a directory inode*/
#define DIR_INODE_TOKEN_NO 6
/* output string size */ 
#define OUT_STRING_SIZE 100

char err_char;												/* char heading an error message to client	*/
char output_buffer[OUT_STRING_SIZE];	/* output buffer 														*/

/*File system declarations */
FreeList fli;						/* List of free i-nodes 				*/
FreeList flb;						/* List of free data blocks 		*/
int freeInodes_no;			/* Number of free i-nodes 			*/
int freeBlocks_no;			/* Number of free data blocks 	*/
int inode_ptr_size;			/* size of an i-node pointer		*/
int block_ptr_size;			/* size of a data block pointer	*/
int max_files_per_dir;	/* max elements in a directory	*/

int socket_fd;					/* socket file descriptor 			*/
int client_fd;					/* client file descriptor				*/

/* File system initialization function
   PRE: VDFS file writable by users
   POST: returns a non-negative file descriptor if successfully initialized, -1 otherwise
   PARAMS: fli*, flb*, freeItems*, freeBlocks* */
int vdfs_init (FreeList*, FreeList*, int*, int*);

/* Socket initialization function
   PRE: SOCKET_NAME file writable by users
   POST: returns a non-negative socket-file descriptor if successfully initialized, -1 otherwise
	 	and initializes server_socket_addr_ptr, client_socket_addr_ptr, server_unix_address and client_unix_address
   PARAMS: ptr to ptr to struct of server's and client's addresses,ptr to server's and client's sockaddr_un structures */
int socket_init (struct sockaddr**,struct sockaddr**,struct sockaddr_un*,struct sockaddr_un*);

/* Commands array initialization function
   PRE: the command array size MUST be big enough to contain all the commands! 
   POST: writes on the commands array every function
	 PARAMS: the command array (of type "command")*/
void commands_init (command*);

/* Calculates the max number of elements in a dir
   PRE: none
   POST: returns the max number of elements in a dir
   PARAMS: none */
int max_files_per_dir_calc(void);

/* SIGINT signal handler
   PRE: none
   POST: none
	 PARAMS: signal number */
void sigint_handler(int); 

#endif
