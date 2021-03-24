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

#include "mytime.h"	/* date and time functions */

/* Returns system date
   PRE: char *buff's size must be at least 11
   POST: writes on char *buff the system date like "YYYY-mm-dd"
   PARAMS: char *buff */
void getdate(char *buff)
{
	struct tm *timep; /* pointer to a structure defined in time.h library	*/
	time_t now; 			/* longint redefined in time.h library							*/
	
	/* read system time */
	time(&now);
	/* formatting time like a struct tm */
	timep=localtime(&now);
	/* create the string */
	strftime(buff,DATE_SIZE,"%Y-%m-%d",timep);
}	
