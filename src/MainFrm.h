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

#ifndef _MAINFRM_DEFINED
#define _MAINFRM_DEFINED

int ask_new_volume( char *volpath, char *vname, int *psize, int *normal );

#define MYTIMERID 123

typedef struct {
	HANDLE m_waiting_emulator_process;
	char m_waiting_doc_win_name[_MAX_PATH];
	char m_waiting_srcvolpath[_MAX_PATH];
	char m_waiting_srcname[_MAX_PATH];
	CTime m_waiting_emu_mtime;
	LONG m_waiting_emu_size;
	DWORD m_start_watch_emulator;
	// CDWordArray m_process_list;
} wait_item_struct;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitter2;

	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	void OnChildRegistration(WPARAM wParam, LPARAM lParam);

// Operations
public:
	void OnProgramProperties() ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	void MountPath( char *name );
	void go_home_dir( void );
	void WaitEmulator( char *name1, char *name2 );
	void WaitEmulator2( 
		HANDLE process,
		LPCSTR doc_win_name,
		LPCSTR srcvolpath,
		LPCSTR srcname
	);
	HWND FindEmuWindow( void );
	void LoadWindowPosition(void);
	void SaveWindowPosition(void);
	void OnCdEjectEjectReload( int reload );

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	UINT m_timer;
	BOOL m_volcheck_enabled;
	BOOL m_was_zoomed;
	char m_waiting_emulator1[100];
	char m_waiting_emulator2[100];
	
	// Pointers to wait_item_struct
	CDWordArray m_wait_items;

	/*
	HANDLE m_waiting_emulator_process;
	char m_waiting_doc_win_name[_MAX_PATH];
	char m_waiting_srcvolpath[_MAX_PATH];
	char m_waiting_srcname[_MAX_PATH];
	CTime m_waiting_emu_mtime;
	LONG m_waiting_emu_size;
	*/

	DWORD m_start_watch_emulator;
	BOOL m_emulator_window_detected;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewDetails();
	afx_msg void OnViewLargeicons();
	afx_msg void OnViewList();
	afx_msg void OnViewSmallicons();
	afx_msg void OnViewOptions();
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnFileNewvolume();
	afx_msg void OnFileVolumefromfloppy();
	afx_msg void OnFileVolumetofloppy();
	afx_msg void OnToolsDumpcddebug();
	afx_msg void OnCdEjectReload();
	afx_msg void OnCdEjectEject();
	afx_msg void OnHelp();
	afx_msg void OnViewMacWindow();
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCopyAppend();
	afx_msg void OnEditCutAppend();
	afx_msg void OnEditSelectAll();
	afx_msg BOOL OnQueryOpen();
	afx_msg void OnClose();
	afx_msg void OnFileEject();
	afx_msg void OnFileReload();
	afx_msg void OnToolsBurn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
