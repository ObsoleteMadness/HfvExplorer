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
#include "OptionsPage3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage3 property page

IMPLEMENT_DYNCREATE(COptionsPage3, CPropertyPage)

COptionsPage3::COptionsPage3() : CPropertyPage(COptionsPage3::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage3)
	m_show_invisible_mac = FALSE;
	m_show_invisible_fat = FALSE;
	m_protect_locked_mac = FALSE;
	m_protect_locked_fat = FALSE;
	//}}AFX_DATA_INIT
}

COptionsPage3::~COptionsPage3()
{
}

void COptionsPage3::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage3)
	DDX_Check(pDX, IDC_SHOW_INVISIBLE_MAC, m_show_invisible_mac);
	DDX_Check(pDX, IDC_SHOW_INVISIBLE_PC, m_show_invisible_fat);
	DDX_Check(pDX, IDC_PROTECT_LOCKED_MAC, m_protect_locked_mac);
	DDX_Check(pDX, IDC_PROTECT_LOCKED_PC, m_protect_locked_fat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage3, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage3)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage3 message handlers
