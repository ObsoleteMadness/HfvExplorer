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

#include "cache.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int m_C0;
	int m_H0;
	int m_S0;
	int m_H0_S0;
	int sector_size;
	int is_inited;
	cachetype cache;
	HANDLE handle;
	long seekpoint;
	int refcount;
	int type;
	ULONG volume_start;
} flcd_device_type;
#define MAX_DEVICES 26
#define MAX_FLOPPIES 2
enum { FLTYPE_FLOPPY=0, FLTYPE_CD, FLTYPE_HD };
extern flcd_device_type flcds[MAX_DEVICES];

void cd_log_enable( int enable, char *path );
void cd_log_restore_enable( int enable, char *path );

void floppy_final( int drive );
HANDLE floppy_init( int drive, BOOL check_geometry );
int floppy_read( HANDLE hFloppy, int drive, ULONG LBA, int count, char *buf );
int floppy_write( HANDLE hFloppy, int drive, ULONG LBA, int count, char *buf );
void floppy_get_geometry( 
	int drive,
	int *pC, int *pH, int *pS, int *psector_size
);

void get_floppy_volume_file_name( int drive, char *name );
long get_floppy_size( HANDLE hfloppy, int drive );

void floppy_module_global_init( void );
void floppy_module_global_final( void );

int get_floppy_max_size( int drive );

int is_any_floppy_present( int drive );
int is_hfs_floppy_present( int drive );

void clear_cache_by_drive_index( int drive );

int exists( const char *path );


int is_hfs_cd_present( int drive );
HANDLE cd_init( int drive, BOOL check_geometry );
void cd_final( int drive );
long get_cd_size( HANDLE hcd, int drive );
int cd_read( HANDLE hcd, int drive, ULONG LBA, int count, char *buf, BOOL is_cd );
long get_cd_size( HANDLE m_hfloppy, int drive );
void get_cd_volume_file_name( int drive, char *name );
int try_mount_cd( HANDLE hcd, int drive );

int is_hfs_hd_present( int drive );
HANDLE hd_init( int drive, BOOL check_geometry );
void hd_final( int drive );
long get_hd_size( HANDLE h, int drive );
// int hd_read( HANDLE h, int drive, ULONG LBA, int count, char *buf );
int hd_write( HANDLE h, int drive, ULONG LBA, int count, char *buf );
void get_hd_volume_file_name( int drive, char *name );
int try_mount_hd( HANDLE h, int drive );

void set_patch_option( int patch );

void dump_first_cd( 
	char *path,
	ULONG start,
	long blocks );

int eject_media( char letter, int reload );

BOOL writeback_flush_all(void);

int silencer( int shutup );

#ifdef __cplusplus
} // extern "C"
#endif
