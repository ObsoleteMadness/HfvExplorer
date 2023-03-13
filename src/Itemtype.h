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

typedef struct {
	CatKeyRec catkeyrec;
	CatDataRec catdatarec;
	// internals
	int search_type;
	LongInt id;
	LongInt id2;
	int volinx;
	int index_to_found;
	int index_found;
	int next_index;
	// CString *searchname;
	char searchname[_MAX_PATH];
} hfs_item_type;

#ifdef __cplusplus
extern "C" {
#endif

int HFSIFACE_get_dir_first( char *volpath, char *path, hfs_item_type *pitem );
int HFSIFACE_get_dir_next( hfs_item_type *pitem );
// This must be called if breaking out of the get loop before completion
void HFSIFACE_get_dir_flush( void );
CatDataRec *HFSIFACE_get_last_Icon13( void );

#ifdef __cplusplus
}
#endif
