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

#include "OptionsPage1.h"
#include "OptionsPage2.h"
#include "OptionsPage3.h"
#include "OptionsPage4.h"
#include "OptionsPage5.h"
#include "OptionsPage6.h"
#include "OptionsPage7.h"
#include "OptionsPage8.h"
#include "OptionsPage9.h"
#include "OptionsPage10.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

class COptionsSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsSheet)

// Construction
public:
	COptionsSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	int m_last_active_page;
	COptionsPage1 m_Page1;
	COptionsPage2 m_Page2;
	COptionsPage3 m_Page3;
	COptionsPage4 m_Page4;
	COptionsPage5 m_Page5;
	COptionsPage6 m_Page6;
	COptionsPage7 m_Page7;
	COptionsPage8 m_Page8;
	COptionsPage9 m_Page9;
	COptionsPage10 m_Page10;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsSheet)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionsSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptionsSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
