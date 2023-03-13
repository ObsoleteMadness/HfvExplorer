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

#if !defined(AFX_ASKPROPERTIES_H__E284DEA2_51A4_11D1_BEAE_444553540000__INCLUDED_)
#define AFX_ASKPROPERTIES_H__E284DEA2_51A4_11D1_BEAE_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// AskProperties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAskProperties dialog

class CAskProperties : public CDialog
{
// Construction
public:
	CAskProperties(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAskProperties)
	enum { IDD = IDD_HFS_ITEM_PROPERTIES };
	short	m_bottom;
	short	m_left;
	short	m_rifht;
	short	m_top;
	CString	m_name;
	long	m_nodeid;
	long	m_parentid;
	CString	m_creator;
	CString	m_type;
	short	m_x;
	short	m_y;
	UINT	m_valence;
	DWORD	m_rsize;
	DWORD	m_dsize;
	BOOL	m_hasbeeninited;
	BOOL	m_hasbundle;
	BOOL	m_hascolors;
	BOOL	m_hascustomicons;
	BOOL	m_hasnoinits;
	BOOL	m_isalias;
	BOOL	m_isdir;
	BOOL	m_isindesktop;
	BOOL	m_isinvisible;
	BOOL	m_isnamelocked;
	BOOL	m_isshared;
	BOOL	m_isstationery;
	BOOL	m_reqswitchlaunch;
	BOOL	m_reserved;
	BOOL	m_reservedcolors;
	BOOL	m_islocked;
	DWORD	m_backup;
	DWORD	m_modified;
	DWORD	m_created;
	//}}AFX_DATA

	BOOL m_all_are_indeed_enable_dont_ask_for_more;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAskProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAskProperties)
	afx_msg void OnPropIsdir();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASKPROPERTIES_H__E284DEA2_51A4_11D1_BEAE_444553540000__INCLUDED_)
