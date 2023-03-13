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
#include "OptionsPage4.h"
#include "shell.h"
#include "vmacpatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage4 property page

IMPLEMENT_DYNCREATE(COptionsPage4, CPropertyPage)

COptionsPage4::COptionsPage4() : CPropertyPage(COptionsPage4::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage4)
	m_default_emulator = -1;
	m_link_name_style = -1;
	m_launch_method = -1;
	//}}AFX_DATA_INIT
}

COptionsPage4::~COptionsPage4()
{
}

void COptionsPage4::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage4)
	DDX_CBIndex(pDX, IDC_COPT4_DEFAULT_EMULATOR, m_default_emulator);
	DDX_CBIndex(pDX, IDC_COPT4_LINK_NAME_STYLE, m_link_name_style);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage4, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage4 message handlers
