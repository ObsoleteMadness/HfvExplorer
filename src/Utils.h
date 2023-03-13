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

#ifndef _UTILS_H_
#define _UTILS_H_

extern int debug;
extern int shadow;

extern int systemiconsize;

enum { DI_NO_ICONS=0, DI_STANDARD_ICONS, DI_PREFER_INDIVIDUAL_ICONS };

extern RGBQUAD exact_mac256[256];

// conversions
void long_to_string( unsigned long u, unsigned char *s );
unsigned long string_to_long( unsigned char *s );
void negate_buffer( unsigned char *p, int count );
// Explorer-style: if old DOS name: NAME5678.EXT -> Name5678.ext
void dos_lfn( unsigned char *name );
void dos_lfn_fdata( WIN32_FIND_DATA *fd );
void rotate_buffer( unsigned char *p, int bytes, int small );

// file name, pc
int is_valid_8_3_name( unsigned char *name );
int isdirectory( const CString &fpath );
int is_extension( char *p, char *ext ) ;
int is_fat_root_directory( char *dir );
int get_fat_parent_directory( char *dir, char *parent );
void get_extension( const char *p, char *extension );

// file name, mac
int get_mac_root_dir( CString name, CString &dir );

// file
void silent_close( CFile *fp );
void silent_delete( CString &name );
int do_create_file( CFile *fp, CString name );
int do_open_file( CFile *fp, CString name );
int do_open_readwrite( CFile *fp, CString name );

// color
HICON map_colors( unsigned char *p, int small, int count, unsigned char **cim );
void init_map_palette();
HBITMAP create_mask_bitmap(int count);
void delete_mask_bitmaps();

void dump_icon( HICON hicon ); //debug

#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif
int set_file_size( char *path, DWORD new_size );
BOOL is_floppy_by_char( char letter );
BOOL is_floppy_by_index( int index );

BOOL can_have_exclusive_access( LPCSTR path );

#ifdef __cplusplus
}
#endif

#endif // _UTILS_H_

