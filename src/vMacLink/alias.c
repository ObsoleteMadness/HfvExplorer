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
#undef ERROR
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
// #include "..\hfs\libhfs\hfs.h"
#include "..\hfs\libhfs\internal.h"
#include "..\hfs\hcwd.h"
#include "..\hfs\hfsutil.h"
#include "..\hfs\suid.h"
#include "..\hfs\hmount.h"
#include "..\hfs\humount.h"
#include "..\hfs\hcopy.h"
#include "..\hfs\copyhfs.h"
#include "..\vmacpatch.h"
#include "..\filehook.h"
#include "alias.h"

int hfs_readdir2(
	hfsdir *dir, 
	hfsdirent *ent,
  CatKeyRec *pkey,
  CatDataRec *pdata
);

#pragma pack(2)
typedef struct {
	unsigned long resource_data_offset;
	unsigned long resource_map_offset;
	unsigned long resource_data_length;
	unsigned long resource_map_length;
} resource_header;

typedef struct {
	char reserved_for_header_copy[16];
	char reserved_for_handle_next_map[4];
	char reserved_for_fref_number[2];
	Integer res_fork_attributes;
	Integer type_list_offset; // from the beginning of the map
	Integer name_list_offset; // from the beginning of the map
	// type list starts here
	// ref list follows
	// resource name list follows
} resource_map_header;

typedef struct {
	unsigned long res_type;
	Integer res_count1; // count-1
	Integer ref_list_offset; // from the beginning of res type list
} resource_type_item;

typedef struct {
	Integer res_id;
	Integer res_name_offset; // from the beginning of the name list
	unsigned char res_attrib;
	unsigned char res_data_offset_hi;
	Integer res_data_offset_lo; // from the beginning of the data
	LongInt reserved; // for handle to resource
} resource_ref_list_entry;

/*
Speedometer							1			1			1			8
Speedometer	+1					1			1			1			C
Speedometer	+2					1			1			1			10
Pegged									1			1			1			4
Shadowgate							1			1			1			8
Rogue										1			1			1			4
Rogue	save							1			1			1			4
Rogue	scores						1			1			1			4
Resedit									2			3			1			8
Brainhex								2			3			1			8
cw bronze readme				1			1			2			
lauri 01								1			1			1			8

*/

typedef struct {
	Integer reserved1;		// 0
	LongInt struct_size;	// == resource_length
	Integer Version;			// 2
	Integer reserved2;		// 0
	char volName[28];			// "\pSystem"
	LongInt drCrDate;			// 0xB0A8 DF27   volume creation date
	Integer fileSystem;		// 0xBD == HFS
	Integer volumeType;		// 0 if HD, 4 if floppy, based on volume size
	LongInt dirNodeID;		// 0x0000 0030
	char fileName[64];		// "\Speedometer 3.23"
	LongInt fileNodeID;		// 0x0000 0032 
	LongInt filCrDat;			// 0xA82D 636E
	OSType type;					// 'APPL'
	OSType creator;				// 'sPd3'
	Integer unknown3;			// 1
	Integer unknown4;			// 1
	char zeroes18[18];
	Integer dirNameLen;
} alis_1;

// char dirName[dirNameLen]; // WORD aligned

#define DIRINFO_FOLLOWS_TRUE 1
#define DIRINFO_FOLLOWS_FALSE 2

/*
typedef struct {
	Integer dirinfo_follows;	// 1  1==dirinfo follows; 2==bytes_used_by_dirs..rootNodeID are missing

	Integer bytes_used_by_dirs;			// 8	directory depth * 4
	// id's of all parent directories, in reverse order
	LongInt reversedParentDirIDs[];	// 0030, 002E, ...

	Integer rootNodeID;			// 2 
	Integer fullPathLen;
} alis_2;
*/

//char fullPath[fullPathLen]; // WORD aligned

/*
typedef struct {
	LongInt endMarker;		// 0xFFFF 0000
} alis_3;
*/
#pragma pack()

// speed is not an issue here.

void endian_w( char *s )
{
	*((unsigned short *)s) = 
		 ((unsigned short)((unsigned char *)s)[0] << 8) |
     ((unsigned short)((unsigned char *)s)[1]);
}

void endian_d( char *s )
{
	*((unsigned long *)s) = 
		 ((unsigned long)((unsigned char *)s)[0] << 24) |
     ((unsigned long)((unsigned char *)s)[1] << 16) |
     ((unsigned long)((unsigned char *)s)[2] <<  8) |
     ((unsigned long)((unsigned char *)s)[3]);
}

unsigned short M16( unsigned short x )
{
	endian_w( (char *)&x );
	return( x );
}

unsigned long M32( unsigned long x )
{
	endian_d( (char *)&x );
	return( x );
}

static int __splitpath( char *path, char *__dir, char *__name )
{
	char *p;
	char dir[MAX_PATH];

	strcpy( dir, path );
	p = strrchr( dir, ':' );
	if(!p) {
		strcpy( __name, path );
		if(__dir) *__dir = 0;
		return(1);
	}
	p++;
	strcpy( __name, p );
	*p = 0;
	if(__dir) strcpy( __dir, dir );
	return(1);
}

static int get_info( 
	hfsvol *vol, 
	char *path, 
	char *volume_name,
	char *file_name,
	char *dir_name,
	LongInt *dirNodeID, 
	LongInt *fileNodeID, 
	LongInt *filCrDat, 
	OSType *type, 
	OSType *creator
)
{
	hfsdirent ent;
	int result = 0;
	hfsdir *dir = 0;
	CatKeyRec catkeyrec;
	CatDataRec catdatarec;
	char parent_dir[_MAX_PATH], *p;
	int parent_dir_len;

	if(!__splitpath( path, parent_dir, file_name )) return(0);
	strcpy( volume_name, path );
	p = strchr( volume_name, ':' );
	if(p) *p = 0;

	dir = hfs_opendir( vol, parent_dir );
	if(dir != 0) {
		while(hfs_readdir2(dir, &ent, &catkeyrec, &catdatarec) >= 0) {
			if(strcmp(catkeyrec.ckrCName,file_name) == 0) {
				*fileNodeID = ent.cnid;
				*dirNodeID = ent.parid;
				*filCrDat = catdatarec.u.fil.filCrDat;
				*type = catdatarec.u.fil.filUsrWds.fdType;
				*creator = catdatarec.u.fil.filUsrWds.fdCreator;
				result = 1;
				break;
			}
		}
		hfs_closedir(dir);
	}

	parent_dir_len = strlen(parent_dir);
	if( parent_dir_len > 0 && parent_dir[parent_dir_len-1] == ':' )
		parent_dir[parent_dir_len-1] = 0;
	if(strchr(parent_dir,':')) {
		if(!__splitpath( parent_dir, NULL, dir_name )) return(0);
	} else {
		strcpy( dir_name, parent_dir ); // return volume name
	}
		
	return( result );
}

int count_colons( char *path )
{
	int count = 0;
	while(path) {
		path = strchr(path,':');
		if(path) {
			count++;
			path++;
		}
	}
	return(count);
}

// vol:dir1:file				level == 2
// vol:dir1:dir2:file		level == 3, 2

LongInt get_subdir_ID( hfsvol *vol, char *path, int level )
{
	char name[_MAX_PATH], *p;
	hfsdir *dir = 0;
	LongInt dirID = 0;
	int i;

	strcpy( name, path );
	p = name;
	for(i=0; i<level; i++) {
		p = strchr(p,':');
		if(p) {
			p++;
		} else {
			return(0); // impossible
		}
	}
	*p = 0;
	dir = hfs_opendir( vol, name );
	if(dir != 0) {
		dirID = dir->dirid;
		hfs_closedir(dir);
	}
	return(dirID);
}

int make_alias( hfsvol *vol, char *path, char **code_bytes )
{
	int bytes_count = 0;
	unsigned char *code;
	resource_header rhead;
	resource_map_header mhead;
	int type_count_1;
	resource_type_item titem;
	resource_ref_list_entry rlentry;
	alis_1 a1;
	int pc, fc_patch;
	LongInt endMarker;
	Integer fullPathLen;

	char alias_name[64];
	int alias_name_len;
	LongInt dirNodeID, fileNodeID, filCrDat;
	OSType type, creator;
	char volume_name[_MAX_PATH];
	char file_name[_MAX_PATH];
	char dir_name[_MAX_PATH];
	
	if(!get_info( 
		vol, 
		path, 
		volume_name,
		file_name,
		dir_name,
		&dirNodeID, 
		&fileNodeID, 
		&filCrDat, 
		&type, 
		&creator) )
	{
		return(0);
	}

	strcpy( alias_name, "HFVExplorer Alias" );
	alias_name_len = strlen(alias_name);

	code = malloc( 5000 );
	if(!code) return(0);
	memset( code, 0, 5000 );

	memset( &rhead, 0, sizeof(resource_header) );
	memset( &mhead, 0, sizeof(resource_map_header) );
	memset( &titem, 0, sizeof(resource_type_item) );
	memset( &rlentry, 0, sizeof(resource_ref_list_entry) );
	memset( &a1, 0, sizeof(alis_1) );

	pc = 0;

	// @ offset 0x0

	// memcpy( code[pc], &rhead, sizeof(resource_header) );
	memset( &code[pc+16], 0, 240 );

	pc += 256;



	// 'alis'
	fc_patch = pc;
	// 'alis' resource length, not part of the alis struct
	// code[pc] = 0xFC;
	pc += 4;

	a1.reserved1 = M16( 0 );
	a1.struct_size = M32( 0xFC ); // patched later
	a1.Version = M16( 2 );
	a1.reserved2 = M16( 0 );
	a1.volName[0] = strlen( volume_name );
	strncpy( &a1.volName[1], volume_name, 27 );
	a1.drCrDate = M32( vol->mdb.drCrDate );
	a1.fileSystem = M16( 0x4244 ); // 0xBD == HFS
	if(vol->vlen >= 4068) { // 2034kB is "big"
		a1.volumeType = M16( 0 ); // HD
	} else {
		a1.volumeType = M16( 4 ); // "floppy"
	}
	a1.dirNodeID = M32( dirNodeID );
	a1.fileName[0] = strlen(file_name);
	strncpy( &a1.fileName[1], file_name, 63 );
	a1.fileNodeID = M32( fileNodeID );
	a1.filCrDat = M32( filCrDat );
	a1.type = M32( type );
	a1.creator = M32( creator );
	a1.unknown3 = M16( 1 ); 	// 1
	a1.unknown4 = M16( 1 );		// 1
	memset( &a1.zeroes18, 0, 18 );
	a1.dirNameLen = M16( (unsigned short)strlen( dir_name ) );
	memcpy( &code[pc], &a1, sizeof(alis_1) );
	pc += sizeof(alis_1);

	memcpy( &code[pc], dir_name, strlen( dir_name ) );
	pc += strlen( dir_name );
	if( pc & 1 ) pc++; // word align

	{
		Integer dirinfo_follows;
		Integer bytes_used_by_dirs;
		LongInt dirID;
		Integer rootNodeID;
		int i, colon_count = count_colons(path);
		
		dirinfo_follows = M16( (unsigned short)(colon_count > 1 ? DIRINFO_FOLLOWS_TRUE : DIRINFO_FOLLOWS_FALSE) );
		memcpy( &code[pc], &dirinfo_follows, sizeof(dirinfo_follows) );
		pc += sizeof(dirinfo_follows);

		if(colon_count > 1) {
			bytes_used_by_dirs = M16( (unsigned short)((colon_count-1)*4) );
			memcpy( &code[pc], &bytes_used_by_dirs, sizeof(bytes_used_by_dirs) );
			pc += sizeof(bytes_used_by_dirs);

			for(i=colon_count; i>1; i--) {
				dirID = M32( get_subdir_ID(vol,path,i) );
				memcpy( &code[pc], &dirID, sizeof(dirID) );
				pc += sizeof(dirID);
			}

			rootNodeID = M16( 2 );	// ?? root node id 
			memcpy( &code[pc], &rootNodeID, sizeof(rootNodeID) );
			pc += sizeof(rootNodeID);
		}
	}
	fullPathLen = M16( (unsigned short)strlen( path ) );
	memcpy( &code[pc], &fullPathLen, sizeof(fullPathLen) );
	pc += sizeof(fullPathLen);

	memcpy( &code[pc], path, strlen( path ) );
	pc += strlen( path );
	if( pc & 1 ) pc++; // word align

	endMarker = M32( 0xFFFF0000 );
	memcpy( &code[pc], &endMarker, sizeof(endMarker) );
	pc += sizeof(endMarker);


	if(pc < 0x200) pc = 0x200;
	if( pc & 1 ) pc++; // word align

	{ LongInt tmp = M32( pc - fc_patch - 4 );
		memcpy( &code[fc_patch], &tmp, sizeof(4) );
		memcpy( &code[fc_patch+6], &tmp, sizeof(4) );
	}

	rhead.resource_data_offset = M32( 0x0100 );
	rhead.resource_data_length = M32( pc - 0x0100 );
	rhead.resource_map_offset  = M32( pc );
	rhead.resource_map_length  = M32( 0x33 + alias_name_len );
	memcpy( &code[0], &rhead, sizeof(resource_header) );

	// @ offset 0x00000200 (resource_map_offset)
	memcpy( &mhead.reserved_for_header_copy, &rhead, sizeof(resource_header) );
	memset( &mhead.reserved_for_handle_next_map, 0, 4 ); // ?
	memset( &mhead.reserved_for_fref_number, 0, 2 ); // ??
	mhead.res_fork_attributes = M16( 0 );
	mhead.type_list_offset = M16( 0x1C );
	mhead.name_list_offset = M16( 0x32 );
	memcpy( &code[pc /*0x200*/], &mhead, sizeof(resource_map_header) );
	pc += sizeof(resource_map_header);

	// @ offset 0x0000021C (resource_map_offset+type_list_offset)
	type_count_1 = M16( 0 );
	memcpy( &code[pc /*0x21C*/], &type_count_1, sizeof(type_count_1) );
	pc += 2;
	titem.res_type = M32( 'alis' ); // CHECK
	titem.res_count1 = M16( 0 );
	titem.ref_list_offset = M16( 0x0A ); //check
	memcpy( &code[pc /*0x21E*/], &titem, sizeof(resource_type_item) );
	pc += sizeof(resource_type_item);

	// @ offset 0x00000226 (resource_map_offset+type_list_offset+ref_list_offset)
	rlentry.res_id = M16( 0 );
	rlentry.res_name_offset = M16( 0 );
	rlentry.res_attrib = 0;
	rlentry.res_data_offset_hi = 0;
	rlentry.res_data_offset_lo = M16( 0 );
	rlentry.reserved = M32( 0 ); // ?? for handle to resource
	memcpy( &code[pc /*0x226*/], &rlentry, sizeof(resource_ref_list_entry) );
	pc += sizeof(resource_ref_list_entry);

	// @ offset 0x00000232 (resource_map_offset+name_list_offset)
	code[pc /*0x232*/] = alias_name_len;
	pc++;
	memcpy( &code[pc /*0x233*/], alias_name, alias_name_len );
	pc += alias_name_len;

	*code_bytes = code;
	bytes_count = pc;

	return(bytes_count);
}
