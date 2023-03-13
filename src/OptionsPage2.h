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
// COptionsPage2 dialog

class COptionsPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPage2)

// Construction
public:
	COptionsPage2();
	~COptionsPage2();

// Dialog Data
	//{{AFX_DATA(COptionsPage2)
	enum { IDD = IDD_OPTIONS_PAGE_2 };
	int		m_copyout_mode;
	int		m_copyin_mode;
	BOOL	m_copyin_appledouble;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPage2)
	afx_msg void OnCopyMap();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
