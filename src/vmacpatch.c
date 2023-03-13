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
#include "vmacpatch.h"

#define ENOUGH			10000
#define MAXINSERT		1000

#define SS_DEVICE1_OFFSET 0x1c
#define SS_DEVICE2_OFFSET 0x6c
#define SS_DEVICE_LENGTH  0x50

// special means that patch only if the old entry doesn't contain backslashes
int patch_entry( char *lines, int linlen, char *entry, char *newval, BOOL special )
{
	char e[200], *p = 0, *start, *end;
	int oldlen, newlen, count;
	int ret = 0;

	e[0] = 0x0d;
	e[1] = 0x0a;
	strcpy( &e[2], entry );

	p = lines;
	for(;;) {
		p = strstr( p, e );
		if(!p) break;
		p += strlen(e);
		if(*p == ' ' || *p == '=') {
			// got it
			while(*p == ' ' || *p == '=') p++;
			if(*p < 32) break;
			start = p;
			while(*p >= 32) p++;
			end = p;
			if(special) {
				for(p=start; p!=end; p++) {
					if(*p == '\\') return(0);
				}
				// do not overwrite but insert at beginning
				end = start;
			}
			oldlen = (int)( (unsigned int)end - (unsigned int)start );
			newlen = strlen(newval);
			count = linlen - (int)((unsigned int)start - (unsigned int)lines) + 1;
			if(newlen > oldlen) {
				memmove( start + (newlen-oldlen), start, count );
			} else if(newlen < oldlen) {
				memmove( start, start + (oldlen-newlen), count );
			}
			memmove( start, newval, newlen );
			ret = 1;
			break;
		}
	}
	return(ret);
}

// Not using mfc, needed elsewhere too
int patch_vmac_ini( char *vmacini, char *vmacdir, char *hfv1, char *hfv2 )
{
	int bytes, ret = 0;
	char *lines = 0;
	HFILE hf;

	lines = (char *)malloc(ENOUGH+1+MAXINSERT);
	if(!lines) return(0);

	hf = _lopen( vmacini, OF_READWRITE );
	if(hf != HFILE_ERROR) {
		bytes = _lread( hf, lines, ENOUGH );
		if(bytes > 0) {
			lines[bytes] = 0;
			if(vmacdir) {
				char dirwithbackslah[200];
				sprintf( dirwithbackslah, "%s\\", vmacdir );
				if(patch_entry( lines, bytes, "ROMPath1", dirwithbackslah, TRUE )) {
					bytes = strlen(lines);
					patch_entry( lines, bytes, "CurrentROMPath", "1", FALSE );
					bytes = strlen(lines);
				}
			}
			bytes = strlen(lines);
			if(hfv1) patch_entry( lines, bytes, "DrivePath1", hfv1, FALSE );
			bytes = strlen(lines);
			if(hfv2) patch_entry( lines, bytes, "DrivePath2", hfv2, FALSE );
			bytes = strlen(lines);
			ret = 1;
		}
		_lclose(hf);
		if(ret) {
			ret = 0;
			_unlink(vmacini);
			hf = _lcreat( vmacini, 0 );
			if(hf != HFILE_ERROR) {
				if((int)_lwrite( hf, lines, bytes ) == bytes) ret = 1;
				_lclose(hf);
			}
		}
	}
	if(lines) free(lines);
	return(ret);
}

int patch_ss_prefs( char *ss_prefs, char *hfv1, char *hfv2 )
{
	int bytes, ret = 0;
	char *lines = 0;
	HFILE hf;

	lines = (char *)malloc(ENOUGH+1);
	if(!lines) return(0);

	hf = _lopen( ss_prefs, OF_READWRITE );
	if(hf != HFILE_ERROR) {
		bytes = _lread( hf, lines, ENOUGH );
		if(bytes > 0 && bytes > SS_DEVICE2_OFFSET+SS_DEVICE_LENGTH) {
			lines[bytes] = 0;
			if(hfv1) {
				memset( &lines[SS_DEVICE1_OFFSET], 0, SS_DEVICE_LENGTH );
				strncpy( &lines[SS_DEVICE1_OFFSET], hfv1, SS_DEVICE_LENGTH-1 );
			}
			if(hfv2) {
				memset( &lines[SS_DEVICE2_OFFSET], 0, SS_DEVICE_LENGTH );
				strncpy( &lines[SS_DEVICE2_OFFSET], hfv2, SS_DEVICE_LENGTH-1 );
			}
			ret = 1;
		}
		_lclose(hf);
		if(ret) {
			ret = 0;
			_unlink(ss_prefs);
			hf = _lcreat( ss_prefs, 0 );
			if(hf != HFILE_ERROR) {
				if((int)_lwrite( hf, lines, bytes ) == bytes) ret = 1;
				_lclose(hf);
			}
		}
	}
	if(lines) free(lines);
	return(ret);
}
