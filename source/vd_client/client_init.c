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

#include "client_init.h"			/* client initialization functions 	*/
#include "aux_client_funct.h"	/* auxiliary client functions				*/

/* Socket initialization function
   PRE: SOCKET_NAME file writable by users
   POST: returns a non-negative client_file descriptor if successfully initialized, -1 otherwise 
	 	and initializes server_socket_addr_ptr and server_unix_address
   PARAMS: ptr to ptr to struct of server's addresses and ptr to server's sockaddr_un structures */
int socket_init (struct sockaddr** server_socket_addr_ptr,struct sockaddr_un *server_unix_address)
{
	/* Server socket address pointers initialization */
	*server_socket_addr_ptr=(struct sockaddr*)server_unix_address;
 
	/* PF_UNIX socket initialization */
	client_fd=socket(PF_UNIX,SOCK_STREAM,DEFAULT_PROTOCOL);
	if(client_fd<0)
	{
		printf(" %d ",client_fd);
		client_fd=-1;
		return client_fd;
	}
  (*server_unix_address).sun_family=PF_UNIX;					/* Domain */
  strcpy((*server_unix_address).sun_path,SOCKET_NAME);/* Name */

	return client_fd;
}

