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
#include "AskDir.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskDir dialog


CAskDir::CAskDir(CWnd* pParent /*=NULL*/)
	: CDialog(CAskDir::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskDir)
	m_new_dir_name = _T("");
	//}}AFX_DATA_INIT
}


void CAskDir::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskDir)
	DDX_Text(pDX, IDC_NEW_DIR_NAME, m_new_dir_name);
	DDV_MaxChars(pDX, m_new_dir_name, 31);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskDir, CDialog)
	//{{AFX_MSG_MAP(CAskDir)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskDir message handlers
