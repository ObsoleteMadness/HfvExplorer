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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "HFVExplorerListView.h"
#include "HFVExplorerTreeView.h"
#include "HFVExplorerDoc.h"
#include "HFVPreviewView.h"
#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerApp:
// See HFVExplorer.cpp for the implementation of this class
//

class CHFVExplorerApp : public CWinApp
{
public:
	CHFVExplorerApp();
	~CHFVExplorerApp();

	void LoadSettings(void);
	void SaveSettings(void);
	void CheckDefaultSettings(void);
	void WriteIconIndex(void);
	void kill_childs();
	void add_child( HWND hi );
	BOOL EnableTimer( BOOL onoff );
	HFONT get_font();
	void set_font( char *name, int ps );

	CStatusBar *m_pStatusBar;
	CHFVExplorerListView *m_list;
	HFVExplorerTreeView *m_tree;
	CHFVExplorerDoc *m_doc;
	CSplitterWnd *m_wndSplitter;
	CSplitterWnd *m_wndSplitter2;
	CMainFrame *m_main;
	CHFVPreviewView *m_pre;
	CString m_opendir;
	HWND m_parent_hwnd;
	int is_desktop_application;
	int m_startdir_processed;

	int		m_dosicons;
	int		m_fat_icon_time_limit;
	BOOL	m_mountall;
	BOOL	m_patch;
	BOOL	m_startup_floppies;
	BOOL	m_startup_cds;
	BOOL	m_startup_hds;
	BOOL	m_timer_enabled;

	BOOL	m_type_mapping_enabled;

	int m_copyout_mode, m_copyin_mode, m_copyin_appledouble;
	CString m_version;
	CString m_prevVersion;
	HFONT m_hfont;
	BOOL	m_is_font_sjis; /* noda */

	int m_next_icon_index;

	int m_show_invisible_mac;
	int m_show_invisible_fat;
	int m_protect_locked_mac;
	int m_protect_locked_fat;

	CString m_executor_dos_path;
	CString m_executor_win32_path;
	CString m_vmac_path;
	CString m_vmac_startup_folder_name;
	CString m_vmac_system_file_path;
	int m_link_name_style;

	int m_default_emulator;

	int m_launch_method;
	int m_use_remover;

	CString	m_fusion_startup_folder_name;
	CString	m_fusion_system_file_path;
	CString	m_fusion_batch_path;
	int m_fusion_use_remover;

	CString	m_shapeshifter_path;
	CString	m_shapeshifter_startup_folder_name;
	CString	m_shapeshifter_system_file_path;
	CString	m_shapeshifter_preferences_file_path;
	CString	m_shapeshifter_amiga_volume_name;
	int m_shapeshifter_use_remover;

	BOOL m_mac_window_mode;
	BOOL m_show_labels;

	BOOL m_use_partial_names;
	BOOL m_disable_autofloppy;
	BOOL m_do_harddisks;

	CString m_hpburner_path;

protected:
	CDWordArray child_array;
/*
	CString m_title;
	CString m_current_directory;
	unsigned long m_current_dir_id;
	unsigned long m_current_parent_id;
	*/

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHFVExplorerApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHFVExplorerApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
