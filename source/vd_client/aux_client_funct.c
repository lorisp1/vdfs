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

#include "aux_client_funct.h"	/* auxiliary client functions */


/* Prints on stdout the help screen
   PRE: none
   POST: none
	 PARAMS: none */
void print_help(void)
{
	printf("\nusage: vd_client [-v] {command} [argument1],[argument2]...\n");
	printf("Sends {command} with its arguments to vd_server and writes on stdout or sterr the result.\n\n");
	printf("Mandatory arguments to long options are mandatory for short options too.\n");
	printf("-h, --help\tdisplay this help and exit\n");
	printf("-v, --verbose\texecute with a more verbose output\n\n");
}

/* Prints on stdout server's output,stderr if line starts with err_char defined in client init.c 
   PRE: client_fd>0 (client connected)
   POST: none
	 PARAMS: none */
void print_server_output(void)
{
	int read_result;	/* return value of read_ln	*/	
	
	read_result=read_ln(client_fd,output_string);
	
	if(read_result<0)
		exit(EXIT_FAILURE);
	
	if(read_result==0)
		return;
	
	/* if the string is an error writes it on stderr */
	if(strchr(output_string,err_char)==output_string)
	{
		/* move cursor into append mode */
		lseek(STDERR_FILENO,0,SEEK_END);
		write(STDERR_FILENO,&(output_string[1]),strlen(output_string));
	}
	/* if the string is not an error writes it on stdout */
	else
	{
		/* move cursor into append mode */
		lseek(STDOUT_FILENO,0,SEEK_END);
		write(STDOUT_FILENO,output_string,strlen(output_string)+1);
	}
	/* print recursively other line[s] */
	print_server_output();
}

/* Writes on string the arguments passed to the program from start to end
   PRE: string already allocated, start<=end
   POST: none
	 PARAMS: string, start position, end position*/
void arg_cat(char *string,char** array,int start,int end)
{	
	int i;	/* generic counter */
	
	/* empty string initialization */
	strcpy(string,"");
	
	for(i=start;i<end;i++)
	{
		strcat(string,array[i]);
		strcat(string," ");
	}
}

/* Reads a line from fd and inserts it into string
   PRE: fd>0
   POST: returns positive value if the string has been successfully read,
	 		0 if the end of the file has been reached, 
			-1 otherwise
	 PARAMS: file descriptor, output string */
int read_ln(int fd,char* string)
{
	int n;	/* read string lenght */
	
	n=read(fd,string,sizeof(char));
	
	if(n<=0 || *string=='\0')
	{
		*string='\0';
		return n;
	}
	return read_ln(fd,++string);
}

