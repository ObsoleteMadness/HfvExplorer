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
#include "AskDump.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskDump dialog


CAskDump::CAskDump(CWnd* pParent /*=NULL*/)
	: CDialog(CAskDump::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskDump)
	m_dump_path = _T("");
	m_blocks = _T("");
	m_start = _T("");
	//}}AFX_DATA_INIT
}


void CAskDump::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskDump)
	DDX_Text(pDX, IDC_DUMP_PATH, m_dump_path);
	DDV_MaxChars(pDX, m_dump_path, 255);
	DDX_Text(pDX, IDC_DUMP_BLOCKS, m_blocks);
	DDV_MaxChars(pDX, m_blocks, 30);
	DDX_Text(pDX, IDC_DUMP_START, m_start);
	DDV_MaxChars(pDX, m_start, 30);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskDump, CDialog)
	//{{AFX_MSG_MAP(CAskDump)
	ON_BN_CLICKED(IDC_BROWSE_DUMP, OnBrowseDump)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskDump message handlers

void CAskDump::OnBrowseDump() 
{
	GetDlgItemText( IDC_DUMP_PATH, m_dump_path );
	CFileDialog dlg( FALSE, _T("DMP"), m_dump_path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("DMP Files|*.dmp|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_dump_path = dlg.GetPathName();
		SetDlgItemText( IDC_DUMP_PATH, m_dump_path );
	}
}
