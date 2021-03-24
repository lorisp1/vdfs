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

#ifndef _server_conf_h
#define _server_conf_h

/********************
 * GENERAL SETTINGS * 
 ********************/

/* Max number of tokens making a request */
#define MAX_TOKENS 4

/***********************
 * FILESYSTEM SETTINGS * 
 ***********************/

/* Absolute path of file containing the file system */
#define VDFS "/tmp/vdfs.mt"

/* File system's data block number */
#define BLOCK_NO 50

/* Data block size (Bytes)*/
#define BLOCK_SIZE 4096

/* File system's i-node number */
#define INODE_NO 50

/* I-node size (Bytes)*/
#define INODE_SIZE 30

/* Max size(chars) of a file name (regular file, link or dir) */
#define MAX_FILENAME_SIZE 8


/*******************
 * SOCKET SETTINGS * 
 *******************/

/* Default socket protocol */
#define DEFAULT_PROTOCOL 0

/* Absolute path of the socket file */
#define SOCKET_NAME "/tmp/vd_socket.mt"

/* Max pending connections number */
#define MAX_CONNECTIONS 5


#endif
