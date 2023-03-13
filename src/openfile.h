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

#include "mactypes.h"

class open_file_type
{
public:
	~open_file_type();
	open_file_type( int type, int view_index );
	open_file_type( int type, HTREEITEM htree );
	open_file_type( int type, HTREEITEM htree, open_file_type *src );

	int get_companion_file();
	int modify_companion_name();

	enum { SystemFAT, SystemHFS };

	enum { IsOpen, IsClosed };
	enum OpenMode { ModeOpen, ModeCreate };
	enum ErrorCode { 
		NoError=0, ReadFailure, WriteFailure, 
		FileNotFound, CantOpen, EndOfFile };

	int Open( int mode = ModeOpen );
	int Read( unsigned char *buf, int count );
	int Write( unsigned char *buf, int count );
	int LastError();
	void ErrorAlert();
	void Close();
	void Delete();
	CString GetPath();

protected:
	int fill_values();
	int fill_values_htree( open_file_type *namefrom );

	int m_is_open;
	int m_file_system;
	int m_mode;
	int m_last_error;
	int m_view_index;
	HTREEITEM m_htree;
	int m_values_ok;
	int m_isdir;
	CFile m_dosfile;
	CString m_path;
	CHFVFile m_macfile;
};
