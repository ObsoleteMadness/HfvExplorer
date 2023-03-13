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
#include "OptionsPage8.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage8 property page

IMPLEMENT_DYNCREATE(COptionsPage8, CPropertyPage)

COptionsPage8::COptionsPage8() : CPropertyPage(COptionsPage8::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage8)
	m_shapeshifter_path = _T("");
	m_shapeshifter_startup_folder_name = _T("");
	m_shapeshifter_system_file_path = _T("");
	m_shapeshifter_preferences_file_path = _T("");
	m_shapeshifter_amiga_volume_name = _T("");
	m_shapeshifter_use_remover = FALSE;
	//}}AFX_DATA_INIT
}

COptionsPage8::~COptionsPage8()
{
}

void COptionsPage8::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage8)
	DDX_Text(pDX, IDC_COPT8_SHAPESHIFTER_PATH, m_shapeshifter_path);
	DDV_MaxChars(pDX, m_shapeshifter_path, 255);
	DDX_Text(pDX, IDC_COPT8_SHAPESHIFTER_STARTUP_FOLDER_NAME, m_shapeshifter_startup_folder_name);
	DDV_MaxChars(pDX, m_shapeshifter_startup_folder_name, 255);
	DDX_Text(pDX, IDC_COPT8_SHAPESHIFTER_SYSTEM_FILE_PATH, m_shapeshifter_system_file_path);
	DDV_MaxChars(pDX, m_shapeshifter_system_file_path, 255);
	DDX_Text(pDX, IDC_COPT8_SHAPESHIFTER_PREFERENCES_FILE_PATH, m_shapeshifter_preferences_file_path);
	DDV_MaxChars(pDX, m_shapeshifter_preferences_file_path, 255);
	DDX_Text(pDX, IDC_COPT8_SHAPESHIFTER_AMIGA_VOLUME_NAME, m_shapeshifter_amiga_volume_name);
	DDV_MaxChars(pDX, m_shapeshifter_amiga_volume_name, 32);
	DDX_Check(pDX, IDC_COPT8_SHAPESHIFTER_USE_REMOVER, m_shapeshifter_use_remover);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage8, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage8)
	ON_BN_CLICKED(IDC_COPT8_BROWSE_SHAPESHIFTER_PATH, OnCopt8BrowseShapeshifterPath)
	ON_BN_CLICKED(IDC_COPT8_BROWSE_SHAPESHIFTER_PREFERENCES_FILE_PATH, OnCopt8BrowseShapeshifterPreferencesFilePath)
	ON_BN_CLICKED(IDC_COPT8_BROWSE_SHAPESHIFTER_STARTUP_FOLDER_NAME, OnCopt8BrowseShapeshifterStartupFolderName)
	ON_BN_CLICKED(IDC_COPT8_BROWSE_SHAPESHIFTER_SYSTEM_FILE_PATH, OnCopt8BrowseShapeshifterSystemFilePath)
	ON_BN_CLICKED(IDC_COPT8_BROWSE_SHAPESHIFTER_AMIGA_VOLUME_NAME, OnCopt8BrowseShapeshifterAmigaVolumeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage8 message handlers

void COptionsPage8::OnCopt8BrowseShapeshifterPath() 
{
	CString s, rest;
	int i;

	GetDlgItemText( IDC_COPT8_SHAPESHIFTER_PATH, m_shapeshifter_path );
	i = m_shapeshifter_path.Find(' ');
	s = m_shapeshifter_path;
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
				_T("Batch Files (*.bat)|*.bat|Program Files (*.exe)|*.exe|WinUAE configuration files (*.uae)|*.uae|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_shapeshifter_path = dlg.GetPathName() + rest;
		SetDlgItemText( IDC_COPT8_SHAPESHIFTER_PATH, m_shapeshifter_path );
	}
}

void COptionsPage8::OnCopt8BrowseShapeshifterPreferencesFilePath() 
{
	GetDlgItemText( IDC_COPT8_SHAPESHIFTER_PREFERENCES_FILE_PATH, m_shapeshifter_preferences_file_path );
	CFileDialog dlg( TRUE, _T(""), m_shapeshifter_preferences_file_path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("ShapeShifter Prefs|ShapeShifter Prefs|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_shapeshifter_preferences_file_path = dlg.GetPathName();
		SetDlgItemText( IDC_COPT8_SHAPESHIFTER_PREFERENCES_FILE_PATH, m_shapeshifter_preferences_file_path );
	}
}

void COptionsPage8::OnCopt8BrowseShapeshifterStartupFolderName() 
{
	// HFS browse not implemented
}

void COptionsPage8::OnCopt8BrowseShapeshifterSystemFilePath() 
{
	GetDlgItemText( IDC_COPT8_SHAPESHIFTER_SYSTEM_FILE_PATH, m_shapeshifter_system_file_path );
	CFileDialog dlg( TRUE, _T("HFV"), m_shapeshifter_system_file_path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("HFS volume files|*.dsk;*.hf*|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_shapeshifter_system_file_path = dlg.GetPathName();
		SetDlgItemText( IDC_COPT8_SHAPESHIFTER_SYSTEM_FILE_PATH, m_shapeshifter_system_file_path );
	}
}

void COptionsPage8::OnCopt8BrowseShapeshifterAmigaVolumeName() 
{
	// Amiga browse not implemented
}
