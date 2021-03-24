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

#ifndef _aux_client_funct_h
#define _aux_client_funct_h

#include "client_init.h"	/* client initialization functions */

/* Prints on stdout the help screen
   PRE: none
   POST: none
	 PARAMS: none */
void print_help(void);

/* Prints on stdout server's output 
   PRE: client_fd>0 (client connected)
   POST: none
	 PARAMS: none */
void print_server_output(void);

/* Writes on string the arguments passed to the program from start to end
   PRE: string already allocated
   POST: none
	 PARAMS: string, start position, end position*/
void arg_cat(char*,char**,int,int);

/* Reads a line from fd and inserts it into string
   PRE: fd>0
   POST: returns positive value if the string has been successfully read,
	 		0 if the end of the file has been reached, 
			-1 otherwise
	 PARAMS: file descriptor, output string */
int read_ln(int,char*);

#endif
