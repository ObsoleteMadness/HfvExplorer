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

#ifndef _TREEVIEW_INCLUDED
#define _TREEVIEW_INCLUDED
/////////////////////////////////////////////////////////////////////////////
// HFVExplorerTreeView view

#include "ftypes.h"

typedef struct {
	int type;
	long id;
	int volinx;
	char path[MAX_PATH];
} lparam_struct;

class HFVExplorerTreeView : public CTreeView
{
protected:
	HFVExplorerTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(HFVExplorerTreeView)
	// HTREEITEM m_root_item;

	void delete_subdir_contents( HTREEITEM root );
	BOOL is_cd_item( HTREEITEM hItem );

	void on_volume_changed( int volinx );
	void Initialize();
	int tree_clear();
	int get_htree_type( HTREEITEM htree );
	void set_style( long style );
	HTREEITEM initialize_volume( 
		int volinx,
		CString root_name, 
		unsigned long id,
		int is_floppy,
		int is_cd,
		int is_hd,
		int is_removable );
	HTREEITEM initialize_fat_volume( 
		int volinx,
		CString *plabel, 
		UINT type,
		CString *ppath,
		BOOL is_floppy );
	HTREEITEM tree_insert( 
			HTREEITEM root_item,
			int volinx,
			LPSTR name, 
			unsigned long id, 
			unsigned long parent_id );
	HTREEITEM tree_fat_insert( 
		HTREEITEM root, 
		int volinx, 
		LPSTR name, 
		LPSTR fpath );
	HTREEITEM find_item( int volinx, HTREEITEM root, unsigned long id );
	HTREEITEM tree_find_fat( HTREEITEM root_item, LPSTR path );
	CString get_directory_name(	HTREEITEM root_item, int volinx, unsigned long id );
	unsigned long get_parent_id( HTREEITEM root_item, int volinx, unsigned long id );
	void expand_dir( HTREEITEM root_item, int volinx, unsigned long parent ) ;
	int initialize_tree();
	LPARAM make_lparam( 
		int type,
		int volinx, 
		long id,
		LPSTR path );
	int get_lparam_volinx( LPARAM lparam );
	int get_lparam_id( LPARAM lparam );
	int get_lparam_type( LPARAM lparam );
	int get_lparam_path( LPARAM lparam, LPSTR lpstr, int maxl );
	void tree_delete_images();
	void select_by_id( HTREEITEM root_item, int volinx, unsigned long id );
	void enable_selchange(int onoff);
	void destroy_pointers();
	void mode_changed( void );

	int m_selchange_enabled;
	CImageList *m_pimagelistSmall;

protected:
	CDWordArray m_pointer_array;

// Attributes
public:
	CTreeCtrl *m_ct;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HFVExplorerTreeView)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~HFVExplorerTreeView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(HFVExplorerTreeView)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
