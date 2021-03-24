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

#ifndef _server_funct_h
#define _server_funct_h

/* WARNING!! This definition sets the size of the command array. 							*/
/* If you add n functions you'll have add to it n.														*/ 
/* Don't change it if you're unsure!																					*/
#define CMD_NO 8

#include <sys/file.h>

#include <string.h>									/* string functions												*/
#include <unistd.h>									/* general unix functions									*/


#include "server_conf.h"						/* server configuration defines						*/
#include "command.h"								/* command structure											*/
#include "server_init.h"						/* server initialization functions 				*/
#include "aux_server_funct.h"				/* auxiliary server functions 						*/

char vd_string[5];									/* string identifying an operation on VD	*/




/* Lists the content of a directory
   PRE: none
   POST: writes on the client file descriptor the command output 
	 	and returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int ls(int,char**);

/* Makes an hard link to the file, or a soft one if -s option is passed
   PRE: none
   POST: writes on the client file descriptor the command output 
	 	and returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int ln(int,char**);

/* Makes an empty directory
   PRE: none
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int _mkdir(int,char**);

/* Copies a file into another path
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int cp(int,char**);

/* Softly deletes the desired file or link
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int del(int,char**);

/* Undoes del
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int undel(int,char**);

/* Definitively removes a deleted file
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int purge(int,char**);

/* Definitively removes a file (del+purge)
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int rm(int,char**);


#endif

