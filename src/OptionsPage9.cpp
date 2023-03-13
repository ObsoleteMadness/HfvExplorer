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
#include "OptionsPage9.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage9 property page

IMPLEMENT_DYNCREATE(COptionsPage9, CPropertyPage)

COptionsPage9::COptionsPage9() : CPropertyPage(COptionsPage9::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage9)
	m_fusion_startup_folder_name = _T("");
	m_fusion_system_file_path = _T("");
	m_fusion_batch_path = _T("");
	m_fusion_use_remover = FALSE;
	//}}AFX_DATA_INIT
}

COptionsPage9::~COptionsPage9()
{
}

void COptionsPage9::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage9)
	DDX_Text(pDX, IDC_COPT9_FUSION_STARTUP_FOLDER_NAME, m_fusion_startup_folder_name);
	DDV_MaxChars(pDX, m_fusion_startup_folder_name, 255);
	DDX_Text(pDX, IDC_COPT9_FUSION_SYSTEM_FILE_PATH, m_fusion_system_file_path);
	DDV_MaxChars(pDX, m_fusion_system_file_path, 255);
	DDX_Text(pDX, IDC_COPT9_FUSION_BATCH_PATH, m_fusion_batch_path);
	DDV_MaxChars(pDX, m_fusion_batch_path, 255);
	DDX_Check(pDX, IDC_COPT9_FUSION_USE_REMOVER, m_fusion_use_remover);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage9, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage9)
	ON_BN_CLICKED(IDC_COPT9_BROWSE_FUSION_STARTUP_FOLDER_NAME, OnCopt9BrowseFusionStartupFolderName)
	ON_BN_CLICKED(IDC_COPT9_BROWSE_FUSION_SYSTEM_FILE_PATH, OnCopt9BrowseFusionSystemFilePath)
	ON_BN_CLICKED(IDC_COPT9_BROWSE_FUSION_BATCH_PATH, OnCopt9BrowseFusionBatchPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage9 message handlers

void COptionsPage9::OnCopt9BrowseFusionStartupFolderName() 
{
	// not implemented
}

void COptionsPage9::OnCopt9BrowseFusionSystemFilePath() 
{
	GetDlgItemText( IDC_COPT9_FUSION_SYSTEM_FILE_PATH, m_fusion_system_file_path );
	CFileDialog dlg( TRUE, _T("HFV"), m_fusion_system_file_path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("HFS volume files|*.dsk;*.hf*|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_fusion_system_file_path = dlg.GetPathName();
		SetDlgItemText( IDC_COPT9_FUSION_SYSTEM_FILE_PATH, m_fusion_system_file_path );
	}
}

void COptionsPage9::OnCopt9BrowseFusionBatchPath() 
{
	CString s, rest;
	int i;

	GetDlgItemText( IDC_COPT9_FUSION_BATCH_PATH, m_fusion_batch_path );
	i = m_fusion_batch_path.Find(' ');
	s = m_fusion_batch_path;
	s.TrimLeft();
	s.TrimRight();
	if( i > 0 ) {
		rest = s.Right( s.GetLength() - i );
		s = s.Left( i );
	} else {
		rest = "";
	}
	CFileDialog dlg( TRUE, _T("BAT"), s,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Batch Files|*.bat|Program Files|*.exe|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_fusion_batch_path = dlg.GetPathName() + rest;
		SetDlgItemText( IDC_COPT9_FUSION_BATCH_PATH, m_fusion_batch_path );
	}
}
