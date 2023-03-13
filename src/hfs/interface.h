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

#ifdef __cplusplus
extern "C" {
#endif

// #ifndef HFS_CNID_ROOTPAR
// #include "libhfs\hfs.h"
// #endif

void hfs_tester(void);

void HFSIFACE_init( HWND hwnd );

int HFSIFACE_delete( char *volpath, char *path );
int HFSIFACE_copy( 
	char *sourcevol, 
	char *source, 
	char *targetvol, 
	char *target,
	int is_dir_copy,
	int move_if_possible,
	int *pwas_moved );
int HFSIFACE_rename( char *volpath, char *oldname, char *newname );
int HFSIFACE_mkdir( char *volpath, char *path );
int HFSIFACE_rmdir( char *volpath, char *path );
int HFSIFACE_format( char *volpath, char *vname, long size, BOOL initialize );

int HFSIFACE_mount(char *path);
int HFSIFACE_umount(char *path);

int HFSIFACE_silencer( int shutup );

int HFSIFACE_get_properties( char *volpath, char *path, hfsdirent *pent );
int HFSIFACE_set_properties( char *volpath, char *path, hfsdirent *pent );

BOOL HFSIFACE_exists_split( int volinx, char *volpath, char *dir_name, char *name );


void make_companion_name( char *target, char *source, int fork );

void HFSIFACE_set_copy_modes( int outmode, int inmode, int appledouble );
void HFSIFACE_set_modeout( int outmode );
void HFSIFACE_set_modein( int inmode, int appledouble );

int mount_flags( char *path );

#ifdef __cplusplus
} //extern "C"
#endif
