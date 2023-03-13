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

#ifndef _LISTVIEW_INCLUDED
#define _LISTVIEW_INCLUDED

// #define PARENTIDFLAG 0xFFFF8000ul

#include "ftypes.h"

typedef struct {
	int type;
	long id;
	int volinx;
	int isdir;
	char path[MAX_PATH];
	HTREEITEM htree;
	HICON icon16;
	HICON icon32;
	unsigned long creation_date_lo, creation_date_hi;
	unsigned long modification_date_lo, modification_date_hi;
	int finder_color;
} list_lparam_struct;

#define FIELD_NAME		0
#define FIELD_SIZE		1
#define FIELD_CREATED 2
#define FIELD_MODIFIED 3
#define FIELD_TYPE		4
#define FIELD_CREATOR 5
#define FIELD_LABEL		6

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerListView view

class CHFVExplorerListView : public CListView
{
protected:
	CHFVExplorerListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHFVExplorerListView)

	// palette stuff
	enum { red = 1, green = 2, blue = 3 };

	RGBQUAD m_rgbPalette[256];
	CPalette m_Palette;
	CPalette* m_pOldPalette;

	CPalette* GetPalette();
	void CreatePalette();

	CListCtrl *m_cl;
	CDWordArray *m_pointer_array;
	// COLORREF m_bkcolor;
	CImageList* m_drag_list;
	int m_dragging;
	int m_iItemDrag, m_iItemDrop;
	CPoint m_ptOrigin, m_ptHotSpot;
	CSize m_sizeDelta;
	HTREEITEM m_TreeiItemDrop, m_save_tree_selection;

	int m_editing_item;
	char m_editing_item_name[_MAX_PATH];
	int m_move_type;

	HCURSOR m_move_cursor, m_copy_cursor;
	BOOL m_shutup_for_now;

	int m_sorted_by_column;
	BOOL m_sort_is_ascending;
	POINT m_mac_origo;

	CStringArray m_clipboard;
	CDWordArray m_clipboard_type;
	CDWordArray m_clipboard_isdir;
	CStringArray m_clipboard_volpath;
	CDWordArray m_clipboard_cutting;
	CDWordArray m_clipboard_volinx;

// Attributes
public:
	void on_properties( int inx );
	void on_volume_changed( int volinx );
	void _OnPaletteChanged(CWnd* pFocusWnd);
	BOOL _OnQueryNewPalette();

	int do_rename( 
		int item, 
		char *old_name,
		char *new_name,
		int check_lock
	);
	BOOL new_folder(void);
	void inx2volname( int inx, char *volpath, int size );
	int copy_item_to_dir( 
		int item, 
		HTREEITEM htree, 
		int action,
		CString *input_path,
		int input_isdir,
		int input_type,
		LPCSTR input_volpath,
		int input_volinx
	);
	void set_cursor_by_type( int control );
	void check_move_type( int control );
	void check_move_type( int control, int shift );
	void update_floppy_view( char *volpath );

	int new_bowser_on( CString start_dir );
	void update_current_view();
	int drop_list_to_tree( int item, HTREEITEM htree, int action );
	void set_drop_target( int iItem, int onoff );
	int tree_view_hit_test( CPoint pt );
	void set_tree_drop_target( HTREEITEM item, int onoff );
	void ensure_pane( int request_col );
	int get_inx_type(int inx);
	int get_inx_isdir(int inx);
	void try_delete_selected_items( BOOL confirm );

	void OnButtonUp(CPoint point);
	int FillList();
	int Initialize();
	void set_style( long style );
	CImageList *m_pimagelist;
	CImageList *m_pimagelistSmall;
	int m_item_count;
	int list_clear();
	int initialize_volume( CString root_name, unsigned long id );
	int list_insert( 
		LPSTR name, 
		int isdir, 
		unsigned long size_in_bytes,
		unsigned long creation_date,
		unsigned long modification_date,
		char *stype,
		char *screator,
		unsigned long id,
		HICON hicon32,
		HICON hicon16,
		HTREEITEM htree,
		int volinx,
		LPPOINT ppoint,
		BOOL is_inited,
		int finder_view_mode,
		int finder_color
	);
	int list_fat_insert(
			LPSTR name, 
			int isdir, 
			DWORD size_in_bytes,
			FILETIME *creation_date,
			FILETIME *modification_date,
			HICON hicon16,
			HICON hicon32,
			char *stype,
			char *screator,
			LPSTR fullpath,
			HTREEITEM htree,
			int volinx );
	void activate_list_item( int inx, int control, int make_shortcut );
	int initialize_list();

	LPARAM make_lparam( 
		int type,
		int volinx, 
		long id,
		LPSTR path,
		int isdir,
		HTREEITEM htree,
		HICON hicon32,
		HICON hicon16,
		unsigned long creation_date_lo, 
		unsigned long creation_date_hi,
		unsigned long modification_date_lo, 
		unsigned long modification_date_hi,
		int finder_color
		);
	int get_lparam_finder_color( LPARAM lparam );
	long get_lparam_id( LPARAM lparam );
	int get_lparam_type( LPARAM lparam );
	int get_lparam_isdir( LPARAM lparam );
	int get_lparam_path( LPARAM lparam, LPSTR lpstr, int maxl );
	HTREEITEM get_lparam_htree( LPARAM lparam );
	HICON get_lparam_icon32( LPARAM lparam );
	HICON get_lparam_icon16( LPARAM lparam );
	int get_lparam_volinx( LPARAM lparam );
	int get_lparam_modification( LPARAM lparam, DWORD *lo, DWORD *hi );
	int get_lparam_creation( LPARAM lparam, DWORD *lo, DWORD *hi );

	int get_path( int inx, CString &doc_name );
	int do_open_with( CString document, CString application, int mac, CString creator );
	int do_open_with( int document, CString app_name, int mac, CString creator );
	int do_open_with( CString document, int application, CString creator );
	int do_open_with( int document, int application, CString creator );
	int do_file_to_folder( int source, int dest, int action );
	int do_open_with_pc( CString document, CString application );
	int do_open_with_mac( CString document, CString application, CString creator );
	int is_mac_item( int inx );
	int is_mac_application( int inx );
	int write_text_file( LPSTR name, LPSTR txt );
	void get_vmac_launcher_name( char *name );
	void add_link_tail( char *lpszPathLink, char *id );
	int check_shortcut_installation( int emulator );
	int do_make_shortcut( int doc_inx, CString application, CString creator, int emulator, int make_shortcut );
	int do_make_shortcut_windows( int doc_inx, CString application, CString creator, int make_shortcut );
	int do_make_shortcut_vmac( int doc_inx, CString application, CString creator, int make_shortcut );
	int do_make_shortcut_e_win32( int doc_inx, CString application, CString creator, int make_shortcut );
	int do_make_shortcut_e_dos( int doc_inx, CString application, CString creator, int make_shortcut );
	int do_make_shortcut_ss( int doc_inx, CString application, CString creator, int make_shortcut );
	int do_make_shortcut_fusion( int doc_inx, CString application, CString creator, int make_shortcut );
	int save_icon( int doc_inx, LPSTR lpszIconFile, int is_doc );
	void get_icons_folder_name( char *name );
	int make_icon_name( LPSTR lpszIconFile );
	int make_e_dos_param_file_name( LPSTR lpszParamsFile );

	void destroy_pointers();

	BOOL ask_set_copy_mode( int outcopy, CString & name );
	BOOL map_selected_item( void );

	void do_report_sort(void);
	DWORD stupid_get_date( int inx, int whichone );

	void set_origo( int x, int y );
	void update_finder_position( int inx, CPoint p );
	void go_up( int control );

	void save_icon_command( int inx );
	void toggle_label_colors( void );
	void set_label_color( int finder_color );

	void edit_cut_copy( BOOL cutting, BOOL append );
	void edit_copy(void);
	void edit_cut(void);
	void edit_paste(void);
	void edit_copy_append(void);
	void edit_cut_append(void);
	void empty_clipboard( void );
	void edit_select_all(void);

	void update_callback(
		LPSTR doc_win_name,
		LPSTR destvolpath,
		LPSTR destname,
		BOOL copy_back
	);

// Operations
public:
	// void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHFVExplorerListView)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CHFVExplorerListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CHFVExplorerListView)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditLabel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
