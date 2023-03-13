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

#include "stdafx.h"
#include "mactypes.h"
#include "openfile.h"
#include "utils.h"
#include "HFVExplorer.h"

#include <direct.h>

int exists( CString &name )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;

	fh = FindFirstFile( name, &FindFileData );
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
	return(fh != INVALID_HANDLE_VALUE);
}

void trim_right( CString *name, char ch )
{
	int len = name->GetLength();
	if(len < 1 || name->GetAt(len-1) != ch ) {
		*name += ch;
	}
}

void ReplaceCharacters( CString *s, char from_ch, char to_ch )
{
	int i, len = s->GetLength();
	for( i=0; i<len; i++ ) {
		if(s->GetAt(i) == from_ch) s->SetAt(i,to_ch);
	}
}

const fat_char = '\\';
const hfs_char = ':';

char get_system_char( int system )
{
	return( system == open_file_type::SystemFAT ? fat_char : hfs_char );
}

void proper_name( CString *name, int system, int request_system )
{
	char from_ch, to_ch;
	if(system != request_system) {
		if(request_system == open_file_type::SystemFAT) {
			from_ch = ':'; 
			to_ch = fat_char; 
		} else {
			from_ch = fat_char; 
			to_ch = ':'; 
		}
		ReplaceCharacters( name, from_ch, to_ch );
	}
}

void proper_dir( 
	CString *dir, 
	int dir_system, 
	int request_system )
{
	char ch = get_system_char(request_system);
	proper_name( dir, dir_system, request_system );
	if(dir->GetAt(dir->GetLength()-1) != ch) *dir += ch;
}

CString make_path( 
	CString dir, 
	int dir_system, 
	CString name,
	int name_system,
	int request_system )
{
	proper_dir( &dir, dir_system, request_system );
	proper_name( &name, name_system, request_system );
	return( dir + name );
}

CString get_title( CString *name, int system )
{
	int i = name->ReverseFind(get_system_char(system));
	if(i) {
		return( name->Right( name->GetLength()-i-1 ) );
	} else {
		return( *name );
	}
}

// with ending '\' or ':'
CString get_dir( CString *name, int system )
{
	int i = name->ReverseFind(get_system_char(system));
	if(i) {
		return( name->Left( i + 1 ) );
	} else {
		return( "" );
	}
}

open_file_type::~open_file_type()
{
	ASSERT( m_is_open == IsClosed );
}

int open_file_type::fill_values()
{
	DWORD dat;
	CListCtrl *cl;
	CHFVExplorerListView *cv;
	char p[MAX_PATH];

	cv = ((CHFVExplorerApp *)AfxGetApp())->m_list;
	if(!cv) return(0);
	cl = cv->m_cl;
	if(!cl) return(0);
	dat = cl->GetItemData( m_view_index );
	if(!dat) return(0);
	if(!cv->get_lparam_path(dat,p,MAX_PATH-1)) return(0);
	m_path = CString( p );
	if(m_path == "") return(0);
	m_isdir = cv->get_lparam_isdir(dat);
	
	/*
	m_cl->get_lparam_id( dat );
	m_cl->get_lparam_isdir( dat );
	m_cl->get_lparam_volinx
	m_cl->get_lparam_type
	m_cl->get_lparam_htree
	*/

	return(1);
}

int open_file_type::fill_values_htree( open_file_type *namefrom)
{
	DWORD dat;
	CTreeCtrl *ct;
	HFVExplorerTreeView *cv;
	char p[MAX_PATH];

	cv = ((CHFVExplorerApp *)AfxGetApp())->m_tree;
	if(!cv) return(0);
	ct = cv->m_ct;
	if(!ct) return(0);
	dat = ct->GetItemData( m_htree );
	if(!dat) return(0);
	if(!cv->get_lparam_path(dat,p,MAX_PATH-1)) return(0);
	m_path = CString( p );
	m_isdir = 1;
	if(m_path == "") return(0);
	m_path = make_path( 
		m_path, 
		m_file_system, 
		get_title( &namefrom->m_path, namefrom->m_file_system ),
		namefrom->m_file_system,
		m_file_system );
	return(1);
}

open_file_type::open_file_type( 
	int type, 
	int view_index )
{
	m_is_open = IsClosed;
	m_mode = ModeOpen;
	m_last_error = NoError;
	m_view_index = view_index;
	m_htree = 0;
	m_values_ok = 0;
	m_file_system = type;
	m_path = "";
	m_isdir = 0;

	if(fill_values()) {
		m_values_ok = 1;
	} else {
		m_values_ok = 0;
	}
}

open_file_type::open_file_type( 
	int type, 
	HTREEITEM htree,
	open_file_type *src )
{
	m_is_open = IsClosed;
	m_mode = ModeOpen;
	m_last_error = NoError;
	m_view_index = -1;
	m_htree = htree;
	m_values_ok = 0;
	m_file_system = type;
	m_path = "";

	if(fill_values_htree(src)) {
		m_values_ok = 1;
	} else {
		m_values_ok = 0;
	}
}

int open_file_type::Open( int mode )
{
	int ok = 0;

	m_mode = mode;

	if(m_is_open == IsClosed) {
		if(m_file_system == SystemHFS) {
		} else { // FAT
			if(m_mode == ModeOpen) {
				if(do_open_file( &m_dosfile, m_path )) {
					ok = 1;
					m_is_open = IsOpen;
					m_last_error = NoError;
				} else {
					m_last_error = CantOpen; // FileNotFound ?
				}
			} else { // create
				silent_delete( m_path );
				if(do_create_file( &m_dosfile, m_path )) {
					ok = 1;
					m_is_open = IsOpen;
					m_last_error = NoError;
				} else {
					m_last_error = CantOpen;
				}
			}
		}
	}
	return(ok);
}

int open_file_type::Read( unsigned char *buf, int count )
{
	int bytes_read, fail = 0;
	TRY
	{
	bytes_read = m_dosfile.Read( buf, count );
	}
	CATCH( CFileException, e )
	{
		fail = 1;
		bytes_read = 0;
	}
	END_CATCH
	if(fail) m_last_error = ReadFailure;
	else if(bytes_read != count) m_last_error = EndOfFile;
	else m_last_error = NoError;
	return(bytes_read);
}

int open_file_type::Write( unsigned char *buf, int count )
{
	int fail = 0;
	TRY
	{
	 m_dosfile.Write( buf, count );
	}
	CATCH( CFileException, e )
	{
		fail = 1;
	}
	END_CATCH
	if(fail) m_last_error = WriteFailure;
	else m_last_error = NoError;
	return(!fail);
}

int open_file_type::LastError()
{
	return( m_last_error );
}

void open_file_type::ErrorAlert()
{
	switch( m_last_error ) {
		case NoError:
			AfxMessageBox( "Unknown error, file " + m_path );
			break;
		case ReadFailure:
			AfxMessageBox( "Read error from file " + m_path );
			break;
		case WriteFailure:
			AfxMessageBox( "Write error to file " + m_path );
			break;
		case FileNotFound:
			AfxMessageBox( "File " + m_path + " could not be opened." );
			break;
		case EndOfFile:
			AfxMessageBox( "EOF reached reading file " + m_path );
			break;
		case CantOpen:
			AfxMessageBox( "Error opening file " + m_path );
			break;
	}
}

void open_file_type::Close()
{
	if(m_is_open != IsClosed) {
		if(m_file_system == SystemHFS) {
		} else { // FAT
			silent_close( &m_dosfile );
			m_is_open = IsClosed;
			m_last_error = NoError;
		}
	}
}

int open_file_type::modify_companion_name()
{
	int ok = 0;
	CString new_title, p, dir;
	CString title = get_title( &m_path, m_file_system );

	if(m_file_system == SystemFAT) {
		if(title.GetAt(0) == '%') {
			new_title = title.Mid(1);
		} else {
			new_title = '%' + title;
		}
		dir = get_dir( &m_path, SystemFAT );
		m_path = make_path( dir, SystemFAT, new_title, SystemFAT, SystemFAT );
		ok = 1;
	}
	return(ok);
}

int open_file_type::get_companion_file()
{
	int ok = 0;

	if(modify_companion_name()) {
		if(exists(m_path)) {
			// should check AppleDouble magic here
			ok = 1;
		} 
	}
	return(ok);
}

int DeleteFATDir(CString &dir)
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok = 1, looping = 0;
	CString wd, name;

	wd = dir;
	trim_right( &wd, fat_char );
	fh = FindFirstFile( wd + "*.*", &FindFileData );
	looping = (fh != INVALID_HANDLE_VALUE);
	while (looping) {
		if( *FindFileData.cFileName != '.') {
			name = wd + CString( FindFileData.cFileName );
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if(!DeleteFATDir(name)) {
					ok = 0;
				}
			} else {
				TRY {
					CFile::Remove( name );
				} CATCH( CFileException, e ) {
					AfxMessageBox( "Could not delete file " + name + "." );
					ok = 0;
				}
				END_CATCH
			}

			// NEW: since we deleted from the volume, the Windows iteration
			// structures are not in sync anymore. Must close and reopen.
			FindClose( fh );
			fh = FindFirstFile( wd + "*.*", &FindFileData );
			ok = (fh != INVALID_HANDLE_VALUE);

		}
		if(ok) 
			looping = FindNextFile( fh, &FindFileData );
		else
			looping = 0;
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
	if(ok) {
		char *lpstr = dir.GetBuffer(MAX_PATH);
		if(RemoveDirectory(lpstr) == 0) {
			AfxMessageBox( "Could not delete directory " + dir + "." );
			ok = 0;
		}
		dir.ReleaseBuffer();
	}
	return(ok);
}

void open_file_type::Delete()
{
	if(m_path == "") return;
	if(m_file_system == SystemHFS) {
		ASSERT(0);
	} else { // FAT
		Close();
		if(m_isdir) {
			DeleteFATDir(m_path);
			if(modify_companion_name()) {
				if(exists(m_path)) {
					// This is not a dir!
					// DeleteFATDir(m_path);
					silent_delete( m_path );
				}
			}
		} else {
			silent_delete( m_path );
			if(modify_companion_name()) {
				if(exists(m_path)) {
					silent_delete( m_path );
				}
			}
		}
	}
}

CString open_file_type::GetPath()
{
	if(m_file_system == SystemHFS) {
		return("");
	} else { // FAT
		return(m_path);
	}
}
