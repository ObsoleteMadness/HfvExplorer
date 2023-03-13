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

#if !defined(AFX_OPTIONSPAGE7_H__63F265C3_6C92_11D2_A7AF_00201881A006__INCLUDED_)
#define AFX_OPTIONSPAGE7_H__63F265C3_6C92_11D2_A7AF_00201881A006__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// COptionsPage7 dialog

class COptionsPage7 : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPage7)

// Construction
public:
	COptionsPage7();
	~COptionsPage7();

// Dialog Data
	//{{AFX_DATA(COptionsPage7)
	enum { IDD = IDD_OPTIONS_PAGE_7 };
	CString	m_vmac_path;
	CString	m_vmac_startup_folder_name;
	CString	m_vmac_system_file_path;
	int		m_launch_method;
	BOOL	m_use_remover;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsPage7)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsPage7)
	afx_msg void OnCopt7BrowseVmacPath();
	afx_msg void OnCopt7BrowseVmacStartupFolderName();
	afx_msg void OnCopt7BrowseVmacSystemFilePath();
	afx_msg void OnCopt7VmacInstall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPAGE7_H__63F265C3_6C92_11D2_A7AF_00201881A006__INCLUDED_)
