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
#include "OptionsPage2.h"
#include "resource.h"
#include "FileTypeMapping.h"
#include "tmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage2 property page

IMPLEMENT_DYNCREATE(COptionsPage2, CPropertyPage)

COptionsPage2::COptionsPage2() : CPropertyPage(COptionsPage2::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage2)
	m_copyout_mode = -1;
	m_copyin_mode = -1;
	m_copyin_appledouble = FALSE;
	//}}AFX_DATA_INIT
}

COptionsPage2::~COptionsPage2()
{
}

void COptionsPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage2)
	DDX_CBIndex(pDX, IDC_COPYOUT_MODE, m_copyout_mode);
	DDX_CBIndex(pDX, IDC_COPYIN_MODE, m_copyin_mode);
	DDX_Check(pDX, IDC_COPYIN_APPLEDOUBLE, m_copyin_appledouble);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage2, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage2)
	ON_BN_CLICKED(IDC_COPY_MAP, OnCopyMap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage2 message handlers

void COptionsPage2::OnCopyMap() 
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CFileTypeMapping dlg;

	tmap_final();
	dlg.m_enabled_param = pApp->m_type_mapping_enabled;
	if (dlg.DoModal() == IDOK) {
		pApp->m_type_mapping_enabled = dlg.m_enabled_param;
	}
	tmap_init();
}
