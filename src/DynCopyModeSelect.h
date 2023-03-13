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

#if !defined(AFX_DYNCOPYMODESELECT_H__19779CF3_D7A5_11D1_A4C4_C8F49C000000__INCLUDED_)
#define AFX_DYNCOPYMODESELECT_H__19779CF3_D7A5_11D1_A4C4_C8F49C000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DynCopyModeSelect.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDynCopyModeSelect dialog

class CDynCopyModeSelect : public CDialog
{
// Construction
public:
	CDynCopyModeSelect(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDynCopyModeSelect)
	enum { IDD = IDD_DYNAMIC_COPYMODE_SELECT };
	BOOL	m_appledouble;
	int		m_copy_mode;
	CString	m_copying_fname;
	//}}AFX_DATA

	int m_said_ok_all;
	int m_outcopy;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDynCopyModeSelect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDynCopyModeSelect)
	virtual void OnOK();
	afx_msg void OnOkall();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DYNCOPYMODESELECT_H__19779CF3_D7A5_11D1_A4C4_C8F49C000000__INCLUDED_)
