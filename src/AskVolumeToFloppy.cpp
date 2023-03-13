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
#include "AskVolumeToFloppy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskVolumeToFloppy dialog


CAskVolumeToFloppy::CAskVolumeToFloppy(CWnd* pParent /*=NULL*/)
	: CDialog(CAskVolumeToFloppy::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskVolumeToFloppy)
	m_drive = -1;
	m_volname = _T("");
	//}}AFX_DATA_INIT
}


void CAskVolumeToFloppy::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskVolumeToFloppy)
	DDX_CBIndex(pDX, IDC_DUMP_TO_FLOPPY, m_drive);
	DDX_Text(pDX, IDC_VOLPATH, m_volname);
	DDV_MaxChars(pDX, m_volname, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskVolumeToFloppy, CDialog)
	//{{AFX_MSG_MAP(CAskVolumeToFloppy)
	ON_BN_CLICKED(IDC_VOLNAME_BROWSE, OnVolnameBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskVolumeToFloppy message handlers

void CAskVolumeToFloppy::OnVolnameBrowse() 
{
	GetDlgItemText( IDC_VOLPATH, m_volname );
	CFileDialog dlg( to_floppy, _T("DSK"), m_volname,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("DSK, HFV and HFx Files|*.dsk;*.hf*|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_volname = dlg.GetPathName();
		SetDlgItemText( IDC_VOLPATH, m_volname );
	}
}

BOOL CAskVolumeToFloppy::OnInitDialog() 
{
	CDialog::OnInitDialog();
	SetWindowText( m_caption );
	return TRUE;
}
