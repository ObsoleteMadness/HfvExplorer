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
#include "OptionsSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

IMPLEMENT_DYNAMIC(COptionsSheet, CPropertySheet)

COptionsSheet::COptionsSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	m_last_active_page = iSelectPage;
	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page10);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
	AddPage(&m_Page8);
	AddPage(&m_Page9);
}

COptionsSheet::COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	m_last_active_page = iSelectPage;
	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	AddPage(&m_Page10);
	AddPage(&m_Page5);
	AddPage(&m_Page6);
	AddPage(&m_Page7);
	AddPage(&m_Page8);
	AddPage(&m_Page9);
}

COptionsSheet::~COptionsSheet()
{
}

BEGIN_MESSAGE_MAP(COptionsSheet, CPropertySheet)
	//{{AFX_MSG_MAP(COptionsSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet message handlers

BOOL COptionsSheet::DestroyWindow() 
{
	m_last_active_page = GetActiveIndex();
	return CPropertySheet::DestroyWindow();
}

BOOL COptionsSheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	CenterWindow();
	return bResult;
}
