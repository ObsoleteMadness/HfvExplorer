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
#include "HFVExplorer.h"
#include "FileTypeMapping.h"
#include "tmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CMapStringToString map_hfs2fat;
static CMapStringToString map_fat2hfs;

/////////////////////////////////////////////////////////////////////////////
// CFileTypeMapping dialog


CFileTypeMapping::CFileTypeMapping(CWnd* pParent /*=NULL*/)
	: CDialog(CFileTypeMapping::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileTypeMapping)
	m_enabled = FALSE;
	m_strip = FALSE;
	m_fat2hfs = FALSE;
	m_hfs2fat = FALSE;
	m_dos = _T("");
	m_creator = _T("");
	m_type = _T("");
	m_comment = _T("");
	//}}AFX_DATA_INIT
	m_enabled_param = FALSE;
}


void CFileTypeMapping::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileTypeMapping)
	DDX_Control(pDX, IDC_FILETYPE_LIST, m_list);
	DDX_Check(pDX, IDC_FILETYPE_ENABLED, m_enabled);
	DDX_Check(pDX, IDC_FILETYPE_STRIP, m_strip);
	DDX_Check(pDX, IDC_FILETYPE_FAT2HFS, m_fat2hfs);
	DDX_Check(pDX, IDC_FILETYPE_HFS2FAT, m_hfs2fat);
	DDX_Text(pDX, IDC_FILETYPE_DOS, m_dos);
	DDV_MaxChars(pDX, m_dos, 8);
	DDX_Text(pDX, IDC_FILETYPE_CREATOR, m_creator);
	DDV_MaxChars(pDX, m_creator, 4);
	DDX_Text(pDX, IDC_FILETYPE_TYPE, m_type);
	DDV_MaxChars(pDX, m_type, 4);
	DDX_Text(pDX, IDC_FILETYPE_COMMENT, m_comment);
	DDV_MaxChars(pDX, m_comment, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileTypeMapping, CDialog)
	//{{AFX_MSG_MAP(CFileTypeMapping)
	ON_BN_CLICKED(IDC_FILETYPE_EXPORT, OnFiletypeExport)
	ON_BN_CLICKED(IDC_FILETYPE_EXPORT_SELECTED, OnFiletypeExportSelected)
	ON_BN_CLICKED(IDC_FILETYPE_DELETE, OnFiletypeDelete)
	ON_BN_CLICKED(IDC_FILETYPE_ENABLED, OnFiletypeEnabled)
	ON_BN_CLICKED(IDC_FILETYPE_IMPORT, OnFiletypeImport)
	ON_LBN_SELCHANGE(IDC_FILETYPE_LIST, OnSelchangeFiletypeList)
	ON_BN_CLICKED(IDC_FILETYPE_NEW, OnFiletypeNew)
	ON_BN_CLICKED(IDC_FILETYPE_SELECT_ALL, OnFiletypeSelectAll)
	ON_BN_CLICKED(IDC_FILETYPE_SELECT_ALL2, OnFiletypeSelectAll2)
	ON_BN_CLICKED(IDC_FILETYPE_STRIP, OnFiletypeStrip)
	ON_BN_CLICKED(IDC_FILETYPE_FAT2HFS, OnFiletypeFat2hfs)
	ON_BN_CLICKED(IDC_FILETYPE_HFS2FAT, OnFiletypeHfs2fat)
	ON_EN_UPDATE(IDC_FILETYPE_DOS, OnUpdateFiletypeDos)
	ON_EN_UPDATE(IDC_FILETYPE_TYPE, OnUpdateFiletypeType)
	ON_EN_UPDATE(IDC_FILETYPE_CREATOR, OnUpdateFiletypeCreator)
	ON_BN_CLICKED(IDC_FILETYPE_DUPLICATE, OnFiletypeDuplicate)
	ON_BN_CLICKED(IDC_FILETYPE_HELP, OnFiletypeHelp)
	ON_EN_UPDATE(IDC_FILETYPE_COMMENT, OnUpdateFiletypeComment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTypeMapping message handlers

void CFileTypeMapping::OnFiletypeDelete() 
{
	int minx, i, count = m_list.GetSelCount();

	if(count == 1) {
		i = m_list.GetCurSel();
		if( i >= 0 ) {
			m_list.DeleteString( i );
			if(i >= m_list.GetCount()) i--;
			m_list.SetCurSel(i);
			m_list.SelItemRange( FALSE, 0, m_list.GetCount()-1 );
			m_list.SetSel(i, TRUE);
		}
	} else if(count > 1) {
		LPINT table;
		table = (LPINT)malloc( sizeof(INT) * count );
		if(table) {
			count = m_list.GetSelItems(count, table);
			minx = table[0];
			for(i=count-1; i>=0; i--) {
				if(table[i] < minx) minx = table[i];
				m_list.DeleteString( table[i] );
			}
			if(m_list.GetCount() > 0) {
				if(minx >= m_list.GetCount()) minx = m_list.GetCount()-1;
				m_list.SetCurSel(minx);
				m_list.SelItemRange( FALSE, 0, m_list.GetCount()-1 );
				m_list.SetSel(minx, TRUE);
			}
			free(table);
		}
	}
	OnSelchangeFiletypeList();
	enable_items();
}

void CFileTypeMapping::OnFiletypeEnabled() 
{
	UpdateData(TRUE);
}

void parse( 
	LPCSTR buf, 
	CString & dos,
	CString & type,	
	CString & creator,
	BOOL & strip, 
	BOOL & fat2hfs, 
	BOOL & hfs2fat,
	CString & comment,
	char separator
)
{
	char *p, *pbase;

	dos = "";
	type = "";
	creator = "";
	strip = FALSE;
	fat2hfs = FALSE;
	hfs2fat = FALSE;
	comment = "";

	pbase = (char *)buf;

	p = strchr(pbase,separator);
	if(p) {
		*p++ = 0;
		dos = CString(pbase);
		pbase = p;
		p = strchr(pbase,separator);
		if(p) {
			*p++ = 0;
			if(*pbase == 'x') strip = TRUE;
			pbase = p;
			p = strchr(pbase+4,separator); // type MUST be at least 4 chars ... and may have a comma!
			if(p) {
				*p++ = 0;
				type = CString(pbase);
				pbase = p;
				p = strchr(pbase+4,separator); // creator MUST be at least 4 chars
				if(p) {
					*p++ = 0;
					creator = CString(pbase);
					pbase = p;
					p = strchr(pbase,separator);
					if(p) {
						*p++ = 0;
						if(*pbase == 'x') hfs2fat = TRUE;
						pbase = p;
						p = strchr(pbase,separator);
						if(p) {
							*p++ = 0;
							if(*pbase == 'x') fat2hfs = TRUE;
							// last one is different
							if(*p == '\"') p++;
							comment = CString(p);
							int len = comment.GetLength();
							if(len > 0 && comment.Mid(len-1,1) == "\"") {
								comment = comment.Left(len-1);
							}
						}
					}
				}
			}
		}
	}
}

void CFileTypeMapping::OnSelchangeFiletypeList() 
{
	int sel_count = m_list.GetSelCount();
	int index = m_list.GetCurSel();

	m_dos = "";
	m_type = "";
	m_creator = "";
	m_strip = FALSE;
	m_fat2hfs = FALSE;
	m_hfs2fat = FALSE;
	m_comment = "";

	if(sel_count == 1 && index >= 0) {
		CString cs;
		m_list.GetText(index,cs);
		parse( (LPCSTR)cs, m_dos,	m_type,	m_creator,	m_strip, m_fat2hfs, m_hfs2fat, m_comment, '\t' );
	}
	UpdateData(FALSE);
	enable_items();
}

void CFileTypeMapping::OnFiletypeNew() 
{
	UpdateData(TRUE);
	set_item(
		m_list.GetCount(),
		"???", "????", "????",
		FALSE,
		TRUE, TRUE,
		"Your description here",
		TRUE
	);
	OnSelchangeFiletypeList();
	enable_items();
	// GetDlgItem(IDC_FILETYPE_DOS)->SetFocus();
	// GetDlgItem(IDC_FILETYPE_DOS)->SendMessage(EM_SETSEL,0,-1);
	GetDlgItem(IDC_FILETYPE_COMMENT)->SetFocus();
	GetDlgItem(IDC_FILETYPE_COMMENT)->SendMessage(EM_SETSEL,0,-1);
}

void CFileTypeMapping::OnFiletypeDuplicate() 
{
	UpdateData(TRUE);
	set_item(
		m_list.GetCurSel()+1,
		m_dos, m_type, m_creator,
		m_strip,
		m_fat2hfs, m_hfs2fat,
		m_comment,
		TRUE
	);
	OnSelchangeFiletypeList();
	enable_items();
	// GetDlgItem(IDC_FILETYPE_DOS)->SetFocus();
	// GetDlgItem(IDC_FILETYPE_DOS)->SendMessage(EM_SETSEL,0,-1);
	GetDlgItem(IDC_FILETYPE_COMMENT)->SetFocus();
	GetDlgItem(IDC_FILETYPE_COMMENT)->SendMessage(EM_SETSEL,0,-1);
}

void CFileTypeMapping::OnFiletypeSelectAll() 
{
	int count = m_list.GetCount();
	if(count == 1) { //KLUDGE
		m_list.SetCurSel(0);
		m_list.SelItemRange( FALSE, 0, 0 );
		m_list.SetSel(0, TRUE);
	} else if(count) {
		m_list.SelItemRange( TRUE, 0, count-1 );
	}
	OnSelchangeFiletypeList();
	enable_items();
}

void CFileTypeMapping::OnFiletypeSelectAll2() 
{
	int count = m_list.GetCount();
	if(count) {
		m_list.SelItemRange( FALSE, 0, count-1 );
	}
	OnSelchangeFiletypeList();
	enable_items();
}

void CFileTypeMapping::show_conflict( BOOL conflict ) 
{
	CString cs;
	if(conflict) cs = "C O N F L I C T ..."; else cs = "";
	GetDlgItem(IDC_FILETYPE_CONFLICT)->SetWindowText( cs );
}

void CFileTypeMapping::enable_items() 
{
	int count = m_list.GetCount();
	int sel_count = m_list.GetSelCount();

	GetDlgItem(IDC_FILETYPE_EXPORT)->EnableWindow(count > 0);
	GetDlgItem(IDC_FILETYPE_EXPORT_SELECTED)->EnableWindow(sel_count > 0);
	GetDlgItem(IDC_FILETYPE_DOS)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_STRIP)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_TYPE)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_CREATOR)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_HFS2FAT)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_FAT2HFS)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_DUPLICATE)->EnableWindow(sel_count == 1);
	GetDlgItem(IDC_FILETYPE_SELECT_ALL)->EnableWindow(count > 0);
	GetDlgItem(IDC_FILETYPE_SELECT_ALL2)->EnableWindow(sel_count > 0);
	GetDlgItem(IDC_FILETYPE_DELETE)->EnableWindow(sel_count > 0);
	GetDlgItem(IDC_FILETYPE_COMMENT)->EnableWindow(sel_count == 1);

	// Conflict?
	BOOL conflict = FALSE;
	if(sel_count == 1) {
		UpdateData(TRUE);
		int i = m_list.GetCurSel();
		if(i >= 0) {
			// ignore self when finding
			i = find_item( m_dos, m_type, m_creator, m_fat2hfs, m_hfs2fat, i );
			if(i >= 0) conflict = TRUE;
		}
	}
	show_conflict( conflict );
}

void CFileTypeMapping::list2control() 
{
}

int CFileTypeMapping::set_item(
	int index,
	LPCSTR dos, 
	LPCSTR type, 
	LPCSTR creator,
	BOOL strip,
	BOOL fat2hfs, 
	BOOL hfs2fat,
	LPCSTR comment,
	BOOL update_selection
) 
{
	CString cs;
	int i;

	cs.Format( 
		"%s\t%s\t%-4s\t%-4s\t%s\t%s\t%s",
		dos, 
		strip ? "x" : " ",
		type, creator,
		hfs2fat ? "x" : " ",
		fat2hfs ? "x" : " ",
		comment
	);
	i = m_list.InsertString( index, (LPCTSTR)cs );
	if(update_selection) {
		m_list.SelItemRange( FALSE, 0, i-1 );
		m_list.SetSel(i, TRUE);
		m_list.SelItemRange( FALSE, i+1, m_list.GetCount()-1 );
	}
	return(i);
}

void CFileTypeMapping::control2list() 
{
	int i = m_list.GetCurSel();

	UpdateData(TRUE);
	if(i >= 0) {
		m_list.DeleteString(i);

		while( m_dos.GetLength() > 0 && m_dos[0] == '.' ) {
			m_dos = m_dos.Mid(1);
		}

		set_item(
			i,
			m_dos, m_type, m_creator,
			m_strip,
			m_fat2hfs, m_hfs2fat,
			m_comment,
			TRUE
		);
	}
	enable_items();
}

void CFileTypeMapping::OnFiletypeStrip() 
{
	control2list();
}

void CFileTypeMapping::OnFiletypeFat2hfs() 
{
	control2list();
}

void CFileTypeMapping::OnFiletypeHfs2fat() 
{
	control2list();
}

void CFileTypeMapping::OnUpdateFiletypeDos() 
{
	control2list();
	if(m_dos.GetLength() == 3) {
		GetDlgItem(IDC_FILETYPE_TYPE)->SetFocus();
		GetDlgItem(IDC_FILETYPE_TYPE)->SendMessage(EM_SETSEL,0,-1);
	}
}

void CFileTypeMapping::OnUpdateFiletypeType() 
{
	control2list();
	if(m_type.GetLength() == 4) {
		GetDlgItem(IDC_FILETYPE_CREATOR)->SetFocus();
		GetDlgItem(IDC_FILETYPE_CREATOR)->SendMessage(EM_SETSEL,0,-1);
	}
}

void CFileTypeMapping::OnUpdateFiletypeCreator() 
{
	control2list();
	if(m_creator.GetLength() == 4 && m_type.GetLength() < 4) {
		GetDlgItem(IDC_FILETYPE_TYPE)->SetFocus();
		GetDlgItem(IDC_FILETYPE_TYPE)->SendMessage(EM_SETSEL,0,-1);
	}
}

void CFileTypeMapping::OnUpdateFiletypeComment() 
{
	control2list();
}

CString make_default_fname( void ) 
{
	char path[_MAX_PATH];
	GetWindowsDirectory( path, sizeof(path) );
	strcat( path, "\\HFVExplorer.ftm" );
	return( CString(path) );
}

CString CFileTypeMapping::ask_save_fname( void ) 
{
	CString path, s;
	CFileDialog dlg( FALSE, _T("FTM"), s,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("File type mappings (*.ftm)|*.ftm|All Files|*.*||") );
	if (dlg.DoModal() == IDOK) {
		path = dlg.GetPathName();
	}
	return( path );
}

CString CFileTypeMapping::ask_open_fname( void ) 
{
	CString path, s;
	CFileDialog dlg( TRUE, _T("FTM"), s,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("File type mappings (*.ftm)|*.ftm|All Files|*.*||") );
	if (dlg.DoModal() == IDOK) {
		path = dlg.GetPathName();
	}
	return( path );
}

void CFileTypeMapping::OnFiletypeImport() 
{
	import( ask_open_fname() );
}

void CFileTypeMapping::OnFiletypeExport() 
{
	export( ask_save_fname(), FALSE );
}

void CFileTypeMapping::OnFiletypeExportSelected() 
{
	export( ask_save_fname(), TRUE );
}

BOOL CFileTypeMapping::OnInitDialog() 
{
  WORD DlgWidthUnits;
	INT stops[10];
	BOOL retval = TRUE;

  DlgWidthUnits = LOWORD(GetDialogBaseUnits()) / 4;

	m_enabled = m_enabled_param;
	CDialog::OnInitDialog();
	stops[0] = 0;
	stops[1] = DlgWidthUnits * 20;
	stops[2] = DlgWidthUnits * 31;
	stops[3] = DlgWidthUnits * 49;
	stops[4] = DlgWidthUnits * 71;
	stops[5] = DlgWidthUnits * 86;
	stops[6] = DlgWidthUnits * 200; // invisible
	m_list.SetTabStops( 7, stops );
	import( make_default_fname() );

	int j, i, count = m_input_types.GetSize(), first_index = m_list.GetCount();
	if(count) {
		for(i=0; i<count; i++) {
			BOOL got_you_already = FALSE;
			for(j=0; j<i; j++) {
				if( m_input_types.GetAt(i) == m_input_types.GetAt(j) &&
						m_input_creators.GetAt(i) == m_input_creators.GetAt(j) &&
						m_input_dos.GetAt(i) == m_input_dos.GetAt(j) )
				{
					got_you_already = TRUE;
					break;
				}
			}
			if(!got_you_already) {
				set_item(
					m_list.GetCount(),
					m_input_dos.GetAt(i), m_input_types.GetAt(i), m_input_creators.GetAt(i),
					FALSE,
					TRUE, TRUE,
					"Your description here",
					TRUE
				);
			}
		}
		m_list.SetCurSel(first_index);
		m_list.SelItemRange( FALSE, 0, m_list.GetCount()-1 );
		m_list.SetSel(first_index, TRUE);
		OnSelchangeFiletypeList();
		enable_items();
		GetDlgItem(IDC_FILETYPE_COMMENT)->SetFocus();
		GetDlgItem(IDC_FILETYPE_COMMENT)->SendMessage(EM_SETSEL,0,-1);
		retval = FALSE;
	}
	enable_items();
	return(retval);
}

void CFileTypeMapping::OnOK() 
{
	export( make_default_fname(), FALSE );
	CDialog::OnOK();
	m_enabled_param = m_enabled;
}

int CFileTypeMapping::find_item(
	CString & x_dos,
	CString & x_type,	
	CString & x_creator,
	BOOL x_fat2hfs, 
	BOOL x_hfs2fat,
	int ignore_index
)
{
	int i, count;
	CString cs;

	BOOL	strip;
	BOOL	fat2hfs;
	BOOL	hfs2fat;
	CString	dos;
	CString	creator;
	CString	type;
	CString	comment;
	
	count = m_list.GetCount();
	for(i=0; i<count; i++) {
		if(i != ignore_index) {
			m_list.GetText(i,cs);
			parse( (LPCSTR)cs, dos,	type,	creator,	strip, fat2hfs, hfs2fat, comment, '\t' );
			if(x_dos.CompareNoCase(dos) == 0) {
				if(fat2hfs && x_fat2hfs) {
					return(i);
				}
			}
			if(x_type.CompareNoCase(type) == 0 && x_creator.CompareNoCase(creator) == 0) {
				if(hfs2fat && x_hfs2fat) {
					return(i);
				}
			}
		}
	}
	return(-1);
}

static void myquote( unsigned char *buf, BOOL encode )
{
	int i, len = strlen((char *)buf), tmplen;
	unsigned char tmp[100];

	if(encode) {
		for( i=len-1; i>=0; i-- ) {
			if(buf[i] < 32 || buf[i] > 127 || buf[i] == '\\') {
				wsprintf( (char *)tmp, "\\%03d", (int)buf[i] );
				tmplen = strlen((char *)tmp); // always 4...
				memmove( &buf[i+tmplen], &buf[i+1], strlen((char *)&buf[i+1])+1 );
				memmove( (char *)&buf[i], (char *)tmp, tmplen );
			}
		}
	} else {
		for( i=len-1; i>=0; i-- ) {
			// "\006..."
			if(buf[i] == '\\' && strlen((char *)&buf[i]) >= 4) {
				buf[i] = (unsigned char)atoi((char *)&buf[i+1]);
				memmove( &buf[i+1], &buf[i+4], strlen((char *)&buf[i+4])+1 );
			}
		}
	}
}

void CFileTypeMapping::import( CString & path ) 
{
	CFile f;
	char *buf;
	DWORD sz;
	CString line;

	BOOL	strip;
	BOOL	fat2hfs;
	BOOL	hfs2fat;
	CString	dos;
	CString	creator;
	CString	type;
	CString	comment;

	if(path == "") return;

	if(f.Open( (LPCTSTR)path, CFile::modeRead )) {
		sz = f.GetLength();
		buf = (char *)malloc( sz+1000 );
		if(buf) {
			if(f.Read( buf, sz ) == sz) {
				unsigned char *p = (unsigned char *)buf;
				buf[sz] = 0; // make it c-string
				while(*p) {
					while(*p && *p < ' ') p++;
					line = "";
					while(*p && *p >= ' ') {
						line += *p;
						p++;
					}

					char *linebuf = (char *)malloc( 1000 );
					strcpy( linebuf, line );
					myquote((unsigned char *)linebuf,FALSE);
					parse( (LPCSTR)linebuf, dos,	type,	creator, strip, fat2hfs, hfs2fat, comment, ',' );
					free(linebuf);
					
					if(dos != "" && type.GetLength() == 4 && creator.GetLength() == 4) {
						int i = find_item( dos, type, creator, fat2hfs, hfs2fat, -1 );
						if(i < 0) { // New item.
							i = m_list.GetCount();
						} else { // Replace item.
							m_list.DeleteString(i);
						}
						set_item(
							i,
							dos, type, creator,
							strip,
							fat2hfs, hfs2fat,
							comment,
							FALSE
						);
					}
				}
			} else {
				AfxMessageBox("Failed to read from file " + path + ".");
			}
			free(buf);
		}
		f.Close();
	} else {
		// Hmm. The file is initially missing. This should be written differently.
		// AfxMessageBox("Cannot open file " + path + ".");
	}
}

void CFileTypeMapping::export( CString & path, BOOL selected_items ) 
{
	CFile f;
	CString line;
	DWORD sz;
	int i, count;

	BOOL	strip;
	BOOL	fat2hfs;
	BOOL	hfs2fat;
	CString	dos;
	CString	creator;
	CString	type;
	CString	comment;
	unsigned char typestr[100];
	unsigned char creatorstr[100];
	unsigned char commentstr[400];

	if(path == "") return;

	if(f.Open( (LPCTSTR)path, CFile::modeCreate | CFile::modeWrite )) {
		count = m_list.GetCount();
		for(i=0; i<count; i++) {
			if(selected_items && m_list.GetSel(i) == 0) continue;
			m_list.GetText(i,line);
			// parse and rebuild to validate critical fields.
			parse( (LPCSTR)line, dos,	type,	creator, strip, fat2hfs, hfs2fat, comment, '\t' );
			strcpy( (char *)typestr, type );
			myquote( typestr, TRUE );
			strcpy( (char *)creatorstr, creator );
			myquote( creatorstr, TRUE );
			strcpy( (char *)commentstr, comment );
			myquote( commentstr, TRUE );
			
			line.Format( 
				"%s,%s,%s,%s,%s,%s,\"%s\"\r\n",
				dos, 
				strip ? "x" : "",
				typestr, creatorstr,
				hfs2fat ? "x" : "",
				fat2hfs ? "x" : "",
				commentstr
			);
			try {
				sz = line.GetLength();
				f.Write(line,sz);
			} catch(...) {
				AfxMessageBox("Write error. Disk may be full.");
				break;
			}
		}
		f.Close();
	} else {
		AfxMessageBox("Cannot create file " + path + ".");
	}
}

void import_map( CString & path ) 
{
	CFile f;
	char *buf;
	DWORD sz;
	CString line;

	BOOL	strip;
	BOOL	fat2hfs;
	BOOL	hfs2fat;
	CString	dos;
	CString	creator;
	CString	type;
	CString	comment;

	if(path == "") return;

	if(f.Open( (LPCTSTR)path, CFile::modeRead )) {
		sz = f.GetLength();
		buf = (char *)malloc( sz+1000 );
		if(buf) {
			if(f.Read( buf, sz ) == sz) {
				unsigned char *p = (unsigned char *)buf;
				buf[sz] = 0; // make it c-string
				while(*p) {
					while(*p && *p < ' ') p++;
					line = "";
					while(*p && *p >= ' ') {
						line += *p;
						p++;
					}

					char *linebuf = (char *)malloc( 1000 );
					strcpy( linebuf, line );
					myquote((unsigned char *)linebuf,FALSE);
					parse( (LPCSTR)linebuf, dos,	type,	creator, strip, fat2hfs, hfs2fat, comment, ',' );
					free(linebuf);

					if(dos != "" && type != "" && creator != "") {
						CString type_creator = type + creator;
						if(type_creator.GetLength() == 8) { // sanity check
							if(hfs2fat) {
								map_hfs2fat.SetAt( type_creator, dos );
							}
							if(fat2hfs) {
								if(strip) type_creator += 'x'; else type_creator += ' ';
								map_fat2hfs.SetAt( dos, type_creator );
							}
						}
					}
				}
			} else {
				AfxMessageBox("Failed to read from file " + path + ".");
			}
			free(buf);
		}
		f.Close();
	} else {
		// The file may be missing.
		// AfxMessageBox("Cannot open file " + path + ".");
	}
}

extern "C" {

int tmap_init( void )
{
	tmap_final();
	import_map( make_default_fname() );
	return(1);
}

int tmap_final( void )
{
	map_hfs2fat.RemoveAll();
	map_fat2hfs.RemoveAll();
	return(1);
}

int tmap_hfs2fat( const char *type, const char *creator, char *extension )
{
  int retval = 0;
	CString key;
	CString rValue;

	key = CString(type) + CString(creator);
	if(map_hfs2fat.Lookup(key, rValue)) {
		strcpy( extension, rValue );
		retval = 1;
	}
  return(retval);
}

int tmap_fat2hfs( char *extension, char *type, char *creator, int *strip )
{
  int retval = 0;
	CString rValue;
	char *buf, ext_upper[100];

	strcpy( ext_upper, extension );
	strupr( ext_upper );
	

	if(map_fat2hfs.Lookup(ext_upper, rValue)) {
		buf = rValue.GetBuffer(100);
		strncpy( type, buf, 4 );
		type[4] = 0;
		strncpy( creator, &buf[4], 4 );
		creator[4] = 0;
		*strip = buf[8] == 'x';
		retval = 1;
		rValue.ReleaseBuffer();
	}
	return(retval);
}

} // extern "C"

void CFileTypeMapping::OnFiletypeHelp() 
{
	AfxMessageBox(
		"These settings apply only when the copy mode is \"raw\" data or resource.\r\n"
		"\r\n"
		"If \"Mapping enabled\" is not selected, no mappings are performed (except those made by other copy modes).\r\n"
		"\r\n"
		"\"Export All/Selected\": Use these to share your definitions with other people. Saves FTM files.\r\n"
		"\r\n"
		"\"Import\": combines FTM files with the current list. If there is a conflict, old definition is overwritten -- no questions asked.\r\n"
		"\r\n"
		"\"Strip\": removes (strips) the DOS extension when a file is copied to HFS volume. Potential pitfall: take care not to copy two or more files which would degenerate to the same name ... only one of them would remain. Sorry about that. Will be fixed some time in future.\r\n"
		"\r\n"
		"Note that you can have multiple mappings for one dos extension in direction HFS->DOS, but only one in direction DOS->HFS. "
		"Similarly, many dos extensions may map to the same type/creator pair, but in the direction HFS->DOS only one type/creator pair is allowed.\r\n"
		"\r\n"
		"If the currently selected item conflicts with some other item, you will see text CONFLICT at the top of the dialog. "
		"Typically you'll see this after every DUPLICATE command until you change the new line.\r\n"
		"Conflicts are removed when you restart the program or open the dialog again.\r\n"
		"\r\n"
		"Your default mappings are saved in \"\\Windows\\HFVExplorer.ftm\". I suggest that you back up this file when you have done most of your definitions.\r\n"
		"\r\n"
		"Tip: select some items in HFVExplorer list view, right-click one of them and give command \"Map selected items\" (or hit F4).\r\n"
		"",
		MB_OK | MB_ICONINFORMATION
	);
}
