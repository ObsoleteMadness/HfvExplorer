/*
 * HFVExplorer
 * Copyright (C) 1997-1998 by Anygraaf Oy
 * Author: Lauri Pesonen, email: lpesonen@clinet.fi or lauri.pesonen@anygraaf.fi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aspecial.h"

unsigned char unhex2( unsigned char *s )
{
	char hex[10];
	sprintf( hex, "0x%c%c", s[0], s[1] );
	return( (unsigned char)strtoul( hex, 0, 0 ) );
}

int ishexchar( unsigned char ch )
{
	if(isdigit(ch)) return(1);
	ch = toupper(ch);
	return( ch >= 'A' && ch <= 'F' );
}

void remove_ardi_special_chars( unsigned char *name )
{
	int i, len, j=0;

	len = strlen((char *)name);
	for(i=0; i<len; i++) {
		if( i<len-2 && name[i] == '%' && 
				ishexchar(name[i+1]) && ishexchar(name[i+2]))
		{
			name[j] = unhex2(&name[i+1]);
			i += 2;
		} else {
			name[j] = name[i];
		}
		j++;
	}
	name[j] = 0;
}
