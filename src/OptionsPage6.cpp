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
#include "OptionsPage6.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage6 property page

IMPLEMENT_DYNCREATE(COptionsPage6, CPropertyPage)

COptionsPage6::COptionsPage6() : CPropertyPage(COptionsPage6::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage6)
	m_executor_win32_path = _T("");
	//}}AFX_DATA_INIT
}

COptionsPage6::~COptionsPage6()
{
}

void COptionsPage6::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage6)
	DDX_Text(pDX, IDC_COPT6_EXECUTOR_WIN32_PATH, m_executor_win32_path);
	DDV_MaxChars(pDX, m_executor_win32_path, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage6, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage6)
	ON_BN_CLICKED(IDC_COPT6_BROWSE_EXECUTOR_WIN32_PATH, OnCopt6BrowseExecutorWin32Path)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage6 message handlers

void COptionsPage6::OnCopt6BrowseExecutorWin32Path() 
{
	CString s, rest;
	int i;

	GetDlgItemText( IDC_COPT6_EXECUTOR_WIN32_PATH, m_executor_win32_path );
	i = m_executor_win32_path.Find(' ');
	s = m_executor_win32_path;
	s.TrimLeft();
	s.TrimRight();
	if( i > 0 ) {
		rest = s.Right( s.GetLength() - i );
		s = s.Left( i );
	} else {
		rest = "";
	}
	CFileDialog dlg( TRUE, _T("EXE"), s,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Program Files|*.exe|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_executor_win32_path = dlg.GetPathName() + rest;
		SetDlgItemText( IDC_COPT6_EXECUTOR_WIN32_PATH, m_executor_win32_path );
	}
}
