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

#include "list.h"	/* list functions */

/* Enqueue free i-node or data block
   PRE: not empty file system
   POST: returns 1 if successfully enqueued, 0 otherwise
   PARAMS: list* (Pointer to a FreeList), phisical address (of a free i-node or data block), freeElements*  (Pointer to an int variable that will contain the number of elements in the FreeList)*/
int enqueue(FreeList *list, int address, int* fe)
{
	/* Case of empty file system ****ERROR**** */
	if (*fe==BLOCK_NO)
		return 0;
	
	if (*list==NULL)
	{
		/* Case of empty list */
		*list=(FreeList)malloc(sizeof(FreeItem));
		if (*list==NULL)
			return 0;
		(*list)->addr=address;
		(*list)->next=NULL;
		(*fe)++;
		return 1;
	}
	/* Case of non-empty list */
	return enqueue(&((*list)->next),address,fe);
}

/* Returns a free item and removes it from the list 
   PRE: none
   POST: returns the address of the first free element of the list and removes it, -1 if the file system is full
   PARAMS: list* (Pointer to a FreeList), freeElements* (Pointer to an int variable that will contain the number of elements in the FreeList)  */
int dequeue(FreeList* list, int* fe) {
	FreeList aux;
	int tmp;
	
	/* Case of full file system */
	if (*fe==0)
		return -1;
	
	tmp=(*list)->addr;
	aux=*list;
	*list=(*list)->next;
	free(aux);
	(*fe)--;
	return tmp;
}
