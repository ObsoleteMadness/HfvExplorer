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

#if !defined(AFX_OPTIONSPAGE4_H__F60DF543_DBA3_11D1_A4D0_5C5606000000__INCLUDED_)
#define AFX_OPTIONSPAGE4_H__F60DF543_DBA3_11D1_A4D0_5C5606000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// COptionsPage4 dialog

class COptionsPage4 : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPage4)

// Construction
public:
	COptionsPage4();
	~COptionsPage4();

// Dialog Data
	//{{AFX_DATA(COptionsPage4)
	enum { IDD = IDD_OPTIONS_PAGE_4 };
	int		m_default_emulator;
	int		m_link_name_style;
	int		m_launch_method;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPage4)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPage4)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPAGE4_H__F60DF543_DBA3_11D1_A4D0_5C5606000000__INCLUDED_)

