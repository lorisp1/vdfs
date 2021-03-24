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

#include "server_funct.h"	/* vd server execution functions */


/* Lists the content of a directory
   PRE: none
   POST: writes on the client file descriptor the command output 
	 	and returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int ls(int param_no,char** params)
{
	int vdfs_fd;																								/* file descriptor of VDFS 																							*/
	int parent_inode;																						/* pointer to parent dir inode 																					*/
	int vd_path_index;																					/* index indicating the position of the vd_path on params string array	*/
	char vd_original_path[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the originale vd_path															*/
	char parent_inode_buffer[INODE_SIZE];												/* string containing parent inode																				*/
	char parent_block_buffer[BLOCK_SIZE];												/* string containing parent data block																	*/
	char *sub_blocks_ptrs[max_files_per_dir];										/* string array containig the sub-blocks pointers of a dir							*/
	char sub_block[MAX_FILENAME_SIZE+inode_ptr_size+2];					/* string containig a sub-block																					*/
	int offset;																									/* the offset from the dir inode start where the pointers list begins		*/
	int pointers_no;																						/* number of the directory pointers																			*/
	int i;																											/* generic counter																											*/
	char status;																								/* - if file is deleted, + if file is not deleted												*/
	char type[1];																								/* type of the element to list																					*/
	
	/* Parameter number control */
	switch(param_no)
	{
		case 2:
		{
			/* set the index of vd_path to 1 */
			vd_path_index=1;
		}break;
		case 3:
		{
			/* if option -a is passed, set the index of vd_path to 2 */
			if(!strcmp(params[1],"-a"))
				vd_path_index=2;
			else
			{
				/* write error output to client */
				sprintf(output_buffer,"%cls: usage: ls [-a] {path/to/list}\n",err_char);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
		}break;
		default:
		{
			/* write error output to client */
			sprintf(output_buffer,"%cls: usage: ls [-a] {path/to/list}\n",err_char);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	
	/* test if vd_path starts with vd_string */
	if(!(strstr(params[vd_path_index],vd_string)==params[vd_path_index]))
	{
		/* write error output to client */
		sprintf(output_buffer,"%cls: %s is not a valid VDFS path!\nIt must start with %s\n",err_char,params[vd_path_index],vd_string);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* open vdfs file in read only */
	vdfs_fd=open(VDFS,O_RDONLY);
	if(vdfs_fd==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cls: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save original vd_path string */
	strcpy(vd_original_path,params[vd_path_index]);
	
	/* try to move to vd_path */
	parent_inode=cd(params[vd_path_index]);
	
	/* if parent_inode is -1, write error to client */
	if(parent_inode==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cls: `%s' :No such file or directory\n",err_char,vd_original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"ls: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,parent_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"ls: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if inode is not a directory list only the file */
	if(parent_inode_buffer[0]!='d')
	{
		sprintf(output_buffer,"%s\n",strrchr(vd_original_path,'/')+1);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return 1;
	}
	
	/* get the sub-blocks pointers array */
	pointers_no=get_sub_block_ptrs(parent_inode_buffer,sub_blocks_ptrs,&offset);
	
	/* move read/write pointer to dir data block start */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(parent_inode*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"ls: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent data block */
	if(read(vdfs_fd,parent_block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"ls: fatal error reading directory content!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* write to client the ls result */
	for(i=0;i<pointers_no;i++)
	{
		/* get a sub-block from directory data block */
		strcpy(sub_block,parent_block_buffer+(atoi(sub_blocks_ptrs[i])*(MAX_FILENAME_SIZE+inode_ptr_size+2)));
		
		/* load the status of the file (- for deleted, + for not deleted) */
		status=*(strchr(sub_block,':')+1);
		
		/* if status is '-' check if must show it the same (-a option passed) */
		if(status=='-')
		{
			if(!strcmp(params[1],"-a"))
			{
				/* get filename from data block */
				*(strchr(sub_block,':'))='\0';
				/* write output to client */
				sprintf(output_buffer,"%s (DELETED)  ",sub_block);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
			}
		}
		
		/* if status is '+' print it in any case */
		else
		{
			/* get filename from data block */
			*(strchr(sub_block,':'))='\0';
			/* write output to client */
			sprintf(output_buffer,"%s",sub_block);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			
			/* move read/write pointer to element inode */
			if(lseek(vdfs_fd,INODE_SIZE*(atoi(sub_block+strlen(sub_block)+1)),SEEK_SET)==-1)
			{
				/* write error output to client */
				strcpy(output_buffer,&err_char);
				strcat(output_buffer,"ls: fatal error!\n");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
	
			/* read element inode first character */
			if(read(vdfs_fd,type,sizeof(char))==-1)
			{
				/* write error output to client */
				strcpy(output_buffer,&err_char);
				strcat(output_buffer,"ls: fatal listing files!\n");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
		
			/* if element is a directory put a final '/' */
			if(type[0]=='d')
			{
				sprintf(output_buffer,"%c  ",'/');
				write(client_fd,output_buffer,strlen(output_buffer)+1);
			}
			else
			{
				sprintf(output_buffer,"  ");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
			}
		}
	}
	
	if(pointers_no!=0)
	{
		/* write newline to client */
		sprintf(output_buffer,"\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
	}
	return 1;
}

/* Makes an hard link to the file, or a soft one if -s option is passed
   PRE: none
   POST: writes on the client file descriptor the command output 
         and returns 1 if successfully completed, -1 otherwise
   PARAMS: parameter number, token array */
int ln(int param_no,char** params)
{
	/* Parameter number control */
	switch (param_no)
	{
		/*test the command sintax and wich ln sub-function to call */
		case 3:
		{
			if ((strncmp(params[1],vd_string,4)!=0))
			{
				sprintf(output_buffer,"%cln: %s is not a valid VDFS path!\nIt must start with %s\n",err_char,params[1],vd_string);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
			if ((strncmp(params[2],vd_string,4)!=0))
			{
				sprintf(output_buffer,"%cln: %s is not a valid VDFS path!\nIt must start with %s\n",err_char,params[2],vd_string);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
			return ln_hard(params[1],params[2]);
		}break;
		case 4:
		{
			if ((strcmp(params[1],"-s")!=0))
			{
				strcpy(output_buffer,"!ln: usage: ln [-s] {source/file} {new/link/file}");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
			if ((strncmp(params[3],vd_string,4)!=0))
			{
				sprintf(output_buffer,"%cln: %s is not a valid VDFS path!\nIt must start with %s\n",err_char,params[3],vd_string);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
			return ln_soft(params[2],params[3]);
		}break;
		default:
		{
			strcpy(output_buffer,"!ln: usage: ln [-s] {source/file} {new/link/file}");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}break;
	}
}

/* Makes an empty directory
   PRE: none
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int _mkdir(int param_no,char** params)
{
	int vdfs_fs;																		/* vdfs file system file descriptor 													*/
	int deq_res_i;																	/* inode pointer 																							*/
	int deq_res_b;																	/* block pointer 																							*/
	int num_tok;																		/* inode token number 																				*/
	int num_files;																	/* file number in directory																		*/
	int old_size;																		/* old parent inode size 																			*/
	int offset;																			/* offset from inode beginning of the sub-block pointers list	*/
	int parent_inode;																/* pointer to parent inode 																		*/
	char* dir;																			/* pointer to new directory 																	*/
	char* parent;																		/* pointer to parent directory 																*/
	char buffer[INODE_SIZE];												/* string containing an inode 																*/
	char date[DATE_SIZE];														/* string containing a date 																	*/
	char block_str[BLOCK_SIZE];											/* string containing parent data block 												*/
	char tmp_path[(MAX_FILENAME_SIZE+1)*INODE_NO];	/* temporary path string 																			*/
	char inode_parent_str[INODE_SIZE];							/* string containing parent inode 														*/
	char* tokens[DIR_INODE_TOKEN_NO];								/* inode tokens 																							*/
	char* tmp_token[max_files_per_dir]; 						/* sub_blocks 																								*/
	
	
	
	/* Case of wrong parameter number */
	if(param_no!=2)
	{
		strcpy(output_buffer,"mkdir: Usage mkdir {source/path/to/file}\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	while((params[1])[strlen(params[1])-1]=='/')
		(params[1])[strlen(params[1])-1]='\0';
		
	/* operation destination analizyng */
	if(strstr(params[1],vd_string)!=params[1])
	{
		strcpy(output_buffer,"mkdir: Usage mkdir VD:/{source/path/to/file}\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if new filename is longer than MAX_FILENAME_SIZE truncate it */
	*(strrchr(params[1],'/')+1+MAX_FILENAME_SIZE)='\0';

	/* check if dir is already existent */
	strcpy(tmp_path, params[1]);
	if(cd(tmp_path)!=-1)
	{
		strcpy(output_buffer,"esisto già!!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	vdfs_fs=open(VDFS,O_RDWR);
	
	/* get parent dir path and directory name */
	dir=strrchr(params[1],'/');
	if(dir==NULL)
		return -1;
	*dir='\0';
	dir++;
	parent=params[1];
	parent_inode=cd(parent);
	
	/* check if there's a file with the same name */
	if(find_in_dir(dir,parent_inode)!=-1)
		return -1;
	
	
	/* move read/write pointer on parent inode */
	if(lseek(vdfs_fs,INODE_SIZE*parent_inode,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"mkdir: Error finding the data block\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent inode */
	if(read(vdfs_fs,buffer,INODE_SIZE)==-1)
		return -1;
	
	strcpy(inode_parent_str,buffer);	
	num_files=get_sub_block_ptrs(buffer,tmp_token,&offset);	
	
	/* explode parent dir */
	num_tok=explode(inode_parent_str,tokens,":");
	
	/* check if parent dir can contain new file */
	if(num_files==max_files_per_dir)
		return -1;
		
	
	/* check filesystem has enough space */
	if(freeInodes_no==0 || freeBlocks_no==0)
		return -1;
	
	/* obtain the pointer to a free i-node */ 
	deq_res_i=dequeue(&fli,&freeInodes_no);
	/* obtain the pointer to a free block */ 
	deq_res_b=dequeue(&flb,&freeBlocks_no);
	
	
	/* prepare new dir sub-block */
	sprintf(dir,"%s:+%d", dir,deq_res_i); 
	
	
	/* new sub-block pointers string */
	char new_list[old_size+number_sizeof(num_files)+1];
	
	/* move read/write pointer on parent data block */
	if(lseek(vdfs_fs,((INODE_SIZE*INODE_NO)+(BLOCK_SIZE*atoi(tokens[3]))),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"mkdir: Error finding the data block\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent data block */
	if(read(vdfs_fs,block_str,BLOCK_SIZE)==-1)
		return -1;
	
	/* get new ordered sub-block pointers list */
	if(new_ordered_permutation(block_str,num_files,tmp_token,dir,new_list)==-1)
		return -1;
	
	/* move read/write pointer on parent inode */
	if(lseek(vdfs_fs,(INODE_SIZE*parent_inode)+offset,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"mkdir: Error finding the data block\n");
		write(client_fd,output_buffer,strlen(buffer)+1);
		return -1;
	}
	
	/* Writes in parent's i-node the new list */
	if(write(vdfs_fs,new_list,strlen(new_list)+1)<strlen(new_list)+1)
		return -1;
	
	/* move read/write pointer on parent data block */
	if(lseek(vdfs_fs,((INODE_SIZE*INODE_NO)+(BLOCK_SIZE*atoi(tokens[3]))+((MAX_FILENAME_SIZE+inode_ptr_size+2)*num_files)),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"mkdir: Error finding the data block\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* Writes in parent's data block dir's name and i-node address */
	if(write(vdfs_fs,dir,MAX_FILENAME_SIZE+inode_ptr_size+2)==-1)
		return -1;		//il file è pieno
	
	/* Create the directory */
	getdate(date);
	sprintf(buffer,"d:%s:%d:%d:%d:",date,1,deq_res_b, 1);
	
	/* move read/write pointer on new directory inode */
	if(lseek(vdfs_fs,INODE_SIZE*deq_res_i,SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"mkdir: Error finding the data block\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	
	/* Writes in VDFS file the directory i-node */
	if(write(vdfs_fs,buffer,INODE_SIZE)<INODE_SIZE)
		return -1;
	
	return 1;
}

/* Copies a file into another path
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: parameter number, token array */
int cp(int param_no,char **params)
{
	/* Case of wrong parameter number */
	if(param_no!=3)
	{
		strcpy(output_buffer,"cp: Usage cp {source/path/to/file} {source/path/to/destination}\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* these two variables are 1 if there is an operation on VD:/ respectively on first or	*/
	/* second parameter, 0 if the operation is on the OS filesystem													*/
	int first_place=0;
	int second_place=0;
	
	/* operation destination analizyng */
	if(strstr(params[1],vd_string)==params[1])
		first_place=1;
	if(strstr(params[2],vd_string)==params[2])
		second_place=1;
		
	/*test if at least one the two parameters starts with VD:/	*/
	if(first_place==0 && second_place==0)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: Abnormal cp error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* copy from VD:/ to OS filesystem */
	if(first_place==1 && second_place==0)
		return cp_from_vd_to_os(params[1],params[2]);
	
	/* copy from OS filesystem to VD:/ */
	else if(first_place==0 && second_place==1)
		return cp_from_os_to_vd(params[2],params[1]);
	
	/* copy from VD:/ to VD:/ */
	else return cp_from_vd_to_vd(params[1],params[2]);
}

/* Softly deletes the desired file or link
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: parameter number, token array */
int del(int param_no,char** params)
{
	int current_dir;																				/* pointer to current directory 									*/
	int vdfs;																								/* vdfs file descriptor 													*/
	int parent_data_block;																	/* pointer to parent dir data block 							*/
	int tmp_ptr;																						/* temporary inode pointer 												*/
	int slash_flag=0;																				/* activated if last path character is '/' 				*/
	char *file;																							/* string containing the name of deleting file 		*/
	char sub_block[MAX_FILENAME_SIZE+inode_ptr_size+2];			/* string containing the deleting file sub-block	*/
	char original_path[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing original path 								*/
	char inode_buffer[INODE_SIZE];													/* string containing an inode 										*/
	char* inode_tokens[DIR_INODE_TOKEN_NO];									/* string array containing inode tokens 					*/
	
	
	if (param_no!=2)
	{
		/* If wrog param number send error message */
		sprintf(output_buffer,"%cdel: usage: del {path}\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	/* Control string format */
	while (*(params[1]+strlen(params[1])-1)=='/')
	{
		slash_flag=1;
		*(params[1]+strlen(params[1])-1)='\0';
	}
	
	/* save original path */
	strcpy(original_path,params[1]);
	
	/* Separate parent directory's path and deleting file name */
	file=strrchr(params[1],'/');
	if (file==NULL)
	{
		/* If wrong path send error message */
		sprintf(output_buffer,"%cdel: %s: invalid path!\n",err_char,params[1]);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	*file='\0';
	file++;
	
	/* Get current directory's i-node address */
	current_dir=cd(params[1]);
	if(current_dir<0)
	{
		/* If unexistent path send error message */
		sprintf(output_buffer,"%cdel: %s/%s: no such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	/* Get deleting element index */
	tmp_ptr=find_in_dir(file,current_dir);
	if(tmp_ptr<0)
	{
		/* If it doesn't exist send error message */
		sprintf(output_buffer,"%cdel: %s/%s: no such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Open VDFS read/write mode */
	vdfs=open(VDFS,O_RDWR);
	
	
	/* Read the entire parent's i-node */
	lseek(vdfs, INODE_SIZE*current_dir, SEEK_SET);
	if ((read(vdfs,inode_buffer,INODE_SIZE))<INODE_SIZE)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%cdel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* explode parent inode */
	if(explode(inode_buffer,inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If failed exploding send error message */
		sprintf(output_buffer,"%cdel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* save parent dir data block pointer */
	parent_data_block=atoi(inode_tokens[3]);
	
	/* move read/write pointer to the beginning of deleting file sub-block */
	lseek(vdfs, (INODE_SIZE*INODE_NO)+(BLOCK_SIZE*parent_data_block)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2)), SEEK_SET);
	
	/* read deleting file sub-block */
	if((read(vdfs,sub_block,MAX_FILENAME_SIZE+inode_ptr_size+2))==-1)
	{
		sprintf(output_buffer,"%cdel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* if element is already deleted write error to client */
	if(*(strrchr(sub_block,':')+1)=='-')
	{
		sprintf(output_buffer,"%cdel: %s is already deleted!\n",err_char,original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* move read/write pointer to deleting file inode */
	lseek(vdfs, INODE_SIZE*(atoi(strrchr(sub_block,'+')+1)), SEEK_SET);
	/* read deleting file inode */
	if((read(vdfs,inode_buffer,INODE_SIZE))==-1)
	{
		sprintf(output_buffer,"%cdel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* if element is not a regular file or a link write error to client */
	if(inode_buffer[0]!='f' && inode_buffer[0]!='l')
	{
		sprintf(output_buffer,"%cdel: %s is not a regular file or link!\n",err_char,original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	if (slash_flag)
	{
		/* If regular file passed like a directory return error */
		sprintf(output_buffer,"%cdel: cannot lstat `%s/%s': Not a directory\n",err_char, params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* mark element sub-block as deleted */
	*(strrchr(sub_block,':')+1)='-';
	
	/* Go back to element's sub-block and rewrite it with - */
	lseek(vdfs, (INODE_SIZE*INODE_NO)+(BLOCK_SIZE*parent_data_block)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2)), SEEK_SET);
	write(vdfs,sub_block,strlen(sub_block)+1);
	
	/* Return exit_success */
	close(vdfs);
	return 1;
}

/* Undoes del command
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: parameter number, token array */
int undel(int param_no,char** params)
{
	int current_dir;																				/* pointer to current directory 									*/
	int vdfs;																								/* vdfs file descriptor 													*/
	int parent_data_block;																	/* pointer to parent dir data block 							*/
	int tmp_ptr;																						/* temporary inode pointer 												*/
	int slash_flag=0;																				/* activated if last path character is '/' 				*/
	char *file;																							/* string containing the name of deleting file 		*/
	char sub_block[MAX_FILENAME_SIZE+inode_ptr_size+2];			/* string containing the deleting file sub-block	*/
	char inode_buffer[INODE_SIZE];													/* string containing an inode 										*/
	char original_path[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing original path 								*/
	char* inode_tokens[DIR_INODE_TOKEN_NO];									/* string array containing inode tokens 					*/
	
	
	if (param_no!=2)
	{
		/* If wrog param number send error message */
		sprintf(output_buffer,"%cundel: usage: undel {path}\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* Control string format */
	if (*(params[1]+strlen(params[1])-1)=='/')
	{
		slash_flag=1;
		*(params[1]+strlen(params[1])-1)='\0';
	}
	
	/* save original path */
	strcpy(original_path,params[1]);
	
	/* Separate parent directory's path and deleting file name */
	file=strrchr(params[1],'/');
	if (file==NULL)
	{
		/* If wrong path send error message */
		sprintf(output_buffer,"%cundel: %s: Invalid path!\n",err_char,params[1]);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	*file='\0';
	file++;
	
	/* Get current directory's i-node address */
	current_dir=cd(params[1]);
	if(current_dir<0)
	{
		/* If unexistent path send error message */
		sprintf(output_buffer,"%cundel: cannot stat `%s/%s': no such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	
	/* Get deleting element index */
	tmp_ptr=find_in_dir(file,current_dir);
	if(tmp_ptr<0)
	{
		/* If it doesn't exist send error message */
		sprintf(output_buffer,"%cundel: cannot stat `%s/%s': no such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Open VDFS read/write mode */
	vdfs=open(VDFS,O_RDWR);
	
	/* Read the entire parent's i-node */
	lseek(vdfs, INODE_SIZE*current_dir, SEEK_SET);
	if ((read(vdfs,inode_buffer,INODE_SIZE))<INODE_SIZE)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%cundel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* explode parent inode */
	if(explode(inode_buffer,inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If failed exploding send error message */
		sprintf(output_buffer,"%cundel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* save parent dir data block pointer */
	parent_data_block=atoi(inode_tokens[3]);
	
	/* move read/write pointer to the beginning of deleting file sub-block */
	lseek(vdfs, (INODE_SIZE*INODE_NO)+(BLOCK_SIZE*parent_data_block)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2)), SEEK_SET);
	
	/* read deleting file sub-block */
	if((read(vdfs,sub_block,MAX_FILENAME_SIZE+inode_ptr_size+2))==-1)
	{
		sprintf(output_buffer,"%cundel: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* if element is not deleted write error to client */
	if(*(strrchr(sub_block,':')+1)!='-')
	{
		sprintf(output_buffer,"%cundel: cannot undelete `%s': Is not a deleted file!\n",err_char,original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	if (slash_flag)
	{
		/* If regular file passed like a directory return error */
		sprintf(output_buffer,"%cundel: cannot lstat `%s/%s': Not a directory\n",err_char, params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* mark element sub-block as undeleted */
	*(strrchr(sub_block,':')+1)='+';
	
	/* Go back to element's sub-block and rewrite it with - */
	lseek(vdfs, (INODE_SIZE*INODE_NO)+(BLOCK_SIZE*parent_data_block)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2)), SEEK_SET);
	write(vdfs,sub_block,strlen(sub_block)+1);
	
	/* Return exit_success */
	close(vdfs);
	return 1;
}

/* Definitively removes a deleted file
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: parameter number, token array */
int purge(int param_no,char** params)
{
	int current_dir;																				/* pointer to current directory 											*/
	int vdfs;																								/* vdfs file descriptor 															*/
	int i,j;																								/* generic indexes																		*/
	int tmp_ptr;																						/* temporary inode pointer 														*/
	int slash_flag=0;																				/* activated if last path character is '/' 						*/
	int file_no;																						/* file number of a dir																*/
	int tmp_offset;																					/* offset of the sub-bock pointers list beginning			*/
	char *file;																							/* string containing the name of deleting file 				*/
	char inode_buffer[INODE_SIZE];													/* string containing an inode 												*/
	char inode[inode_ptr_size];															/* string containing an inode pointer									*/
	char block[block_ptr_size];															/* string containing a block pointer									*/
	char file_inode_buffer[INODE_SIZE];											/* string containing an inode													*/
	char new_file_inode_buffer[INODE_SIZE];									/* string containing new inode												*/
	char* file_inode_tokens[DIR_INODE_TOKEN_NO];						/* string array containing deleting file inode tokens	*/
	char* inode_tokens[DIR_INODE_TOKEN_NO];									/* string array containing inode tokens 							*/
	char* ordered_indexes[max_files_per_dir];								/* string array containing new ordered indexes				*/
	
	
	if (param_no!=2)
	{
		/* If wrog param number send error message */
		sprintf(output_buffer,"%cpurge: usage: rm {path}\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* Control string format */
	if (*(params[1]+strlen(params[1])-1)=='/')
	{
		slash_flag=1;
		*(params[1]+strlen(params[1])-1)='\0';
	}
	
	/* Separate parent directory's path and deleting file name */
	file=strrchr(params[1],'/');
	if (file==NULL)
	{
		/* If wrong path send error message */
		sprintf(output_buffer,"%cpurge: %s: Invalid path!\n",err_char,params[1]);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	*file='\0';
	file++;
	
	/* Get current directory's i-node address */
	current_dir=cd(params[1]);
	if(current_dir<0)
	{
		/* If unexistent path send error message */
		sprintf(output_buffer,"%cpurge: cannot stat `%s/%s': No such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	
	/* Get deleting element index */
	tmp_ptr=find_in_dir(file,current_dir);
	if(tmp_ptr<0)
	{
		/* If it doesn't exist send error message */
		sprintf(output_buffer,"%cpurge: cannot stat `%s/%s': No such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Open VDFS read/write mode */
	vdfs=open(VDFS,O_RDWR);
	
	/* Read the entire parent's i-node */
	lseek(vdfs, INODE_SIZE*current_dir, SEEK_SET);
	if ((read(vdfs,inode_buffer,INODE_SIZE))<INODE_SIZE)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* save parent_inode's lenght */
	tmp_offset=strlen(inode_buffer);
	
	/* explode parent inode */
	if (explode(inode_buffer,inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* save the offset of the sub-block pointers list beginning */
	tmp_offset-=strlen(inode_tokens[5]);
	
	/* Read deleting element's i-node address */
	lseek(vdfs,(INODE_NO*INODE_SIZE)+(atoi(inode_tokens[3])*BLOCK_SIZE)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2))+strlen(file)+1,SEEK_SET);
	if ((read(vdfs,inode,inode_ptr_size))<inode_ptr_size)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* If file isn't marked as deleted return error */
	if (*inode!='-')
	{
		sprintf(output_buffer,"%cpurge: cannot purge `%s/%s': Is not a deleted file\nUse rm {<path>} or del {<path>}\n",err_char, params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Reading element's i-node */
	lseek(vdfs,(atoi(inode+1)*INODE_SIZE),SEEK_SET);
	if ((read(vdfs, file_inode_buffer, INODE_SIZE))<INODE_SIZE)
	{
		/* If cannot read send error message */
		sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Explode deleting file's inode */
	if (explode(file_inode_buffer,file_inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If wrong i-node send error message */
		sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Check deleting element's type */
	if (strcmp(file_inode_tokens[0],"d")==0)
	{
		/* If deleting element is a directory send error message */
		sprintf(output_buffer,"%cpurge: cannot remove `%s/%s': is a directory\n",err_char, params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	if (slash_flag)
	{
		/* If regular file passed like a directory return error */
		sprintf(output_buffer,"%cpurge: cannot lstat `%s/%s': Not a directory\n",err_char, params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Decrement hard link counter of file */
	sprintf(file_inode_tokens[2],"%d",(atoi(file_inode_tokens[2])-1));
	
	/* If no more hard links free file's i-node and blocks */
	if (atoi(file_inode_tokens[2])<1)
	{
		/* Free file's i-node */
		if (enqueue(&fli, atoi(inode+1),&freeInodes_no)==0)
		{
			/* If failed freeing i-node send error message */
			sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			close(vdfs);
			return -1;
		}
		/* Free file's block[s] */
		if (enqueue(&flb, atoi(file_inode_tokens[3]),&freeBlocks_no)==0)
		{
			/* If failed freeing block send error message */
			sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			close(vdfs);
			return -1;
		}
		strcpy(block,file_inode_tokens[3]);
		for(i=1;i<atoi(file_inode_tokens[4]);i++)
		{
			/* Read next block's address */
			lseek(vdfs,(INODE_NO*INODE_SIZE)+(BLOCK_SIZE*atoi(block))+BLOCK_SIZE-block_ptr_size-1,SEEK_SET);
			read(vdfs,block,block_ptr_size+1);
			/* Free current block */
			if (enqueue(&flb,atoi(block),&freeBlocks_no)==0)
			{
				/* If failed freeing block send error message */
				sprintf(output_buffer,"%cpurge: fatal error!\n",err_char);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				close(vdfs);
				return -1;
			}
		}
	}
	
	/* Explode parent directory's list of contents */
	file_no=explode(inode_tokens[5], ordered_indexes, "-");
	i=0;
	
	/* Find element's pointer position in parent's list of contents */
	while((i<file_no)&&(atoi(ordered_indexes[i])!=tmp_ptr))
		i++;
	if (i==file_no)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* string that will contain the new indexes list */
	char new_indexes[file_no*(1+number_sizeof(max_files_per_dir))];	
	
	/* Initialize string */
	new_indexes[0]='\0';
	
	/* Fill string with indexes excluding the one removed*/
	for (j=0;j<file_no;j++)
	{
		if(j!=i)
		{
			strcat(new_indexes,ordered_indexes[j]);
			strcat(new_indexes,"-");
		}
	}
	
	/* Remove last '-' character */
	new_indexes[strlen(new_indexes)]='\0';
	
	/* Write new list of indexes in parent's i-node */
	lseek(vdfs,(INODE_SIZE*current_dir)+tmp_offset, SEEK_SET);
	if(write(vdfs,new_indexes,strlen(new_indexes)+1)<strlen(new_indexes)+1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Write new file's i-node */
	sprintf(new_file_inode_buffer,"%s:%s:%s:%s:%s",file_inode_tokens[0],file_inode_tokens[1],file_inode_tokens[2],file_inode_tokens[3],file_inode_tokens[4]);
	lseek(vdfs,(atoi(inode+1)*INODE_SIZE),SEEK_SET);
	if(write(vdfs,new_file_inode_buffer,strlen(new_file_inode_buffer)+1)<strlen(new_file_inode_buffer)+1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Return EXIT_SUCCESS */
	close(vdfs);
	return 1;
}

/* Definitively removes a file (del+purge)
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: parameter number, token array */
int rm(int param_no,char** params)
{
	int current_dir;																				/* pointer to current directory 											*/
	int vdfs;																								/* vdfs file descriptor 															*/
	int i,j;																								/* generic indexes																		*/
	int tmp_ptr;																						/* temporary inode pointer 														*/
	int slash_flag=0;																				/* activated if last path character is '/' 						*/
	int file_no;																						/* file number of a dir																*/
	int tmp_offset;																					/* offset of the sub-bock pointers list beginning			*/
	int tmp_list_size, tmp_inode_size;											/* temporary string sizes															*/
	char *file;																							/* string containing the name of deleting file 				*/
	char inode_buffer[INODE_SIZE];													/* string containing an inode 												*/
	char inode[inode_ptr_size];															/* string containing an inode pointer									*/
	char block[block_ptr_size];															/* string containing a block pointer									*/
	char file_inode_buffer[INODE_SIZE];											/* string containing an inode													*/
	char new_file_inode_buffer[INODE_SIZE];									/* string containing new inode												*/
	char original_parent[(MAX_FILENAME_SIZE+1)*INODE_NO];		/* string containing original parent path							*/
	char* file_inode_tokens[DIR_INODE_TOKEN_NO];						/* string array containing deleting file inode tokens	*/
	char* inode_tokens[DIR_INODE_TOKEN_NO];									/* string array containing inode tokens 							*/
	char* ordered_indexes[max_files_per_dir];
	
	
	if (param_no!=2)
	{
		/* If wrog param number send error message */
		sprintf(output_buffer,"%crm: usage: rm {path}\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* Control string format */
	if (*(params[1]+strlen(params[1])-1)=='/')
	{
		slash_flag=1;
		*(params[1]+strlen(params[1])-1)='\0';
	}
	
	/* Separate parent directory's path and deleting file name */
	file=strrchr(params[1],'/');
	if (file==NULL)
	{
		/* If wrong path send error message */
		sprintf(output_buffer,"%crm: %s: invalid path!\n",err_char,params[1]);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	*file='\0';
	file++;
	
	/* Save parent's path */
	strcpy(original_parent, params[1]);
	
	/* Get current directory's i-node address */
	current_dir=cd(params[1]);
	if(current_dir<0)
	{
		/* If unexistent path send error message */
		sprintf(output_buffer,"%crm: cannot stat `%s/%s': no such file or directory\n",err_char,original_parent,file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;;
	}
	
	/* Get deleting element index */
	tmp_ptr=find_in_dir(file,current_dir);
	if(tmp_ptr<0)
	{
		/* If it doesn't exist send error message */
		sprintf(output_buffer,"%crm: cannot stat `%s/%s': no such file or directory\n",err_char,params[1],file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Open VDFS read/write mode */
	vdfs=open(VDFS,O_RDWR);
	
	/* Read the entire parent's i-node */
	lseek(vdfs, INODE_SIZE*current_dir, SEEK_SET);
	if ((read(vdfs,inode_buffer,INODE_SIZE))<INODE_SIZE)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* get inode's lenght */
	tmp_inode_size=strlen(inode_buffer);
	
	/* Get parent's sub-blocks */
	if (explode(inode_buffer,inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* save the sub-block pointers list offset */
	tmp_offset=tmp_inode_size-strlen(inode_tokens[5]);
	
	/* Read deleting element's i-node address */
	lseek(vdfs,(INODE_NO*INODE_SIZE)+(atoi(inode_tokens[3])*BLOCK_SIZE)+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2))+strlen(file)+1,SEEK_SET);
	if ((read(vdfs,inode,inode_ptr_size))<inode_ptr_size)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Turn i-node positive*/
	*inode='+';
	
	/* Check deleting element's type */
	lseek(vdfs,(atoi(inode)*INODE_SIZE),SEEK_SET);
	if ((read(vdfs,file_inode_buffer,INODE_SIZE))<INODE_SIZE)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	if (explode(file_inode_buffer,file_inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* If wrong i-node send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	if ((strcmp(file_inode_tokens[0],"f")!=0) && (strcmp(file_inode_tokens[0],"l")!=0))
	{
		/* If deleting element is a directory send error message */
		sprintf(output_buffer,"%crm: cannot remove `%s/%s': Is not a regular file or a link\n",err_char,original_parent,file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	if (slash_flag)
	{
		/* If regular file passed like a directory return error */
		sprintf(output_buffer,"%crm: cannot lstat `%s/%s': Not a directory\n",err_char,original_parent,file);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Decrement hard link counter of file */
	sprintf(file_inode_tokens[2],"%d",(atoi(file_inode_tokens[2])-1));
	
	/* If no more hard links free file's i-node and blocks */
	if (atoi(file_inode_tokens[2])<1)
	{
		/* Free file's i-node */
		if (enqueue(&fli, atoi(inode),&freeInodes_no)==0)
		{
			/* If failed freeing i-node send error message */
			sprintf(output_buffer,"%crm: fatal error!\n",err_char);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			close(vdfs);
			return -1;
		}
		/* Free file's blocks */
		if (enqueue(&flb, atoi(file_inode_tokens[3]),&freeBlocks_no)==0)
		{
			/* If failed freeing block send error message */
			sprintf(output_buffer,"%crm: fatal error!\n",err_char);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			close(vdfs);
			return -1;
		}
		strcpy(block,file_inode_tokens[3]);
		for(i=1;i<atoi(file_inode_tokens[4]);i++)
		{
			/* Read next block's address */
			lseek(vdfs,(INODE_NO*INODE_SIZE)+(BLOCK_SIZE*atoi(block))+BLOCK_SIZE-block_ptr_size-1,SEEK_SET);
			read(vdfs,block,block_ptr_size+1);
			/* Free current block */
			if (enqueue(&flb,atoi(block),&freeBlocks_no)==0)
			{
				/* If failed freeing block send error message */
				sprintf(output_buffer,"%crm: fatal error!\n",err_char);
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				close(vdfs);
				return -1;
			}
		}
	}
	
	/* Explode parent directory's list of contents */
	file_no=explode(inode_tokens[5], ordered_indexes, "-");
	i=0;
	
	/* Find element's pointer position in parent's list of contents */
	while((i<file_no)&&(atoi(ordered_indexes[i])!=tmp_ptr))
		i++;	
	if (i==file_no)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Allocate string that will contain the new indexes list */
	char new_ordered_indexes[tmp_list_size];
	
	/* Initialize string */
	*new_ordered_indexes='\0';
	
	/* Fill string with indexes excluding the one removed*/
	for (j=0;j<file_no;j++)
	{
		if(j!=i)
		{	
			strcat(new_ordered_indexes,ordered_indexes[j]);
			strcat(new_ordered_indexes,"-");
		}
	}
	
	/* Remove last '-' character */
	new_ordered_indexes[strlen(new_ordered_indexes)]='\0';
	
	/* Write new list of indexes in parent's i-node */
	lseek(vdfs,(INODE_SIZE*current_dir)+tmp_offset, SEEK_SET);
	if(write(vdfs,new_ordered_indexes,strlen(new_ordered_indexes)+1)<strlen(new_ordered_indexes)+1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Write new file's i-node */
	sprintf(new_file_inode_buffer,"%s:%s:%s:%s:%s",file_inode_tokens[0],file_inode_tokens[1],file_inode_tokens[2],file_inode_tokens[3],file_inode_tokens[4]);
	lseek(vdfs,(atoi(inode)*INODE_SIZE),SEEK_SET);
	if(write(vdfs,new_file_inode_buffer,strlen(new_file_inode_buffer)+1)<strlen(new_file_inode_buffer)+1)
	{
		/* If failed reading send error message */
		sprintf(output_buffer,"%crm: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs);
		return -1;
	}
	
	/* Return EXIT_SUCCESS */
	close(vdfs);
	return 1;
}

