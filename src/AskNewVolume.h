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

#if !defined(AFX_ASKNEWVOLUME_H__289936E4_4772_11D1_93B1_1AD35A000000__INCLUDED_)
#define AFX_ASKNEWVOLUME_H__289936E4_4772_11D1_93B1_1AD35A000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AskNewVolume.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAskNewVolume dialog

class CAskNewVolume : public CDialog
{
// Construction
public:
	CAskNewVolume(CWnd* pParent = NULL);   // standard constructor

	CString get_floppy_a_name(void);
	CString get_floppy_b_name(void);

// Dialog Data
	//{{AFX_DATA(CAskNewVolume)
	enum { IDD = IDD_ASK_NEW_VOLUME };
	CString	m_volname;
	CString	m_volsize_ex;
	CString	m_volpath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAskNewVolume)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAskNewVolume)
	afx_msg void OnVolnameBrowse();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASKNEWVOLUME_H__289936E4_4772_11D1_93B1_1AD35A000000__INCLUDED_)
