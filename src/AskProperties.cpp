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

#include "afx.h"
#include "stdafx.h"
#include "HFVExplorer.h"
#include "AskProperties.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAskProperties dialog


CAskProperties::CAskProperties(CWnd* pParent /*=NULL*/)
	: CDialog(CAskProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAskProperties)
	m_bottom = 0;
	m_left = 0;
	m_rifht = 0;
	m_top = 0;
	m_name = _T("");
	m_nodeid = 0;
	m_parentid = 0;
	m_creator = _T("");
	m_type = _T("");
	m_x = 0;
	m_y = 0;
	m_valence = 0;
	m_rsize = 0;
	m_dsize = 0;
	m_hasbeeninited = FALSE;
	m_hasbundle = FALSE;
	m_hascolors = FALSE;
	m_hascustomicons = FALSE;
	m_hasnoinits = FALSE;
	m_isalias = FALSE;
	m_isdir = FALSE;
	m_isindesktop = FALSE;
	m_isinvisible = FALSE;
	m_isnamelocked = FALSE;
	m_isshared = FALSE;
	m_isstationery = FALSE;
	m_reqswitchlaunch = FALSE;
	m_reserved = FALSE;
	m_reservedcolors = FALSE;
	m_islocked = FALSE;
	m_backup = 0;
	m_modified = 0;
	m_created = 0;
	//}}AFX_DATA_INIT
	m_all_are_indeed_enable_dont_ask_for_more = FALSE;
}


void CAskProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAskProperties)
	DDX_Text(pDX, IDC_PROP_BOTTOM, m_bottom);
	DDX_Text(pDX, IDC_PROP_LEFT, m_left);
	DDX_Text(pDX, IDC_PROP_RIGHT, m_rifht);
	DDX_Text(pDX, IDC_PROP_TOP, m_top);
	DDX_Text(pDX, IDC_PROP_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 31);
	DDX_Text(pDX, IDC_PROP_NODEID, m_nodeid);
	DDX_Text(pDX, IDC_PROP_PARENTID, m_parentid);
	DDX_Text(pDX, IDC_PROP_CREATOR, m_creator);
	DDV_MaxChars(pDX, m_creator, 4);
	DDX_Text(pDX, IDC_PROP_TYPE, m_type);
	DDV_MaxChars(pDX, m_type, 4);
	DDX_Text(pDX, IDC_PROP_X, m_x);
	DDX_Text(pDX, IDC_PROP_Y, m_y);
	DDX_Text(pDX, IDC_PROP_VALENCE, m_valence);
	DDV_MinMaxUInt(pDX, m_valence, 0, 65535);
	DDX_Text(pDX, IDC_PROP_RSIZE, m_rsize);
	DDX_Text(pDX, IDC_PROP_DSIZE, m_dsize);
	DDX_Check(pDX, IDC_PROP_HASBEENINITED, m_hasbeeninited);
	DDX_Check(pDX, IDC_PROP_HASBUNDLE, m_hasbundle);
	DDX_Check(pDX, IDC_PROP_HASCOLORS, m_hascolors);
	DDX_Check(pDX, IDC_PROP_HASCUSTOMICONS, m_hascustomicons);
	DDX_Check(pDX, IDC_PROP_HASNOINITS, m_hasnoinits);
	DDX_Check(pDX, IDC_PROP_ISALIAS, m_isalias);
	DDX_Check(pDX, IDC_PROP_ISDIR, m_isdir);
	DDX_Check(pDX, IDC_PROP_ISINDESKTOP, m_isindesktop);
	DDX_Check(pDX, IDC_PROP_ISINVISIBLE, m_isinvisible);
	DDX_Check(pDX, IDC_PROP_ISNAMELOCKED, m_isnamelocked);
	DDX_Check(pDX, IDC_PROP_ISSHARED, m_isshared);
	DDX_Check(pDX, IDC_PROP_ISSTATIONERY, m_isstationery);
	DDX_Check(pDX, IDC_PROP_REQSWITCHLAUNCH, m_reqswitchlaunch);
	DDX_Check(pDX, IDC_PROP_RESERVED, m_reserved);
	DDX_Check(pDX, IDC_PROP_RESERVEDCOLORS, m_reservedcolors);
	DDX_Check(pDX, IDC_PROP_ISLOCKED, m_islocked);
	DDX_Text(pDX, IDC_PROP_BACKUP, m_backup);
	DDX_Text(pDX, IDC_PROP_MODIFIED, m_modified);
	DDX_Text(pDX, IDC_PROP_CREATED, m_created);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAskProperties, CDialog)
	//{{AFX_MSG_MAP(CAskProperties)
	ON_BN_CLICKED(IDC_PROP_ISDIR, OnPropIsdir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAskProperties message handlers

// This isn't used anymore. Why ask trouble?
// What would I benefit?

/*
void CAskProperties::OnEnableAll() 
{
	if(m_all_are_indeed_enable_dont_ask_for_more) {
		AfxMessageBox( 
			"No. The \"Directory\" entry cannot be enabled. Please don't ask.", 
			MB_OK | MB_ICONINFORMATION);
		return;
	}
	if(AfxMessageBox( 
		"READ THIS CAREFULLY:\r\n"
		"\r\n"
		"By enabling all buttons you can do MAJOR DAMAGE! "
		"This option is meant to be used ONLY as a desperate last resort when trying to recover dead volume by hand. "
		"Don't come crying to me if you mess up your whole volume!\r\n"
		"\r\n"
		"Are you SURE you want to do this?", 
		MB_YESNO | MB_DEFBUTTON2|MB_ICONWARNING) != IDNO) 
	{
		// enable. god bless him/her.
		GetDlgItem(IDC_PROP_TYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_CREATOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_TOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_LEFT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_RIGHT)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_BOTTOM)->EnableWindow(TRUE);

		GetDlgItem(IDC_PROP_NODEID)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_PARENTID)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_RESERVED)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_ISALIAS)->EnableWindow(TRUE);

		GetDlgItem(IDC_PROP_VALENCE)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_RSIZE)->EnableWindow(TRUE);
		GetDlgItem(IDC_PROP_DSIZE)->EnableWindow(TRUE);
			
		m_all_are_indeed_enable_dont_ask_for_more = TRUE;
	}
}
*/

void CAskProperties::OnPropIsdir() 
{
}

BOOL CAskProperties::OnInitDialog() 
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	CDialog::OnInitDialog();

	if(m_isdir) {
		GetDlgItem(IDC_PROP_TYPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROP_CREATOR)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_PROP_TOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROP_LEFT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROP_RIGHT)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROP_BOTTOM)->EnableWindow(FALSE);
	}
	CenterWindow();

	if(pApp->get_font()) {
		::SendMessage( 
			GetDlgItem(IDC_PROP_NAME)->GetSafeHwnd(), 
			WM_SETFONT,
			(WPARAM)pApp->get_font(),
			MAKELPARAM(TRUE,0) );
	}

	return TRUE;
}
