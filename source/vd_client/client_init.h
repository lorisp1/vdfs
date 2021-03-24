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

#ifndef _client_init_h
#define _client_init_h

#include <sys/file.h>			/* file operations functions								*/
#include <sys/socket.h>		/* socket communication functions 					*/
#include <sys/types.h>		/* socket types			 												*/
#include <sys/un.h>				/* unix type socket communication functions */
#include <unistd.h>  			/* unix various functions										*/
#include <stdio.h>				/* standard i/o															*/
#include <stdlib.h>				/* standard c																*/

#include "client_conf.h"	/* client configuration defines							*/

/* output string size */ 
#define OUT_STRING_SIZE 100

int client_fd;														/* client file descriptor										*/
char verbose_string[VERBOSE_STRING_SIZE];	/* output string														*/
char output_string[OUT_STRING_SIZE];			/* output string														*/
char err_char;														/* char heading an error message to client	*/

/* Socket initialization function
   PRE: SOCKET_NAME file writable by users
   POST: returns a non-negative client_file descriptor if successfully initialized, -1 otherwise 
	 	and initializes server_socket_addr_ptr and server_unix_address
   PARAMS: ptr to ptr to struct of server's addresses and ptr to server's sockaddr_un structures */
int socket_init (struct sockaddr**,struct sockaddr_un*);

#endif
