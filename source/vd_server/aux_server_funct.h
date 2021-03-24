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

#ifndef _aux_server_funct_h
#define _aux_server_funct_h

#include <sys/stat.h>			/* file stats 											*/
#include <sys/file.h>			/* file unix functions							*/
#include <sys/types.h>		/* native types 										*/
#include <unistd.h>				/* various unix functions 					*/
#include <errno.h>				/* error codes library 							*/

#include "server_conf.h"	/* server configuration defines			*/
#include "command.h"			/* command structure								*/
#include "server_init.h"	/* server initialization functions	*/

/* Explodes the string in a word array and returns the token number
   PRE: string must have already been initialized
   POST: token array contains the words of the string
   PARAMS: string, token array, separator character */
int explode(char*,char**,char*);

/* Add to the previous ordered directory content position list the new file position
   PRE: newfile not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: string of the block content, its size, array of strings(numbers) ordered_permutation, 
	 	string new_file, string new ordered permutation*/
int new_ordered_permutation(char*,int,char**,char*,char*);

/* Executes the function pointed by fn_ptr
   PRE: params not null
   POST: returns the exit status of the called function, -2 if the function is not defined
	 PARAMS: parameter number, token array */
int execute(int param_no,char** params,command *);

/* Returns data block address of path directory
   PRE: string must have already been initialized
   POST: returns data block address (>0) if directory exists, -1 otherwise
   PARAMS: directory path (eg. fo del VD:/dir/file use cd("VD:/dir")) */
int cd(char*);

/* ***Internal function, not to use out of this file*** */
int __recursive_cd(char** path_tokens,int dir_block,int depth);

/* Find a file in a directory
   PRE: i-node number passed is a valid directory i-node
   POST: returns the position of the file named filename in current dir, -1 if it doesn't exist
   PARAMS:  name of file to find, directory inode*/
int find_in_dir(char*,int);

/* Returns in destination array the sampled block of a directory
   PRE: none
   POST: void
   PARAMS: string containing a dir block, array of strings, number of filled parts (elements in dir)
	 		string array containing the block pointers of the dir */
void string_sample(char*,char**,int,char **);

/* Copies a file from VD filesytem to OS filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path, string os_path */
int cp_from_vd_to_os(char*,char*);

/* Copies a file from OS filesytem to VD filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path, string os_path */
int cp_from_os_to_vd(char*,char*);

/* Copies a file from VD filesytem to VD filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path_from, string vd_path_to */
int cp_from_vd_to_vd(char*,char*);

/* Calculates the size(digits) of the number
   PRE: none
   POST: returns the size of the passed number
   PARAMS: none */
int number_sizeof(int);

/* Creates an hard link from VDFS to VDFS
   PRE: ??????
   POST: ??????
   PARAMS: ???? */
int ln_hard(char*,char*);

/* Creates an hard link from VDFS to VDFS
   PRE: ??????
   POST: ??????
   PARAMS: ???? */
int ln_soft(char *string1,char *string2);

/* Writes on array string the sub-block pointers of a directory and the 
		position of the pointers list begininng in the dir inode, NULL if directory is empty
   PRE: none
   POST: returns the number of pointers, -1 if failed (for example inode is a file) 
   PARAMS: string dir_inode, string array of sub-block pointers, pointer to the offset */
int get_sub_block_ptrs(char*,char**,int*);

/* Reads a line from fd and inserts it into string
   PRE: fd>0
   POST: returns positive value if the string has been successfully read,
	 		0 if the end of the file has been reached, 
			-1 otherwise
	 PARAMS: file descriptor, output string */
int read_ln(int,char*);

#endif
