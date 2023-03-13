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

/////////////////////////////////////////////////////////////////////////////
// COptionsPage1 dialog

class COptionsPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPage1)

// Construction
public:
	COptionsPage1();
	~COptionsPage1();

// Dialog Data
	//{{AFX_DATA(COptionsPage1)
	enum { IDD = IDD_OPTIONS_PAGE_1 };
	int		m_dosicons;
	BOOL	m_mountall;
	BOOL	m_patch;
	BOOL	m_startup_floppies;
	BOOL	m_startup_cds;
	int		m_fat_icon_time_limit;
	CString	m_psize;
	CString	m_font;
	BOOL	m_is_font_sjis;
	BOOL	m_startup_hds;
	//}}AFX_DATA

/* noda added m_is_font_sjis (no comments inside AFX_DATA) */


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPage1)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPage1)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
