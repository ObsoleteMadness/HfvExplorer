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

#include "afx.h"
#include "stdafx.h"
#include "HFVExplorer.h"
#include "AskNewVolume.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskNewVolume dialog


CAskNewVolume::CAskNewVolume(CWnd* pParent /*=NULL*/)
	: CDialog(CAskNewVolume::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskNewVolume)
	m_volname = _T("");
	m_volsize_ex = _T("");
	m_volpath = _T("");
	//}}AFX_DATA_INIT
}


void CAskNewVolume::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskNewVolume)
	DDX_Text(pDX, IDC_VOLNAME, m_volname);
	DDV_MaxChars(pDX, m_volname, 27);
	DDX_CBString(pDX, IDC_VOLSIZE_EX, m_volsize_ex);
	DDX_CBString(pDX, IDC_VOLPATH, m_volpath);
	DDV_MaxChars(pDX, m_volpath, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskNewVolume, CDialog)
	//{{AFX_MSG_MAP(CAskNewVolume)
	ON_BN_CLICKED(IDC_VOLNAME_BROWSE, OnVolnameBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskNewVolume message handlers

void CAskNewVolume::OnVolnameBrowse() 
{
	GetDlgItemText( IDC_VOLPATH, m_volpath );
	CFileDialog dlg( FALSE, _T("DSK"), m_volpath,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("DSK, HFV and HFx Files|*.dsk;*.hf*|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_volpath = dlg.GetPathName();
		SetDlgItemText( IDC_VOLPATH, m_volpath );
	}
}

BOOL CAskNewVolume::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if(m_volpath != "") {
		HWND w = GetSafeHwnd();
		if(w) {
			::EnableWindow( ::GetDlgItem(w,IDC_VOLPATH), FALSE );
			::EnableWindow( ::GetDlgItem(w,IDC_VOLNAME_BROWSE), FALSE );
		}
	}
	return TRUE;
}

void CAskNewVolume::OnOK() 
{
	/*
	UpdateData(TRUE);
	if(m_volname == "") {
		AfxMessageBox( "You must give a name for the volume." );
	} if(m_volpath == "") {
		AfxMessageBox( "Please define the volume path." );
	} else if(atol(m_volsize_ex.GetBuffer(100)) == 0) {
		AfxMessageBox( "Volume size cannot be zero." );
	} else {
		CDialog::OnOK();
	}
	*/
	CDialog::OnOK();
}

CString CAskNewVolume::get_floppy_a_name(void) 
{
	return( "<Format floppy A:>" );
}

CString CAskNewVolume::get_floppy_b_name(void) 
{
	return( "<Format floppy B:>" );
}

BOOL CAskNewVolume::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	char *p = m_volpath.GetBuffer(_MAX_PATH);

	if(wParam == IDOK) {
		UpdateData(TRUE);
		if(m_volname == "") {
			AfxMessageBox( "You must give a name for the volume." );
			return(0);
		} if(m_volpath == "") {
			AfxMessageBox( "Please define the volume path." );
			return(0);
		} else if(atol(m_volsize_ex.GetBuffer(100)) == 0) {
			AfxMessageBox( "Volume size cannot be zero." );
			return(0);
		} else if( strlen(p) < 5 || !isalpha(p[0]) ||
					p[1] != ':' || p[2] != '\\')
		{
			if(m_volpath != get_floppy_a_name() &&
				 m_volpath != get_floppy_b_name()) 
			{
				AfxMessageBox( "Please define *full* volume path." );
				return(0);
			}
		}
	}
	
	return CDialog::OnCommand(wParam, lParam);
}
