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
#include "OptionsPage1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage1 property page

IMPLEMENT_DYNCREATE(COptionsPage1, CPropertyPage)

COptionsPage1::COptionsPage1() : CPropertyPage(COptionsPage1::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage1)
	m_dosicons = -1;
	m_mountall = FALSE;
	m_patch = FALSE;
	m_startup_floppies = FALSE;
	m_startup_cds = FALSE;
	m_fat_icon_time_limit = 0;
	m_psize = _T("");
	m_font = _T("");
	m_is_font_sjis = FALSE; /* noda */
	m_startup_hds = FALSE;
	//}}AFX_DATA_INIT
}

COptionsPage1::~COptionsPage1()
{
}

void COptionsPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage1)
	DDX_CBIndex(pDX, IDC_DOSICONS, m_dosicons);
	DDX_Check(pDX, IDC_OPTIONS_MOUNT_ALL, m_mountall);
	DDX_Check(pDX, IDC_OPTIONS_PATCH, m_patch);
	DDX_Check(pDX, IDC_OPTIONS_STARTUP_FLOPPIES, m_startup_floppies);
	DDX_Check(pDX, IDC_OPTIONS_STARTUP_CDS, m_startup_cds);
	DDX_Text(pDX, IDC_FAT_ICON_TIME_LIMIT, m_fat_icon_time_limit);
	DDV_MinMaxInt(pDX, m_fat_icon_time_limit, 0, 9999999);
	DDX_CBString(pDX, IDC_PSIZE, m_psize);
	DDV_MaxChars(pDX, m_psize, 10);
	DDX_CBString(pDX, IDC_FONT, m_font);
	DDV_MaxChars(pDX, m_font, 100);
	DDX_Check(pDX, IDC_OPTIONS_IS_FONT_SJIS, m_is_font_sjis);
	DDX_Check(pDX, IDC_OPTIONS_STARTUP_HDS, m_startup_hds);
	//}}AFX_DATA_MAP
}
/* noda added IDC_OPTIONS_IS_FONT_SJIS (no comments inside AFX_DATA_MAP) */



BEGIN_MESSAGE_MAP(COptionsPage1, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static int CALLBACK EnumFontFamProc(
	ENUMLOGFONT FAR *lpelf,
	NEWTEXTMETRIC FAR *lpntm,
	int FontType,
	LPARAM lParam
)
{
	HWND hwnd = (HWND)lParam;
	::SendDlgItemMessage( 
		hwnd, 
		IDC_FONT,
		CB_ADDSTRING,
		0,
		(LPARAM)lpelf->elfFullName
	);
	return(1);
}

BOOL COptionsPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	HDC hdc = ::GetDC( ::GetDesktopWindow() );
	(void)EnumFontFamilies(
		hdc, 
		NULL,
		(FONTENUMPROC)EnumFontFamProc,
		(LPARAM)GetSafeHwnd()
	);
	::ReleaseDC( ::GetDesktopWindow(), hdc );
	
	return TRUE;
}

void COptionsPage1::OnOK() 
{
	CPropertyPage::OnOK();
}
