// #define ZAPPER

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

//#include "afx.h"
#include "stdafx.h"
#include "HFVExplorer.h"

#include "HFVExplorerDoc.h"
#include "HFVExplorerListView.h"
#include "CFATVolume.h"
#include "special.h"
#include "utils.h"
#include "hfs\libhfs\hfs.h"
#include "hfs\interface.h"
#include "AskNewVolume.h"
#include "MainFrm.h"
#include "floppy.h"
#include "adouble.h"
#include "aspecial.h"

#ifdef _DEBUG
// #define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerDoc

IMPLEMENT_DYNCREATE(CHFVExplorerDoc, CDocument)

BEGIN_MESSAGE_MAP(CHFVExplorerDoc, CDocument)
	//{{AFX_MSG_MAP(CHFVExplorerDoc)
	ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerDoc construction/destruction

void dbdb(char *s)
{
	OutputDebugString( s );
	OutputDebugString( "\r\n" );
}

void clear_all_caches(void) 
{
	int drive;
	for(drive=0; drive<MAX_DEVICES; drive++) {
		clear_cache_by_drive_index( drive );
	}
}

void CHFVExplorerDoc::init_mac_icons()
{
	m_icon_array = new CDWordArray;
	m_icon_array->SetSize( 200, 20 );
	init_hash_table();
}

void CHFVExplorerDoc::destroy_icon_cache()
{
	int i, count = m_icon_array->GetSize();
	DWORD dw;

	for(i=0; i<count; i++) {
		dw = m_icon_array->GetAt( i );
		if(dw) {
			DestroyIcon( (HICON)dw );
		}
	}
	m_icon_array->RemoveAll();
}

void CHFVExplorerDoc::add_to_icon_cache( HICON hicon )
{
	ASSERT( m_icon_array != 0 );
	if(hicon) m_icon_array->Add( (DWORD)hicon );
}

int CHFVExplorerDoc::get_volume_index_by_filename( CString name )
{
	int i;

	for(i=0; i<m_hfs_count; i++) {
		if( name.CompareNoCase( m_volumes[i].m_file_name ) == 0 ) return(i);
	}
	return(-1);
}

void CHFVExplorerDoc::set_floppy_volume_dirty( char *path )
{
	int inx, drive;
	char name[_MAX_PATH];

	for(drive=0; drive<MAX_FLOPPIES; drive++) {
		if(is_floppy_by_index(drive)) {
			get_floppy_volume_file_name( drive, name );
			if(stricmp( name, path ) == 0) {
				// clear_cache_by_drive_index( drive );
				inx = get_volume_index_by_filename( CString(path) );
				if(inx >= 0) {
					m_volumes[inx].m_floppy_update_needed = TRUE;
				}
				break;
			}
		}
	}
	for(drive=2; drive<MAX_DEVICES; drive++) {
		get_hd_volume_file_name( drive, name );
		if(stricmp( name, path ) == 0) {
			// clear_cache_by_drive_index( drive );
			inx = get_volume_index_by_filename( CString(path) );
			if(inx >= 0) {
				m_volumes[inx].m_floppy_update_needed = TRUE;
			}
			break;
		}
	}
}

int CHFVExplorerDoc::get_volume_index_by_volumename( CString name )
{
	int i;

	for(i=0; i<m_hfs_count; i++) {
		if(name.CompareNoCase( m_volumes[i].m_volume_name ) == 0 ) {
			return(i);
		}
	}
	return(-1);
}

int check_zero_size( char *lpszPathName )
{
	int ret = 1;
	DWORD sizelo, sizehi;
	HANDLE hFile;
	char vname[_MAX_PATH];
	int file_size, init;

	if(lpszPathName && *lpszPathName) {
		hFile = CreateFile(
			lpszPathName,
			GENERIC_READ,
			0, // not shared now
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL );
		if(hFile != INVALID_HANDLE_VALUE) {
			sizelo = GetFileSize( hFile, &sizehi );
			CloseHandle( hFile );
			if(sizehi == 0 && sizelo == 0) {
				*vname = 0;
				if(ask_new_volume( lpszPathName, vname, &file_size, &init )) {
					if(0 != HFSIFACE_format( lpszPathName, vname, file_size, init )) {
						ret = 0;
					}
				} else {
					ret = 0;
				}
			}
		}
	}
	return(ret);
}

int CHFVExplorerDoc::insert_volume( 
	CString name, 
	int is_floppy,
	int is_cd,
	int is_hd,
	int is_removable )
{
	char *ppp = name.GetBuffer(_MAX_PATH);
	if(!check_zero_size( ppp )) {
		return(-1);
	}

	if(m_hfs_count < MAX_HFS_VOLUMES) {
		int i = get_volume_index_by_filename( name );
		if(i >= 0) { // already mounted
			return(-1);
		} else {
			for(i=0; i<m_hfs_count; i++) {
				if( m_volumes[i].m_free ) {
					break;
				}
			}
			m_volumes[i].m_file_name = name;
			m_volumes[i].m_free = 0;
			m_volumes[i].m_opened = 0;
			m_volumes[i].m_volume_name = "";
			m_volumes[i].m_file_opened = 0;
			m_volumes[i].m_rootitem = 0;
			m_volumes[i].m_floppy_update_needed = 0;
			m_volumes[i].m_is_floppy = is_floppy;
			m_volumes[i].m_is_cd = is_cd;
			m_volumes[i].m_is_hd = is_hd;
			m_volumes[i].m_is_removable = is_removable;
			if(i == m_hfs_count) {
				m_hfs_count++;
				return(m_hfs_count-1);
			} else {
				return(i);
			}
		}
	} else {
		return(-1);
	}
}

void CHFVExplorerDoc::enum_volumes_ext( LPSTR mask, LPSTR ext )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok;

	fh = FindFirstFile( CString(mask) + "\\*." + ext, &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		if(*FindFileData.cFileName != '%') { // skip resource fork files
			(void)insert_volume( CString(mask) + "\\" + CString(FindFileData.cFileName), 0, 0, 0, 0 );
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
}

void CHFVExplorerDoc::enum_floppies( void )
{
	char name[_MAX_PATH];
	int i;

	for(i=0; i<MAX_FLOPPIES; i++) {
		if(is_hfs_floppy_present(i)) {
			get_floppy_volume_file_name( i, name );
			if(get_volume_index_by_filename( CString(name) ) < 0 ) {
				insert_volume( CString(name), 1, 0, 0, 0 );
			}
		}
	}
}

void CHFVExplorerDoc::enum_cds(void)
{
	char name[_MAX_PATH];
	char rootdir[20], letter;
	int i;

	for( letter = 'C'; letter <= 'Z'; letter++ ) {
		i = (int)( letter - 'A' );
		wsprintf( rootdir, "%c:\\", letter );
		if(GetDriveType( rootdir ) == DRIVE_CDROM) {
			if(is_hfs_cd_present(i)) {
				get_cd_volume_file_name( i, name );
				if(get_volume_index_by_filename( CString(name) ) < 0 ) {
					insert_volume( CString(name), 0, 1, 0, 0 );
				}
			}
		}
	}
}

void CHFVExplorerDoc::enum_hds(void)
{
	char name[_MAX_PATH];
	char rootdir[20], letter;
	int i, type;
	CHFVExplorerApp *pApp = (CHFVExplorerApp *)AfxGetApp();

	for( letter = 'C'; letter <= 'Z'; letter++ ) {

		// if(letter != 'J') continue;

		i = (int)( letter - 'A' );
		wsprintf( rootdir, "%c:\\", letter );
		type = GetDriveType( rootdir );
		// if(type == DRIVE_REMOVABLE || type == DRIVE_FIXED) {
		if( type == DRIVE_REMOVABLE || 
			 (type == DRIVE_FIXED && pApp->m_do_harddisks) )
		{
			if(is_hfs_hd_present(i)) {
				get_hd_volume_file_name( i, name );
				if(get_volume_index_by_filename( CString(name) ) < 0 ) {
					insert_volume( CString(name), 0, 0, 1, type==DRIVE_REMOVABLE );
				}
			}
		}
	}
}

void CHFVExplorerDoc::enum_hfs_volumes( 
	LPSTR mask, BOOL floppies, BOOL cds, BOOL hds
)
{
	clear_all_caches();
	if(floppies) enum_floppies();
	if(cds) enum_cds();
	if(hds) enum_hds();

	// I don't want to say *.HF* here ...

	enum_volumes_ext( mask, "HFV" );
	enum_volumes_ext( mask, "DSK" );
	enum_volumes_ext( mask, "HF0" );
	enum_volumes_ext( mask, "HF1" );
	enum_volumes_ext( mask, "HF2" );
	enum_volumes_ext( mask, "HF3" );
	enum_volumes_ext( mask, "HF4" );
	enum_volumes_ext( mask, "HF5" );
	enum_volumes_ext( mask, "HF6" );
	enum_volumes_ext( mask, "HF7" );
	enum_volumes_ext( mask, "HF8" );
	enum_volumes_ext( mask, "HF9" );
}

unsigned long get_magic( CFile *fp )
{
	apple_single_double_header h;

	fp->Read( &h, sizeof(apple_single_double_header) );
	mappc_apple_single_double_header(&h);
	return(h.Magic);
}

unsigned long get_section_offset( 
	CFile *fp, 
	LongInt id,
	unsigned long *plength )
{
	apple_single_double_header h;
	apple_single_double_entry e;
	int i;

	if(plength) *plength = 0;
	fp->Seek( 0, CFile::begin );
	fp->Read( &h, sizeof(apple_single_double_header) );
	mappc_apple_single_double_header(&h);
	if(h.Magic == APPLE_DOUBLE_MAGIC || h.Magic == APPLE_SINGLE_MAGIC) {
		for(i=0; i<h.entry_count; i++) {
			fp->Read( &e, sizeof(apple_single_double_entry) );
			mappc_apple_single_double_entry(&e);
			if(e.id == id) {
				if(plength) *plength = e.length;
				return( e.offset );
			}
		}
	}
	return( 0 );
}

/* sample document appledouble 
	id	0x0000000a  - mac file info
	offset	0x00000092
	length	0x00000004

	id	0x00000008 - file dates info
	offset	0x00000096
	length	0x00000010

	id	0x00000009 - finder info
	offset	0x000000a6
	length	0x00000020

	id	0x00000002 - res fork
	offset	0x00000200
	length	0x00000166
*/

// s is of form %<name>. is there <name> ?
int CHFVExplorerDoc::get_apple_double( 
	CString dir, 
	LPSTR s, 
	HICON *phicon16,
	HICON *phicon32,
	LPSTR pure_name,
	LPSTR stype,
	LPSTR screator,
	int *do_show
) 
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	char search_name[MAX_PATH];
	int ok = 0;

	if(*s == '%') {
		strcpy( pure_name, &s[1] );
		strcpy( search_name, pure_name );
	} else {
		strcpy( search_name, "%" );
		strcat( search_name, s );
		strcpy( pure_name, s );
	}
		
	*do_show = 1;

	fh = FindFirstFile( dir + search_name, &FindFileData );
	if(fh != INVALID_HANDLE_VALUE) {
		FindClose( fh );
		unsigned long magic, start;

		// pair found ...
		CFile f, *fp;
		fp = &f;
		*phicon16 = *phicon32 = 0;
		if(!do_open_file( fp, dir + CString("%") + CString(pure_name) ) ) {
			return(0);
		}
		magic = get_magic( fp );
		if(magic == APPLE_SINGLE_MAGIC) {
			// to make this work properly we should open EVERY file.
		} else if(magic == APPLE_DOUBLE_MAGIC) {
			unsigned long type = 0;
			unsigned long creator = 0;
			FInfo finfo;

			finfo.fdFlags = 0;

			// if(strcmp(pure_name,"Browser") == 0) shadow = 1;

			*stype = 0;
			*screator = 0;
			start = get_section_offset( fp, Finder_Info, NULL );
			if(start) {
				fp->Seek( start, CFile::begin );
				fp->Read( &finfo, sizeof(FInfo) );
				type = finfo.fdType;
				creator = finfo.fdCreator;
				MACPC_D(type);
				MACPC_D(creator);
				MACPC_W(finfo.fdFlags);
			}
			if(type && creator) {
				if((finfo.fdFlags & fInvisible) && !m_mac_show_invisibles) {
					*do_show = 0;
					return(1);
				}
				start = get_section_offset( fp, Resource_Fork, NULL );
				if(start) {
					if(finfo.fdFlags & kHasCustomIcon) {
						// AfxMessageBox( "File " + CString(pure_name) + " has custom icon." );
#ifndef ZAPPER
						load_custom_icons( fp, start, phicon16, phicon32 );
#endif
					}
					if(!*phicon32) *phicon32 = map_type_and_creator (type, creator, 0 );
					if(!*phicon16) *phicon16 = map_type_and_creator (type, creator, 1 );
					
					if(finfo.fdFlags & kHasBundle) {
						if(!*phicon16 || !*phicon32) {
							process_bundle2( fp, start, creator );
							if(!*phicon32) *phicon32 = map_type_and_creator (type, creator, 0 );
							if(!*phicon16) *phicon16 = map_type_and_creator (type, creator, 1 );
						}
					}
				}
				long_to_string( type, (unsigned char *)stype );
				long_to_string( creator, (unsigned char *)screator );
			}
			// if(strcmp(pure_name,"Browser") == 0) shadow = 0;
			ok = 1;
		}
		silent_close( fp );
	}
	return(ok);
}

extern "C" {
int get_apple_double_type_creator( 
	char *name, 
	char *stype, char *screator,
	unsigned long *pstart,
	unsigned long *plength
	) 
{
	int ok = 0;
	unsigned long magic, start;
	CFile f, *fp;
	unsigned long type = 0;
	unsigned long creator = 0;
	FInfo finfo;

	fp = &f;
	finfo.fdFlags = 0;

	lstrcpy( stype, "TEXT" );
	lstrcpy( screator, "????" );

	if(!do_open_file( fp, CString(name) ) ) {
		return(0);
	}
	magic = get_magic( fp );
	if(magic == APPLE_SINGLE_MAGIC) {
		// to make this work properly we should open EVERY file.
	} else if(magic == APPLE_DOUBLE_MAGIC) {
		*pstart = get_section_offset( fp, Resource_Fork, plength );
		if(*pstart) {
			start = get_section_offset( fp, Finder_Info, NULL );
			if(start) {
				fp->Seek( start, CFile::begin );
				fp->Read( &finfo, sizeof(FInfo) );
				type = finfo.fdType;
				creator = finfo.fdCreator;
				MACPC_D(type);
				MACPC_D(creator);
				ok = 1; // Executor makes sometimes 0,0 AppleDouble.
				if(type && creator) {
					long_to_string( type, (unsigned char *)stype );
					long_to_string( creator, (unsigned char *)screator );
				}
			}
		}
	}
	silent_close( fp );

	return(ok);
}
} // extern "C"

HICON get_dos_icon( CHFVExplorerApp *pApp, char *p )
{
	char executable[MAX_PATH];
	HICON hicon;

	// pApp = (CHFVExplorerApp *)AfxGetApp();

	if(pApp->m_dosicons == DI_NO_ICONS) return(0);

	if(pApp->m_dosicons == DI_PREFER_INDIVIDUAL_ICONS) {
		hicon = ::ExtractIcon( pApp->m_hInstance, p, 0 );
		if(hicon && hicon != (HICON)1) {
			// dump_icon( hicon );
			return(hicon);
		}
	}
	if(is_extension( p, ".EXE" )) {
		hicon = ::ExtractIcon( pApp->m_hInstance, p, 0 );
		return( hicon );
	}
	if( (unsigned int)FindExecutable( p, "", executable ) > 32) {
		hicon = ::ExtractIcon( pApp->m_hInstance, executable, 0 );
		if(hicon && hicon != (HICON)1) return( hicon );
	}
	if(pApp->m_dosicons == DI_PREFER_INDIVIDUAL_ICONS) {
		// we already know that there is no icon
		return(0);
	}
	hicon = ::ExtractIcon( pApp->m_hInstance, p, 0 );
	if(hicon == (HICON)1) hicon = 0;
	return( hicon );
}

int CHFVExplorerDoc::ignore_this( CObArray *parr, WIN32_FIND_DATA *fd ) 
{
	int i, count = parr->GetSize();
	char *name = fd->cFileName;

	if(fd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
		if(!m_pc_show_invisibles) {
			return(1);
		}
	}
	for(i=0; i<count; i++) {
		CString *s = (CString*)parr->GetAt( i );
		if(s->CompareNoCase(name) == 0) return(1);
	}
	return(0);
}

#pragma optimize("g",off)
/*
void CHFVExplorerDoc::finder_dat_walk( int volinx, LPSTR path, int list )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString mask;
	unsigned char *finderdat = 0;

	mask = CString(path);
	if(mask.Right(1) != "\\") mask += "\\";

	fh = FindFirstFile( mask + "FINDER.DAT", &FindFileData );
	if(fh == INVALID_HANDLE_VALUE) return(0);
	FindClose( fh );
	finderdat = 0;
	return(1);
}
*/

int is_afp_directory( char *path )
{
	int len, result = 0;
	char _path[_MAX_PATH];
	HANDLE hStream;
	OSVERSIONINFO osv;

	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if( !GetVersionEx(&osv) ||
			osv.dwPlatformId != VER_PLATFORM_WIN32_NT )
	{
		return(0);
	}

	strcpy( _path, path );
	len = strlen( _path );
	if( len > 3 && _path[len-1] == '\\' ) _path[len-1] = 0;
	strcat( _path, ":AFP_AfpInfo" );
	hStream = CreateFile( _path,
												GENERIC_READ,
												FILE_SHARE_WRITE | FILE_SHARE_READ,
												NULL,
												OPEN_EXISTING,
												0,
												NULL );
	if( hStream != INVALID_HANDLE_VALUE ) {
		result = 1;
		CloseHandle(hStream);
	}
	return(result);
}

int CHFVExplorerDoc::get_afp( 
	CString dir, 
	LPSTR s, 
	HICON *phicon16,
	HICON *phicon32,
	LPSTR pure_name,
	LPSTR stype,
	LPSTR screator,
	int *do_show
) 
{
	int ok = 0;
	unsigned long type = 0;
	unsigned long creator = 0;
	FInfo finfo;
	CFile f, *fp = &f;

	strcpy( pure_name, s );
	finfo.fdFlags = 0;

	*do_show = 1;

	*phicon16 = *phicon32 = 0;
	if(!do_open_file( fp, dir + s + CString(":AFP_AfpInfo") ) ) {
		return(0);
	}
	*stype = 0;
	*screator = 0;
	fp->Seek( 16, CFile::begin );
	fp->Read( &finfo, sizeof(FInfo) );
	type = finfo.fdType;
	creator = finfo.fdCreator;
	MACPC_D(type);
	MACPC_D(creator);
	MACPC_W(finfo.fdFlags);
	silent_close( fp );

	if((finfo.fdFlags & fInvisible) && !m_mac_show_invisibles) {
		*do_show = 0;
		return(1);
	}

	if(!do_open_file( fp, dir + s + CString(":AFP_Resource") ) ) {
		return(0);
	}

	if(type && creator) {
		if(finfo.fdFlags & kHasCustomIcon) {
			// AfxMessageBox( "File " + CString(pure_name) + " has custom icon." );
#ifndef ZAPPER
			load_custom_icons( fp, 0, phicon16, phicon32 );
#endif
		}
		if(!*phicon32) *phicon32 = map_type_and_creator (type, creator, 0 );
		if(!*phicon16) *phicon16 = map_type_and_creator (type, creator, 1 );
		
		if(finfo.fdFlags & kHasBundle) {
			if(!*phicon16 || !*phicon32) {
				process_bundle2( fp, 0, creator );
				if(!*phicon32) *phicon32 = map_type_and_creator (type, creator, 0 );
				if(!*phicon16) *phicon16 = map_type_and_creator (type, creator, 1 );
			}
		}
		long_to_string( type, (unsigned char *)stype );
		long_to_string( creator, (unsigned char *)screator );
	}
	ok = 1;
	silent_close( fp );
	return(ok);
}

int get_fat_dir_count( char *path, BOOL ignore_resource_files )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	CString mask;
	int ok, count = 0;

	mask = CString(path);
	if(mask.Right(1) != "\\") mask += "\\";

	fh = FindFirstFile( mask + "*.*", &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		UPDATE_TIME_CONSUMING(1);
		if(!ignore_resource_files || *FindFileData.cFileName != '%') {
			if(*FindFileData.cFileName != '.') {
				count++;
			}
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
	return( count );
}

void CHFVExplorerDoc::fat_walk( int volinx, LPSTR path, int list )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok, isdir;
	CString mask, fpath;
	CHFVExplorerApp	*pApp;
	HTREEITEM root;
	HICON hicon16 = 0;
	HICON hicon32 = 0;
	LPSTR p;
	HTREEITEM htree;
	int do_show;
	int is_afp_dir = 0;
	char pure_name[_MAX_PATH];
	char mac_type[20], mac_creator[20];

	dbdb( "fat_walk" );

	/*
	if(finder_dat_walk( volinx, path, list )) {
		return;
	}
	*/

	pApp = (CHFVExplorerApp *)AfxGetApp();

	mask = CString(path);
	if(mask.Right(1) != "\\") mask += "\\";

	root = pApp->m_tree->GetTreeCtrl().GetSelectedItem();

	START_TIME_CONSUMING(1000);

	is_afp_dir = is_afp_directory(path);

	// scan AppleDouble files first
	CObArray ignore_array;
	fh = FindFirstFile( mask + "%*.*", &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		UPDATE_TIME_CONSUMING(1);
		if( get_apple_double( 
						mask,
						FindFileData.cFileName,
						&hicon16,
						&hicon32,
						pure_name,
						mac_type,
						mac_creator,
						&do_show
		)) {
			p = pure_name;
			fpath = mask + CString(pure_name);

			// add %<name>
			CString resource_name = CString("%") + CString(pure_name);
			CString *save_name = new CString( resource_name );
			ignore_array.Add( (CObject*)save_name);
			// add <name>
			save_name = new CString( pure_name );
			ignore_array.Add( (CObject*)save_name);
			if(do_show) {
				// wrong - should check name without %
				// isdir = (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				isdir = isdirectory( fpath );
				p = fpath.GetBuffer(MAX_PATH);
				remove_ardi_special_chars( (unsigned char *)pure_name );
				mac_to_pc_charset( (unsigned char *)pure_name );
				if(isdir) {
					htree = pApp->m_tree->tree_fat_insert( 
						root, volinx, pure_name, p );
					FindFileData.nFileSizeLow = get_fat_dir_count( p, TRUE );
				} else {
					htree = 0;
					FindFileData.nFileSizeLow = 0;
				}

				if(list) {
					pApp->m_list->list_fat_insert( 
						pure_name,
						isdir,
						FindFileData.nFileSizeLow,
						&FindFileData.ftCreationTime,
						&FindFileData.ftLastWriteTime,
						hicon16,
						hicon32,
						mac_type,
						mac_creator,
						p,
						htree,
						volinx
						);
				} else {
					if(hicon16) DestroyIcon(hicon16);
					if(hicon32) DestroyIcon(hicon32);
				}
				fpath.ReleaseBuffer(MAX_PATH);
			}
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );

	// scan dirs next
	fh = FindFirstFile( mask + "*.*", &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		UPDATE_TIME_CONSUMING(1);
		isdir = (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if(isdir && *FindFileData.cFileName != '.') {
			if(!ignore_this( &ignore_array, &FindFileData )) {
				dos_lfn_fdata( &FindFileData );
				fpath = mask + CString(FindFileData.cFileName);
				p = fpath.GetBuffer(MAX_PATH);
				htree = pApp->m_tree->tree_fat_insert( 
					root, volinx, 
					FindFileData.cFileName, p );
				if(list) {
					FindFileData.nFileSizeLow = get_fat_dir_count( p, FALSE );
					pApp->m_list->list_fat_insert( 
						(LPSTR)FindFileData.cFileName,
						isdir,
						FindFileData.nFileSizeLow,
						&FindFileData.ftCreationTime,
						&FindFileData.ftLastWriteTime,
						0, 0, "", "",
						p, htree, volinx
						);
				}
				fpath.ReleaseBuffer(MAX_PATH);
			}
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );

	DWORD icon_stop_time = 0;
	int save_m_dosicons	= pApp->m_dosicons;
	if(pApp->m_dosicons != DI_NO_ICONS && pApp->m_fat_icon_time_limit > 0) {
		icon_stop_time = GetTickCount() + (DWORD)pApp->m_fat_icon_time_limit*1000;
	}

	// scan normal files next
	fh = FindFirstFile( mask + "*.*", &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		UPDATE_TIME_CONSUMING(1);
		isdir = (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if(!isdir) {
			if(!ignore_this( &ignore_array, &FindFileData )) {
				dos_lfn_fdata( &FindFileData );
				fpath = mask + CString(FindFileData.cFileName);
				p = fpath.GetBuffer(MAX_PATH);
				if(list) {
					if(icon_stop_time != 0 && GetTickCount() >= icon_stop_time) {
						pApp->m_dosicons = DI_NO_ICONS;
						icon_stop_time = 0;
					}
					*mac_type = *mac_creator = 0;
					hicon16 = hicon32 = 0;
					if(is_afp_dir) {
						if( !get_afp( 
										mask,
										FindFileData.cFileName,
										&hicon16,
										&hicon32,
										pure_name,
										mac_type,
										mac_creator,
										&do_show ) )
						{
							// hicon16 = hicon32 = 0;
						}
					}
					if(!hicon32) {
						hicon16 = get_dos_icon( pApp, p );
						hicon32 = hicon16;
					}
					pApp->m_list->list_fat_insert( 
						(LPSTR)FindFileData.cFileName,
						isdir,
						FindFileData.nFileSizeLow,
						&FindFileData.ftCreationTime,
						&FindFileData.ftLastWriteTime,
						hicon16, hicon32,
						mac_type, mac_creator,
						p, 0, volinx
					);
				}
				fpath.ReleaseBuffer(MAX_PATH);
			}
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );

	pApp->m_dosicons = save_m_dosicons;

	int i, count = ignore_array.GetSize();
	for(i=0; i<count; i++) {
		CString *s = (CString*)ignore_array.GetAt( i );
		if(s) {
			delete s;
		}
	}
	ignore_array.RemoveAll();

	pApp->m_list->do_report_sort();

	END_TIME_CONSUMING;

	dbdb( "fat_walk END" );

}
#pragma optimize("",on)

int CHFVExplorerDoc::insert_fat_volume( char letter, UINT type )
{
	if(m_fat_count < MAX_FAT_VOLUMES) {
		m_fats[m_fat_count].m_letter = letter;
		m_fats[m_fat_count].m_type = type;
		m_fats[m_fat_count].m_rootitem = 0;
		m_fats[m_fat_count].m_volume_name.Format( "%c:\\", letter );
		m_fat_count++;
		return(m_fat_count-1);
	} else {
		return(-1);
	}
}

void CHFVExplorerDoc::enum_fat_volumes()
{
	UINT type;
	char rootdir[20], letter;

	for( letter = 'A'; letter <= 'Z'; letter++ ) {
		wsprintf( rootdir, "%c:\\", letter );
		type = GetDriveType( rootdir );
		switch( type ) {
			case DRIVE_REMOVABLE:
			case DRIVE_FIXED:
			case DRIVE_REMOTE:
			case DRIVE_CDROM:
			case DRIVE_RAMDISK:
				insert_fat_volume( letter, type );
				break;
		}
	}
}

void CHFVExplorerDoc::update_invisible_attributes()
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	m_mac_show_invisibles = pApp->m_show_invisible_mac;
	m_pc_show_invisibles = pApp->m_show_invisible_fat;
}

CHFVExplorerDoc::CHFVExplorerDoc()
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	pApp->m_doc = this;

	// m_mac_show_invisibles = 1;
	// m_pc_show_invisibles = 1;
	update_invisible_attributes();

	if( pApp->m_prevVersion != "1.2.0" && // ??? what's this.
			pApp->m_prevVersion != "1.2.1" &&
			pApp->m_prevVersion != "1.2.2" &&
			pApp->m_prevVersion != "1.2.3" &&
			pApp->m_prevVersion != "1.2.4" &&
			pApp->m_prevVersion != "1.2.5" ) 
	{
		if(exists_icon_cache_file()) {
			int answer = AfxMessageBox(
				"This version of HFVExplorer has many fixes that make Mac icons display (more) correctly. "
				"In order this to be possible, however, the old icon cache file HFVICONS.DAT must be deleted "
				"and rebuilt. This is a safe operation. Is it ok to delete it now?"
				"",
				MB_YESNO
			);
			if(answer == IDYES) {
				if(delete_cache_OK() && !exists_icon_cache_file()) {
					AfxMessageBox( 
						"The cache file is now deleted and will be recreated. "
						"You may notice that some icons seem to be missing, typically preferences and documents. This is normal, "
						"and they will eventually reappear as you scan through the folders containing the creator applications, "
						"which contain the bundle and icon resources. "
						"You may also notice that at first the browsing is a bit slower -- this is because HFVExplorer "
						"must spend some time in building the icons."
						"\r\n\r\n"
						"Have a nice day!"
						"",
						MB_OK
					);
				} else {
					AfxMessageBox( 
						"The icon cache file could not be deleted. If you have other installations of HFVExplorer, please "
						"make sure to manually delete the file HFVICONS.DAT from each one of them. Do this only once, "
						"since the file will immediately reappear. Only old data is invalid."
						"\r\n\r\n"
						"Have a nice day!"
						"",
						MB_OK
					);
				}
			} else {
				AfxMessageBox( "Your icons may not be correct until the cache file is deleted.", MB_OK );
			}
		}
	}

	init_mac_icons();

	m_hfs_count = 0;
	m_fat_count = 0;

	// enum_hfs_volumes("");
	char dir[_MAX_PATH+1];
	char *cuthere;
	::GetModuleFileName( pApp->m_hInstance, dir, _MAX_PATH );
	cuthere = strrchr( dir, '\\' );
	if(cuthere) *cuthere = 0;
	enum_hfs_volumes(
		dir,
		pApp->m_startup_floppies,
		pApp->m_startup_cds,
		pApp->m_startup_hds
	);

	enum_fat_volumes();

	HDC dc = GetDC(0);
	if(dc) {
		m_bits_per_pixel = GetDeviceCaps( dc, BITSPIXEL );
		ReleaseDC( 0, dc );
	} else {
		m_bits_per_pixel = 1;
	}
}

CHFVExplorerDoc::~CHFVExplorerDoc()
{
	destroy_icon_cache();
	delete m_icon_array;
}

BOOL CHFVExplorerDoc::OnNewDocument()
{
	// if (!CDocument::OnNewDocument()) return FALSE;
	return ( OnOpenDocument("") );
}

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerDoc serialization

void CHFVExplorerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerDoc diagnostics

#ifdef _DEBUG
void CHFVExplorerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHFVExplorerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

CHFVFile *speed_fp = 0;

BOOL CHFVExplorerDoc::hfs_find_item( LongInt node_index, hfs_item_type *pitem )
{
	NodeDescriptor ndescr;
	CatKeyRec catkeyrec;
	CatDataRec catdatarec;
	LongInt index;
	unsigned char key_length;
	Integer record, node_offset;

	unsigned long fnode0;
	unsigned long nodestart;
	
	nodestart = node_index * NODESIZE;
	fnode0 = nodestart;

	speed_fp->Seek( fnode0, CFile::begin );
	speed_fp->Read( &ndescr, sizeof(NodeDescriptor) );
	macpc_ndescr( &ndescr );
	switch( ndescr.ndType ) {
		case ndIndxNode:
			for( record=0; record<ndescr.ndNRecs; record++) {
				speed_fp->Read( &key_length, 1 );
				catkeyrec.ckrKeyLen = key_length;
				speed_fp->Read( &catkeyrec.ckrResrv1, 0x25 );
				macpc_catkeyrec( &catkeyrec );
				speed_fp->Read( &index, sizeof(LongInt) );
				if(key_length) { // not deleted index node
					MACPC_D(index);
					fnode0 = speed_fp->Seek( 0, CFile::current );
					if(hfs_find_item( index, pitem )) { // recursion
						return(1);
					}
					speed_fp->Seek( fnode0, CFile::begin );
				}
			}
			break;
		case ndHdrNode:
			// never here
			break;
		case ndMapNode:
			// not yet
			break;
		case ndLeafNode:
			for( record=0; record<ndescr.ndNRecs; record++) {
				speed_fp->Seek( nodestart + NODESIZE - (record+1)*2, CFile::begin );
				speed_fp->Read( &node_offset, sizeof(Integer) );
				MACPC_W(node_offset);
				speed_fp->Seek( nodestart + node_offset, CFile::begin );
				speed_fp->Read( &key_length, 1 );

				if(key_length) { // not deleted node
					catkeyrec.ckrKeyLen = key_length;
					speed_fp->Read( &catkeyrec.ckrResrv1, key_length );
					macpc_catkeyrec( &catkeyrec );
					if( (key_length & 1) == 0 ) 
						speed_fp->Seek( 1, CFile::current );
					speed_fp->Read( &catdatarec, sizeof(CatDataRec) );
					macpc_catdatarec( &catdatarec );

					int found = 0;
					if((catdatarec.u.f.filUsrWds.fdFlags & fInvisible) &&
							!m_mac_show_invisibles) 
					{
						found = 0;
					} else {
						switch( pitem->search_type ) {
							case SEARCH_TYPE_FIND_NAME:
								if(catkeyrec.ckrParID == pitem->id) {
									// if(pitem->searchname && 
									//	 pitem->searchname->CompareNoCase(CString(catkeyrec.ckrCName)) == 0) found = 1;
									if(*pitem->searchname && stricmp(pitem->searchname,(char *)catkeyrec.ckrCName) == 0) found = 1;
								}
								break;
							case SEARCH_TYPE_ID:
							case SEARCH_TYPE_ID_CALLBACK:
								if(catkeyrec.ckrParID == pitem->id) {
									found = 1;
								}
								break;
							case SEARCH_TYPE_FILES:
								if(catdatarec.cdrType == cdrFilRec &&
									 catdatarec.u.f.filUsrWds.fdType == pitem->id &&
									 catdatarec.u.f.filUsrWds.fdCreator == pitem->id2)
								{
									found = 1;
								}
								break;
							case SEARCH_TYPE_FOLDERS:
								if(catdatarec.cdrType == cdrDirRec) found = 1;
								break;
							case SEARCH_TYPE_ALL:
								found = 1;
								break;
							case SEARCH_TYPE_THISFOLDER:
								if(catdatarec.cdrType == cdrDirRec) {
									if(catdatarec.u.d.dirDirID == pitem->id) {
										found = 1;
									}
								}
								break;
						} // switch
					} // if else

					if( found ) {
						pitem->index_found++;
						if( pitem->index_to_found == pitem->index_found ||
							  pitem->search_type == SEARCH_TYPE_ID_CALLBACK ||
								pitem->search_type == SEARCH_TYPE_FILES ) {
							memcpy( &pitem->catkeyrec, &catkeyrec, key_length );
							memcpy( &pitem->catdatarec, &catdatarec, sizeof(CatDataRec) );
							if( pitem->search_type == SEARCH_TYPE_ID_CALLBACK ) {
								// fast looping, this is O(n)
								hfs_walk_callback(pitem,FINDER_VIEW_MODE_ICON);
							} else {
								// this shit is O(n2/2)!!!
								return(1);
							}
						}
					}
				}
			}
			break;
	}
	return(0);
}

BOOL CHFVExplorerDoc::hfs_find_first( 
	int volinx,
	long id, 
	long id2, 
	CString searchname,
	hfs_item_type *pitem, 
	int stype )
{
	memset( pitem, 0, sizeof(hfs_item_type) );
	pitem->volinx = volinx;
	if(searchname != "") {
		strcpy( pitem->searchname, searchname );
	} else {
		*pitem->searchname = 0;
	}
	pitem->id = id;
	pitem->id2 = id2;
	pitem->next_index = 0;

	pitem->index_to_found = 0;
	pitem->index_found = -1;
	pitem->search_type = stype;

	CHFVVolume *pvol = &m_volumes[pitem->volinx];
	speed_fp = &pvol->m_catalog_file;

	if(hfs_find_item( m_volumes[volinx].m_catalog_file.m_header_rec.bthRoot, pitem )) {
		pitem->next_index++;
		return(1);
	} else {
		return(0);
	}
}

BOOL CHFVExplorerDoc::hfs_find_next( hfs_item_type *pitem )
{
	pitem->index_to_found = pitem->next_index;
	pitem->index_found = -1;

	if(hfs_find_item( 
		m_volumes[pitem->volinx].m_catalog_file.m_header_rec.bthRoot,
		pitem )) {
		pitem->next_index++;
		return(1);
	} else {
		return(0);
	}
}

int CHFVExplorerDoc::walk_resource_fork( CFile *fp, LongInt type_to_check, Integer id_to_check )
{
	resource_header res_header;
	resource_map_header map_header;
	resource_type_item type_item;
	resource_ref_list_entry ref_entry;
	int res, typ;
	long offs1, offs2, res_data_offset;
	Integer type_count1; // count-1
	LongInt data_len;
	long g_offset;

	UPDATE_TIME_CONSUMING(1);

	g_offset = fp->Seek( 0, CFile::current );

	fp->Read( &res_header, sizeof(resource_header) );
	macpc_resource_header( &res_header );
	fp->Seek( g_offset+res_header.resource_map_offset, CFile::begin );
	fp->Read( &map_header, sizeof(resource_map_header) );
	macpc_resource_map_header( &map_header );

	offs1 = g_offset + res_header.resource_map_offset + map_header.type_list_offset;
	fp->Seek( offs1, CFile::begin );
	fp->Read( &type_count1, sizeof(type_count1) );
	MACPC_W(type_count1);
	offs1 += sizeof(type_count1);

	// Bad mistake! Can't do it this way. They use 65535 as zero...
	// for( typ=0; typ<=type_count1; typ++ ) {

	type_count1++;
	for( typ=0; typ<type_count1; typ++ ) {


		fp->Seek( offs1 + typ * sizeof(resource_type_item), CFile::begin );
		fp->Read( &type_item, sizeof(resource_type_item) );
		macpc_resource_type_item( &type_item );
		if(type_item.res_type == type_to_check) {
			offs2 = (long)res_header.resource_map_offset + 
							map_header.type_list_offset + type_item.ref_list_offset;
			offs2 += g_offset;

			Integer res_count = type_item.res_count1 + 1;
			for( res=0; res<res_count; res++ ) {
				fp->Seek( offs2 + res * sizeof(resource_ref_list_entry), CFile::begin );
				fp->Read( &ref_entry, sizeof(resource_ref_list_entry) );
				macpc_resource_ref_list_entry( &ref_entry );
				if( ref_entry.res_id == id_to_check ) {
					// found item
					res_data_offset = 
							g_offset + res_header.resource_data_offset +
							( ((long)ref_entry.res_data_offset_hi << 16) |
								ref_entry.res_data_offset_lo );
					fp->Seek( res_data_offset, CFile::begin );
					// now read data
					fp->Read( &data_len, sizeof(data_len) );
					MACPC_D(data_len);
					// read "data_len" bytes
				}
			}
		}
	}
	return(1);
}

int CHFVExplorerDoc::walk_double_resource_fork( CFile *fp, LongInt type_to_check, Integer id_to_check )
{
	apple_single_double_header h;
	apple_single_double_entry e;
	int i;

	fp->Read( &h, sizeof(apple_single_double_header) );
	mappc_apple_single_double_header(&h);
	if(h.Magic == APPLE_DOUBLE_MAGIC) {
		for(i=0; i<h.entry_count; i++) {
			fp->Read( &e, sizeof(apple_single_double_entry) );
			mappc_apple_single_double_entry(&e);
			if(e.id == Resource_Fork) {
				fp->Seek( e.offset, CFile::begin );
				walk_resource_fork( fp, type_to_check, id_to_check );
				break;
			}
		}
	}
	return(1);
}

HANDLE CHFVExplorerDoc::mac_load_any_resource( 
	int volinx, 
	CatDataRec *pCDR, 
	LongInt type_to_check,
	int nocheck,
	Integer id_to_check )
{
	resource_header res_header;
	resource_map_header map_header;
	resource_type_item type_item;
	resource_ref_list_entry ref_entry;
	int res, typ;
	long offs1, offs2, res_data_offset;
	Integer type_count1; // count-1
	LongInt data_len;
	long g_offset;
	HANDLE h = 0;

	CHFVFile f;
	CHFVFile *fp = &f;

	// g_offset = fp->Seek( 0, CFile::current );
	if(!fp->open_by_CDR( &m_volumes[volinx], pCDR, CHFVFile::OpenResourceFork )) {
		return(0);
	}
	g_offset = 0;


	fp->Read( &res_header, sizeof(resource_header) );
	macpc_resource_header( &res_header );
	if(!res_header.resource_data_length ||
		 !res_header.resource_map_length) return(0);
	fp->Seek( g_offset+res_header.resource_map_offset, CFile::begin );
	fp->Read( &map_header, sizeof(resource_map_header) );
	macpc_resource_map_header( &map_header );

	offs1 = g_offset + res_header.resource_map_offset + map_header.type_list_offset;
	fp->Seek( offs1, CFile::begin );
	fp->Read( &type_count1, sizeof(type_count1) );
	MACPC_W(type_count1);
	offs1 += sizeof(type_count1);

	type_count1++;
	for( typ=0; typ<type_count1; typ++ ) {

		fp->Seek( offs1 + typ * sizeof(resource_type_item), CFile::begin );
		fp->Read( &type_item, sizeof(resource_type_item) );
		macpc_resource_type_item( &type_item );
		if(type_item.res_type == type_to_check) {
			offs2 = (long)res_header.resource_map_offset + 
							map_header.type_list_offset + type_item.ref_list_offset;
			offs2 += g_offset;

			Integer res_count = type_item.res_count1+1;
			for( res=0; res<res_count; res++ ) {
				fp->Seek( offs2 + res * sizeof(resource_ref_list_entry), CFile::begin );
				fp->Read( &ref_entry, sizeof(resource_ref_list_entry) );
				macpc_resource_ref_list_entry( &ref_entry );
				if( (ref_entry.res_attrib & resCompressed) == 0) {
					if( nocheck || ref_entry.res_id == id_to_check ) {
						res_data_offset = 
								g_offset + res_header.resource_data_offset +
								( ((long)ref_entry.res_data_offset_hi << 16) |
									ref_entry.res_data_offset_lo );
						fp->Seek( res_data_offset, CFile::begin );
						// now read data
						fp->Read( &data_len, sizeof(data_len) );
						MACPC_D(data_len);
						// read "data_len" bytes
						if(h) GlobalFree(h);
						// Add 2000 to make space for compressed icons ...
						h = GlobalAlloc( GHND, data_len+2000 );
						if(h) {
							LPSTR p = (LPSTR)GlobalLock( h );
							fp->Read( p, data_len );
							GlobalUnlock( h );
							return(h);
						}
					}
				}
			}
		}
	}
	return(h);
}

HANDLE CHFVExplorerDoc::mac_load_any_resource2( 
	CFile *fp,
	unsigned long g_offset,
	LongInt type_to_check,
	int nocheck,
	Integer id_to_check )
{
	resource_header res_header;
	resource_map_header map_header;
	resource_type_item type_item;
	resource_ref_list_entry ref_entry;
	int res, typ;
	long offs1, offs2, res_data_offset;
	Integer type_count1; // count-1
	LongInt data_len;
	HANDLE h = 0;

	fp->Seek( g_offset, CFile::begin );
	fp->Read( &res_header, sizeof(resource_header) );
	macpc_resource_header( &res_header );
	if(!res_header.resource_data_length ||
		 !res_header.resource_map_length) return(0);
	fp->Seek( g_offset+res_header.resource_map_offset, CFile::begin );
	fp->Read( &map_header, sizeof(resource_map_header) );
	macpc_resource_map_header( &map_header );

	offs1 = g_offset + res_header.resource_map_offset + map_header.type_list_offset;
	fp->Seek( offs1, CFile::begin );
	fp->Read( &type_count1, sizeof(type_count1) );
	MACPC_W(type_count1);
	offs1 += sizeof(type_count1);
	for( typ=0; typ<=type_count1; typ++ ) {
		fp->Seek( offs1 + typ * sizeof(resource_type_item), CFile::begin );
		fp->Read( &type_item, sizeof(resource_type_item) );
		macpc_resource_type_item( &type_item );
		if(type_item.res_type == type_to_check) {
			offs2 = (long)res_header.resource_map_offset + 
							map_header.type_list_offset + type_item.ref_list_offset;
			offs2 += g_offset;

			Integer res_count = type_item.res_count1+1;
			for( res=0; res<res_count; res++ ) {
				fp->Seek( offs2 + res * sizeof(resource_ref_list_entry), CFile::begin );
				fp->Read( &ref_entry, sizeof(resource_ref_list_entry) );
				macpc_resource_ref_list_entry( &ref_entry );
				if( (ref_entry.res_attrib & resCompressed) == 0) {
					if( nocheck || ref_entry.res_id == id_to_check ) {
						res_data_offset = 
								g_offset + res_header.resource_data_offset +
								( ((long)ref_entry.res_data_offset_hi << 16) |
									ref_entry.res_data_offset_lo );
						fp->Seek( res_data_offset, CFile::begin );
						// now read data
						fp->Read( &data_len, sizeof(data_len) );
						MACPC_D(data_len);
						// read "data_len" bytes
						// Add 2000 to make space for compressed icons ...
						h = GlobalAlloc( GHND, data_len+2000 );
						if(h) {
							LPSTR p = (LPSTR)GlobalLock( h );
							fp->Read( p, data_len );
							GlobalUnlock( h );
							return(h);
						}
					}
				}
			}
		}
	}
	return(h);
}

// #include "bndl.cpp"

HICON CHFVExplorerDoc::mac_load_icon( int volinx, CatDataRec *pCDR, int small )
{
	HICON hicon = 0;
	HANDLE h = 0;
	int dim;

	int iscolor = 0;

	dim = small ? 16 : 32;

	if(pCDR->u.f.filUsrWds.fdFlags & kHasBundle) {
		if(m_bits_per_pixel >= 8) {
			h = mac_load_any_resource ( 
						volinx, pCDR,
						small ? (unsigned long)'ics8' :	(unsigned long)'icl8',
						1, 0 );
		}
		if(h) {
			iscolor = 1;
		} else {
			h = mac_load_any_resource( 
						volinx,
						pCDR,
						small ? (unsigned long)'ics#' : (unsigned long)'ICN#',
						1, 0 );
		}
		
		if(h) {
			unsigned char *p = (unsigned char *)GlobalLock( h );
			CHFVExplorerApp	*pApp;
			pApp = (CHFVExplorerApp *)AfxGetApp();

			if(iscolor) {
				unsigned char mask[COLORICONSIZE+100];
				int bytes;

				bytes = small ? (COLORICONSIZE>>2) : COLORICONSIZE;
				memset( mask, 0, bytes );
				negate_buffer( (unsigned char *)p, bytes );

				/*
				int i;
				for(i=0; i<bytes; i++) {
					if(p[i] == (unsigned char)255) {
						mask[i] = (unsigned char)p[i];
						// p[i] = (unsigned char)0;
					}
				}
				*/

				hicon = CreateIcon( 
					pApp->m_hInstance, dim, dim, 1, 8,
					(unsigned char *)mask,
					(unsigned char *)p
				);
			} else {
				int half;
				negate_buffer( (unsigned char *)p, small ? (BW_ICONSIZE>>2) : BW_ICONSIZE );
				half = (BW_ICONSIZE>>1);
				if(small) half = (BW_ICONSIZE>>3);
				hicon = CreateIcon( 
					pApp->m_hInstance, dim, dim, 1, 1, 
					(unsigned char *)p,
					(unsigned char *)p + half
				);
			}
			GlobalUnlock( h );
		}
	}
	return(hicon);
}

void CHFVExplorerDoc::hfs_walk_callback( hfs_item_type *pitem, int finder_view_mode )
{
	int isdir;
	unsigned long size_in_bytes;
	unsigned long creation_date;
	unsigned long modification_date;
	char stype[100];
	char screator[100];
	unsigned long id;
	HICON hicon16, hicon32;
	HTREEITEM htree;
	// hfs_point point;
	POINT p;
	int is_inited = 0;
	int finder_color = FINDER_COLOR_NONE;

	// Special handling for folder icon. Some folks don't
	// put the invisible flag properly on.
	if(!m_mac_show_invisibles) {
		if(strcmp((char *)pitem->catkeyrec.ckrCName,"Icon\x00D") == 0) {
		 	return;
		}		
	}

	p.x = -100;
	p.y = -100;

	if(*pitem->catkeyrec.ckrCName) {
		id = 0;
		isdir = 0;
		size_in_bytes = 0;
		creation_date = 0;
		modification_date = 0;
		*stype = 0;
		*screator = 0;
		hicon16 = hicon32 = 0;

		if(pitem->catdatarec.cdrType == cdrDirRec) isdir = 1;
		if(pitem->catdatarec.cdrType == cdrThdRec) isdir = 1;
		if(isdir) {
			strcpy( stype, "Folder" );
			if(pitem->catdatarec.cdrType == cdrDirRec) {
				if((pitem->catdatarec.u.d.dirUsrInfo.frFlags & fInvisible) && !m_mac_show_invisibles) {
					return;
				}
				finder_color = (pitem->catdatarec.u.d.dirUsrInfo.frFlags & kColorMask) >> 1;
				is_inited = (pitem->catdatarec.u.d.dirUsrInfo.frFlags & kHasBeenInited) != 0;
				size_in_bytes = pitem->catdatarec.u.d.dirVal;
				creation_date = pitem->catdatarec.u.d.dirCrDat;
				modification_date = pitem->catdatarec.u.d.dirMdDat;
				id = pitem->catdatarec.u.d.dirDirID;

				// Try to load folder custom icon
				CatDataRec *pIcon13catdatarec;
				pIcon13catdatarec = HFSIFACE_get_last_Icon13();
				if(pIcon13catdatarec) {
#ifndef ZAPPER
					load_custom_icons( 
							pitem->volinx,
							pIcon13catdatarec,
							&hicon16, 
							&hicon32 );
#endif
				}
				p.x = (int)(signed short)pitem->catdatarec.u.d.dirUsrInfo.frLocation.h;
				p.y = (int)(signed short)pitem->catdatarec.u.d.dirUsrInfo.frLocation.v;
			}

		} else {
			if((pitem->catdatarec.u.f.filUsrWds.fdFlags & fInvisible) && !m_mac_show_invisibles) {
				return;
			}

			finder_color = (pitem->catdatarec.u.f.filUsrWds.fdFlags & kColorMask) >> 1;
			
			is_inited = (pitem->catdatarec.u.f.filUsrWds.fdFlags & kHasBeenInited) != 0;
			OSType fdType = pitem->catdatarec.u.f.filUsrWds.fdType;
			OSType fdCreator = pitem->catdatarec.u.f.filUsrWds.fdCreator;
			if(pitem->catdatarec.cdrType == cdrFilRec) {
				size_in_bytes = pitem->catdatarec.u.f.filLgLen + 
												pitem->catdatarec.u.f.filRLgLen;
				creation_date = pitem->catdatarec.u.f.filCrDat;
				modification_date = pitem->catdatarec.u.f.filMdDat;
				long_to_string( fdType,(unsigned char *)stype );
				long_to_string( fdCreator,(unsigned char *)screator );

				// First try to find custom icons
#ifndef ZAPPER
				if(pitem->catdatarec.u.f.filUsrWds.fdFlags & kHasCustomIcon) {
					load_custom_icons( 
							pitem->volinx,
							&pitem->catdatarec,
							&hicon16, 
							&hicon32 );
				}
#endif

				// Try to load from cache
				if(!hicon32) hicon32 = map_type_and_creator ( fdType, fdCreator, 0 );
				if(!hicon16) hicon16 = map_type_and_creator ( fdType, fdCreator, 1 );

				// Check if it has a bundle...
				if(!hicon16 || !hicon32) {
					process_bundle(
						pitem->volinx,
						&pitem->catdatarec,
						fdCreator
					);
					// ... and perhaps the icon is in cache now?
					if(!hicon32) hicon32 = map_type_and_creator ( fdType, fdCreator, 0 );
					if(!hicon16) hicon16 = map_type_and_creator ( fdType, fdCreator, 1 );
				}

				// File aliases
				if(!hicon32 && fdType == 'adrp')
					hicon32 = map_type_and_creator ( 'APPL', fdCreator, 0 );
				if(!hicon16 && fdType == 'adrp')
					hicon16 = map_type_and_creator ( 'APPL', fdCreator, 1 );

				// Folder aliases .. this is not correct
				if(!hicon32 && fdType == 'fdrp')
					hicon32 = map_type_and_creator ( 'APPL', fdCreator, 0 );
				if(!hicon16 && fdType == 'fdrp')
					hicon16 = map_type_and_creator ( 'APPL', fdCreator, 1 );

				p.x = (int)(signed short)pitem->catdatarec.u.f.filUsrWds.fdLocation.h;
				p.y = (int)(signed short)pitem->catdatarec.u.f.filUsrWds.fdLocation.v;
			}
		}

		CHFVExplorerApp	*pApp;
		pApp = (CHFVExplorerApp *)AfxGetApp();
		if(isdir) {
			htree = pApp->m_tree->tree_insert( 
				m_volumes[pitem->volinx].m_rootitem,
				pitem->volinx,
				(LPSTR)pitem->catkeyrec.ckrCName, 
				id, pitem->id );
		} else {
			htree = 0;
		}

		// This would be an alternate way to implement label colors.
		/*
		if(finder_color != FINDER_COLOR_NONE) {
			if(hicon32) {
				HICON old_hicon32 = hicon32;
				hicon32 = make_finder_colored_icon( hicon32, finder_color );
				if(hicon32 != 0 && hicon32 != old_hicon32) {
					add_to_icon_cache( hicon32 );
				}
			}
		}
		*/

		pApp->m_list->list_insert( 
			(LPSTR)pitem->catkeyrec.ckrCName,
			isdir,
			size_in_bytes,
			creation_date,
			modification_date,
			stype,
			screator,
			id,
			hicon32,
			hicon16,
			htree,
			pitem->volinx,
			&p,
			is_inited,
			finder_view_mode,
			finder_color
		);
	}
}

// not used any more
void CHFVExplorerDoc::hfs_walk( int volinx, long id, CString searchname )
{
	hfs_item_type item;
	// TRACE( "hfs_walk\n" );
	START_TIME_CONSUMING(1000);
	(void)hfs_find_first( volinx, id, 0, searchname, &item, SEARCH_TYPE_ID_CALLBACK );
	END_TIME_CONSUMING;
}

void CHFVExplorerDoc::set_current_hitem( HTREEITEM hItem )
{
	CHFVExplorerApp	*pApp;

	if(hItem) {
		pApp = (CHFVExplorerApp *)AfxGetApp();
		pApp->m_tree->GetTreeCtrl().EnsureVisible( hItem );
		pApp->m_tree->GetTreeCtrl().SelectItem( hItem );
	}
}

HTREEITEM CHFVExplorerDoc::get_current_hitem()
{
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	return( pApp->m_tree->GetTreeCtrl().GetSelectedItem() );
}

/*
void CHFVExplorerDoc::open_parent_directory(unsigned long id)
{
	HTREEITEM hItem;
	int volinx;

	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();
	if(hItem) {
		if( id == ID_GOTO_PARENT ) {
			hItem = pApp->m_tree->GetTreeCtrl().GetParentItem( hItem );
			if(hItem) {
				pApp->m_tree->GetTreeCtrl().SelectItem( hItem );
			}
		} else {
			long dat;
			dat = pApp->m_tree->GetTreeCtrl().GetItemData( hItem );
			volinx = pApp->m_tree->get_lparam_volinx(dat);
			if(pApp->m_tree->get_lparam_type(dat) == TREE_FAT) {
				hItem = pApp->m_list->get_lparam_htree(dat);
				if(hItem) {
					pApp->m_tree->GetTreeCtrl().SelectItem( hItem );
					// open_fat_directory( volinx, dir );
				}
			} else {
				pApp->m_tree->select_by_id( m_volumes[volinx].m_rootitem, volinx, id );
				open_sub_directory( volinx, id, "" );
			}
		}
	}
}
*/

CString CHFVExplorerDoc::get_directory (
	HTREEITEM hItem, 
	int *pmac )
{
	;
	CHFVExplorerApp	*pApp;
	unsigned long id;
	int volinx;

	*pmac = 0;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	if(!hItem) hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();
	if(hItem) {
		long dat;
		dat = pApp->m_tree->GetTreeCtrl().GetItemData( hItem );
		switch(pApp->m_tree->get_lparam_type(dat)) {
			case LIST_UNKNOWN:
			case LIST_SPEC:
				return("");
			case LIST_HFS:
				*pmac = 1;
				volinx = pApp->m_tree->get_lparam_volinx(dat);
				id = pApp->m_tree->get_lparam_id(dat);
				return( pApp->m_tree->get_directory_name( 
					m_volumes[volinx].m_rootitem, 
					volinx, id ) );
				break;
			case LIST_FAT:
				char path[MAX_PATH];
				if(pApp->m_tree->get_lparam_path(dat,path,MAX_PATH)) {
					return(CString(path));
				}
				break;
		}
	}
	return("");
}

void CHFVExplorerDoc::new_hfs_walk( int volinx, long id, CString searchname )
{
	hfs_item_type item;
	int ret;
	char *volpath, *path;
	int finder_view_mode = FINDER_VIEW_MODE_ICON;

	// TRACE( "new_hfs_walk\n" );
	START_TIME_CONSUMING(1000);

	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	CString doc_name;

	if(searchname == "") {
		doc_name =
			pApp->m_tree->get_directory_name( m_volumes[volinx].m_rootitem, volinx, id ) +
			CString(":");
	} else {
		doc_name = searchname + CString(":");
	}
	
	volpath = m_volumes[volinx].m_file_name.GetBuffer( _MAX_PATH );
	path = doc_name.GetBuffer( _MAX_PATH );

	// Find out original Mac view mode
	if(pApp->m_mac_window_mode) {
		char parent_dir[_MAX_PATH], this_name[_MAX_PATH], *ptmp;
		strcpy( parent_dir, path );
		ptmp = strrchr( parent_dir, ':' );
		if(ptmp) { // always
			*ptmp = 0;
			ptmp = strrchr( parent_dir, ':' );
			if(ptmp) {
				ptmp++;
				strcpy( this_name, ptmp );
				*ptmp = 0;
				memset( &item, 0, sizeof(item) );
				item.volinx = volinx;
				item.id = pApp->m_tree->get_parent_id( m_volumes[volinx].m_rootitem, volinx, id );
				ret = HFSIFACE_get_dir_first( volpath, parent_dir, &item );
				if (ret == 0) {
					while (ret == 0) {
						mac_to_pc_charset( item.catkeyrec.ckrCName );
						if(strcmp((char*)item.catkeyrec.ckrCName,this_name) == 0) {
							finder_view_mode = get_finder_view_mode( item.catdatarec.u.d.dirUsrInfo.frView );
							break;
						}
						ret = HFSIFACE_get_dir_next( &item );
					}
					HFSIFACE_get_dir_flush();
				}
			}
		}
		switch(finder_view_mode) {
			case FINDER_VIEW_MODE_ICON:
			case FINDER_VIEW_MODE_ICON_BUTTON:
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_ICON) {
					pApp->m_list->set_style( LVS_ICON );
				}
				break;
			case FINDER_VIEW_MODE_ICON_SMALL:
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_SMALLICON) {
					pApp->m_list->set_style( LVS_SMALLICON );
				}
				break;
			case FINDER_VIEW_MODE_ICON_NAME:
				pApp->m_list->m_sorted_by_column = FIELD_NAME;
				pApp->m_list->m_sort_is_ascending = 1;
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) {
					pApp->m_list->set_style( LVS_REPORT );
				}
				break;
			case FINDER_VIEW_MODE_ICON_DATE:
				pApp->m_list->m_sorted_by_column = FIELD_MODIFIED;
				pApp->m_list->m_sort_is_ascending = 1;
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) {
					pApp->m_list->set_style( LVS_REPORT );
				}
				break;
			case FINDER_VIEW_MODE_ICON_SIZE:
				pApp->m_list->m_sorted_by_column = FIELD_SIZE;
				pApp->m_list->m_sort_is_ascending = 1;
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) {
					pApp->m_list->set_style( LVS_REPORT );
				}
				break;
			case FINDER_VIEW_MODE_ICON_KIND:
				pApp->m_list->m_sorted_by_column = FIELD_TYPE;
				pApp->m_list->m_sort_is_ascending = 1;
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) {
					pApp->m_list->set_style( LVS_REPORT );
				}
				break;
			case FINDER_VIEW_MODE_ICON_LABEL:
				pApp->m_list->m_sorted_by_column = FIELD_CREATOR;
				pApp->m_list->m_sort_is_ascending = 1;
				if((GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK) != LVS_REPORT) {
					pApp->m_list->set_style( LVS_REPORT );
				}
				break;
		}
	}

	// Find out max/min scroll bounds
#define OUTOFBOUNDS 10000
	int widow_style = (GetWindowLong(pApp->m_list->m_cl->m_hWnd,GWL_STYLE) & LVS_TYPEMASK);
	if(pApp->m_mac_window_mode != 0 && (widow_style == LVS_ICON || widow_style == LVS_SMALLICON)) {
		int x = OUTOFBOUNDS, y = OUTOFBOUNDS, px, py;
		int is_inited = 0, is_invisible = 0;
		memset( &item, 0, sizeof(item) );
		item.volinx = volinx;
		item.id = id;
		ret = HFSIFACE_get_dir_first( volpath, path, &item );
		while (ret == 0) {
			if(item.catdatarec.cdrType == cdrDirRec) {
				is_inited = (item.catdatarec.u.d.dirUsrInfo.frFlags & kHasBeenInited) != 0;
				is_invisible = (item.catdatarec.u.d.dirUsrInfo.frFlags & fInvisible);
				px = (int)(signed short)item.catdatarec.u.d.dirUsrInfo.frLocation.h;
				py = (int)(signed short)item.catdatarec.u.d.dirUsrInfo.frLocation.v;
				if(px == -1 && py == -1) is_inited = 0;
				if(is_invisible == 0 || m_mac_show_invisibles) {
					if(is_inited || is_inside_magic(px,py)) {
						px = unmagic(px);
						py = unmagic(py);
						if(px < x) x = px;
						if(py < y) y = py;
					}
				}
			} else if(item.catdatarec.cdrType == cdrFilRec) {
				is_inited = (item.catdatarec.u.f.filUsrWds.fdFlags & kHasBeenInited) != 0;
				is_invisible = (item.catdatarec.u.f.filUsrWds.fdFlags & fInvisible);
				if(strcmp((char *)item.catkeyrec.ckrCName,"Icon\x00D") == 0) is_invisible = 1;
				px = (int)(signed short)item.catdatarec.u.f.filUsrWds.fdLocation.h;
				py = (int)(signed short)item.catdatarec.u.f.filUsrWds.fdLocation.v;
				if(px == -1 && py == -1) is_inited = 0;
				if(is_invisible == 0 || m_mac_show_invisibles) {
					if(is_inited || is_inside_magic(px,py)) {
						px = unmagic(px);
						py = unmagic(py);
						if(px < x) x = px;
						if(py < y) y = py;
					}
				}
			}
			ret = HFSIFACE_get_dir_next( &item );
		}
		if(x == OUTOFBOUNDS) x = 0;
		if(y == OUTOFBOUNDS) y = 0;
		pApp->m_list->set_origo(x,y);
	}
#undef OUTOFBOUNDS

	// enumerate items into list view
	memset( &item, 0, sizeof(item) );
	item.volinx = volinx;
	item.id = id;

	ret = HFSIFACE_get_dir_first( volpath, path, &item );

	while (ret == 0) {
		mac_to_pc_charset( item.catkeyrec.ckrCName );
		hfs_walk_callback( &item, finder_view_mode );
		ret = HFSIFACE_get_dir_next( &item );
	}

	doc_name.ReleaseBuffer();
	m_volumes[volinx].m_file_name.ReleaseBuffer();
	pApp->m_list->do_report_sort();
	END_TIME_CONSUMING;
}

void CHFVExplorerDoc::open_sub_directory( int volinx, unsigned long id, CString partial_name )
{
	unsigned long parent_link_id = 0;
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	CString dirname;

	if(id == configurations_id) {
		pApp->m_list->initialize_volume( "", 0 );
	} else {
		int i;
		if(!m_volumes[volinx].m_opened) return;
		parent_link_id = pApp->m_tree->get_parent_id( m_volumes[volinx].m_rootitem, volinx, id );

		for(i=1; i<=2; i++) {
			int old_icon_count = get_icon_cache_count();
			if(parent_link_id) {
				pApp->m_tree->expand_dir( m_volumes[volinx].m_rootitem,
									volinx, parent_link_id );
				pApp->m_list->initialize_volume( 
					pApp->m_tree->get_directory_name( m_volumes[volinx].m_rootitem, volinx, parent_link_id ),
					ID_GOTO_PARENT );
			} else {
				pApp->m_list->initialize_volume( "", 0 );
			}
			new_hfs_walk( volinx, id, partial_name );
			if( old_icon_count == get_icon_cache_count()) break;
			// else new items in directory. walk again to ensure proper icons.
		}
		if(partial_name != "") 
			dirname = partial_name;
		else
			dirname = pApp->m_tree->get_directory_name( m_volumes[volinx].m_rootitem, volinx, id );
	}

	if(pApp->m_pStatusBar) {
		CString msg;
		if(id == configurations_id) {
			msg = "Exploring Configurations";
		} else {
			msg.Format( 
				"Exploring \"%s\": %ld kB free", 
				dirname, 
				m_volumes[volinx].m_MDB.drFreeBks * m_volumes[volinx].m_MDB.drAlBlkSiz / 1024 );
		}
		pApp->m_pStatusBar->SetPaneText( 0, msg );
	}
}

#pragma optimize("",off)
void CHFVExplorerDoc::open_fat_directory( int volinx, LPSTR path, int list )
{
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();

	dbdb("open_fat_directory");
	if(list) {
		if(is_fat_root_directory(path)) {
			pApp->m_list->initialize_volume( "", 0 );
			// pApp->m_list->initialize_volume( "", ID_GOTO_PARENT );
		} else {
			// Remove the last subdir
			char buf[_MAX_PATH], *p;
			strcpy( buf, path );
			p = strrchr( buf, '\\' );
			if(p) {
				if( p[-1] == ':' ) p++;
				*p = 0;
			}
			pApp->m_list->initialize_volume( buf, ID_GOTO_PARENT );
		}
	}
	fat_walk( volinx, path, list );
	dbdb("open_fat_directory END");
}
#pragma optimize("",on)

// must be modified to search fat volumes too
CString CHFVExplorerDoc::hfs_find_match( 
	LPSTR type, 
	LPSTR creator, 
	int prefered_volinx )
{
	unsigned long ut, uc;
	int found = 0;
	hfs_item_type item;
	int volinx;

	ut = string_to_long( (unsigned char *)type );
	uc = string_to_long( (unsigned char *)creator );

	CString path;
	path = "";
	START_TIME_CONSUMING(1000);

	found = 0; // hfs_find_first( prefered_volinx, ut, uc, "", &item, SEARCH_TYPE_FILES );
	prefered_volinx = -1;

	if(found) {
		volinx = prefered_volinx;
	} else {
		for( volinx=0; volinx<m_hfs_count; volinx++) {
		// for( volinx=0; volinx<m_hfs_count && volinx != prefered_volinx; volinx++) {
			found = hfs_find_first( volinx, ut, uc, "", &item, SEARCH_TYPE_FILES );
			if( found ) break;
		}
	}
	if( found ) {
		while( found ) {
			UPDATE_TIME_CONSUMING(1);
			if(path == "") {
				path = CString(item.catkeyrec.ckrCName);
			} else {
				path = CString(item.catkeyrec.ckrCName) + ":" + path;
			}
			if(item.catkeyrec.ckrParID == CNID_ROOT_ID) {
				found = 0;
			} else {
				found = hfs_find_first( volinx, item.catkeyrec.ckrParID, 0, "", &item, SEARCH_TYPE_THISFOLDER );
			}
		}
		path = CString(m_volumes[volinx].m_volume_name) + ":" + path;
	}
	END_TIME_CONSUMING;
	return( path );
}

BOOL CHFVExplorerDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	int i;
	int param_is_volume = 0;
	CHFVExplorerApp	*pApp;
	int showme = -1;

	pApp = (CHFVExplorerApp *)AfxGetApp();

	pApp->m_tree->enable_selchange(0);

	OnCleanDocument();
	// if (!CDocument::OnOpenDocument(lpszPathName)) return FALSE;

	if(lpszPathName && *lpszPathName ) {
		param_is_volume = is_extension( (char *)lpszPathName, ".HF*" );
		param_is_volume |= is_extension( (char *)lpszPathName, ".DSK" );
	}

	if(param_is_volume) {
		i = insert_volume( CString(lpszPathName), 0, 0, 0, 0 );
		if(i >= 0) {
			if(m_volumes[i].open()) {
				m_volumes[i].m_rootitem = 
					pApp->m_tree->initialize_volume( 
						i, m_volumes[i].m_volume_name, 
						CNID_ROOT_ID, 
						m_volumes[i].m_is_floppy,
						m_volumes[i].m_is_cd,
						m_volumes[i].m_is_hd,
						m_volumes[i].m_is_removable);
				pApp->AddToRecentFileList(lpszPathName);
				if(showme < 0) showme = i;
			} else {
				// m_hfs_count--;
				m_volumes[i].m_volume_name = "";
				m_volumes[i].m_file_opened = 0;
				m_volumes[i].m_file_name = "";
				m_volumes[i].m_free = 1;
				m_volumes[i].m_rootitem = 0;
				m_volumes[i].m_opened = 0;
			}
		}
	} 

	pApp->m_tree->initialize_tree();

	for( i=0; i<m_hfs_count; i++) {
		m_volumes[i].m_rootitem = 0;
		if(!m_volumes[i].m_free && m_volumes[i].open()) {
			m_volumes[i].m_rootitem = 
				pApp->m_tree->initialize_volume( 
					i, m_volumes[i].m_volume_name, 
					CNID_ROOT_ID, 
					m_volumes[i].m_is_floppy,
					m_volumes[i].m_is_cd,
					m_volumes[i].m_is_hd,
					m_volumes[i].m_is_removable );
			if(showme < 0) showme = i;
		} else {
			m_volumes[i].m_volume_name = "";
			m_volumes[i].m_file_opened = 0;
			m_volumes[i].m_file_name = "";
			m_volumes[i].m_free = 1;
			m_volumes[i].m_rootitem = 0;
			m_volumes[i].m_opened = 0;
		}
	}

	for( i=0; i<m_fat_count; i++) {
		m_fats[i].m_rootitem = 
			pApp->m_tree->initialize_fat_volume( 
				i, 
				&m_fats[i].m_volume_name, 
				m_fats[i].m_type,
				&m_fats[i].m_volume_name,
				toupper(m_fats[i].m_letter) < 'C' );
	}

	if(showme >= 0) {
		open_sub_directory( showme, CNID_ROOT_ID, m_volumes[showme].m_volume_name );
	}

	pApp->m_tree->enable_selchange(1);
	// if(m_hfs_count && m_volumes[0].m_rootitem) {
	if(showme >= 0) {
		set_current_hitem( m_volumes[showme].m_rootitem );
	}
	
	return(1);
}

int CHFVExplorerDoc::my_split_path( CString path, CString &volname, int *volinx, CString &dir, int mac ) 
{
	int i, inx;
	inx = path.Find( mac ? ':' : '\\' );
	if(inx < 0) {
		volname = path;
		dir = "";
	} else {
		volname = path.Left( inx );
		dir = path.Right( path.GetLength() - inx - 1 );
	}
	for( i=0; i<m_hfs_count; i++ ) {
		if(volname.CompareNoCase(m_volumes[i].m_volume_name) == 0) {
			*volinx = i;
			return(1);
		}
	}
	return(0);
}

void CHFVExplorerDoc::build_pc_tree( int volinx, CString path )
{
	char *p, *start;
	int first = 1;
	char subpath[MAX_PATH];
	int i;
	HTREEITEM htree;
	CHFVExplorerApp	*pApp;

	dbdb( "build_pc_tree");

	pApp = (CHFVExplorerApp *)AfxGetApp();
	start = path.GetBuffer(MAX_PATH);
	p = start;
	p = strchr( p, '\\' );
	htree = m_fats[volinx].m_rootitem;

	while(p) {
		strcpy( subpath, start );
		i = (long)p - (long)start;
		if(first) i++; // root dir backlash
		first = 0;
		subpath[i] = 0;
		open_fat_directory( volinx, subpath, 1 ); // ZAPZAP 0
		htree = pApp->m_tree->tree_find_fat( htree, subpath );
		if(htree) {
			pApp->m_tree->GetTreeCtrl().EnsureVisible( htree );
			pApp->m_tree->GetTreeCtrl().SelectItem( htree );
		}
		p = strchr( p+1, '\\' );
	}
	pApp->m_tree->enable_selchange(1);
	htree = pApp->m_tree->tree_find_fat( htree, start );
	if(htree) {
		pApp->m_tree->GetTreeCtrl().EnsureVisible( htree );
		pApp->m_tree->GetTreeCtrl().SelectItem( htree );
	}
	path.ReleaseBuffer();
	dbdb( "build_pc_tree END");
}

void CHFVExplorerDoc::my_chdir( CString path, int mac )
{
	int volinx, dummy;
	CString volname, dir, subdir, rest;
	CHFVExplorerApp	*pApp;
	hfs_item_type item;
	long id = 0;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	if(path == "") return;

	if(!pApp->m_tree) return;

	if(!mac) {
		int i, volinx = -1;
		int save_view_dos_icons = pApp->m_dosicons;
		pApp->m_dosicons = DI_NO_ICONS;

		for( i=0; i<m_fat_count; i++) {
			HTREEITEM htree;
			LPSTR r = path.GetBuffer(MAX_PATH);
			htree = pApp->m_tree->tree_find_fat(m_fats[i].m_rootitem,r);
			path.ReleaseBuffer();
			if(htree) {
				pApp->m_tree->enable_selchange(1);
				pApp->m_tree->GetTreeCtrl().EnsureVisible( htree );
				pApp->m_tree->GetTreeCtrl().SelectItem( htree );
				return;
			}
			if(toupper(*path) == toupper(m_fats[i].m_letter)) volinx = i;
		}
		pApp->m_dosicons = save_view_dos_icons;
		if(volinx >= 0) {
			pApp->m_tree->GetTreeCtrl().SelectItem( m_fats[volinx].m_rootitem );
			build_pc_tree( volinx, path );
		}
		return;
	}

	if(my_split_path( path, volname, &volinx, dir, mac )) {
		pApp->m_tree->enable_selchange(1);
		set_current_hitem( m_volumes[volinx].m_rootitem );
		open_sub_directory( volinx, CNID_ROOT_ID, m_volumes[volinx].m_volume_name );
		set_current_hitem( m_volumes[volinx].m_rootitem );

		START_TIME_CONSUMING(1000);
		id = CNID_ROOT_ID;
		while(dir != "") {
			(void)my_split_path( dir, subdir, &dummy, rest, mac );
			if( !hfs_find_first( volinx, id, 0, subdir, &item, SEARCH_TYPE_FIND_NAME )) {
				break;
			}
			if(item.catdatarec.cdrType != cdrDirRec) {
				break;
			}
			id = item.catdatarec.u.d.dirDirID;
			if(id) {
				open_sub_directory( volinx, id, "" );
				pApp->m_tree->select_by_id( m_volumes[volinx].m_rootitem, volinx, id );
			}
			dir = rest;
		}
		END_TIME_CONSUMING;
	}
}

void CHFVExplorerDoc::OnCleanDocument()
{
	CHFVExplorerApp	*pApp;
	int i;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	pApp->m_list->list_clear();

	if(pApp->m_tree) {
		pApp->m_tree->enable_selchange(0);
		pApp->m_tree->tree_clear();
	}
	for( i=0; i<m_hfs_count; i++) {
		m_volumes[i].close();
	}
}

void CHFVExplorerDoc::OnCloseDocument() 
{
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	if(pApp->m_tree) {
		pApp->m_tree->enable_selchange(0);
		pApp->m_tree->tree_delete_images();
	}
	OnCleanDocument();
	CDocument::OnCloseDocument();
}

void CHFVExplorerDoc::check_hfv_file_times( int ask ) 
{
	int i, updated_count = 0, mac, item_updated;
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();

	// first memorize volume and dir
	CString currdir = get_directory( 0, &mac );

	for( i=0; i<m_hfs_count; i++) {
		if(m_volumes[i].m_opened && m_volumes[i].update_if_needed(ask)) {
			updated_count++;
			item_updated = i;
		}
	}

	if(updated_count == 1) {
		pApp->m_tree->on_volume_changed( item_updated );
		pApp->m_list->on_volume_changed( item_updated );
	} else if(updated_count > 0) {
		pApp->m_list->list_clear();
		if(pApp->m_tree) {
			pApp->m_tree->tree_clear();
		}
		OnOpenDocument("") ;

		// then restore volume and dir
		if(currdir != "") {
			my_chdir( currdir, mac );
		} 
	}
}

void CHFVExplorerDoc::CheckNewMedia( BOOL check_floppies, BOOL check_cds, BOOL check_hds)
{
	int mac;

	clear_all_caches();

	int prev_silence = silencer( 1 );

	CString currdir = get_directory( 0, &mac );
	if(check_floppies) enum_floppies();
	if(check_hds) enum_hds();
	if(check_cds) enum_cds();

	OnOpenDocument("") ;

	if(currdir != "") my_chdir( currdir, mac );

	silencer( prev_silence );
}

void CHFVExplorerDoc::CheckNewFloppy( int index )
{
	char name[_MAX_PATH];

	if(is_floppy_by_index(index)) {
		clear_all_caches();
		enum_floppies();
		OnOpenDocument("");
		get_floppy_volume_file_name( index, name );
	}
}

void CHFVExplorerDoc::OnViewRefresh() 
{
	CheckNewMedia( TRUE, TRUE, TRUE);

	/*
	char xx[30];
	int fa = ::GetProfileInt("Test","fa",0);
	int fb = ::GetProfileInt("Test","fb",0);

	fa++;
	if(fa > 15) {
		fa=0; 
		fb++;
		if(fb > 15) {
			fa = fb = 0;
		}
	}
	sprintf( xx, "%d",fa);
	::WriteProfileString("Test","fa",xx);
	sprintf( xx, "%d",fb);
	::WriteProfileString("Test","fb",xx);
	sprintf( xx, "%d,%d",fa,fb);
	AfxMessageBox( xx );

	clear_hash_table();
	delete_cache();
	int mac;
	CString currdir = get_directory( 0, &mac );
	OnOpenDocument("") ;
	if(currdir != "") my_chdir( currdir, mac );
	*/
}

void CHFVExplorerDoc::set_hfs_volume_clean( char *path )
{
	int inx;

	inx = get_volume_index_by_filename( CString(path) );
	if(inx >= 0) {
		m_volumes[inx].update_time_stamp();
	}
}
