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

#include "aux_server_funct.h"


/* Explodes the string in a word array divided by chars in separators string
   and returns the token number
   PRE: string must have already been initialized
   POST: token array contains the tokens in the string
   PARAMS: string, token array, char separator */
int explode(char* string,char** tokens,char* separators)
{
	int array_ind=0;
	
	if((tokens[array_ind]=strtok(string,separators))!=NULL)
		array_ind++;
	while((tokens[array_ind]=strtok(NULL,separators))!=NULL)
		array_ind++;
	return array_ind;
}

/* Add to the previous ordered directory content position list the new file position
   PRE: newfile not null
   POST: returns 1 if successfully completed, -1 otherwise
   PARAMS: string of the block content, its size, array of strings(numbers) ordered_permutation, 
	 	string new_file, string new ordered permutation*/
int new_ordered_permutation(char* block,int size,char** old_permutation,char* new_file,char* new_permutation)
{
	int i;																/* generic counter 																				*/
	int flag=0;														/* generic flag																						*/
	char tmp_string[number_sizeof(size)];	/* char array for transforming an int to a string 				*/
	char *samples[size];									/* string array containing the file entries of the block 	*/
	int occourrencies[max_files_per_dir];	/* int array containing the file entries of the block 		*/
	int free_index=0;											/* index of a free directory sub-block										*/
	
	if(new_file==NULL)
		return -1;
	
	if(size==0)
	{
		sprintf(new_permutation,"%d-",free_index);
		return free_index;
	} 
	/* obtain the array representing the content of the block */
	/* every sample is in the form <filename>:<inode_ptr>			*/
	string_sample(block,samples,size,old_permutation);
	
	/* occourrencies array initialization */
	for(i=0;i<max_files_per_dir;i++)
		occourrencies[i]=0;	
		
	/* extract from samples the filename 	*/
	/* and initialize occourrencies array */
	for(i=0;i<size;i++)
	{	
		*(strchr(samples[atoi(old_permutation[i])],':'))='\0';
		occourrencies[atoi(old_permutation[i])]=1;
	}
	
	/* new permutation string initialization */
	*new_permutation='\0';
	
	/* create new_permutation */
	for(i=0;i<size;i++)
	{
		/* test if the current element of the ordered list is major of new_file	*/
		/* or if the new element is the last of the new permutation							*/
		if(strcmp(samples[atoi(old_permutation[i])],new_file)>0 && !flag)
		{
			/* find a free sub-block index */
			while(occourrencies[free_index]==1)
				free_index++;
				
			sprintf(tmp_string,"%d-",free_index);
			strcat(new_permutation,tmp_string);
			/* this flag indicates that new_file pointer has been inserted into new_permutation */
			flag=1;
			//i++;
		}
		/* writes on new_permutation the current pointer with a separator '-' character */
		strcat(new_permutation,old_permutation[i]);
		strcat(new_permutation,"-");
	}
	/* test if new file has not been inserted yet */
	if(flag==0)
	{
	/* find a free sub-block index */
		while(occourrencies[free_index]==1)
			free_index++;
		sprintf(tmp_string,"%d-",free_index);
		strcat(new_permutation,tmp_string);
	}
	
	return free_index;
}

/* Executes the function pointed by fn_ptr
   PRE: params not null
   POST: returns the exit status of the called function, -2 if the function is not defined
	 PARAMS: parameter number, token array */
int execute(int param_no,char** params,command *commands)
{
	int i=0;	/* command counter */
	
	/* finds the command to launch */
	while(strcmp(params[0],commands[i].cmd_name)!=0 && i<CMD_NO)
		i++;
	if(i>=CMD_NO)
		return -2;
	return (*(commands[i].funct))(param_no,params);
}

/* Returns i-node address of path directory
   PRE: string must have already been initialized
   POST: returns i-node address (>=0) if directory exists, -1 otherwise
   PARAMS: directory path (eg. for del VD:/dir/file use cd("VD:/dir")) */
int cd(char* path)
{
	char* path_tokens[INODE_NO];
	int depth;
	
	/* Explode path to tokens */
	depth=explode(path,path_tokens,"/");
	/* If empty string return -1 */
	if (depth<1)
		return -1;
	
	/* If path not regular return -1 */
	if (strncmp(path_tokens[0],vd_string,3))
		return -1;
	
	return __recursive_cd(&(path_tokens[1]), 0, depth-1);
	
}

/* ***Internal function, not to use out of this file*** */
int __recursive_cd(char** path_tokens,int dir_inode,int depth)
{
	char inode_buffer[INODE_SIZE];
	char* inode_tokens[DIR_INODE_TOKEN_NO];
	int vdfs;
	int next_dir;
	char inode_ptr_buffer[inode_ptr_size];
	
	/* If deleted directory return error */
	if (dir_inode<0)
		return -1;
	/* If reached searched directory return its data block address */
	if (depth==0)
		return dir_inode;
	
	/* Opening VDFS reading mode */
	vdfs=open(VDFS, O_RDONLY);
	
	/* Read current dir i-node */
	lseek(vdfs, INODE_SIZE*dir_inode, SEEK_SET);
	if(read(vdfs, inode_buffer, INODE_SIZE)!=INODE_SIZE)
	{
		close(vdfs);
		return -1;
	}
	/* Tokenize current dir i-node */
	if (explode(inode_buffer, inode_tokens, ":")<DIR_INODE_TOKEN_NO)
	{
		close(vdfs);
		return -1;
	}
	/* Check current dir type */
	if (strcmp(inode_tokens[0], "d")!=0)
	{
		close(vdfs);
		return -1;
	}
	/* Find child's position in this file */
	next_dir=find_in_dir(path_tokens[0], dir_inode);
	if(next_dir<0)
	{
		close(vdfs);
		return -1;
	}
	/* Read next_file's i-node address from current dir block */
	lseek(vdfs,(INODE_SIZE*INODE_NO)+(BLOCK_SIZE*atoi(inode_tokens[3]))+(next_dir*(MAX_FILENAME_SIZE+inode_ptr_size+2))+strlen(path_tokens[0])+1,SEEK_SET);
	if (read(vdfs, inode_ptr_buffer, inode_ptr_size)!=inode_ptr_size)
	{
		close(vdfs);
		return -1;
	}
	
	close(vdfs);
	
	return __recursive_cd(&(path_tokens[1]),atoi(inode_ptr_buffer),depth-1);
}

/* Find a file in a directory
   PRE: i-node number passed is a valid directory i-node
   POST: returns the position of the file named filename in current dir, -1 if it doesn't exist
   PARAMS:  name of file to find, directory inode*/
int find_in_dir(char* filename, int dir_inode)
{
	int vdfs;																/* vdfs file descriptor 													*/
	int start=0, end;												/* generic array indexes 													*/
	int res;																/* strcmp result																	*/
	char inode_buffer[INODE_SIZE];					/* string containing an inode 										*/
	char block_buffer[BLOCK_SIZE];					/* string containing a data block 								*/
	char* inode_tokens[DIR_INODE_TOKEN_NO];	/* string array containing an exploded inode 			*/
	char* pointers[max_files_per_dir];			/* string array containing the sub-block pointers	*/
	char* samples[max_files_per_dir];				/* string array containing the sub-blocks 				*/
	
	
	/* Open VDFS reading mode */
	vdfs=open(VDFS, O_RDONLY);
	
	/* Read directory's i-node */
	lseek(vdfs, INODE_SIZE*dir_inode, SEEK_SET);
	read(vdfs, inode_buffer, INODE_SIZE);
	/* Get directory's data block address (inode_tokens[3]) */
	if (explode(inode_buffer,inode_tokens,":")<DIR_INODE_TOKEN_NO-1)
	{
		close(vdfs);
		return -1;
	}
	/* Count elements in current dir */
	end=explode(inode_tokens[5],pointers,"-");
	/* Read current directory's datablock */
	lseek(vdfs,(INODE_SIZE*INODE_NO)+(BLOCK_SIZE*atoi(inode_tokens[3])),SEEK_SET);
	if (read(vdfs, block_buffer, BLOCK_SIZE)<BLOCK_SIZE)
	{
		close(vdfs);
		return -1;
	}
	close(vdfs);
	/* Sample block in elements array */
	string_sample(block_buffer, samples, end,pointers);
	/* Find filename in dir */
	while(start<end)
	{
		*(strchr(samples[atoi(pointers[start+((end-start)/2)])],':'))='\0';
		res=strcmp(samples[atoi(pointers[start+((end-start)/2)])], filename);
		if(res<0)
			start=start+((end-start)/2)+1;
		else if(res>0)
			end=start+((end-start)/2);
		else
			return atoi(pointers[start+((end-start)/2)]);
	}
	return -1;
}

/* Returns in destination array the sampled block of a directory
   PRE: none
   POST: void
   PARAMS: string containing a dir block, array of strings, number of filled parts (elements in dir)
	 		string array containing the block pointers of the dir */
void string_sample(char* parent_dir_block, char** destination, int sample_no, char **pointers)
{
	int i;	/* generic counter */
	
	for (i=0;i<sample_no;i++)
		destination[atoi(pointers[i])]=parent_dir_block+((atoi(pointers[i])) * (MAX_FILENAME_SIZE+inode_ptr_size+2));
}

/* Copies a file from VD filesytem to OS filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path, string os_path */
int cp_from_vd_to_os(char *vd_path,char *os_path)
{
	int parent_inode;																						/* pointer to parent dir inode 																			*/
	int parent_sub_block;																				/* pointer to parent sub-block 																			*/
	int needed_blocks;																					/* source file block number 																				*/
	int vdfs_fd;																								/* file descriptors of source OS fs file and destination VD fs file	*/
	char block_buffer[BLOCK_SIZE];															/* strings for inode and inode writing 															*/
	char type;																									/* source file type 																								*/
	char source_inode_buffer[INODE_SIZE];												/* string containing source parent inode 														*/
	char source_sub_block[MAX_FILENAME_SIZE+inode_ptr_size+2];	/* string containing source file sub-block 													*/
	char vd_original_path[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the original vd_path 													*/
	char *exp_source_inode[DIR_INODE_TOKEN_NO];									/* string array containing parent inode tokens 											*/
	
	
	/* open vdfs file in read-write */
	vdfs_fd=open(VDFS,O_RDWR);
	if(vdfs_fd==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: fatal error opening VDFS!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save vd_path_from */
	strcpy(vd_original_path,vd_path);
	
	/* try to move to source file parent directory */
	*(strrchr(vd_path,'/'))='\0';
	parent_inode=cd(vd_path);
	if(parent_inode==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,source_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* explode parent dir inode */
	if(explode(source_inode_buffer,exp_source_inode,":")!=DIR_INODE_TOKEN_NO)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* find in parent directory the source file */
	parent_sub_block=find_in_dir(strrchr(vd_original_path,'/')+1,parent_inode);
	if(parent_sub_block==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to source file sub-block*/
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_source_inode[3])*BLOCK_SIZE)+parent_sub_block*(MAX_FILENAME_SIZE+inode_ptr_size+2),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read source file sub-block */
	if(read(vdfs_fd,source_sub_block,MAX_FILENAME_SIZE+inode_ptr_size+2)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to source file inode*/
	if(lseek(vdfs_fd,atoi(strrchr(source_sub_block,':')+1)*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read source file inode */
	if(read(vdfs_fd,source_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* check if source is a regular file or a link*/
	type=source_inode_buffer[0];
	if(type!='f' && type!='l')
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		sprintf(output_buffer,"cp: %s is not a regular file or a link!\n",vd_original_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* explode source file inode */
	if(explode(source_inode_buffer,exp_source_inode,":")!=DIR_INODE_TOKEN_NO-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* get source file size */
	needed_blocks=atoi(exp_source_inode[4]);
	
	/*****************/
	/* FILE WRITING  */
	/*****************/
	
	int i=0; 										/* generic counter 																	*/
	int j; 											/* generic counter 																	*/
	int source_block_read_ptr;	/* pointer to the current source block to read 			*/
	int destination_fd;					/* file descriptor of the destination OS filesytem	*/
	
	/* initialize first source data block */
	source_block_read_ptr=atoi(exp_source_inode[3]);
	
	/* open destination file in write mode and truncate it if already existent */
	destination_fd=open(os_path, O_WRONLY|O_CREAT|O_TRUNC, 0777);
	/* if can't open os_path try to consider the new filename equal to the old one */
	if(destination_fd<0)
	{
		strcat(os_path,"/");
		strcat(os_path,strrchr(vd_original_path,'/')+1);
		destination_fd=open(os_path, O_WRONLY|O_CREAT|O_TRUNC, 0777);
		
		/* if file is still not opened write error to client */
		if(destination_fd<0)
		{
			/* write error output to client */
			sprintf(output_buffer,"%ccp: cannot open '%s' in write mode!\n",err_char,os_path);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	
	/* copy data blocks on destination file*/
	while(i<needed_blocks)
	{
		/* delete block_buffer content */
		for(j=0;j<BLOCK_SIZE;j++)
			block_buffer[j]='\0';
			
		/* move read/write pointer on the source data block on vdfs */
		if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(source_block_read_ptr*BLOCK_SIZE),SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	
		/* read data block from source file */
		if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		if(i<needed_blocks-1)
		{
			/* block writing */
			if(write(destination_fd,block_buffer,BLOCK_SIZE-block_ptr_size)==-1)
			{
				/* write error output to client */
				strcpy(output_buffer,&err_char);
				strcat(output_buffer,"cp: fatal error writing file on OS filesytem!\n");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
		}
		else
		{
			/* last block writing */
			if(write(destination_fd,block_buffer,strlen(block_buffer))==-1)
			{
				/* write error output to client */
				strcpy(output_buffer,&err_char);
				strcat(output_buffer,"cp: fatal error writing file on OS filesytem!\n");
				write(client_fd,output_buffer,strlen(output_buffer)+1);
				return -1;
			}
		}
			i++;
		
		/* obtain next source data block pointer */
		if(i!=needed_blocks)
			source_block_read_ptr=atoi(block_buffer+BLOCK_SIZE-block_ptr_size);
	}
	
	/* close VDFS */
	close(vdfs_fd);
	/* close destination file */
	close(destination_fd);
	
	/* write output to client */
	strcpy(output_buffer,"cp: Copy ok!\n");
	write(client_fd,output_buffer,strlen(output_buffer)+1);
	
	return 1;
}

/* Copies a file from OS filesytem to VD filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path, string os_path */
int cp_from_os_to_vd(char *vd_path,char *os_path)
{
	int free_inode_ptr;																					/* pointers to free inode 												*/
	int free_block_ptr;																					/* pointers to free block 												*/
	int parent_inode;																						/* pointer to parent dir inode 										*/
	int parent_sub_block;																				/* pointer to parent sub-block 										*/
	int needed_blocks;																					/* source file block number 											*/
	int exp_result;																							/* number of strings created by explode function 	*/
	int source_fd;																							/* file descriptors of source OS fs file 					*/
	int vdfs_fd;																								/* file descriptors VDFS file 										*/
	char block_buffer[BLOCK_SIZE];															/* string for block writing 											*/
	char inode_buffer[INODE_SIZE];															/* string for inode writing 											*/
	char parent_inode_buffer[INODE_SIZE];												/* string containing parent inode 								*/
	char new_filename[MAX_FILENAME_SIZE+1];											/* new filename of OS filesytem source file 			*/
	char vd_original_path[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the originale vd_path 				*/
	char *sub_blocks[max_files_per_dir];												/* string array containig the sub-blocks of a dir	*/
	struct stat file_stat;																			/* stat structure 																*/
	
	
	/* open vdfs file in read-write */
	vdfs_fd=open(VDFS,O_RDWR);
	if(vdfs_fd==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* open source file in read-write */
	source_fd=open(os_path,O_RDWR);
	if(source_fd==-1)
	{
		/* test if path is a directory */
		if(errno==EISDIR)
		{
			/* write error output to client */
			sprintf(output_buffer,"%ccp: omitting directory `%s'\n",err_char,os_path);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,os_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save vd_path */
	strcpy(vd_original_path,vd_path);
	
	/* try to move to vd_path */
	parent_inode=cd(vd_path);
	
	/* if can't move to vd_path try to consider last part of vd_path as the new filename */
	if(parent_inode==-1)
	{
		/* if new filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(vd_original_path,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(vd_original_path,'/')+1);
		
		/* try to move to new_filename's parent dir */
		*(strrchr(vd_original_path,'/'))='\0';
		strcpy(vd_path,vd_original_path);
		parent_inode=cd(vd_path);
	
		/* if parent_inode is still -1, this mean that the path doesn't exists */
		if(parent_inode==-1)
		{
			/* write error output to client */
			sprintf(output_buffer,"%ccp: cannot stat `%s' :No such file or directory\n",err_char,vd_original_path);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		strcat(vd_original_path,"/");
	}
	
	/* if can move to vd_path the new filename is the old OS filesystem's one */
	else
	{
		/* if old filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(os_path,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(os_path,'/')+1);
	}
	
	if(strcmp(vd_string,vd_original_path))
	{
		strcat(vd_original_path,"/");
	}
	/* save original vd_path with new filename */
	strcat(vd_original_path,new_filename);
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,parent_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save parent inode string */
	char original_parent_inode[INODE_SIZE];	/* parent inode backup string */
	strcpy(original_parent_inode,parent_inode_buffer);
	
	int offset=0;	/* offset of the pointer list of parent directory */
	/* get on sub_blocks string array the pointers of parent directory */
	exp_result=get_sub_block_ptrs(parent_inode_buffer,sub_blocks,&offset);
	
	/* check if dir can contain a new file */
	if(exp_result==max_files_per_dir)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot store files in %s anymore!\n",err_char,vd_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if file already exists remove it*/
	parent_sub_block=find_in_dir(new_filename,parent_inode);
	if(parent_sub_block!=-1)
	{
		char cmd[]="rm";
		char *rm_cmd[2];
		
		rm_cmd[0]=cmd;
		rm_cmd[1]=vd_original_path;
		if(rm(2,rm_cmd)==-1)
		{
			/* write error output to client */
			printf(output_buffer,"%ccp: cannot overwrite '%s' :No such file or directory\n",err_char,vd_path);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;	
		}
		/* decrement number of directory sub-blocks */
		exp_result--;
		
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* refresh parent dir inode */
	if(read(vdfs_fd,parent_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save parent inode string */
	char original_parent_inode[INODE_SIZE];	/* parent inode backup string */
	strcpy(original_parent_inode,parent_inode_buffer);
	
	/* get on sub_blocks string array the pointers of parent directory */
	exp_result=get_sub_block_ptrs(parent_inode_buffer,sub_blocks,&offset);
	}
	
	/* stat source file */
	if(fstat(source_fd,&file_stat)<0)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	printf("\nfile size: %ld\n",((long)file_stat.st_size));
	
	/*calculate the number of needed blocks and verify if there is enough space */
	needed_blocks=file_stat.st_size/(BLOCK_SIZE-block_ptr_size-1);
	if(file_stat.st_size%BLOCK_SIZE!=0)
		needed_blocks++;printf("\nneeded blocks: %d\n",needed_blocks);
	if(needed_blocks>freeBlocks_no)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: not enough space to store '%s'\n",err_char,vd_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* obtain the pointer to a free i-node */ 
	free_inode_ptr=dequeue(&fli,&freeInodes_no);
	/* obtain the pointer to a free block */ 
	free_block_ptr=dequeue(&flb,&freeBlocks_no);
	
	/* check if file system is full */
	if(free_block_ptr==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: file system is full!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/*******************/
	/* I-NODE WRITING */
	/******************/
	char type;						/* type of the file 			*/
	char date[DATE_SIZE];	/* string containing date	*/
	
	/* check if source file is a regular file */
	if(S_ISREG(file_stat.st_mode))
		type='f';
	else
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_path);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* get system date */
	getdate(date);
	
	/*prepare inode_buffer */
	sprintf(inode_buffer,"%c:%s:%d:%d:%d",type,date,1,free_block_ptr,needed_blocks);
	
	/* move read/write pointer on the free i-node */
	if(lseek(vdfs_fd,free_inode_ptr*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* i-node writing */
	if(write(vdfs_fd,inode_buffer,strlen(inode_buffer))==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/*****************/
	/* BLOCK WRITING */
	/*****************/
	
	int i=1; 	/* generic counter */
	int j; 		/* generic counter */
	
	
	/* FIRST BLOCK WRITING */
	
	/* delete block_buffer content */
		for(j=0;j<BLOCK_SIZE;j++)
			block_buffer[j]='\0';
	
	/* move read/write pointer on the first free block on vdfs*/
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(free_block_ptr*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read first data block from source file */
	if(read(source_fd,block_buffer,BLOCK_SIZE-block_ptr_size)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* block writing */
	if(write(vdfs_fd,block_buffer,strlen(block_buffer))==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* copy other blocks */
	while(i<needed_blocks)
	{
		
		/* obtain the pointer to a free block */ 
		free_block_ptr=dequeue(&flb,&freeBlocks_no);
		
		/* prepare link to next block */
		sprintf(block_buffer,"%d",free_block_ptr);
		
		/* next block pointer writing */
		if(write(vdfs_fd,block_buffer,inode_ptr_size)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* delete block_buffer content */
		for(j=0;j<BLOCK_SIZE;j++)
			block_buffer[j]='\0';
			
		/* move read/write pointer on next free block on vdfs*/
		if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(free_block_ptr*BLOCK_SIZE),SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* read data block from source file */
		if(read(source_fd,block_buffer,BLOCK_SIZE-block_ptr_size)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* block writing */
		if(write(vdfs_fd,block_buffer,strlen(block_buffer))==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		i++;
	}
	/* close source file */
	close(source_fd);	
	
	/************************************************/
	/* ADD NEW FILE RECORD TO PARENT DIR DATA BLOCK */
	/************************************************/
	
	/* delete block_buffer content */
	for(j=0;j<BLOCK_SIZE;j++)
		block_buffer[j]='\0';
		
	char new_permutation[exp_result*(number_sizeof(BLOCK_SIZE/(MAX_FILENAME_SIZE+inode_ptr_size+2))+1)];
	int sub_block_position;																/* position a the free directory sub-block	*/
	char new_record[MAX_FILENAME_SIZE+inode_ptr_size+2];	/* string containig the new file sub-block	*/
	char *exp_dir_inode[DIR_INODE_TOKEN_NO];							/* string array containig the exploded dir inode	*/
	
	/* explode inode_buffer into exp_dir_inode */
	explode(original_parent_inode,exp_dir_inode,":");
	
	/* move read/write pointer on beginning of parent data block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read content of parent directory */
	if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}

	/* obtain position of the sub-block in parent directory and the new string of pointers */
	sub_block_position=new_ordered_permutation(block_buffer,exp_result,sub_blocks,new_filename,new_permutation);
	if(sub_block_position==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on beginning of ordered pointer list parent dir inode on vdfs*/
	if(lseek(vdfs_fd,(parent_inode*INODE_SIZE)+offset,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new parent inode ordered pointer list writing */
	if(write(vdfs_fd,new_permutation,strlen(new_permutation)+1)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* prepare new record for parent data block */
	sprintf(new_record,"%s:+%d",new_filename,free_inode_ptr);
	
	/* move read/write pointer to the first free sub-block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE)+(sub_block_position*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new file sub-block writing */
	if(write(vdfs_fd,new_record,MAX_FILENAME_SIZE+inode_ptr_size+2)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* close VDFS */
	close(vdfs_fd);
	
	return 1;
}

/* Copies a file from VD filesytem to VD filesytem
   PRE: params not null
   POST: returns 1 if successfully completed, -1 otherwise
	 PARAMS: string vd_path_from, string vd_path_to */
int cp_from_vd_to_vd(char *vd_path_from,char *vd_path_to)
{
	int free_inode_ptr;																							/* pointers to free inode 																					*/
	int free_block_ptr;																							/* pointers to free block 																					*/
	int parent_sub_block;																						/* pointer to parent sub-block 																			*/
	int needed_blocks;																							/* source file block number 																				*/
	int from_parent_inode;																					/* inode pointer of from directory 																	*/
	int to_parent_inode;																						/* inode pointer of to directory 																		*/
 	int exp_result;																									/* number of strings created by explode function 										*/
	int destination_sub_block;																			/* position of the sub-block in its parent directory 								*/
	int vdfs_fd;																										/* file descriptors of source OS fs file and destination VD fs file	*/
	char type;																											/* source file type 																								*/
	char block_buffer[BLOCK_SIZE];																	/* string for block writing 																				*/
	char inode_buffer[INODE_SIZE];																	/* string for inode writing 																				*/
	char parent_inode_buffer[INODE_SIZE];														/* string containing parent inode 																	*/
	char original_parent_inode[INODE_SIZE];													/* string containing parent inode 																	*/
	char source_inode_buffer[INODE_SIZE];														/* string containing source parent inode 														*/
	char *exp_source_inode[DIR_INODE_TOKEN_NO];											/* string array containing parent inode tokens 											*/
	char destination_inode_buffer[INODE_SIZE];											/* string containing destination parent inode 											*/
	char source_sub_block[MAX_FILENAME_SIZE+inode_ptr_size+2];			/* string containing source file sub-block 													*/
	char new_filename[MAX_FILENAME_SIZE+1];													/* new filename of OS filesytem source file 												*/
	char vd_original_path_from[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the originale vd_path_from 										*/
	char vd_original_path_to[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];		/* string containing the originale vd_path_to 											*/
	char *sub_blocks[max_files_per_dir];														/* string array containig the sub-blocks of a dir 									*/
	
	/* open vdfs file in read-write */
	vdfs_fd=open(VDFS,O_RDWR);
	if(vdfs_fd==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: fatal error opening VDFS!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save vd_path_from */
 	strcpy(vd_original_path_from,vd_path_from);
	
	/* try to move to source file parent directory */
	*(strrchr(vd_path_from,'/'))='\0';
	from_parent_inode=cd(vd_path_from);
	if(from_parent_inode==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_original_path_from);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,from_parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,source_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* explode parent dir inode */
	if(explode(source_inode_buffer,exp_source_inode,":")!=DIR_INODE_TOKEN_NO)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* find in parent directory the source file */
	parent_sub_block=find_in_dir(strrchr(vd_original_path_from,'/')+1,from_parent_inode);
	if(parent_sub_block==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_original_path_from);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to source file sub-block*/
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_source_inode[3])*BLOCK_SIZE)+(parent_sub_block*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read source file sub-block */
	if(read(vdfs_fd,source_sub_block,MAX_FILENAME_SIZE+inode_ptr_size+2)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer to source file inode*/
	if(lseek(vdfs_fd,atoi(strrchr(source_sub_block,':')+1)*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read source file inode */
	if(read(vdfs_fd,source_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* check if source is a regular file */
	type=source_inode_buffer[0];
	if(type!='f' && type!='l')
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		sprintf(output_buffer,"cp: %s is not a regular file or a link!\n",vd_original_path_from);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* explode source file inode */
	if(explode(source_inode_buffer,exp_source_inode,":")!=DIR_INODE_TOKEN_NO-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* check if filesytem has enough space */
	needed_blocks=atoi(exp_source_inode[4]);
	if(needed_blocks>freeBlocks_no)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: not enough space on VD Filesystem!\nTry removing some other file[s]\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save vd_path_to */
	strcpy(vd_original_path_to,vd_path_to);
	
	/* try to move to vd_path_to */
	to_parent_inode=cd(vd_path_to);
	
	/* if can't move to vd_path_to try to consider last part of vd_path_to as the new filename */
	if(to_parent_inode==-1)
	{
		sprintf(new_filename,"%s",strrchr(vd_original_path_to,'/')+1);
		
		/* if filename is too long truncate it */
		new_filename[MAX_FILENAME_SIZE]='\0';
		/* try to move to new_filename's parent dir */
		*(strrchr(vd_original_path_to,'/'))='\0';
		strcpy(vd_path_to,vd_original_path_to);
		to_parent_inode=cd(vd_path_to);
	
		/* if parent_inode is still -1, this mean that the path doesn't exists */
		if(to_parent_inode==-1)
		{
			/* write error output to client */
			sprintf(output_buffer,"%ccp: cannot stat '%s' :No such file or directory\n",err_char,vd_path_to);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		strcat(vd_original_path_to,"/");
	}
	
	/* if can move to vd_path_to the new filename is the old one */
	else
		sprintf(new_filename,"%s",strrchr(vd_original_path_from,'/')+1);
	
	if(strcmp(vd_string,vd_original_path_to))
	{
		strcat(vd_original_path_to,"/");
	}
	
	/* save original vd_path_to with new filename */
	strcat(vd_original_path_to,new_filename);
	
	/* move read/write pointer to destination dir inode*/
	if(lseek(vdfs_fd,to_parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read destination dir inode */
	if(read(vdfs_fd,destination_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save destination inode string */
	char original_destination_inode[INODE_SIZE];	/* parent inode backup string */
	strcpy(original_destination_inode,destination_inode_buffer);
	
	int offset=0;	/* offset of the pointer list of parent directory */
	/* get on sub_blocks string array the pointers of parent directory */
	exp_result=get_sub_block_ptrs(destination_inode_buffer,sub_blocks,&offset);
	
	/* check if dir can contain a new file */
	if(exp_result==max_files_per_dir)
	{
		/* write error output to client */
		sprintf(output_buffer,"%ccp: cannot store files in %s anymore!\n",err_char,vd_path_to);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if file already exists remove it*/
	destination_sub_block=find_in_dir(new_filename,to_parent_inode);
	if(destination_sub_block!=-1)
	{
		char cmd[]="rm";
		char *rm_cmd[2];
		
		rm_cmd[0]=cmd;
		rm_cmd[1]=vd_original_path_to;
		if(rm(2,rm_cmd)==-1)
		{
			/* write error output to client */
			printf(output_buffer,"%ccp: cannot overwrite '%s' :No such file or directory\n",err_char,vd_path_to);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;	
		}
		/* decrement number of directory sub-blocks */
		exp_result--;
		
		/* move read/write pointer to destination dir inode */
		if(lseek(vdfs_fd,to_parent_inode*INODE_SIZE,SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	
		/* refresh parent dir inode */
		if(read(vdfs_fd,destination_inode_buffer,INODE_SIZE)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	
		/* save parent inode string */
		strcpy(original_parent_inode,parent_inode_buffer);
	
		/* get on sub_blocks string array the pointers of parent directory */
		exp_result=get_sub_block_ptrs(destination_inode_buffer,sub_blocks,&offset);
	}
	
	/* obtain the pointer to a free i-node */ 
	free_inode_ptr=dequeue(&fli,&freeInodes_no);
	/* obtain the pointer to a free block */ 
	free_block_ptr=dequeue(&flb,&freeBlocks_no);
	
	/* check if file system is full */
	if(free_block_ptr==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: file system is full!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/******************/
	/* I-NODE WRITING */
	/******************/
	
	char date[DATE_SIZE];
	
	/* get system date */
	getdate(date);
	
	/*prepare inode_buffer */
	sprintf(inode_buffer,"%c:%s:%d:%d:%d",type,date,1,free_block_ptr,needed_blocks);
	
	/* move read/write pointer on the free i-node */
	if(lseek(vdfs_fd,free_inode_ptr*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* i-node writing */
	if(write(vdfs_fd,inode_buffer,strlen(inode_buffer))==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/*****************/
	/* BLOCK WRITING */
	/*****************/
	
	int i=1; 										/* generic counter */
	int j; 											/* generic counter */
	int source_block_read_ptr;	/* pointer to the current source block to read */
	
	/* initialize first source data block */
	source_block_read_ptr=atoi(exp_source_inode[3]);
	
	/* FIRST BLOCK WRITING */
	/* delete block_buffer content */
	for(j=0;j<BLOCK_SIZE;j++)
		block_buffer[j]='\0';
	
	/* move read/write pointer on the first source data block on vdfs*/
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(source_block_read_ptr*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read first data block from source file */
	if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* obtain next source data block pointer */
	if(needed_blocks>1)
		source_block_read_ptr=atoi(block_buffer+BLOCK_SIZE-block_ptr_size);
	
	/* move read/write pointer on the first free block on vdfs*/
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(free_block_ptr*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if file takes only one block write it for its lenght */
	if(needed_blocks==1)
	{
		/* block writing */
		if(write(vdfs_fd,block_buffer,strlen(block_buffer)+1)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	else
	{
		/* block writing */
		if(write(vdfs_fd,block_buffer,strlen(block_buffer)-block_ptr_size+1)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	
	/* copy other blocks */
	while(i<needed_blocks)
	{
		/* obtain the pointer to a free block */ 
		free_block_ptr=dequeue(&flb,&freeBlocks_no);
		
		/* prepare link to next block */
		sprintf(block_buffer,"%d",free_block_ptr);
		
		/* next block pointer writing */
		if(write(vdfs_fd,block_buffer,inode_ptr_size)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* delete block_buffer content */
		for(j=0;j<BLOCK_SIZE;j++)
			block_buffer[j]='\0';
			
		/* move read/write pointer on the source data block on vdfs*/
		if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(source_block_read_ptr*BLOCK_SIZE),SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	
		/* read data block from source file */
		if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* move read/write pointer on next free block on vdfs*/
		if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(free_block_ptr*BLOCK_SIZE),SEEK_SET)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		
		/* block writing */
		if(write(vdfs_fd,block_buffer,strlen(block_buffer)-block_ptr_size+1)==-1)
		{
			/* write error output to client */
			strcpy(output_buffer,&err_char);
			strcat(output_buffer,"cp: fatal error!\n");
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
		i++;
		
		/* obtain next source data block pointer */
		if(i!=needed_blocks)
			source_block_read_ptr=atoi(block_buffer+BLOCK_SIZE-block_ptr_size);
	}
	
	/************************************************/
	/* ADD NEW FILE RECORD TO PARENT DIR DATA BLOCK */
	/************************************************/
	
	/* delete block_buffer content */
	for(j=0;j<BLOCK_SIZE;j++)
		block_buffer[j]='\0';
		
	char new_permutation[exp_result+2];
	int sub_block_position;																/* position a the free directory sub-block	*/
	char new_record[MAX_FILENAME_SIZE+inode_ptr_size+2];	/* string containig the new file sub-block	*/
	char *exp_dir_inode[DIR_INODE_TOKEN_NO];							/* string array containig the exploded dir inode	*/
	
	/* explode inode_buffer into exp_dir_inode */
	explode(original_destination_inode,exp_dir_inode,":");
	
	/* move read/write pointer on beginning of parent data block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read content of parent directory */
	if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}

	/* obtain position of the sub-block in parent directory and the new string of pointers */
	sub_block_position=new_ordered_permutation(block_buffer,exp_result,sub_blocks,new_filename,new_permutation);
	if(sub_block_position==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on beginning of ordered pointer list parent dir inode on vdfs*/
	if(lseek(vdfs_fd,(to_parent_inode*INODE_SIZE)+offset,SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new parent inode ordered pointer list writing */
	if(write(vdfs_fd,new_permutation,strlen(new_permutation)+1)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* prepare new record for parent data block */
	sprintf(new_record,"%s:+%d",new_filename,free_inode_ptr);
	
	/* move read/write pointer to the first free sub-block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE)+(sub_block_position*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new file sub-block writing */
	if(write(vdfs_fd,new_record,MAX_FILENAME_SIZE+inode_ptr_size+2)==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"cp: fatal error!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* close VDFS */
	close(vdfs_fd);
	
	return 1;
}


/* Calculates the size(digits) of the number
   PRE: none
   POST: returns the size of the passed number
   PARAMS: none */
int number_sizeof(int number)
{
	int size=0;	/* size of the number */
	
	while(number>1)
	{
		number/=10;
		size++;
	}
	return size;
}

/* Creates an hard link from VDFS to VDFS
   PRE: both params are a valid VDFS path
   POST: creates an hard link from VDFS to VDFS and returns 1 if successfully completed, -1 otherwise
   PARAMS: source file, link destination */
int ln_hard(char *source,char *destination)
{
	char *source_filename;																					/* name of source file 															*/
	int found_inode;																								/* i-node to link at 																*/
	int parent_inode;																								/* pointer to destination parent dir inode 					*/
	int current_dir;																								/* pointer to source parent dir inode 							*/
	int parent_sub_block;																						/* pointer to parent sub-block 											*/
	int exp_result;																									/* number of strings created by explode function 		*/
	char *sub_blocks[max_files_per_dir];														/* string array containig the sub-blocks of a dir		*/
	int vdfs_fd;																										/* file descriptors of destination VD fs file 			*/
	char block_buffer[BLOCK_SIZE];																	/* strings for inode and inode writing					 		*/
	char parent_inode_buffer[INODE_SIZE];														/* string containing parent inode 									*/
	char new_filename[MAX_FILENAME_SIZE+1];													/* new filename of OS filesytem source file 				*/
	char original_destination[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the original destination path	*/
	char original_source[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];				/* string containing the original source path 			*/
	int tmp_ptr;																										/* temporary pointer 																*/
	
	/* save paths */
	strcpy(original_destination,destination);
	strcpy(original_source,source);
	
	/* try to move to path */
	parent_inode=cd(destination);
	
	/* if can't move to destination try to consider last part of destination as the new filename */
	if(parent_inode==-1)
	{
		/* if new filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(original_destination,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(original_destination,'/')+1);
		
		/* try to move to new_filename's parent dir */
		*(strrchr(original_destination,'/'))='\0';
		parent_inode=cd(original_destination);
	
		/* if parent_inode is still -1, this mean that the path doesn't exists */
		if(parent_inode==-1)
		{
			/* write error output to client */
			sprintf(output_buffer,"%cln: cannot stat '%s' :No such file or directory\n",err_char,destination);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	else
	{
		/* if old filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(source,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(source,'/')+1);
		strcat(original_destination,"/");
	}
	
	if(strcmp(vd_string,original_destination))
	{
		strcat(original_destination,"/");
	}
	
	/* save original destination with new filename */
	strcat(original_destination,new_filename);
	
	vdfs_fd=open(VDFS,O_RDWR);
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,parent_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save parent inode string */
	char original_parent_inode[INODE_SIZE];	/* parent inode backup string */
	strcpy(original_parent_inode,parent_inode_buffer);
	
	int offset=0;	/* offset of the pointer list of parent directory */
	/* get on sub_blocks string array the pointers of parent directory */
	exp_result=get_sub_block_ptrs(parent_inode_buffer,sub_blocks,&offset);
	
	/* check if dir can contain a new file */
	if(exp_result==max_files_per_dir)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: cannot store files in %s anymore!\n",err_char,destination);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if file already exists send error*/
	parent_sub_block=find_in_dir(new_filename,parent_inode);
	if(parent_sub_block!=-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: `%s/%s': File exists\n",err_char,destination,new_filename);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* Separate directory's path and linking file name */
	source_filename=strrchr(original_source,'/');
	if (source_filename==NULL)
	{
		/* If wrong path send error message */
		sprintf(output_buffer,"%cln: %s: invalid path!\n",err_char,source);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	*source_filename='\0';
	source_filename++;
	/* Get current directory's i-node address */
	current_dir=cd(original_source);
	if(current_dir<0)
	{
		/* If unexistent path send error message */
		sprintf(output_buffer,"%cln: %s: no such file or directory\n",err_char,original_source);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	/* Get linking element index */
	if((tmp_ptr=find_in_dir(source_filename,current_dir))<0)
	{
		/* If it doesn't exist send error message */
		sprintf(output_buffer,"%cln: %s: no such file or directory\n",err_char,original_source);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		close(vdfs_fd);
		return -1;
	}
	
	/* move read/write pointer on beginning of parent inode */
	if(lseek(vdfs_fd,(current_dir*INODE_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char tmp_buffer[INODE_SIZE];
	
	/* read parent dir inode */
	if(read(vdfs_fd,tmp_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char *tmp_tok[DIR_INODE_TOKEN_NO];
	
	if (explode(tmp_buffer,tmp_tok,":")<DIR_INODE_TOKEN_NO-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on linking file's sub-block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(BLOCK_SIZE*atoi(tmp_tok[3]))+(tmp_ptr*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char tmp_sb_buffer[MAX_FILENAME_SIZE+inode_ptr_size+2];
	
	if(read(vdfs_fd,tmp_sb_buffer,(MAX_FILENAME_SIZE+inode_ptr_size+2))==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	if (*(strchr(tmp_sb_buffer,':')+1)=='-')
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: cannot stat `%s': no such file or directory\n",err_char,original_source);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save linking file inode address */
	found_inode=atoi(strchr(tmp_sb_buffer,':')+1);
	
	/* move read/write pointer on linking file's inode */
	if(lseek(vdfs_fd,found_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char tmp_inode_buf[INODE_SIZE];
	
	if(read(vdfs_fd,tmp_inode_buf,INODE_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* check if the source file is a directory */
	if(tmp_inode_buf[0]=='d')
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: can't make an hard link to a directory!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char *exp_inode[DIR_INODE_TOKEN_NO];
	explode(tmp_inode_buf,exp_inode,":");
	sprintf(tmp_inode_buf,"%s:%s:%d:%s:%s",exp_inode[0],exp_inode[1],(atoi(exp_inode[2])+1),exp_inode[3],exp_inode[4]);
	
	/* move read/write pointer on linking file's inode */
	if(lseek(vdfs_fd,found_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	if(write(vdfs_fd,tmp_inode_buf,INODE_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	
	char *exp_dir_inode[DIR_INODE_TOKEN_NO];							/* string array containig the exploded dir inode	*/
	
	/* explode inode_buffer into exp_dir_inode */
	explode(original_parent_inode,exp_dir_inode,":");
	
	/* move read/write pointer on beginning of parent data block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read content of parent directory */
	if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	int sub_block_position;
	char new_permutation[exp_result*(number_sizeof(BLOCK_SIZE/(MAX_FILENAME_SIZE+inode_ptr_size+2))+1)];
	/* obtain position of the sub-block in parent directory and the new string of pointers */
	sub_block_position=new_ordered_permutation(block_buffer,exp_result,sub_blocks,new_filename,new_permutation);
	if(sub_block_position==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on beginning of ordered pointer list parent dir inode on vdfs*/
	if(lseek(vdfs_fd,(parent_inode*INODE_SIZE)+offset,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new parent inode ordered pointer list writing */
	if(write(vdfs_fd,new_permutation,strlen(new_permutation)+1)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	char new_record[MAX_FILENAME_SIZE+inode_ptr_size+2];
	/* prepare new record for parent data block */
	sprintf(new_record,"%s:+%d",new_filename,found_inode);
	
	/* move read/write pointer to the first free sub-block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE)+(sub_block_position*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new file sub-block writing */
	if(write(vdfs_fd,new_record,strlen(new_record)+1)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* close VDFS */
	close(vdfs_fd);
	return 1;
}

/* Creates a soft link from source to destination 
   PRE: second param is a valid VDFS path
   POST: creates a soft link from source to destination and returns 1 if successfully completed, -1 otherwise
   PARAMS: source file, link destination */
int ln_soft(char *source,char *destination)
{
	int free_inode_ptr;																							/* pointer to free inode 													*/
	int free_block_ptr;																							/* pointer to free block												 	*/
	int parent_inode;																								/* pointer to parent dir inode									 	*/
	int parent_sub_block;																						/* pointer to parent sub-block 										*/
	int exp_result;																									/* number of strings created by explode function	*/
	int vdfs_fd;																										/* file descriptors of destination VD fs file 		*/
	char block_buffer[BLOCK_SIZE];																	/* strings for block writing 											*/
	char inode_buffer[INODE_SIZE];																	/* strings for inode writing 											*/
	char parent_inode_buffer[INODE_SIZE];														/* string containing parent inode 								*/
	char new_filename[MAX_FILENAME_SIZE+1];													/* new filename of OS filesytem source file 			*/
	char original_destination[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];	/* string containing the original destination 		*/
	char original_source[((MAX_FILENAME_SIZE+1)*INODE_NO)+1];				/* string containing the original source 					*/
	char *sub_blocks[max_files_per_dir];														/* string array containig the sub-blocks of a dir	*/
	
	/* save path */
	strcpy(original_destination,destination);
	strcpy(original_source,source);
	
	/* try to move to path */
	parent_inode=cd(destination);
	
	/* if can't move to destination try to consider last part of destination as the new filename */
	if(parent_inode==-1)
	{
		/* if new filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(original_destination,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(original_destination,'/')+1);
		
		/* try to move to new_filename's parent dir */
		*(strrchr(original_destination,'/'))='\0';
		parent_inode=cd(original_destination);
	
		/* if parent_inode is still -1, this mean that the path doesn't exists */
		if(parent_inode==-1)
		{
			/* write error output to client */
			sprintf(output_buffer,"%cln: cannot stat '%s' :No such file or directory\n",err_char,destination);
			write(client_fd,output_buffer,strlen(output_buffer)+1);
			return -1;
		}
	}
	else
	{
		/* if old filename is longer than MAX_FILENAME_SIZE truncate it */
		*(strrchr(source,'/')+1+MAX_FILENAME_SIZE)='\0';
		sprintf(new_filename,"%s",strrchr(source,'/')+1);
		strcat(original_destination,"/");
	}
	
	if(strcmp(vd_string,original_destination))
	{
		strcat(original_destination,"/");
	}
	
	/* save originale destination with new filename */
	strcat(original_destination,new_filename);
	
	vdfs_fd=open(VDFS,O_RDWR);
	
	/* move read/write pointer to dir inode of vdfs*/
	if(lseek(vdfs_fd,parent_inode*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read parent dir inode */
	if(read(vdfs_fd,parent_inode_buffer,INODE_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* save parent inode string */
	char original_parent_inode[INODE_SIZE];	/* parent inode backup string */
	strcpy(original_parent_inode,parent_inode_buffer);
	
	int offset=0;	/* offset of the pointer list of parent directory */
	/* get on sub_blocks string array the pointers of parent directory */
	exp_result=get_sub_block_ptrs(parent_inode_buffer,sub_blocks,&offset);
	
	/* check if dir can contain a new file */
	if(exp_result==max_files_per_dir)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: cannot store files in %s anymore!\n",err_char,destination);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* if file already exists send error */
	parent_sub_block=find_in_dir(new_filename,parent_inode);
	if(parent_sub_block!=-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: `%s/%s': File exists\n",err_char,destination,new_filename);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* obtain the pointer to a free i-node */
	free_inode_ptr=dequeue(&fli,&freeInodes_no);
	/* obtain the pointer to a free block */ 
	free_block_ptr=dequeue(&flb,&freeBlocks_no);
	
	/* check if file system is full */
	if(free_block_ptr==-1)
	{
		/* write error output to client */
		strcpy(output_buffer,&err_char);
		strcat(output_buffer,"ln: file system is full!\n");
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char date[DATE_SIZE];	/* string containing the date */
	
	/* get system date */
	getdate(date);
	
	/* create inode_buffer */
	sprintf(inode_buffer,"%c:%s:%d:%d:%d",'l',date,1,free_block_ptr,1);
	
	/* move read/write pointer on the free i-node */
	if(lseek(vdfs_fd,free_inode_ptr*INODE_SIZE,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* i-node writing */
	if(write(vdfs_fd,inode_buffer,strlen(inode_buffer))==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on the first free block on vdfs */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(free_block_ptr*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* block writing */
	if(write(vdfs_fd,original_source,strlen(original_source)+1)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	char *exp_dir_inode[DIR_INODE_TOKEN_NO];	/* string array containig the exploded dir inode	*/
	
	/* explode inode_buffer into exp_dir_inode */
	explode(original_parent_inode,exp_dir_inode,":");
	
	/* move read/write pointer on beginning of parent data block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* read content of parent directory */
	if(read(vdfs_fd,block_buffer,BLOCK_SIZE)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	int sub_block_position;
	char new_permutation[exp_result*(number_sizeof(BLOCK_SIZE/(MAX_FILENAME_SIZE+inode_ptr_size+2))+1)];
	/* obtain position of the sub-block in parent directory and the new string of pointers */
	sub_block_position=new_ordered_permutation(block_buffer,exp_result,sub_blocks,new_filename,new_permutation);
	if(sub_block_position==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* move read/write pointer on beginning of ordered pointer list parent dir inode on vdfs*/
	if(lseek(vdfs_fd,(parent_inode*INODE_SIZE)+offset,SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new parent inode ordered pointer list writing */
	if(write(vdfs_fd,new_permutation,strlen(new_permutation)+1)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	char new_record[MAX_FILENAME_SIZE+inode_ptr_size+2];
	/* prepare new record for parent data block */
	sprintf(new_record,"%s:+%d",new_filename,free_inode_ptr);
	
	/* move read/write pointer to the first free sub-block */
	if(lseek(vdfs_fd,(INODE_NO*INODE_SIZE)+(atoi(exp_dir_inode[3])*BLOCK_SIZE)+(sub_block_position*(MAX_FILENAME_SIZE+inode_ptr_size+2)),SEEK_SET)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* new file sub-block writing */
	if(write(vdfs_fd,new_record,strlen(new_record)+1)==-1)
	{
		/* write error output to client */
		sprintf(output_buffer,"%cln: fatal error!\n",err_char);
		write(client_fd,output_buffer,strlen(output_buffer)+1);
		return -1;
	}
	
	/* close VDFS */
	close(vdfs_fd);
	return 1;
}

/* Writes on array string the sub-block pointers of a directory and the 
		position of the pointers list begininng in the dir inode, NULL if directory is empty
   PRE: none
   POST: returns the number of pointers, -1 if failed (for example inode is a file) 
   PARAMS: string dir_inode, string array of sub-block pointers, pointer to the offset */
int get_sub_block_ptrs(char *dir_inode,char **pointers,int *offset)
{
	
	char *exp_dir_inode[DIR_INODE_TOKEN_NO];	/* string array containig the exploded dir inode	*/
	
	/* writes on dir_inode lenght */
	*offset=strlen(dir_inode);
	
	/* test if directory is empty */
	if(strrchr(dir_inode,':')==dir_inode+strlen(dir_inode)-1)
	{
		pointers=NULL;
		return 0;
	}
	
	/* explode inode_buffer into exp_dir_inode */
	if(explode(dir_inode,exp_dir_inode,":")!=DIR_INODE_TOKEN_NO)
		return -1;
	
	/* check if dir_inode is really a directory inode */
	if(*(exp_dir_inode[0])!='d')
		return -1;
	
	/* get offset of the pointer list start */
	(*offset)-=strlen(exp_dir_inode[DIR_INODE_TOKEN_NO-1]);
	
	/* explode the pointer list and return */
	return explode(exp_dir_inode[DIR_INODE_TOKEN_NO-1],pointers,"-");
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

