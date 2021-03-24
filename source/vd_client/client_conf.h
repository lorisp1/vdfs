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

#ifndef _client_conf_h
#define _client_conf_h

/********************
 * GENERAL SETTINGS * 
 ********************/

/* Size of the verbose string */
#define VERBOSE_STRING_SIZE 500

/*******************
 * SOCKET SETTINGS * 
 *******************/

/* Default socket protocol */
#define DEFAULT_PROTOCOL 0

/* Absolute path of the socket file */
#define SOCKET_NAME "/tmp/vd_socket.mt"

/* Time(s) to wait before retrying to connect */
#define SLEEP_TIME 1

/* Max number of connection tries */
#define MAX_CONN_TRIES 2

/* Max request string size */
#define MAX_REQUEST_SIZE 2000

#endif
