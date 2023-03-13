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

#if !defined(AFX_FILETYPEMAPPING_H__7FB1E441_68A1_11D2_A7AB_00201881A006__INCLUDED_)
#define AFX_FILETYPEMAPPING_H__7FB1E441_68A1_11D2_A7AB_00201881A006__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FileTypeMapping.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileTypeMapping dialog

class CFileTypeMapping : public CDialog
{
// Construction
public:
	CFileTypeMapping(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFileTypeMapping)
	enum { IDD = IDD_FILETYPE_MAPPINGS };
	CListBox	m_list;
	BOOL	m_enabled;
	BOOL	m_strip;
	BOOL	m_fat2hfs;
	BOOL	m_hfs2fat;
	CString	m_dos;
	CString	m_creator;
	CString	m_type;
	CString	m_comment;
	//}}AFX_DATA


	BOOL	m_enabled_param;
	CStringArray m_input_types;
	CStringArray m_input_creators;
	CStringArray m_input_dos;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTypeMapping)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void list2control();
	void control2list();
	void enable_items();
	int set_item(
		int index,
		LPCSTR dos, 
		LPCSTR type, 
		LPCSTR creator,
		BOOL strip,
		BOOL fat2hfs, 
		BOOL hfs2fat,
		LPCSTR comment,
		BOOL update_selection
	);
	/*
	void parse( 
		LPCSTR buf, 
		CString & dos,
		CString & type,	
		CString & creator,
		BOOL & strip, 
		BOOL & fat2hfs, 
		BOOL & hfs2fat,
		char separator
	);
	*/
	void import( CString & path );
	void export( CString & path, BOOL selected_items );
	// CString make_default_fname( void );
	CString ask_save_fname( void );
	CString ask_open_fname( void );
	int find_item(
		CString & x_dos,
		CString & x_type,	
		CString & x_creator,
		BOOL x_fat2hfs, 
		BOOL x_hfs2fat,
		int ignore_index
	);
	void show_conflict( BOOL conflict );

	// Generated message map functions
	//{{AFX_MSG(CFileTypeMapping)
	afx_msg void OnFiletypeExport();
	afx_msg void OnFiletypeExportSelected();
	afx_msg void OnFiletypeDelete();
	afx_msg void OnFiletypeEnabled();
	afx_msg void OnFiletypeImport();
	afx_msg void OnSelchangeFiletypeList();
	afx_msg void OnFiletypeNew();
	afx_msg void OnFiletypeSelectAll();
	afx_msg void OnFiletypeSelectAll2();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnFiletypeStrip();
	afx_msg void OnFiletypeFat2hfs();
	afx_msg void OnFiletypeHfs2fat();
	afx_msg void OnUpdateFiletypeDos();
	afx_msg void OnUpdateFiletypeType();
	afx_msg void OnUpdateFiletypeCreator();
	afx_msg void OnFiletypeDuplicate();
	afx_msg void OnFiletypeHelp();
	afx_msg void OnUpdateFiletypeComment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILETYPEMAPPING_H__7FB1E441_68A1_11D2_A7AB_00201881A006__INCLUDED_)
