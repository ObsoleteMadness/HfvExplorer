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
#include "DynCopyModeSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynCopyModeSelect dialog


CDynCopyModeSelect::CDynCopyModeSelect(CWnd* pParent /*=NULL*/)
	: CDialog(CDynCopyModeSelect::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDynCopyModeSelect)
	m_appledouble = FALSE;
	m_copy_mode = -1;
	m_copying_fname = _T("");
	m_said_ok_all = 0;
	//}}AFX_DATA_INIT
}


void CDynCopyModeSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDynCopyModeSelect)
	DDX_Check(pDX, IDC_COPYIN_APPLEDOUBLE, m_appledouble);
	DDX_CBIndex(pDX, IDC_COPY_MODE, m_copy_mode);
	DDX_Text(pDX, IDC_COPYING_FNAME, m_copying_fname);
	DDV_MaxChars(pDX, m_copying_fname, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDynCopyModeSelect, CDialog)
	//{{AFX_MSG_MAP(CDynCopyModeSelect)
	ON_BN_CLICKED(IDOKALL, OnOkall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynCopyModeSelect message handlers

void CDynCopyModeSelect::OnOK() 
{
	CDialog::OnOK();
}

void CDynCopyModeSelect::OnOkall() 
{
	m_said_ok_all = 1;
	OnOK();
}

BOOL CDynCopyModeSelect::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_COPYIN_APPLEDOUBLE)->EnableWindow( !m_outcopy );

	return TRUE;
}
