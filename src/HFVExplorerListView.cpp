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
#include "HFVExplorerListView.h"
#include "HFVExplorerDoc.h"
#include "HFVPreviewView.h"
#include "MainFrm.h"
#include "special.h"
#include "own_ids.h"
#include "utils.h"
#include "openfile.h"
#include "shell.h"
#include <direct.h>
#include "hfs\libhfs\hfs.h"
#include "hfs\interface.h"
#include "AskDir.h"
#include "AskProperties.h"
#include "DynCopyModeSelect.h"
#include "icon.h"
#include "floppy.h"
#include "FileTypeMapping.h"
#include "tmap.h"

extern "C" {
#include "hfs\copyout.h"
}

#ifdef _DEBUG
// #define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerListView

enum { ACTION_ASK, ACTION_COPY, ACTION_MOVE };

IMPLEMENT_DYNCREATE(CHFVExplorerListView, CListView)

#define NCOLUMNS 6

LPARAM CHFVExplorerListView::make_lparam( 
	int type,
	int volinx, 
	long id,
	LPSTR path,
	int isdir,
	HTREEITEM htree,
	HICON icon32,
	HICON icon16,
	unsigned long creation_date_lo, 
	unsigned long creation_date_hi,
	unsigned long modification_date_lo, 
	unsigned long modification_date_hi,
	int finder_color )
{
	list_lparam_struct *p = new list_lparam_struct;
	if(!p) return(0);
	m_pointer_array->Add( (DWORD)p );
	p->type = type;
	p->volinx = volinx;
	p->id = id;
	p->isdir = isdir;
	p->htree = htree;
	p->icon16 = icon16;
	p->icon32 = icon32;
	p->creation_date_lo = creation_date_lo;
	p->creation_date_hi = creation_date_hi;
	p->modification_date_lo = modification_date_lo;
	p->modification_date_hi = modification_date_hi;
	p->finder_color = finder_color;
	if(path && *path) {
		strncpy( p->path, path, MAX_PATH-1 );
		p->path[MAX_PATH-1] = 0;
	} else {
		p->path[0] = 0;
	}
	return( (LPARAM)p );
}

int CHFVExplorerListView::get_lparam_finder_color( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(FINDER_COLOR_NONE);
	return( p->finder_color );
}

HICON CHFVExplorerListView::get_lparam_icon16( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	return( p->icon16 );
}

HICON CHFVExplorerListView::get_lparam_icon32( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	return( p->icon32 );
}

long CHFVExplorerListView::get_lparam_id( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	return( p->id );
}

int CHFVExplorerListView::get_lparam_isdir( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	return( p->isdir );
}

int CHFVExplorerListView:: get_inx_isdir(int inx)
{
	DWORD dat = m_cl->GetItemData( inx );
	if(!dat) return(-1);
	return(get_lparam_isdir(dat));
}

int CHFVExplorerListView::get_lparam_volinx( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(-1);
	return( p->volinx );
}

int CHFVExplorerListView::get_lparam_type( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(LIST_UNKNOWN);
	return( p->type );
}

int CHFVExplorerListView::get_lparam_modification( LPARAM lparam, DWORD *lo, DWORD *hi )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	*lo = p->modification_date_lo;
	*hi = p->modification_date_hi;
	return(1);
}

int CHFVExplorerListView::get_lparam_creation( LPARAM lparam, DWORD *lo, DWORD *hi )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	*lo = p->creation_date_lo;
	*hi = p->creation_date_hi;
	return(1);
}

int CHFVExplorerListView:: get_inx_type(int inx)
{
	DWORD dat = m_cl->GetItemData( inx );
	if(!dat) return(-1);
	return(get_lparam_type(dat));
}

HTREEITEM CHFVExplorerListView::get_lparam_htree( LPARAM lparam )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	return( p->htree );
}

int CHFVExplorerListView::get_lparam_path( LPARAM lparam, LPSTR lpstr, int maxl )
{
	list_lparam_struct *p = (list_lparam_struct *)lparam;
	if(!p) return(0);
	if(p->path && *p->path) {
		int len = max(maxl,MAX_PATH-1);
		strncpy( lpstr, p->path, len );
		lpstr[len] = 0;
		return(1);
	} else {
		return(0);
	}
}

void CHFVExplorerListView::destroy_pointers()
{
	int i, count = m_pointer_array->GetSize();
	DWORD dw;
	list_lparam_struct *p;

	for(i=0; i<count; i++) {
		dw = m_pointer_array->GetAt( i );
		if(dw) {
			p = (list_lparam_struct *)dw;
			delete p;
		}
	}
	m_pointer_array->RemoveAll();
}

#define LISTICONCOUNT 12
#define MAXEXTRAIMAGES 200

CHFVExplorerListView::CHFVExplorerListView()
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	m_sorted_by_column = FIELD_NAME;
	m_sort_is_ascending = TRUE;

	m_shutup_for_now = FALSE;

	m_move_cursor = pApp->LoadCursor( IDC_POINTER );
	m_copy_cursor = pApp->LoadCursor( IDC_POINTER_COPY );

	m_cl = &this->GetListCtrl();

	m_pointer_array = new CDWordArray;
	m_pointer_array->SetSize( 400, 50 );
	m_dragging = 0;

	m_item_count = 0;
	m_pimagelist = new CImageList();		
	m_pimagelistSmall = new CImageList();

	ASSERT(m_pimagelist != NULL && m_pimagelistSmall != NULL);

	// for version 1.2.7 fixed TRUE -> ILC_MASK|ILC_COLOR24
	m_pimagelist->Create(32, 32, ILC_MASK|ILC_COLOR24, LISTICONCOUNT, MAXEXTRAIMAGES);
	m_pimagelistSmall->Create(16, 16, ILC_MASK|ILC_COLOR24, LISTICONCOUNT, MAXEXTRAIMAGES);

	CreatePalette();

	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON1));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON2));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON3));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON4));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON5));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON6));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON7));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON8));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON9));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON10));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON11));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON12));
	m_pimagelist->Add(pApp->LoadIcon(IDI_ICON1_YELLOW));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_PROJECT_2));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_PROJECT_1));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_PERSONAL));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_COOL));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_IN_PROGRESS));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_HOT));
	m_pimagelist->Add(pApp->LoadIcon(IDI_LABEL_ESSENTIAL));

	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON1));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON2));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON3));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON4));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON5));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON6));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON7));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON8));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON9));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON10));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON11));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON12));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON1_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_PROJECT_2));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_PROJECT_1));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_PERSONAL));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_COOL));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_IN_PROGRESS));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_HOT));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_LABEL_ESSENTIAL));

	m_pimagelist->SetOverlayImage( 13, 1 );
	m_pimagelist->SetOverlayImage( 14, 2 );
	m_pimagelist->SetOverlayImage( 15, 3 );
	m_pimagelist->SetOverlayImage( 16, 4 );
	m_pimagelist->SetOverlayImage( 17, 5 );
	m_pimagelist->SetOverlayImage( 18, 6 );
	m_pimagelist->SetOverlayImage( 19, 7 );

	m_pimagelistSmall->SetOverlayImage( 13, 1 );
	m_pimagelistSmall->SetOverlayImage( 14, 2 );
	m_pimagelistSmall->SetOverlayImage( 15, 3 );
	m_pimagelistSmall->SetOverlayImage( 16, 4 );
	m_pimagelistSmall->SetOverlayImage( 17, 5 );
	m_pimagelistSmall->SetOverlayImage( 18, 6 );
	m_pimagelistSmall->SetOverlayImage( 19, 7 );

	m_pOldPalette = NULL;

	empty_clipboard();
}

CHFVExplorerListView::~CHFVExplorerListView()
{
	if(m_pimagelist) {
		delete m_pimagelist;
		m_pimagelist = 0;
	}
	if(m_pimagelistSmall) {
		delete m_pimagelistSmall;
		m_pimagelistSmall = 0;
	}
	((CHFVExplorerDoc*)m_pDocument)->clear_hash_table();
	destroy_pointers();
	delete m_pointer_array;
}

int CHFVExplorerListView::Initialize()
{
	LV_COLUMN		lvcolumn;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	if(pApp->get_font()) {
		::SendMessage( 
			m_cl->m_hWnd, 
			WM_SETFONT,
			(WPARAM)pApp->get_font(),
			MAKELPARAM(TRUE,0) );
	}

	// m_bkcolor = RGB( 255,255,192 );
	// m_cl->SetBkColor( m_bkcolor );

	list_clear();
	ASSERT(m_pimagelist != NULL && m_pimagelistSmall != NULL);
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Name";
	lvcolumn.iSubItem = FIELD_NAME;
	lvcolumn.cx = 170;
	m_cl->InsertColumn(FIELD_NAME, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Size";
	lvcolumn.iSubItem = FIELD_SIZE;
	lvcolumn.cx = 75;
	m_cl->InsertColumn(FIELD_SIZE, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Created";
	lvcolumn.iSubItem = FIELD_CREATED;
	lvcolumn.cx = 140; // 90;
	m_cl->InsertColumn(FIELD_CREATED, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Modified";
	lvcolumn.iSubItem = FIELD_MODIFIED;
	lvcolumn.cx = 140; // 90;
	m_cl->InsertColumn(FIELD_MODIFIED, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Type";
	lvcolumn.iSubItem = FIELD_TYPE;
	lvcolumn.cx = 50;
	m_cl->InsertColumn(FIELD_TYPE, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Creator";
	lvcolumn.iSubItem = FIELD_CREATOR;
	lvcolumn.cx = 65;
	m_cl->InsertColumn(FIELD_CREATOR, &lvcolumn);

	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = "Label";
	lvcolumn.iSubItem = FIELD_LABEL;
	lvcolumn.cx = 85;
	m_cl->InsertColumn(FIELD_LABEL, &lvcolumn);

	// this is no limit, just allocates this much instead of small pieces
	m_cl->SetItemCount( 500 ); 
  
	// CreatePalette();
	{
		CWnd* pActiveWnd = CWnd::GetActiveWindow();
		if (pActiveWnd != NULL)
			pActiveWnd->SendMessage(WM_QUERYNEWPALETTE);
	}

	return(1);
}

void CHFVExplorerListView::set_style( long style )
{
	long lStyleOld;

	lStyleOld = GetWindowLong(m_cl->m_hWnd, GWL_STYLE);

	lStyleOld &= ~LVS_LIST;
	lStyleOld &= ~LVS_SMALLICON;
	lStyleOld &= ~LVS_OWNERDRAWFIXED;
	lStyleOld &= ~LVS_SORTDESCENDING;
	lStyleOld &= ~LVS_ALIGNLEFT;
	lStyleOld &= ~LVS_REPORT;
	// lStyleOld &= ~LVS_SHOWSELALWAYS;
	lStyleOld &= ~LVS_NOCOLUMNHEADER;

	lStyleOld &= ~LVS_AUTOARRANGE;
	lStyleOld &= ~(unsigned)LVS_NOSORTHEADER;

	lStyleOld |= (unsigned)LVS_NOSORTHEADER;
	// lStyleOld |= LVS_AUTOARRANGE;

	// lStyleOld |= LVS_SINGLESEL;

	lStyleOld |= LVS_EDITLABELS;
	lStyleOld |= LVS_SORTASCENDING;
	lStyleOld |= LVS_SHOWSELALWAYS;


	// TEST
	// lStyleOld |= LVS_OWNERDRAWFIXED;
	// lStyleOld |= LVS_NOLABELWRAP;


	lStyleOld |= style;

	SetWindowLong(m_cl->m_hWnd, GWL_STYLE, lStyleOld);

	switch( style ) {
		case LVS_LIST:
			break;
		case LVS_REPORT:
			do_report_sort();
			break;
		case LVS_ICON:
			break;
		case LVS_SMALLICON:
			break;
	}
}

int is_mac_path( CString &name )
{
	char *s;
	int i;

	s = name.GetBuffer(MAX_PATH);
	if( isalpha(s[0]) && s[1] == ':' && s[2] == '\\' )
		i = 0;
	else 
		i = 1;
	name.ReleaseBuffer();

	return(i);
}

int CHFVExplorerListView::initialize_volume( CString root_name, unsigned long id )
{
	LV_ITEM	lvitem;
	int iActualItem;
	LPSTR buf;

	list_clear();

	if(id) {
		 // call by value
		if(((CHFVExplorerApp *)AfxGetApp())->m_mac_window_mode) {
			if(root_name == "" || is_mac_path(root_name)) {
				// In mac windowed mode, no up puttons.
				return(1);
			}
		}

		if(root_name == "") {
			root_name = "Root folder";
		} else {
			root_name = " ..up to " + root_name;
		}
		buf = root_name.GetBuffer( MAX_PATH );
		lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
		lvitem.iItem = m_item_count;
		lvitem.iSubItem = 0;
		lvitem.pszText = buf;
		lvitem.iImage = 2;
		lvitem.lParam = (LPARAM)make_lparam( LIST_SPEC, 0, id, "", 1, 0, 0, 0, 0, 0, 0, 0, FINDER_COLOR_NONE );
		lvitem.state = LVIS_FOCUSED | LVIS_SELECTED;
		lvitem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
		iActualItem = m_cl->InsertItem(&lvitem);
		m_item_count++;
		root_name.ReleaseBuffer();
	}

	return(1);
}

int CHFVExplorerListView::initialize_list()
{
	list_clear();
	return(1);
}

int CHFVExplorerListView::list_clear()
{
	m_cl->DeleteAllItems();
	m_item_count = 0;
	return(1);
}

// This origo is used to remove any Finder scrolling effect.
void CHFVExplorerListView::set_origo( int x, int y )
{
	m_mac_origo.x = x;
	m_mac_origo.y = y;
	// m_mac_origo.x = 0;
	// m_mac_origo.y = 0;
}

int CHFVExplorerListView::list_insert(
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
)
{
	LV_ITEM	lvitem;
	int image, iActualItem;
	char buf[200];

	// add_to_icon_cache( hicon32 );
	// add_to_icon_cache( hicon16 );

	if(isdir) {
		image = 0;
	} else if(strcmp(stype,"|!@$") == 0) {
		image = 8;
	} else if(strcmp(stype,"APPL") == 0) {
		image = 4;
	} else if(strcmp(stype,"FFIL") == 0 ||
				strcmp(stype,"ffil") == 0 ||
				strcmp(stype,"tfil") == 0) {
		image = 6;
	} else if(strcmp(stype,"DFIL") == 0 ||
				strcmp(stype,"dfil") == 0) {
		image = 10;
	} else if(strcmp(stype,"TEXT") == 0 ||
				strcmp(stype,"PICT") == 0 ||
				strcmp(stype,"ttxt") == 0 ||
				strcmp(stype,"ttro") == 0) {
		image = 11;
	} else if(strcmp(stype,"ZSYS") == 0 ||
				strcmp(stype,"CLIP") == 0 ||
				strcmp(stype,"FNDR") == 0) {
		image = 7;
	} else if(strcmp(stype,"PREF") == 0 ||
				strcmp(stype,"pref") == 0) {
		image = 9;
	} else if(strcmp(stype,"fdrp") == 0) {
		// Folder alias
		image = 0;
	} else {
		// image = 1;
		image = 8;
	}

	if(hicon32 || hicon16) {
		image = m_pimagelist->GetImageCount();
		m_pimagelist->Add(hicon32 ? hicon32 : hicon16);
		m_pimagelistSmall->Add(hicon16 ? hicon16 : hicon32);
	}

	lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvitem.iItem = m_item_count;
	lvitem.iSubItem = FIELD_NAME;
	lvitem.pszText = name;
	lvitem.iImage = image;
	lvitem.lParam = make_lparam( LIST_HFS, volinx, id, "", isdir, htree,
		hicon32 ? hicon32 : hicon16, hicon16 ? hicon16 : hicon32,
		creation_date, 0, modification_date, 0, finder_color );

	lvitem.state = 0;
	lvitem.stateMask = 0;

	if(m_cl->GetItemCount() == 0) {
		lvitem.mask |= LVIF_STATE;
		lvitem.state = LVIS_FOCUSED | LVIS_SELECTED;
		lvitem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	}

	iActualItem = m_cl->InsertItem(&lvitem);

	if(isdir) {
		wsprintf( buf, "%ld items", size_in_bytes );
	} else {
		wsprintf( buf, "%ld KB", (size_in_bytes + 1023) / 1024 );
	}
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_SIZE;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	mac_date_to_string( creation_date, buf );
	// wsprintf( buf, "%08x", creation_date );
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_CREATED;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	mac_date_to_string( modification_date, buf );
	// wsprintf( buf, "%08x", modification_date );
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_MODIFIED;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_TYPE;
	lvitem.pszText = stype;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_CREATOR;
	lvitem.pszText = screator;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_LABEL;
	lvitem.pszText = get_label_text( finder_color );
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	// not so simple
	if(((CHFVExplorerApp *)AfxGetApp())->m_mac_window_mode) {
		POINT p;
		if(ppoint->x == -1 && ppoint->y == -1) is_inited = 0;
		// 0,0 is quite difficult in practice...
		if(is_inited || is_inside_magic(ppoint->x,ppoint->y)) {
			p.x = unmagic(ppoint->x) - m_mac_origo.x;
			p.y = unmagic(ppoint->y) - m_mac_origo.y;
			// p.y = (int)( (float)p.y * 1.20 );
			m_cl->SetItemPosition(iActualItem, p);
		}
	}

	m_item_count++;

	if(((CHFVExplorerApp *)AfxGetApp())->m_show_labels && finder_color) {
		m_cl->SetItemState( iActualItem, INDEXTOOVERLAYMASK(finder_color), LVIS_OVERLAYMASK );
	}

	// UpdateWindow();

	return(1);
}

// Use magic square to tell Finder where to put the item.
// Makes sure that inited flag is cleared
void CHFVExplorerListView::update_finder_position( int inx, CPoint p )
{
	if(get_inx_type(inx) == LIST_HFS) {
		CString fullpath;
		char volpath[_MAX_PATH];
		hfsdirent ent;

		get_path( inx, fullpath );
		inx2volname( inx, volpath, _MAX_PATH );
		char *path = fullpath.GetBuffer( _MAX_PATH );

		if(0 == HFSIFACE_get_properties( volpath, path, &ent )) {
			ent.fdlocation.h = (signed short)( p.x - FINDER_MAGIC_OFFSET + m_mac_origo.x );
			ent.fdlocation.v = (signed short)( p.y - FINDER_MAGIC_OFFSET + m_mac_origo.y );
			ent.fdflags &= ~HFS_FNDR_HASBEENINITED;
			if(0 == HFSIFACE_set_properties( volpath, path, &ent )) {
				((CHFVExplorerDoc*)m_pDocument)->set_hfs_volume_clean( volpath );
				// update_floppy_view(volpath);
			} else {
				// AfxMessageBox( "Failed to set item properties" );
			}
		}
		fullpath.ReleaseBuffer();
	}
}

void pc_date_to_string( FILETIME *time, char *buf )
{
	SYSTEMTIME sys;
	FileTimeToSystemTime( time, &sys );
	wsprintf( 
		buf, "%04d-%02d-%02d %02d:%02d:%02d", 
		sys.wYear,
		sys.wMonth,
		sys.wDay,
		sys.wHour,
		sys.wMinute,
		sys.wSecond
		);
}

int CHFVExplorerListView::list_fat_insert(
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
		int volinx
)
{
	LV_ITEM	lvitem;
	int image, iActualItem;
	char buf[200];

	// add_to_icon_cache( hicon32 );
	// add_to_icon_cache( hicon16 );

	if(isdir) {
		image = 12; // 0;
	} else {
		image = 1;
	}

	if(hicon16 || hicon32) {
		image = m_pimagelistSmall->GetImageCount();
		m_pimagelistSmall->Add(hicon16 ? hicon16 : hicon32);
		m_pimagelist->Add(hicon32 ? hicon32 : hicon16);
	}

	lvitem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvitem.iItem = m_item_count;
	lvitem.iSubItem = FIELD_NAME;
	lvitem.pszText = name;
	lvitem.iImage = image;
	lvitem.lParam = make_lparam( LIST_FAT, volinx, 0, fullpath, isdir, htree,
				hicon32 ? hicon32 : hicon16, hicon16 ? hicon16 : hicon32,
				creation_date->dwLowDateTime, creation_date->dwHighDateTime,
				modification_date->dwLowDateTime, modification_date->dwHighDateTime,
				FINDER_COLOR_NONE
	);
	if(m_cl->GetItemCount() == 0) {
		lvitem.mask |= LVIF_STATE;
		lvitem.state = LVIS_FOCUSED | LVIS_SELECTED;
		lvitem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	}
	
	iActualItem = m_cl->InsertItem(&lvitem);

	if(isdir) {
		wsprintf( buf, "%ld items", size_in_bytes );
	} else {
		wsprintf( buf, "%ld KB", (size_in_bytes + 1023) / 1024 );
	}
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_SIZE;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	pc_date_to_string( creation_date, buf );
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_CREATED;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	pc_date_to_string( modification_date, buf );
	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_MODIFIED;
	lvitem.pszText = buf;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_TYPE;
	lvitem.pszText = stype;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_CREATOR;
	lvitem.pszText = screator;
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = iActualItem;
	lvitem.iSubItem = FIELD_LABEL;
	lvitem.pszText = get_label_text( FINDER_COLOR_NONE );
	lvitem.iImage = 0; // don't care
	m_cl->SetItem(&lvitem);
	
	m_item_count++;
	return(1);
}

int CHFVExplorerListView::FillList()
{
	m_cl->SetImageList(m_pimagelist, LVSIL_NORMAL);
	m_cl->SetImageList(m_pimagelistSmall, LVSIL_SMALL);
	return(1);
}

BEGIN_MESSAGE_MAP(CHFVExplorerListView, CListView)
	//{{AFX_MSG_MAP(CHFVExplorerListView)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_WM_SYSKEYDOWN()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_NOTIFY_REFLECT(LVN_BEGINRDRAG, OnBeginrdrag)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_WM_KEYUP()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_COMMAND(ID_EDIT_LABEL, OnEditLabel)
	ON_WM_MEASUREITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerListView diagnostics

#ifdef _DEBUG
void CHFVExplorerListView::AssertValid() const
{
	CListView::AssertValid();
}

void CHFVExplorerListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerListView message handlers

#define MAXCMDLEN 1024

// Used to make it compatible with versions bwfore 20p
int is_already_ok_spec( int ch )
{
	return(0);

	if( ch == 0x80 || // Ä
		0x8A || // ä
		0x85 || // Ö
		0x9A || // ö
		0x81 || // Å
		0x8C || // å
		0x9F || // ü
		0xC4) // f
	{
		return(1);
	} else {
		return(0);
	}
}

int mac_spec_char( int ch, char *hexword )
{
	int result = 0;
	if(isalnum(ch) || strchr(" !#&/()=?.,;:-",ch) || is_already_ok_spec(ch)) {
		result = 0;
	} else {
		sprintf( hexword, "::%02x", (int)ch );
		result = 1;
	}
	return(result);
}

// OBSOLETE
void macish( CString *cs )
{
	LPSTR s;
	s = cs->GetBuffer( MAXCMDLEN );
	pc_to_mac_charset( (unsigned char *)s );
	cs->ReleaseBuffer();
}

CString make_mac_chrset( CString cs )
{
	LPSTR s = cs.GetBuffer( MAXCMDLEN );
	char news[MAXCMDLEN];

	int i, len = strlen( s );
	int j = 0;
	char hexword[20];

	pc_to_mac_charset( (unsigned char*)s );
	for( i=0; i<len; i++ ) {
		int ch;
		ch = (int)(unsigned char)s[i];
		if(mac_spec_char( ch, hexword )) {
			memcpy( &news[j], hexword, strlen(hexword) );
			j += strlen(hexword);
		} else {
			news[j++] = ch;
		}
	}
	news[j] = 0;
	cs.ReleaseBuffer();
	return( CString(news) );
}

// #define EXE_ERRBASE 4000

void get_execute_error_string( HINSTANCE hi, CString &s )
{
	// cannot use this - ids change: s->LoadString( EXE_ERRBASE + (int)hi );
	switch( (int)hi ) {
		case 0:
			s = "The operating system is out of memory or resources";
			break;
		case ERROR_FILE_NOT_FOUND:
			s = "The specified file was not found";
			break;
		case ERROR_PATH_NOT_FOUND	:
			s = "The specified path was not found";
			break;
		case ERROR_BAD_FORMAT	:
			s = "The .EXE file is invalid (non-Win32 .EXE or error in .EXE image)";
			break;
		case SE_ERR_ACCESSDENIED	:
			s = "The operating system denied access to the specified file.";
			break;
		case SE_ERR_ASSOCINCOMPLETE	:
			s = "The filename association is incomplete or invalid";
			break;
		case SE_ERR_DDEBUSY	:
			s = "The DDE transaction could not be completed because other DDE transactions were being processed";
			break;
		case SE_ERR_DDEFAIL	:
			s = "The DDE transaction failed";
			break;
		case SE_ERR_DDETIMEOUT	:
			s = "The DDE transaction could not be completed because the request timed out";
			break;
		case SE_ERR_DLLNOTFOUND	:
			s = "The specified dynamic-link library was not found.";
			break;

		/*
		case SE_ERR_FNF	:
			s = "The specified file was not found.";
			break;
		case SE_ERR_PNF	:
			s = "The specified path was not found";
			break;
		*/
		case SE_ERR_NOASSOC	:
			s = "There is no application associated with the given filename extension";
			break;
		case SE_ERR_OOM	:
			s = "There was not enough memory to complete the operation";
			break;
		case SE_ERR_SHARE	:
			s = "A sharing violation occurred";
			break;
		default:
			s = "Unknown error";
	}
}

void pc_activate( char *path )
{
	HINSTANCE hi;

	hi = ShellExecute(
		NULL, // hwnd
		"open",
		path,
		NULL, // params
		NULL, // dir
		SW_SHOWNORMAL );
	if( hi <= (HINSTANCE)32 ) {
		CString errstr;
		get_execute_error_string( hi, errstr );
		AfxMessageBox( "Could not open file \"" + 
			CString(path) + "\":\n" + errstr + ".", MB_OK | MB_ICONSTOP );
	}
}

int CHFVExplorerListView::get_path( int inx, CString &doc_name )
{
	DWORD dat;

	dat = m_cl->GetItemData( inx );
	if(!dat) return(0);
	doc_name = "";
	if(get_lparam_type(dat) == LIST_FAT) {
		char fullpath[MAX_PATH];
		if(!get_lparam_path(dat,fullpath,MAX_PATH-1)) return(0);
		doc_name += CString( fullpath );
	} else { // LIST_HFS
		int dummy;
		doc_name += ((CHFVExplorerDoc*)m_pDocument)->get_directory(0,&dummy) +
							CString(":") + m_cl->GetItemText( inx, 0 );
	}
	return(1);
}

/*
int copy_file( open_file_type *sfp, open_file_type *dfp )
{
	const BLOCKSIZE = 512;
	unsigned char buf[BLOCKSIZE];
	int ok = 0, bytes, del_dest = 0;

	if(sfp->Open( open_file_type::ModeOpen )) {
		if(dfp->Open( open_file_type::ModeCreate )) {
			START_TIME_CONSUMING(1000);
			int copying = 1;
			while(copying) {
				UPDATE_TIME_CONSUMING(0);
				bytes = sfp->Read( buf, BLOCKSIZE );
				if( bytes == 0 ) {
					copying = 0;
					if(sfp->LastError() != open_file_type::EndOfFile) {
						sfp->ErrorAlert();
					} else {
						ok = 1;
					}
				} else {
					if(!dfp->Write( buf, bytes )) {
						dfp->ErrorAlert();
						copying = 0;
						del_dest = 1;
					}
				}
			} // while copying
		} else {
			dfp->ErrorAlert();
		}
	} else {
		sfp->ErrorAlert();
	}
	dfp->Close();
	sfp->Close();
	END_TIME_CONSUMING;
	if(del_dest) {
		dfp->Delete();
	}
	return(ok);
}

int copy_file_list( int source, int dest, int stype, int dtype )
{
	open_file_type s_fp( 
		stype == LIST_FAT ? open_file_type::SystemFAT :
				open_file_type::SystemHFS, source );
	open_file_type d_fp( 
		dtype == LIST_FAT ? open_file_type::SystemFAT :
				open_file_type::SystemHFS, dest );

	return(copy_file( &s_fp, &d_fp ));
}
*/

int ask_dir( char *new_name )
{
	CAskDir dlg;
	int ret;

	dlg.m_new_dir_name = CString(new_name);
	ret = (dlg.DoModal() == IDOK);
	if(ret) {
		lstrcpy( new_name, dlg.m_new_dir_name.GetBuffer(_MAX_PATH) );
	}
	return(ret);
}
	
BOOL CHFVExplorerListView::new_folder(void)
{
	CHFVExplorerApp	*pApp;
	CTreeCtrl *tc;
	HFVExplorerTreeView *tv;
	HTREEITEM hItem;
	BOOL ok = FALSE;
	CString cs;
	char destvolpath[_MAX_PATH];
	char new_dir_name[_MAX_PATH];
	char new_name[_MAX_PATH];
	int len, volinx;
	DWORD data;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	tv = pApp->m_tree;
	tc = tv->m_ct;
	hItem = tc->GetSelectedItem();
	data = tc->GetItemData( hItem );
	volinx = tv->get_lparam_volinx( data );

	if(tv->get_lparam_type(data) == LIST_HFS) {
		cs = ((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name;
	} else {
		cs = ((CHFVExplorerDoc*)m_pDocument)->m_fats[volinx].m_volume_name;
	}
	lstrcpy( destvolpath, cs.GetBuffer( _MAX_PATH ) );
	cs.ReleaseBuffer();

	int ismac;
	cs = ((CHFVExplorerDoc*)m_pDocument)->get_directory( hItem, &ismac );
	lstrcpy( new_dir_name, cs.GetBuffer( _MAX_PATH ) );
	cs.ReleaseBuffer();
	len = strlen(new_dir_name);

	#define MAX_ATTEMPTS 50
	if(ismac) {
		int new_index = 0, i;
		for( i=0; i<MAX_ATTEMPTS; i++ ) {
			lstrcpy( new_name, "New Folder" );
			if(i) {
				// start (n) from (2)
				wsprintf( &new_name[strlen(new_name)], " (%d)", i+1 );
			}
			if( !HFSIFACE_exists_split( volinx, destvolpath, new_dir_name, new_name )) {
				break;
			}
		}
		if(i == MAX_ATTEMPTS) lstrcpy( new_name, "New Folder (N)" );
	} else {
		lstrcpy( new_name, "New Folder" );
	}
	#undef MAX_ATTEMPTS

	if(ask_dir( new_name ) == 0) return(0);

	if(ismac) {
		if(len && new_dir_name[len-1] != ':')
			lstrcat( new_dir_name, ":" );
		lstrcat( new_dir_name, new_name );
	} else {
		if(len && new_dir_name[len-1] != '\\')
			lstrcat( new_dir_name, "\\" );
		lstrcat( new_dir_name, new_name );
	}

	if(ismac) {
		ok = (0 == HFSIFACE_mkdir( destvolpath, new_dir_name ));
		if(ok) {
			update_floppy_view(destvolpath);
		}
	} else {
		ok = (0 != CreateDirectory(new_dir_name,NULL));
		if(ok) update_current_view();
	}
	return(ok);
}

#define ASKCOPYMODE 6

BOOL CHFVExplorerListView::ask_set_copy_mode( 
	int outcopy, CString & name
)
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	BOOL ret;
	static int modein = 0, modeout = 0, ad = -1;

	if( (outcopy && pApp->m_copyout_mode != ASKCOPYMODE) ||
			(!outcopy && pApp->m_copyin_mode != ASKCOPYMODE) )
	{
		return(TRUE);
	}

	if(m_shutup_for_now) return(TRUE);

	CDynCopyModeSelect d;

	if(outcopy) {
		d.m_copy_mode = modeout;
	} else {
		d.m_copy_mode = modein;
	}
	if(ad >= 0) 
		d.m_appledouble = ad;
	else
		d.m_appledouble = pApp->m_copyin_appledouble;

	d.m_copying_fname = name;
	char *p = d.m_copying_fname.GetBuffer( _MAX_PATH );
	// mac_to_pc_charset( (unsigned char *)p );
	d.m_copying_fname.ReleaseBuffer();

	d.m_outcopy = outcopy;

	int answer = d.DoModal();
	ret = (answer == IDOK);
	if(ret) {
		if(d.m_said_ok_all) m_shutup_for_now = TRUE;
		if(outcopy) {
			modeout = d.m_copy_mode;
			HFSIFACE_set_modeout( modeout );
		} else {
			modein = d.m_copy_mode;
			ad = d.m_appledouble;
			HFSIFACE_set_modein( modein, ad );
		}
	}
	return(ret);
}

// Temp:New Folder    Temp:New Folder
BOOL is_same_folder( LPCSTR srcname, LPCSTR destname )
{
	BOOL result = stricmp(srcname,destname) == 0;
	return(result);
}

// Temp:New Folder    Temp:New Folder:New Folder
BOOL is_subfolder( LPCSTR srcname, LPCSTR destname )
{
	int len1 = strlen(srcname), len2 = strlen(destname);
	BOOL result = FALSE;

	if(len2 > len1) {
		if(strnicmp(srcname,destname,len1) == 0) {
			char ch = destname[len1];
			if(ch == ':' || ch == '\\' || ch == 0) {
				result = TRUE;
			}
		}
	}

	return(result);
}

// Temp:New Folder:xxx     Temp:New Folder
BOOL is_original_folder( LPCSTR srcname, LPCSTR destname )
{
	int len1 = strlen(srcname), len2 = strlen(destname);
	BOOL result = FALSE;

	if(len2 < len1) {
		if(strnicmp(srcname,destname,len2) == 0) {
			char ch = srcname[len2];
			if(ch == 0) {
				result = TRUE; // not reached
			} else if(ch == ':' && !strchr(&srcname[len2+1],':')) {
				result = TRUE;
			} else if(ch == '\\' && !strchr(&srcname[len2+1],'\\')) {
				result = TRUE;
			}
		}
	}

	return(result);
}

int CHFVExplorerListView::copy_item_to_dir( 
	int item, 
	HTREEITEM htree, 
	int action,
	CString *input_path,
	int input_isdir,
	int input_type,
	LPCSTR input_volpath,
	int input_volinx
)
{
	CHFVExplorerApp	*pApp;
	CTreeCtrl *tc;
	HFVExplorerTreeView *tv;
	// int dtype;
	int ok=0, volinx, isdir, was_moved;
	int hfs_source = 0, hfs_dest = 0;
	DWORD data;

	char srcvolpath[_MAX_PATH];
	char destvolpath[_MAX_PATH];
	char *srcname;
	char destname[_MAX_PATH];
	CString cs;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	tv = pApp->m_tree;
	tc = tv->m_ct;
	data = tc->GetItemData( htree );
	volinx = tv->get_lparam_volinx( data );

	// Say it's ok, for multi-selection user errors
	if(tv->get_lparam_type(data) == LIST_SPEC) return(1);
	if(item >= 0) {
		input_type = get_lparam_type(m_cl->GetItemData(item));
		if(input_type == LIST_SPEC) return(1);
	} else {
		if(input_type == LIST_SPEC) return(1);
	}

	if(get_lparam_type(data) == LIST_HFS) {
		hfs_dest = 1;
		cs = ((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name;
	} else {
		hfs_dest = 0;
		cs = ((CHFVExplorerDoc*)m_pDocument)->m_fats[volinx].m_volume_name;
	}
	lstrcpy( destvolpath, cs.GetBuffer( _MAX_PATH ) );

	int ismac;
	cs = ((CHFVExplorerDoc*)m_pDocument)->get_directory( htree, &ismac );
	lstrcpy( destname, cs.GetBuffer( _MAX_PATH ) );

	hfs_source = input_type == LIST_HFS;

	if(item >= 0) {
		get_path( item, cs );
		isdir = get_lparam_isdir(m_cl->GetItemData(item));
		inx2volname( item, srcvolpath, _MAX_PATH );
		input_volinx = get_lparam_volinx(m_cl->GetItemData(item));
	} else {
		cs = *input_path;
		isdir = input_isdir;
		strcpy( srcvolpath, input_volpath );
	}
	srcname = cs.GetBuffer( _MAX_PATH );

	CString ActionString = action == ACTION_MOVE ? "move" : "copy";

	if(input_type == tv->get_lparam_type(data) && input_volinx == volinx) {
		if(isdir) {
			if(is_same_folder(srcname,destname)) {
				AfxMessageBox( 
					"Cannot " + ActionString + " \"" + CString(srcname) + "\" to folder \"" + CString(destname) + "\". The destination folder is the same as the source folder."
				);
				return(0);
			}
			if(is_subfolder(srcname,destname)) {
				AfxMessageBox( 
					"Cannot " + ActionString + " \"" + CString(srcname) + "\" to folder \"" + CString(destname) + "\". The destination folder is a subfolder of the source folder."
				);
				return(0);
			}
		}
		if(is_original_folder(srcname,destname)) {
			AfxMessageBox( 
				"Cannot " + ActionString + " \"" + CString(srcname) + "\" to folder \"" + CString(destname) + "\". Duplication within same folder is not implemented."
			);
			return(0);
		}
	}

	if(hfs_dest != hfs_source) {
		if(!ask_set_copy_mode( hfs_source, cs )) {
			return(0);
		}
	}

	if( 0 == HFSIFACE_copy( 
		srcvolpath, 
		srcname, 
		destvolpath, 
		destname,
		isdir,
		(action == ACTION_MOVE),
		&was_moved )) 
	{
		update_floppy_view(destvolpath);
		update_floppy_view(srcvolpath);
		ok = 1;
	}
	return(ok);
}

int fat_rename( char *pOldName, char *pNewName )
{
	int result = 0;
	TRY
	{
		CFile::Rename( pOldName, pNewName );
		result = 1;
	}
	CATCH( CFileException, e )
	{
		result = 0;
	}
	END_CATCH
	return( result );
}

int CHFVExplorerListView::do_rename( 
	int item, 
	char *old_name,
	char *new_name,
	int check_lock
)
{
	DWORD ds = m_cl->GetItemData( item );
	char volpath[_MAX_PATH];
	char old_path[_MAX_PATH];
	char new_path[_MAX_PATH];
	int result = 0;
	int ismac, len;
	char tlet[2];

	if(!ds || get_lparam_type(ds) == LIST_SPEC) return(0);

	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	HTREEITEM hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();
	if(!hItem) return(0);

	CString cs = ((CHFVExplorerDoc*)m_pDocument)->get_directory( hItem, &ismac );
	lstrcpy( old_path, cs.GetBuffer( _MAX_PATH ) );
	lstrcpy( new_path, cs.GetBuffer( _MAX_PATH ) );

	tlet[0] = (get_lparam_type(ds) == LIST_HFS) ? ':' : '\\';
	tlet[1] = 0;

	len = lstrlen(old_path);
	if(len && old_path[len-1] != *tlet) lstrcat(old_path,tlet);

	len = lstrlen(new_path);
	if(len && new_path[len-1] != *tlet) lstrcat(new_path,tlet);

	lstrcat( old_path, old_name );
	lstrcat( new_path, new_name );

	if(get_lparam_type(ds) == LIST_HFS) {
		inx2volname( item, volpath, _MAX_PATH );
		if(check_lock) {
			hfsdirent ent;
			if(0 == HFSIFACE_get_properties( volpath, old_path, &ent )) {
				if((ent.fdflags & HFS_FNDR_NAMELOCKED) != 0) {
					AfxMessageBox( 
						"You must remove namelock before you can rename this item. "
						"Click with right mouse button, select Properties and uncheck \"Is name-locked\"."
					);
					return(0);
				}
			}
		}
		result = (0 == HFSIFACE_rename( volpath, old_path, new_path ));
		if(result) update_floppy_view(volpath);
	} else {
		result = fat_rename( old_name, new_name );
	}
	return(result);
}


int CHFVExplorerListView::do_file_to_folder( 
	int source, int dest, int action
)
{
	DWORD ds = m_cl->GetItemData( source );
	DWORD dd = m_cl->GetItemData( dest );
	char srcvolpath[_MAX_PATH];
	char destvolpath[_MAX_PATH];
	char *srcname;
	char *destname;
	CString cs, cd;
	int isdir, was_moved;

	if(!ds || ! dd) return(0);
	if(get_lparam_type(ds) == LIST_SPEC) return(1);

	get_path( source, cs );
	get_path( dest, cd );

	if(get_lparam_type(dd) == LIST_SPEC && 
	   get_lparam_id(dd) == ID_GOTO_PARENT)
	{
		CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
		HTREEITEM hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();
		if(hItem) {
			hItem = pApp->m_tree->GetTreeCtrl().GetParentItem( hItem );
			if(hItem) {
				return( copy_item_to_dir( source, hItem, action, 0, 0, 0, 0, 0 ) );
			}
		}
		return(0);
	}

	srcname = cs.GetBuffer( _MAX_PATH );
	destname = cd.GetBuffer( _MAX_PATH );

	inx2volname( source, srcvolpath, _MAX_PATH );
	inx2volname( dest, destvolpath, _MAX_PATH );

	if(action == ACTION_ASK) {
		action = ACTION_COPY;
	}

	isdir = get_lparam_isdir(m_cl->GetItemData(source));
	if( 0 == HFSIFACE_copy( 
		srcvolpath, 
		srcname, 
		destvolpath, 
		destname,
		isdir,
		(action == ACTION_MOVE),
		&was_moved )) 
	{
		update_floppy_view(destvolpath);
		update_floppy_view(srcvolpath);
		return(1);
	}
	return(0);
}

int CHFVExplorerListView::is_mac_application( int inx )
{
	CString type = m_cl->GetItemText( inx, FIELD_TYPE );
	return( type == "APPL" );
}

int is_pc_application( CString &app_name )
{
	char *s;
	int i;

	s = app_name.GetBuffer(MAX_PATH);
	if(is_extension( s, ".EXE" )) i = 1;
	else if(is_extension( s, ".COM" )) i = 1;
	else if(is_extension( s, ".BAT" )) i = 1;
	else i = 0;
	app_name.ReleaseBuffer();

	return(i);
}

int CHFVExplorerListView::is_mac_item( int inx )
{
	CString type;

	type = m_cl->GetItemText( inx, FIELD_TYPE );
	return( type != "" );
}

int CHFVExplorerListView::new_bowser_on( CString start_dir )
{
	CHFVExplorerApp	*pApp;
	char modulex[MAX_PATH];
	HINSTANCE hi;
	LPSTR p, dir = NULL;
	char params[MAX_PATH];

	pApp = (CHFVExplorerApp *)AfxGetApp();

	GetModuleFileName( pApp->m_hInstance, modulex, MAX_PATH-1 );
	p = start_dir.GetBuffer(MAX_PATH);
	wsprintf( params, "\"@!%ld %s\"", (long)pApp->m_main->m_hWnd, p );
	start_dir.ReleaseBuffer();
	hi = ShellExecute( this->m_hWnd, "open", modulex, params, dir, SW_SHOW );
	if( hi <= (HINSTANCE)32 ) {
		CString errstr;
		get_execute_error_string( hi, errstr );
		AfxMessageBox( "Could not start \"" + 
			CString(modulex) + "\":\n" + errstr + ".", MB_OK | MB_ICONSTOP );
		return(0);
	//} else {
	//	pApp->add_child( hi );
	}
	return(1);
}

void make_configur_path( char *cmd, CString *ppath, CString &creator )
{
	char small[10];
	char name[300];
	unsigned char *cr;
	char *p;
	int i;

	strcpy( name, cmd );
	*ppath = "";
	p = strchr( name, ' ' );
	if(p) {
		p = strrchr( p, '\\' );
		if(p) {
			*p = 0;
			strcat( name, "\\CONFIGUR\\" );
			cr = (unsigned char *)creator.GetBuffer(MAX_PATH);
			for(i=0; i<4; i++) {
				sprintf( small, "%02x", (int)cr[i] );
				strcat( name, small );
			}
			creator.ReleaseBuffer();
			strcat( name, ".ECF" );
			*ppath = CString( name );
		}
	}
}

#if 0
"Spin"
// This Configuration file (5370696e.ecf) was built by Executor
Comments = "Color Globe";
WindowName = "Globe";
BitsPerPixel = 1;
ScreenLocation = { 0, 0 };
SystemVersion = 6.0.7;
RefreshNumber = 0;
Delay = 0;
Options = {BlitInOSEvent, SoundOn, PassPostscript, NewLineToCR, NoWarn32};
#endif

void get_ecf_entry( char *buf, char *entryname, char *entryval )
{
	int i;
	char *p;

	*entryval = 0;
	while( *buf ) {
		if(strncmp( buf, entryname, strlen(entryname)) == 0) {
			p = strchr(buf, '=');
			if(p) {
				p++;
				while(*p == ' ') p++;
				i = 0;
				while(*p != 0 && *p != ';') {
					entryval[i++] = *p++;
				}
				entryval[i] = 0;
			}
			return;
		} else {
		}
	}
}

void patch_entry( char *cmd, char *name, char *entry )
{
	char *p1, *p2;

	p1 = strstr( cmd, name );
	if(p1) {
		// must delete old "-bpp x"
		p2 = strchr( p1, ' ');
		if(p2) {
			while(*p2 == ' ') p2++;
			if(*p2) {
				while(*p2 != 0 && *p2 != ' ') p2++;
				if(*p2) {
					// range is now [p1,p2)
					while(*p2) {
						*p1++ = *p2++;
					}
					*p1++ = 0;
				}
			}
		}
	}
	strcat( cmd, name );
	strcat( cmd, " " );
	strcat( cmd, entry );
	strcat( cmd, " " );
}

void try_patch_bpp( char *cmd, CString &creator )
{
	CString path;
	CFile f;
	char entry[200];

	make_configur_path( cmd, &path, creator );
	if(path != "" && do_open_file( &f, path )) {
		DWORD nCount = f.GetLength();
		int bytes_read;
		char *lpBuf = new char[nCount+100];
		if(lpBuf) {
			bytes_read = (int)f.Read( lpBuf, nCount );
			if(bytes_read == (int)nCount) {
				lpBuf[nCount] = 0;
				get_ecf_entry( lpBuf, "BitsPerPixel", entry );
				if(*entry) {
					patch_entry( cmd, "-bpp", entry );
				}
				get_ecf_entry( lpBuf, "SystemVersion", entry );
				if(*entry) {
					patch_entry( cmd, "-system", entry );
				}
			}
			delete [] lpBuf;
		}
		silent_close( &f );
	}
}

int CHFVExplorerListView::do_open_with_mac( CString document, CString application, CString creator )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString CmdLine;
	char cmd[MAXCMDLEN];
	int len;

	// CmdLine = CString( "X:\\Emulator\\MAC\\EXECUTOR\\Executor.exe -keyboard Suomi -skipdrives AB " );

	char *p = pApp->m_executor_win32_path.GetBuffer( _MAX_PATH );
	strcpy( cmd, p );
	pApp->m_executor_win32_path.ReleaseBuffer();

	len = strlen(cmd);
	if(len > 0 && cmd[len-1] != ' ') strcat( cmd, " " );

#if 0
	if( creator != "" ) {
		try_patch_bpp( cmd, creator );
	}
#endif

	CmdLine = CString(cmd);

	if(application != "") {
		if(is_mac_path(application)) {
			CmdLine += "\"" + make_mac_chrset(application) + CString("\" ");
		} else {
			CString first;
			macish( &application );
			application.OemToAnsi();
			first = application.Left(1);
			first.MakeLower();
			application = first + application.Mid(1);
			CmdLine += "\"" + application + CString("\" ");
		}
	}
	if(is_mac_path(document)) {
		CmdLine += "\"" + make_mac_chrset(document) + "\"";
	} else {
		CString first;
		macish( &document );
		document.OemToAnsi();
		first = document.Left(1);
		first.MakeLower();
		document = first + document.Mid(1);
		CmdLine += "\"" + document + "\"";
	}
	// macish( &CmdLine );
	// CmdLine.OemToAnsi();

	// AfxMessageBox( CmdLine );

	UINT err = WinExec( CmdLine, SW_SHOW );
	if(err <= 31) {
		CString msg;
		msg = "Could not start " + CString(CmdLine) +
					".\nPlease use execut95.exe to define Executor path and options.";
		AfxMessageBox( msg );
		return(0);
	}
	return(1);
}

int CHFVExplorerListView::do_open_with_pc( CString document, CString application )
{
	HINSTANCE hi;

	if(application != "") {
		hi = ShellExecute( NULL,	"open",	application, document, NULL, SW_SHOW );
	} else {
		hi = ShellExecute( NULL,	"open",	document, NULL, NULL, SW_SHOW );
	}
	if( hi <= (HINSTANCE)32 ) {
		CString errstr;
		get_execute_error_string( hi, errstr );
		AfxMessageBox( "Could not open execute \"" + 
			application + " " + document + "\":\n" + errstr + ".", MB_OK | MB_ICONSTOP );
		return(0);
	}
	return(1);
}

int CHFVExplorerListView::do_open_with( CString document, CString application, int mac, CString creator )
{
	if(mac) 
		return( do_open_with_mac( document, application, creator ) );
	else
		return( do_open_with_pc( document, application ) );
}

int CHFVExplorerListView::do_open_with( int document, CString app_name, int mac, CString creator )
{
	CString doc_name;

	if(!get_path(document,doc_name)) return(0);
	if(mac) 
		return( do_open_with_mac( doc_name, app_name, creator ) );
	else
		return( do_open_with_pc( doc_name, app_name ) );
}

int CHFVExplorerListView::do_open_with( CString document, int application, CString creator )
{
	CString app_name;
	int mac = is_mac_item( application );

	if(mac) {
		if(!is_mac_application( application )) {
			AfxMessageBox( "You can only drop documents on applications." );
			return(0);
		}
	}

	if(!get_path(application,app_name)) return(0);
	/*
	if(!mac) {
		if(!is_pc_application( app_name )) {
			AfxMessageBox( "You can only drop documents on applications." );
			return(0);
		}
	}
	*/
	return(do_open_with( document, app_name, mac, creator ));
}

int CHFVExplorerListView::do_open_with( int document, int application, CString creator )
{
	CString doc_name;

	if(!get_path(document,doc_name)) return(0);
	return(do_open_with( doc_name, application, creator ));
}

void CHFVExplorerListView::get_icons_folder_name( char *name )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	char *p;

	GetModuleFileName( pApp->m_hInstance, name, _MAX_PATH );
	p = strrchr( name, '\\' );
	if(p) *p = 0;
	strcat( name, "\\Links" );
}

void CHFVExplorerListView::get_vmac_launcher_name( char *name )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	char *p;

	GetModuleFileName( pApp->m_hInstance, name, _MAX_PATH );
	p = strrchr( name, '\\' );
	if(p) *p = 0;
	strcat( name, "\\vMacLink.exe" );
}

int CHFVExplorerListView::make_icon_name( LPSTR lpszIconFile )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	char folder[_MAX_PATH];

	*lpszIconFile = 0;
	get_icons_folder_name( folder );
	if(!exists(folder)) {
		if(!CreateDirectory(folder,NULL)) {
			CString cs = "Cannot create the icon folder \"" + CString(folder) + "\".";
			AfxMessageBox( cs ); 
			return(0);
		} else {
			CString cs = "The folder \"" + CString(folder) + "\" was created. "
				"All shortcut icons (and E/DOS parameter files) will be in this folder.";
			AfxMessageBox( cs );
		}
	}
	int i = 0;
	do {
		sprintf( 
			lpszIconFile, 
			"%s\\Ic%06d.ico", 
			folder,
			++pApp->m_next_icon_index );
			if(++i > 1000) break; // panic out, use this whatever icon it was.
	} while( exists(lpszIconFile) );
	pApp->WriteIconIndex();
	return(1);
}

// assumes that "make_icon_name" was just called!
int CHFVExplorerListView::make_e_dos_param_file_name( LPSTR lpszParamsFile )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	char folder[_MAX_PATH];

	*lpszParamsFile = 0;
	get_icons_folder_name( folder );
	if(!exists(folder)) {
		CString cs = "Folder \"" + CString(folder) + "\" does not exist.";
		AfxMessageBox( cs ); 
		return(0);
	}
	sprintf( 
		lpszParamsFile, 
		"%s\\Ic%06d.txt", 
		folder,
		pApp->m_next_icon_index );
	return(1);
}

int CHFVExplorerListView::save_icon( int doc_inx, LPSTR lpszIconFile, int is_doc )
{
	DWORD dat;
	HICON hicon16 = 0, hicon32 = 0;
	int ret = 0;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	dat = m_cl->GetItemData( doc_inx );
	if(dat) {
		hicon16 = get_lparam_icon16(dat);
		hicon32 = get_lparam_icon32(dat);
		if(!hicon32) {
			if(is_doc) {
				hicon32 = pApp->LoadIcon(IDI_ICON9);
			} else {
				hicon32 = pApp->LoadIcon(IDI_ICON5);
			}
		}
		if(!hicon16) {
			if(is_doc) {
				hicon16 = pApp->LoadIcon(IDI_ICON9);
			} else {
				hicon16 = pApp->LoadIcon(IDI_ICON5);
			}
		}
		if(hicon16 && hicon32) {
			ret = SaveIcon( hicon16, hicon32, lpszIconFile );
		}
	}
	return(ret);
}

void pc_validate_charset( LPSTR s )
{
	int i, len = strlen(s);

	for(i=0; i<len; i++) {
		switch( s[i] ) {
			case '\\': s[i] = '_'; break;
			case '/':  s[i] = '-'; break;
			case ':':  s[i] = ';'; break;
			case '*':  s[i] = '@'; break;
			case '?':  s[i] = '!'; break;
			case '\"': s[i] = '-'; break;
			case '<':  s[i] = '['; break;
			case '>':  s[i] = ']'; break;
			case '|':  s[i] = '-'; break;
		}
	}
}

void make_shortcut_name( LPCSTR objpath, LPSTR linkname )
{
	char *p;
	int ismac;

	*linkname = 0;

	ismac = is_mac_path(CString(objpath));
	if(ismac) {
		p = strrchr( objpath, ':' );
	} else {
		p = strrchr( objpath, '\\' );
	}
	if(p) {
		p++;
		strcpy( linkname, p );
		if(ismac) {
			pc_validate_charset( linkname );
		} else {
			p = strrchr( linkname, '.' );
			if(p) *p = 0;
		}
	}
}

int CHFVExplorerListView::check_shortcut_installation( int emulator )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	switch( emulator ) {
		case ID_OWN_SHORTCUT_CREATE_E_DOS:
			if(pApp->m_executor_dos_path == "") return(0);
			break;
		case ID_OWN_SHORTCUT_CREATE_E_WIN32:
			if(pApp->m_executor_win32_path == "") return(0);
			break;
		case ID_OWN_SHORTCUT_CREATE_VMAC:
			if(pApp->m_vmac_path == "") return(0);
			if(pApp->m_vmac_startup_folder_name == "") return(0);
			if(pApp->m_vmac_system_file_path == "") return(0);
			break;
		case ID_OWN_SHORTCUT_CREATE_SS:
			if(pApp->m_shapeshifter_path == "") return(0);
			if(pApp->m_shapeshifter_startup_folder_name == "") return(0);
			if(pApp->m_shapeshifter_system_file_path == "") return(0);
			if(pApp->m_shapeshifter_preferences_file_path == "") return(0);
			break;
		case ID_OWN_SHORTCUT_CREATE_FUSION:
			if(pApp->m_fusion_startup_folder_name == "") return(0);
			if(pApp->m_fusion_system_file_path == "") return(0);
			break;
		case ID_OWN_SHORTCUT_WINDOWS:
			return(1);
			break;
	}
	return(1);
}

int CHFVExplorerListView::write_text_file( LPSTR name, LPSTR txt )
{
	int ret = 0;
	CFile f;
	if(name != "" && do_create_file( &f, name )) {
		TRY
		{
			char *crlf = "\r\n";
			f.Write( txt, strlen(txt) );
			f.Write( crlf, strlen(crlf) );
			ret = 1;
		}
		CATCH( CFileException, e )
		{
		}
		END_CATCH
		silent_close( &f );
	}
	return(ret);
}

/*
"My Document.lnk"
"My Document Win32.lnk"
"My Document (Win32).lnk"
"Win32 - My Document.lnk"
*/
void CHFVExplorerListView::add_link_tail( char *lpszPathLink, char *id )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	char basename[MAX_PATH];

	strcpy( basename, lpszPathLink );
	switch( pApp->m_link_name_style ) {
		case 0:
			break;
		case 1:
			sprintf( lpszPathLink, "%s %s", basename, id );
			break;
		case 2:
			sprintf( lpszPathLink, "%s (%s)", basename, id );
			break;
		case 3:
			sprintf( lpszPathLink, "%s - %s", id, basename );
			break;
	}
}

void make_short_name( CString &path )
{
	char *p, short_name[_MAX_PATH];

	p = path.GetBuffer( _MAX_PATH );
	*short_name = 0;
	GetShortPathName( p, short_name, _MAX_PATH );
	if(*short_name) lstrcpy( p, short_name );
	path.ReleaseBuffer();
}

int CHFVExplorerListView::do_make_shortcut_e_dos( int doc_inx, CString application, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	char lpszArguments[MAX_PATH];
	char param_file_name[MAX_PATH];
	int showCmd, retval = 0;
	int is_doc = (application != "");

	if(!get_path(doc_inx,document)) return(0);

	p = pApp->m_executor_dos_path.GetBuffer( _MAX_PATH );
	strcpy( lpszPathObj, p );
	pApp->m_executor_dos_path.ReleaseBuffer();

	*lpszArguments = 0;
	p = strchr( lpszPathObj, ' ' );
	if(p) {
		*p = 0;
		p++;
		while(*p == ' ') p++;
		strcpy( lpszArguments, p );
	}

	if(make_shortcut) {
		make_shortcut_name( document, lpszPathLink );
		add_link_tail( lpszPathLink, "DOS" );
		strcat( lpszPathLink, ".lnk" );
		make_shortcut_name( document, lpszDesc );
		make_icon_name( lpszIconFile );
		make_e_dos_param_file_name( param_file_name );
		if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
			CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
			AfxMessageBox( cs );
			*lpszIconFile = 0;
		}
	} else {
		GetModuleFileName( pApp->m_hInstance, param_file_name, _MAX_PATH );
		p = strrchr( param_file_name, '\\' );
		if(p) *p = 0;
		strcat( param_file_name, "\\TmpParam.txt" );
	}

	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWMAXIMIZED;

	CmdLine = CString(lpszArguments);
	if(CmdLine != "") CmdLine = CmdLine + " ";

	if(application != "") {
		if(is_mac_path(application)) {
			CmdLine += "\"" + make_mac_chrset(application) + CString("\" ");
		} else {
			CString first;
			macish( &application );
			application.OemToAnsi();
			first = application.Left(1);
			first.MakeLower();
			application = first + application.Mid(1);
			make_short_name( application );
			CmdLine += "\"" + application + CString("\" ");
		}
	}
	if(is_mac_path(document)) {
		CmdLine += "\"" + make_mac_chrset(document) + "\"";
	} else {
		CString first;
		macish( &document );
		document.OemToAnsi();
		first = document.Left(1);
		first.MakeLower();
		document = first + document.Mid(1);
		make_short_name( document );
		CmdLine += "\"" + document + "\"";
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );
	write_text_file( param_file_name, args );
	CmdLine.ReleaseBuffer();

	char param_string[MAX_PATH];
	*param_string = '@';
	GetShortPathName( param_file_name, &param_string[1], MAX_PATH-1 );
 
	if(make_shortcut) {
		HRESULT hs = CreateLinkDesktop(
			GetSafeHwnd(),
			lpszPathObj,
			lpszPathLink,
			lpszDesc,
			lpszIconFile,
			lpszWorkingDirectory,
			param_string,
			showCmd,
			TRUE
		);
		retval = (hs == 0);
	} else {
		HINSTANCE hinst = ShellExecute( 
			NULL,	
			"open",	
			lpszPathObj,
			param_string, 
			lpszWorkingDirectory, 
			showCmd
		);
		retval = (hinst > (HINSTANCE)32);
	}

	return(retval);
}

int CHFVExplorerListView::do_make_shortcut_e_win32( int doc_inx, CString application, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	char lpszArguments[MAX_PATH];
	int showCmd, retval = 0;
	int is_doc = (application != "");

	if(!get_path(doc_inx,document)) return(0);

	p = pApp->m_executor_win32_path.GetBuffer( _MAX_PATH );
	strcpy( lpszPathObj, p );
	pApp->m_executor_win32_path.ReleaseBuffer();

	*lpszArguments = 0;
	p = strchr( lpszPathObj, ' ' );
	if(p) {
		*p = 0;
		p++;
		while(*p == ' ') p++;
		strcpy( lpszArguments, p );
	}

	if(make_shortcut) {
		make_shortcut_name( document, lpszPathLink );
		add_link_tail( lpszPathLink, "Win32" );
		strcat( lpszPathLink, ".lnk" );
		make_shortcut_name( document, lpszDesc );
		make_icon_name( lpszIconFile );
		if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
			CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
			AfxMessageBox( cs );
			*lpszIconFile = 0;
		}
	}

	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWNORMAL;

	CmdLine = CString(lpszArguments);
	if(CmdLine != "") CmdLine = CmdLine + " ";

	if(application != "") {
		if(is_mac_path(application)) {
			CmdLine += "\"" + make_mac_chrset(application) + CString("\" ");
		} else {
			CString first;
			macish( &application );
			application.OemToAnsi();
			first = application.Left(1);
			first.MakeLower();
			application = first + application.Mid(1);
			CmdLine += "\"" + application + CString("\" ");
		}
	}
	if(is_mac_path(document)) {
		CmdLine += "\"" + make_mac_chrset(document) + "\"";
	} else {
		CString first;
		macish( &document );
		document.OemToAnsi();
		first = document.Left(1);
		first.MakeLower();
		document = first + document.Mid(1);
		CmdLine += "\"" + document + "\"";
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );
	if(make_shortcut) {
		HRESULT hs = CreateLinkDesktop(
			GetSafeHwnd(),
			lpszPathObj,
			lpszPathLink,
			lpszDesc,
			lpszIconFile,
			lpszWorkingDirectory,
			args,
			showCmd,
			TRUE
		);
		retval = (hs == 0);
	} else {
		HINSTANCE hinst = ShellExecute( 
			NULL,	
			"open",	
			lpszPathObj,
			args, 
			lpszWorkingDirectory, 
			showCmd
		);
		retval = (hinst > (HINSTANCE)31);
		if(retval) {
			pApp->m_main->WaitEmulator( "Exewin32", "exec_win32" );
		}
	}
	CmdLine.ReleaseBuffer();

	return(retval);
}

void CHFVExplorerListView::update_callback(
	LPSTR doc_win_name,
	LPSTR destvolpath,
	LPSTR destname,
	BOOL copy_back
)
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	int copyret, was_moved;
	BOOL do_delete = TRUE, do_update_finder = TRUE;
	hfsdirent ent, ent2;

	if(copy_back) {
		if(0 != HFSIFACE_get_properties( destvolpath, destname, &ent )) {
			do_update_finder = FALSE;
		}
		HFSIFACE_set_modein( 4, pApp->m_copyin_appledouble );
		copyret = HFSIFACE_copy( 
			doc_win_name, 
			doc_win_name,
			destvolpath, 
			destname,
			FALSE,
			FALSE,
			&was_moved
		);
		if( copyret != 0 ) {
			do_delete = FALSE;
			AfxMessageBox( "Failed to copy the file back. If you need the changes, the temporary file \"" + CString(doc_win_name) + "\" is still available." );
		} else if(do_update_finder) {
			if(0 == HFSIFACE_get_properties( destvolpath, destname, &ent2 )) {
				ent2.fdlocation.h = (signed short)( unmagic(ent.fdlocation.h) - FINDER_MAGIC_OFFSET );
				ent2.fdlocation.v = (signed short)( unmagic(ent.fdlocation.v) - FINDER_MAGIC_OFFSET );
				memcpy( &ent2.u.file.type, ent.u.file.type, 5 );
				memcpy( &ent2.u.file.creator, ent.u.file.creator, 5 );
				ent2.crdate = ent.crdate;
				ent2.fdflags &= ~HFS_FNDR_HASBEENINITED;
				HFSIFACE_set_properties( destvolpath, destname, &ent2 );
			}
			update_floppy_view(destvolpath);
		}
	}
	if(do_delete) {
		SetFileAttributes( doc_win_name, FILE_ATTRIBUTE_NORMAL );
		DeleteFile(doc_win_name);
	}
}

int CHFVExplorerListView::do_make_shortcut_windows( int doc_inx, CString type, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	int retval = 0;
	CString document;
	char extension[_MAX_PATH], *old_extension,
			 doc_win_name[_MAX_PATH],
			 doc_win_folder[_MAX_PATH], srcvolpath[_MAX_PATH], *srcname;

	if(!get_path(doc_inx,document)) return(0);

	srcname = document.GetBuffer(_MAX_PATH);
	old_extension = strrchr( srcname, '.' );
	document.ReleaseBuffer();

	if(!old_extension || strlen(old_extension) > 6) {
		if( !tmap_hfs2fat( (LPCSTR)type, (LPCSTR)creator, extension ) ) {
			AfxMessageBox( 
				"There is no file type mapping from (" + type + "," + creator +
				"). Please define it in the dialog that will open next."
			);
			if(!map_selected_item()) {
				return(FALSE);
			}
		}

		if( !tmap_hfs2fat( (LPCSTR)type, (LPCSTR)creator, extension ) ) {
			return(FALSE);
		}
	}

	if(!GetTempPath( sizeof(doc_win_folder), doc_win_folder )) {
		AfxMessageBox( "Could not find the directory for temporary files. Check your TMP or TEMP environment variables." );
		return(FALSE);
	}
	// GetTempPath always returns a trailing backslash.
	doc_win_folder[strlen(doc_win_folder)-1] = 0;
	strcpy( doc_win_name, doc_win_folder );

	inx2volname( doc_inx, srcvolpath, _MAX_PATH );

	HFSIFACE_set_modeout(4);

	int was_moved, copyret;
	srcname = document.GetBuffer(_MAX_PATH);

	copyret = HFSIFACE_copy( 
		srcvolpath, 
		srcname,
		doc_win_name, 
		doc_win_name,
		FALSE,
		FALSE,
		&was_moved
	);
	if( copyret != 0 ) {
		AfxMessageBox( "Failed to copy the file." );
		return(FALSE);
	}
	document.ReleaseBuffer();

	copyout_get_last_open_file( doc_win_name, sizeof(doc_win_name) );

	DWORD attribs = FILE_ATTRIBUTE_TEMPORARY;
	if(mount_flags(srcvolpath) == 0) {
		attribs |= FILE_ATTRIBUTE_READONLY;
	}
	SetFileAttributes( doc_win_name, attribs );

	/*
	char app_win_path[_MAX_PATH];
	HINSTANCE hinst = FindExecutable(
		doc_win_name,
		doc_win_folder,
		app_win_path
	);
	if(hinst <= (HINSTANCE)32) {
		AfxMessageBox( 
			"There is no Windows association for file extension \"" + CString(extension) + "\"."
		);
		update_callback( doc_win_name, "", "", FALSE );
		return(FALSE);
	}
	*/

	SHELLEXECUTEINFO sh;
	memset( &sh, 0, sizeof(SHELLEXECUTEINFO) );
  sh.cbSize = sizeof(SHELLEXECUTEINFO);
	sh.hwnd = GetSafeHwnd();
  sh.lpVerb = "open",	

  sh.lpFile = doc_win_name;
  sh.lpParameters = "";

	/*
  sh.lpFile = app_win_path;
  sh.lpParameters = doc_win_name;
	*/

  sh.lpDirectory = doc_win_folder;
  sh.nShow = SW_SHOWNORMAL; //SW_SHOWMAXIMIZED;
	sh.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellExecuteEx( &sh );

	if(sh.hInstApp <= (HINSTANCE)32) {
		AfxMessageBox( 
			"Could not start \"" + CString(doc_win_name) + "\"."
		);
		update_callback( doc_win_name, "", "", FALSE );

		return(FALSE);
	} else {
		pApp->m_main->WaitEmulator2( sh.hProcess, doc_win_name, srcvolpath, document );
	}

	return(retval);
}

int CHFVExplorerListView::do_make_shortcut_vmac( int doc_inx, CString application, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	int showCmd, volinx;
	int is_doc = (application == "D:"), retval = 0;

	if(pApp->m_launch_method == 0) {
		application = CString("S:");
	}

	if(!get_path(doc_inx,document)) return(0);

	volinx = get_lparam_volinx(m_cl->GetItemData(doc_inx));

	get_vmac_launcher_name( lpszPathObj );

	if(make_shortcut) {
		make_shortcut_name( document, lpszPathLink );
		add_link_tail( lpszPathLink, "vMac" );
		strcat( lpszPathLink, ".lnk" );
		make_shortcut_name( document, lpszDesc );
		make_icon_name( lpszIconFile );
		if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
			CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
			AfxMessageBox( cs );
			*lpszIconFile = 0;
		}
	}

	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWMAXIMIZED;

	if(is_mac_path(document)) {
		CmdLine = 
						((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name + 
						CString("?") +
						application + 
						make_mac_chrset(document);
	} else {
		AfxMessageBox( "Objects on FAT volumes cannot be launched to vMac. Please select a target that is located on a DSK or HFV volume file." );
		return(0);
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );
	if(make_shortcut) {
		HRESULT hs = CreateLinkDesktop(
			GetSafeHwnd(),
			lpszPathObj,
			lpszPathLink,
			lpszDesc,
			lpszIconFile,
			lpszWorkingDirectory,
			args,
			showCmd,
			TRUE
		);
		retval = (hs == 0);
	} else {
		HINSTANCE hinst = ShellExecute( 
			NULL,	
			"open",	
			lpszPathObj,
			args, 
			lpszWorkingDirectory, 
			showCmd
		);

		retval = (hinst > (HINSTANCE)31);
		if(retval) {
			pApp->m_main->WaitEmulator( "vMac", "vMac Win32" );
		}
		/*
		if(retval) {
			if(pApp->GetProfileInt("Setup","TerminateWarningIssued",0) == 0) {
				pApp->WriteProfileInt("Setup","TerminateWarningIssued",1);
				CString cs = "For your safety, HFVExplorer will now quit.";
				AfxMessageBox( cs );
			}
			PostQuitMessage(0);
		}
		*/
	}
	CmdLine.ReleaseBuffer();
	return(retval);
}

int CHFVExplorerListView::do_make_shortcut_ss( int doc_inx, CString application, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	int showCmd, volinx;
	int is_doc = (application == "D:"), retval = 0;

	if(pApp->m_launch_method == 0) {
		application = CString("S:");
	}

	if(!get_path(doc_inx,document)) return(0);

	volinx = get_lparam_volinx(m_cl->GetItemData(doc_inx));

	get_vmac_launcher_name( lpszPathObj );

	if(make_shortcut) {
		make_shortcut_name( document, lpszPathLink );
		add_link_tail( lpszPathLink, "ShapeShifter" );
		strcat( lpszPathLink, ".lnk" );
		make_shortcut_name( document, lpszDesc );
		make_icon_name( lpszIconFile );
		if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
			CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
			AfxMessageBox( cs );
			*lpszIconFile = 0;
		}
	}

	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWMAXIMIZED;

	if(is_mac_path(document)) {
		CmdLine = 
						CString("-SS ") +
						((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name + 
						CString("?") +
						application + 
						make_mac_chrset(document);
	} else {
		AfxMessageBox( "Objects on FAT volumes cannot be launched to ShapeShifter. Please select a target that is located on a DSK or HFV volume file." );
		return(0);
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );
	if(make_shortcut) {
		HRESULT hs = CreateLinkDesktop(
			GetSafeHwnd(),
			lpszPathObj,
			lpszPathLink,
			lpszDesc,
			lpszIconFile,
			lpszWorkingDirectory,
			args,
			showCmd,
			TRUE
		);
		retval = (hs == 0);
	} else {
		HINSTANCE hinst = ShellExecute( 
			NULL,	
			"open",	
			lpszPathObj,
			args, 
			lpszWorkingDirectory, 
			showCmd
		);

		retval = (hinst > (HINSTANCE)31);
		if(retval) {
			// pApp->m_main->WaitEmulator( "", "" );
		}
	}
	CmdLine.ReleaseBuffer();
	return(retval);
}

int CHFVExplorerListView::do_make_shortcut_fusion( int doc_inx, CString application, CString creator, int make_shortcut )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	int showCmd, volinx;
	int is_doc = (application == "D:"), retval = 0;

	if(pApp->m_launch_method == 0) {
		application = CString("S:");
	}

	if(!get_path(doc_inx,document)) return(0);

	volinx = get_lparam_volinx(m_cl->GetItemData(doc_inx));

	get_vmac_launcher_name( lpszPathObj );

	if(make_shortcut) {
		make_shortcut_name( document, lpszPathLink );
		add_link_tail( lpszPathLink, "Fusion" );
		strcat( lpszPathLink, ".lnk" );
		make_shortcut_name( document, lpszDesc );
		make_icon_name( lpszIconFile );
		if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
			CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
			AfxMessageBox( cs );
			*lpszIconFile = 0;
		}
	}

	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWMAXIMIZED;

	if(is_mac_path(document)) {
		CmdLine = 
						CString("-FU ") +
						((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name + 
						CString("?") +
						application + 
						make_mac_chrset(document);
	} else {
		AfxMessageBox( "Objects on FAT volumes cannot be launched to Fusion. Please select a target that is located on a DSK or HFV volume file." );
		return(0);
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );
	if(make_shortcut) {
		HRESULT hs = CreateLinkDesktop(
			GetSafeHwnd(),
			lpszPathObj,
			lpszPathLink,
			lpszDesc,
			lpszIconFile,
			lpszWorkingDirectory,
			args,
			showCmd,
			TRUE
		);
		retval = (hs == 0);
	} else {
		HINSTANCE hinst = ShellExecute( 
			NULL,	
			"open",	
			lpszPathObj,
			args, 
			lpszWorkingDirectory, 
			showCmd
		);
		retval = (hinst > (HINSTANCE)31);
		if(retval) {
			// pApp->m_main->WaitEmulator( "", "" );
		}
	}
	CmdLine.ReleaseBuffer();
	return(retval);
}

/*
int CHFVExplorerListView::do_make_shortcut( int doc_inx, CString application, CString creator, int emulator )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CString document;
	CString CmdLine;
	char *p;

	char lpszPathObj[MAX_PATH];
  char lpszPathLink[MAX_PATH];
	char lpszDesc[MAX_PATH];
	char lpszIconFile[MAX_PATH];
	char lpszWorkingDirectory[MAX_PATH];
	char lpszArguments[MAX_PATH];
	int showCmd;

	if(!get_path(doc_inx,document)) return(0);

	if(!check_shortcut_installation(emulator)) {
		AfxMessageBox( "Please check the program locations and parameters in Options dialog before making shortcuts." );
		return(0);
	}
	
	switch( emulator ) {
		case ID_OWN_SHORTCUT_CREATE_E_DOS:
			p = pApp->m_executor_dos_path.GetBuffer( _MAX_PATH );
			strcpy( lpszPathObj, p );
			pApp->m_executor_dos_path.ReleaseBuffer();
			break;
		case ID_OWN_SHORTCUT_CREATE_E_WIN32:
			p = pApp->m_executor_win32_path.GetBuffer( _MAX_PATH );
			strcpy( lpszPathObj, p );
			pApp->m_executor_win32_path.ReleaseBuffer();
			break;
		case ID_OWN_SHORTCUT_CREATE_VMAC:
			get_vmac_launcher_name( lpszPathObj );
			// (void)ShellExecute( NULL,	"open",	"I:\\EXECUTOR\\vTest\\vMacW32.EXE.lnk", NULL, NULL, SW_SHOWNORMAL );
			break;
	}

	if(*lpszPathObj == 0) {
		AfxMessageBox( "Please run \"Execut95\" first to define your Executor path." );
	}

	*lpszArguments = 0;
	p = strchr( lpszPathObj, ' ' );
	if(p) {
		*p = 0;
		p++;
		while(*p == ' ') p++;
		strcpy( lpszArguments, p );
	}

	make_shortcut_name( document, lpszPathLink );
	add_link_tail( lpszPathLink, "xxx" );
	strcat( lpszPathLink, ".lnk" );
	make_shortcut_name( document, lpszDesc );
	strcpy( lpszWorkingDirectory, lpszPathObj );
	p = strrchr( lpszWorkingDirectory, '\\' );
	if(p) *p = 0;
	showCmd = SW_SHOWMAXIMIZED;

	int is_doc;
	if(emulator == ID_OWN_SHORTCUT_CREATE_VMAC) {
		is_doc = (application == "D:");
	} else {
		is_doc = (application != "");
	}

	make_icon_name( lpszIconFile );
	if(!save_icon( doc_inx, lpszIconFile, is_doc )) {
		CString cs = "Failed to save the icon to file \"" + CString(lpszIconFile) + ".\"";
		AfxMessageBox( cs );
		*lpszIconFile = 0;
	}

	CmdLine = CString(lpszArguments);
	if(CmdLine != "") CmdLine = CmdLine + " ";

	if(emulator == ID_OWN_SHORTCUT_CREATE_VMAC) {
	} else {
	}
	if(application != "") {
		if(is_mac_path(application)) {
			CmdLine += "\"" + make_mac_chrset(application) + CString("\" ");
		} else {
			CString first;
			macish( &application );
			application.OemToAnsi();
			first = application.Left(1);
			first.MakeLower();
			application = first + application.Mid(1);
			CmdLine += "\"" + application + CString("\" ");
		}
	}
	if(is_mac_path(document)) {
		CmdLine += "\"" + make_mac_chrset(document) + "\"";
	} else {
		CString first;
		macish( &document );
		document.OemToAnsi();
		first = document.Left(1);
		first.MakeLower();
		document = first + document.Mid(1);
		CmdLine += "\"" + document + "\"";
	}

	LPSTR args = CmdLine.GetBuffer( MAX_PATH );

	HRESULT hs = CreateLinkDesktop(
		GetSafeHwnd(),
		lpszPathObj,
		lpszPathLink,
		lpszDesc,
		lpszIconFile,
		lpszWorkingDirectory,
		args,
		showCmd,
		TRUE
	);
	CmdLine.ReleaseBuffer();
	if(hs == 0) {
		AfxMessageBox( "The shortcut was placed on the Desktop." );
		return(1);
	} else {
		AfxMessageBox( "Failed to create the shortcut." );
		return(0);
	}
}
*/

int CHFVExplorerListView::do_make_shortcut( int doc_inx, CString application, CString creator, int emulator, int make_shortcut )
{
	int ret = 0;

	if(!check_shortcut_installation(emulator)) {
		AfxMessageBox( "Please check the program locations and parameters in Options dialog before making shortcuts." );
		return(0);
	}
	
	switch( emulator ) {
		case ID_OWN_SHORTCUT_CREATE_E_DOS:
			ret = do_make_shortcut_e_dos( doc_inx, application, creator, make_shortcut );
			break;
		case ID_OWN_SHORTCUT_CREATE_E_WIN32:
			ret = do_make_shortcut_e_win32( doc_inx, application, creator, make_shortcut );
			break;
		case ID_OWN_SHORTCUT_CREATE_VMAC:
			ret = do_make_shortcut_vmac( doc_inx, application, creator, make_shortcut );
			break;
		case ID_OWN_SHORTCUT_CREATE_SS:
			ret = do_make_shortcut_ss( doc_inx, application, creator, make_shortcut );
			break;
		case ID_OWN_SHORTCUT_CREATE_FUSION:
			ret = do_make_shortcut_fusion( doc_inx, application, creator, make_shortcut );
			break;
		case ID_OWN_SHORTCUT_WINDOWS:
			ret = do_make_shortcut_windows( doc_inx, application, creator, make_shortcut );
			break;
	}
	if(make_shortcut) {
		if(ret) {
			AfxMessageBox( "A shortcut was placed on your desktop." );
		} else {
			AfxMessageBox( "Failed to create a shortcut." );
		}
	}
	return(0);
}

void CHFVExplorerListView::go_up( int control )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	HTREEITEM hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();

	if(hItem) {
		hItem = pApp->m_tree->GetTreeCtrl().GetParentItem( hItem );
		if(hItem) {
			if(control) {
				CString where;
				int ismac;
				where = ((CHFVExplorerDoc*)m_pDocument)->get_directory(hItem, &ismac);
				if(where != "") {
					new_bowser_on( where );
				}
			} else {
				pApp->m_tree->GetTreeCtrl().EnsureVisible( hItem );
				pApp->m_tree->GetTreeCtrl().SelectItem( hItem );
			}
		}
	}
}

void CHFVExplorerListView::activate_list_item( int inx, int control, int make_shortcut ) 
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	unsigned long dat, id = 0;
	CString name, type, creator, host_program;
	int entry_type, isdir, volinx;
	char path[MAX_PATH];
	int emulator = 0;

	if(control < 0) emulator = -control;

	dat = m_cl->GetItemData( inx );
	entry_type = get_lparam_type(dat);
	id = get_lparam_id(dat);
	isdir = get_lparam_isdir(dat);
	volinx = get_lparam_volinx( dat );
	name = m_cl->GetItemText( inx, 0 );

	if(entry_type == LIST_SPEC) {
		if(make_shortcut) {
			AfxMessageBox( "You must be kidding." );
			return;
		}
		if(id == ID_GOTO_PARENT) {
			go_up( control );
		}
		return;
	}

	if(isdir) {
		if(make_shortcut) {
			AfxMessageBox( "Cannot make shortcuts to folders." );
			return;
		}
		HTREEITEM hItem = get_lparam_htree(dat);
		if(hItem) {
			if(control) {
				CString where;
				int ismac;
				where = ((CHFVExplorerDoc*)m_pDocument)->get_directory(hItem, &ismac);
				if(where != "") {
					new_bowser_on( where );
				}
			} else {
				pApp->m_tree->GetTreeCtrl().EnsureVisible( hItem );
				pApp->m_tree->GetTreeCtrl().SelectItem( hItem );
			}
		}
		return;
	}

	if(entry_type == LIST_UNKNOWN || entry_type == LIST_SPEC) return;

	type = m_cl->GetItemText( inx, FIELD_TYPE );

	if(make_shortcut && entry_type == LIST_FAT) {
		if(emulator == ID_OWN_SHORTCUT_CREATE_VMAC || emulator == ID_OWN_SHORTCUT_CREATE_SS) {
			AfxMessageBox( "vMac and ShapeShifter shortcuts cannot refer to DOS volumes." );
			return;
		}
		// Hmmm. could be AppleDouble shortcut. Go on.
	}

	if(entry_type == LIST_FAT && type == "") {
		if(make_shortcut) {
			AfxMessageBox( "Please make Windows shortcuts in Explorer." );
			return;
		}
		if(get_lparam_path(dat,path,MAX_PATH-1)) {
			pc_activate( path );
		}
		return;
	}

	if(entry_type == LIST_HFS && emulator == 0) {
		if(pApp->m_default_emulator == 0) emulator = ID_OWN_SHORTCUT_CREATE_E_DOS;
		else if(pApp->m_default_emulator == 1) emulator = ID_OWN_SHORTCUT_CREATE_E_WIN32;
		else if(pApp->m_default_emulator == 2) emulator = ID_OWN_SHORTCUT_CREATE_VMAC;
		else if(pApp->m_default_emulator == 3) emulator = ID_OWN_SHORTCUT_CREATE_SS;
		else if(pApp->m_default_emulator == 4) emulator = ID_OWN_SHORTCUT_CREATE_FUSION;
		else if(pApp->m_default_emulator == 5) emulator = ID_OWN_SHORTCUT_WINDOWS;
		else emulator = ID_OWN_SHORTCUT_CREATE_E_WIN32;
	}

	host_program = "";
	creator = m_cl->GetItemText( inx, FIELD_CREATOR );

	if(emulator == ID_OWN_SHORTCUT_CREATE_VMAC || emulator == ID_OWN_SHORTCUT_CREATE_SS) {
		if(type == "APPL") {
			host_program = "A:";
		} else {
			host_program = "D:";
		}
	} else if(entry_type == LIST_HFS && emulator == ID_OWN_SHORTCUT_WINDOWS) {
		if(type == "APPL") {
			AfxMessageBox( "You cannot launch Mac applications with Windows programs, only documents." );
			return;
		} else {
			host_program = type;
		}
	} else {
		if(type != "APPL") {
			LPSTR cr;
			cr = creator.GetBuffer(MAX_PATH);
			host_program = ((CHFVExplorerDoc*)m_pDocument)->hfs_find_match("APPL",cr,volinx);
			creator.ReleaseBuffer();
			if(host_program == "") {
				if(type == "TEXT" || type == "ttro" || type == "ttxt" ) {
					host_program = "System:Shareware:Tex-Edit:Tex-Edit";
				} else {
					AfxMessageBox( "Could not find association for " + name );
					return;
				}
			} else {
				//AfxMessageBox( "Found " + host_program );
				//return;
			}
		}
	}

	if(emulator) {
		do_make_shortcut( inx, host_program, creator, emulator, make_shortcut );
	} else {
		// Nowadays we fall here only on DOS volumes.
		do_open_with( inx, host_program, 1, creator );
	}
}

void CHFVExplorerListView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	LV_HITTESTINFO ht;

	ht.pt = point;
	if( m_cl->HitTest( &ht ) >= 0 ) {
		if( ht.flags == LVHT_ONITEMICON || ht.flags == LVHT_ONITEMLABEL ||
				ht.flags == LVHT_ONITEMSTATEICON) {
			activate_list_item( ht.iItem, (nFlags & MK_CONTROL) != 0, FALSE );
			return;
		}
	}
	
	CListView::OnLButtonDblClk(nFlags, point);
}

#pragma optimize("g",off)
void CHFVExplorerListView::update_current_view()
{
	CHFVExplorerApp	*pApp;
	CTreeCtrl *tc;
	HFVExplorerTreeView *tv;
	HTREEITEM htree;
	int volinx;
	char path[MAX_PATH];

	pApp = (CHFVExplorerApp *)AfxGetApp();
	tv = pApp->m_tree;
	tc = tv->m_ct;

	htree = tc->GetSelectedItem();
	DWORD dw = tc->GetItemData( htree );
	if(tv->get_lparam_path( dw, path, MAX_PATH ) && m_pDocument) {
		volinx = tv->get_lparam_volinx( dw );
		if(volinx >= 0) {
			((CHFVExplorerDoc*)m_pDocument)->open_fat_directory( volinx, path, 1 );
		}
	}
}
#pragma optimize("",on)

void CHFVExplorerListView::inx2volname( 
	int inx, 
	char *volpath, 
	int size )
{
	unsigned long dat = m_cl->GetItemData( inx );
	int volinx = get_lparam_volinx( dat );
	CString cs;

	if(get_lparam_type(dat) == LIST_HFS) {
		cs = ((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name;
	} else {
		// cs = ((CHFVExplorerDoc*)m_pDocument)->m_fats[volinx].m_volume_name;
		get_path( inx, cs );
	}
	char *p = cs.GetBuffer( size );
	strcpy( volpath, p );
	cs.ReleaseBuffer();
}

/*
int do_delete_fat_directory( char *path )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok;
	char mask[_MAX_PATH];
	char new_source[_MAX_PATH];
	int result = 0;

	sprintf( mask, "%s\\*.*", path );
	fh = FindFirstFile( mask, &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	while(ok) {
		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(*FindFileData.cFileName != '.') {
				sprintf( new_source, "%s\\%s", path, FindFileData.cFileName );
				result = do_delete_fat_directory( new_source );
				if(result != 0) break;
			}
		} else {
			sprintf( new_source, "%s\\%s", path, FindFileData.cFileName );
			if(!DeleteFile(new_source)) {
				AfxMessageBox( "Could not delete directory " + CString(new_source) );
				result = 1;
				break;
			}
		}
		ok = FindNextFile( fh, &FindFileData );
	}
	if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
	if(result == 0) {
		if(!RemoveDirectory(path)) {
			AfxMessageBox( "Could not delete directory " + CString(path) );
			result = 1;
		}
	}
	return(result);
}
*/

BOOL CHFVExplorerListView::map_selected_item( void )
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	CFileTypeMapping dlg;
	int inx, count;
	BOOL retval = FALSE;

	tmap_final();
	dlg.m_enabled_param = pApp->m_type_mapping_enabled;

	inx = -1; count = 0;
	while( (inx = m_cl->GetNextItem(inx,LVNI_SELECTED)) >= 0 ) {
		if(get_inx_type(inx) == LIST_HFS) {
			CString fullpath;
			char volpath[_MAX_PATH];
			hfsdirent ent;

			get_path( inx, fullpath );
			inx2volname( inx, volpath, _MAX_PATH );
			char *path = fullpath.GetBuffer( _MAX_PATH );

			if(0 == HFSIFACE_get_properties( volpath, path, &ent )) {
				if( (ent.flags & HFS_ISDIR) == 0 ) {
					ent.u.file.creator[4] = 0;
					ent.u.file.type[4] = 0;
					dlg.m_input_types.Add(CString( ent.u.file.type ));
					dlg.m_input_creators.Add(CString( ent.u.file.creator ));
					dlg.m_input_dos.Add(CString("???"));
					count++;
				}
			}
			fullpath.ReleaseBuffer();
		} else if(get_inx_type(inx) == LIST_FAT) {
			CString fullpath;
			get_path( inx, fullpath );
			if(!get_inx_isdir(inx)) {
				char extension[100];
				char *path = fullpath.GetBuffer( _MAX_PATH );
				get_extension( path, extension );
				strupr( extension );
				fullpath.ReleaseBuffer();
				if(*extension) {
					dlg.m_input_types.Add(CString("????"));
					dlg.m_input_creators.Add(CString("????"));
					dlg.m_input_dos.Add(CString(extension));
					count++;
				}
			}
		}
	}
	if(count > 0) {
		if (dlg.DoModal() == IDOK) {
			pApp->m_type_mapping_enabled = dlg.m_enabled_param;
			retval = TRUE;
		}
	} else {
		AfxMessageBox( "Please select at least one file to map. You cannot map folders or special buttons." );
	}
	tmap_init();
	return(retval);
}

#pragma optimize("g",off)
void CHFVExplorerListView::try_delete_selected_items( BOOL confirm )
{
	int inx = -1, count = 0;
	int stype, isdir;
	CString nam, fullpath;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	CDWordArray items;

	// Just get the count
	while( (inx = m_cl->GetNextItem(inx,LVNI_SELECTED)) >= 0 ) count++;

	if(count == 0) return;
	if(count == 1) {
		inx = m_cl->GetNextItem(-1,LVNI_SELECTED);
		stype = get_inx_type(inx);
		isdir = get_inx_isdir(inx);
		if(stype == LIST_SPEC) return;
		get_path( inx, fullpath );
		if(isdir) {
			nam = "Really delete folder \"" + fullpath + "\"?";
		} else {
			nam = "Really delete file \"" + fullpath + "\"?";
		}
	} else {
		nam.Format( "Really delete selected %d items?", count );
	}
	if(confirm) {
		if(AfxMessageBox( nam, MB_YESNO ) == IDNO ) 
		{
			return;
		}
	}

	BOOL prevTimer = pApp->EnableTimer( FALSE );
	// pApp->m_tree->enable_selchange(0);

	int fat_changed = 0;
	
	inx = -1; count = 0;
	while( (inx = m_cl->GetNextItem(inx,LVNI_SELECTED)) >= 0 ) {
		items.Add(inx);
		count++;
	}
	
	int i;
	for(i=0; i<count; i++) {
		inx = items[i];
		stype = get_inx_type(inx);
		isdir = get_inx_isdir(inx);
		if(stype == LIST_SPEC) continue;
		get_path( inx, fullpath );
	
		if(inx >= 0) {
			if(stype == LIST_FAT) {
				open_file_type s_fp( open_file_type::SystemFAT, inx );
				s_fp.Delete();
				fat_changed = 1;
				/*
				if(isdir) {
					char *path = fullpath.GetBuffer( _MAX_PATH );
					do_delete_fat_directory(path);
				} else {
					open_file_type s_fp( open_file_type::SystemFAT, inx );
					s_fp.Delete();
				}
				*/
			} else {
				int ret;
				char volpath[_MAX_PATH];

				inx2volname( inx, volpath, _MAX_PATH );
				char *path = fullpath.GetBuffer( _MAX_PATH );
				if(isdir) {
					ret = HFSIFACE_rmdir( volpath, path );
				} else {
					ret = HFSIFACE_delete( volpath, path );
				}
				fullpath.ReleaseBuffer();
				if(0 != ret) {
					AfxMessageBox( "Failed to delete from HFV volume" );
					break;
				} else {
					update_floppy_view(volpath);
					// Timer will take care.
					// update_current_view();
				}
			}
		}
	}
	if(fat_changed) update_current_view();
	pApp->EnableTimer( prevTimer );
	// pApp->m_tree->enable_selchange(1);
}
#pragma optimize("",on)

void CHFVExplorerListView::toggle_label_colors( void ) 
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	int inx, color;

	pApp->m_show_labels = !pApp->m_show_labels;

	inx = -1;
	color = FINDER_COLOR_NONE;

	while( (inx = m_cl->GetNextItem(inx,LVNI_ALL)) >= 0 ) {
		if(pApp->m_show_labels) {
			long dat = m_cl->GetItemData( inx );
			int finder_color = get_lparam_finder_color( dat );
			color = INDEXTOOVERLAYMASK(finder_color);
		}
		m_cl->SetItemState( inx, color, LVIS_OVERLAYMASK );
	}
}

void CHFVExplorerListView::set_label_color( int finder_color ) 
{
	int inx, color;
	LV_ITEM	lvitem;
	long dat;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();

	color = INDEXTOOVERLAYMASK(finder_color);
	inx = -1;
	memset( &lvitem, 0, sizeof(lvitem) );

	while( (inx = m_cl->GetNextItem(inx,LVNI_ALL|LVNI_SELECTED)) >= 0 ) {
		if(get_inx_type(inx) == LIST_HFS) {
			lvitem.mask = LVIF_TEXT;
			lvitem.iItem = inx;
			lvitem.iSubItem = FIELD_LABEL;
			lvitem.pszText = get_label_text( finder_color );
			lvitem.iImage = 0; // don't care
			m_cl->SetItem(&lvitem);
			dat = m_cl->GetItemData( inx );
			if(dat) {
				list_lparam_struct *p = (list_lparam_struct *)dat;
				p->finder_color = finder_color;
				dat = (long)p;
				m_cl->SetItemData(inx,dat);
			}

			CString fullpath;
			char volpath[_MAX_PATH];
			hfsdirent ent;

			get_path( inx, fullpath );
			inx2volname( inx, volpath, _MAX_PATH );
			char *path = fullpath.GetBuffer( _MAX_PATH );

			if(0 == HFSIFACE_get_properties( volpath, path, &ent )) {
				ent.fdflags &= ~kColorMask;
				ent.fdflags |= ((finder_color << 1) & kColorMask);
				if(0 == HFSIFACE_set_properties( volpath, path, &ent )) {
					((CHFVExplorerDoc*)m_pDocument)->set_hfs_volume_clean( volpath );
				}
			}
			fullpath.ReleaseBuffer();

			if(pApp->m_show_labels) {
				m_cl->SetItemState( inx, color, LVIS_OVERLAYMASK );
			}
		}
	}
}

void CHFVExplorerListView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu cm, cpop, cpop2, cpop3;
	int inx, hit = 0;
	int pop_created = 0;
	int pop_created2 = 0;
	int pop_created3 = 0;
	long id, dat;
	// CString str;
	char *label_string;

	inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
	if(inx >= 0) hit = 1;

	if(((CHFVExplorerApp *)AfxGetApp())->m_show_labels)
		label_string = "Hide labels";
	else
		label_string = "Show labels";

	if(cm.CreatePopupMenu()) {
		if(hit) {
			dat = m_cl->GetItemData( inx );
			id = get_lparam_id(dat);
			if(get_lparam_isdir(dat)) {
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FOLDER_OPEN, "&Open Folder\tEnter" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_DELETE, "&Delete\tDel" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_MAP_THIS, "&Map selected files...\tF4" );
				cm.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_NEW_FOLDER, "&New Folder..." );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SAVE_ICON, "Save &icon..." );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_TOGGLE_LABEL_COLORS, label_string );
				if(cpop3.CreatePopupMenu()) {
					pop_created3 = 1;
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_NONE, get_label_text(FINDER_COLOR_NONE) );
					cpop3.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_ESSENTIAL, get_label_text(FINDER_COLOR_ESSENTIAL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_HOT, get_label_text(FINDER_COLOR_HOT) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_IN_PROGRESS, get_label_text(FINDER_COLOR_IN_PROGRESS) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_COOL, get_label_text(FINDER_COLOR_COOL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PERSONAL, get_label_text(FINDER_COLOR_PERSONAL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PROJECT_1, get_label_text(FINDER_COLOR_PROJECT_1) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PROJECT_2, get_label_text(FINDER_COLOR_PROJECT_2) );
					cm.AppendMenu( MF_POPUP | MF_STRING | MF_ENABLED, (UINT)cpop3.m_hMenu, "Label..." );
				}
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_PROGRAM_OPTIONS, "&Options..." );
				cm.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FOLDER_PROPERTIES, "&Properties..." );
			} else {
				CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
				if(pApp->m_default_emulator == 0) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (E/DOS)\tEnter" );
				} else if(pApp->m_default_emulator == 1) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (E/Win32)\tEnter" );
				} else if(pApp->m_default_emulator == 2) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (vMac)\tEnter" );
				} else if(pApp->m_default_emulator == 3) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (Shapeshifter)\tEnter" );
				} else if(pApp->m_default_emulator == 4) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (Fusion DOS)\tEnter" );
				} else if(pApp->m_default_emulator == 5) {
					cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_OPEN, "&Open (Associated Windows program)\tEnter" );
				}
				
				/*
				if(cpop.CreatePopupMenu()) {
					pop_created = 1;
					// mac
					cpop.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, ID_OWN_WITH_MAC_FIRST, "TextPad" );
					cpop.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
					// dos
					cpop.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, ID_OWN_WITH_PC_FIRST, "PC: WordPad" );
					cpop.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, ID_OWN_WITH_PC_FIRST+1, "PC: NotePad" );
					cpop.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
					cpop.AppendMenu( MF_STRING | MF_DISABLED | MF_GRAYED, ID_OWN_WITH_LIST, "Edit list..." );
				}
				*/
				if(cpop.CreatePopupMenu()) {
					pop_created = 1;
					// mac
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+0, "Executor &DOS" );
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+1, "Executor &Win32" );
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+2, "&vMac" );
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+3, "&Shapeshifter" );
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+4, "&Fusion DOS" );
					cpop.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_WITH_MAC_FIRST+5, "&Associated Windows program" );
				}
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_DELETE, "&Delete\tDel" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_MAP_THIS, "&Map selected files..\tF4" );
				
				if(pop_created) cm.AppendMenu( MF_POPUP | MF_STRING | MF_ENABLED, (UINT)cpop.m_hMenu, "Open &with..." );
				if(cpop2.CreatePopupMenu()) {
					pop_created2 = 1;
					cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_CREATE_E_DOS, "Executor &DOS" );
					cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_CREATE_E_WIN32, "Executor &Win32" );
					cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_CREATE_VMAC, "&vMac" );
					cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_CREATE_SS, "&Shapeshifter" );
					cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_CREATE_FUSION, "&Fusion DOS" );
					// cpop2.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_WINDOWS, "&Associated Windows program" );
					cm.AppendMenu( MF_POPUP | MF_STRING | MF_ENABLED, (UINT)cpop2.m_hMenu, "Create &Shortcut for..." );
				}
				cm.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_NEW_FOLDER, "&New Folder..." );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SAVE_ICON, "Save &icon..." );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_TOGGLE_LABEL_COLORS, label_string );
				if(cpop3.CreatePopupMenu()) {
					pop_created3 = 1;
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_NONE, get_label_text(FINDER_COLOR_NONE) );
					cpop3.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_ESSENTIAL, get_label_text(FINDER_COLOR_ESSENTIAL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_HOT, get_label_text(FINDER_COLOR_HOT) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_IN_PROGRESS, get_label_text(FINDER_COLOR_IN_PROGRESS) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_COOL, get_label_text(FINDER_COLOR_COOL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PERSONAL, get_label_text(FINDER_COLOR_PERSONAL) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PROJECT_1, get_label_text(FINDER_COLOR_PROJECT_1) );
					cpop3.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_SHORTCUT_COLOR_PROJECT_2, get_label_text(FINDER_COLOR_PROJECT_2) );
					cm.AppendMenu( MF_POPUP | MF_STRING | MF_ENABLED, (UINT)cpop3.m_hMenu, "Label..." );
				}
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_PROGRAM_OPTIONS, "&Options..." );
				cm.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
				cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_FILE_PROPERTIES, "&Properties..." );
			}
		} else {
			cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_NEW_FOLDER, "&New Folder..." );
			cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_TOGGLE_LABEL_COLORS, label_string );
			cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_PROGRAM_OPTIONS, "&Options..." );
		}
		if(cm.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this )) {
		}
		if(pop_created) cpop.DestroyMenu();
		if(pop_created2) cpop2.DestroyMenu();
		if(pop_created3) cpop3.DestroyMenu();
		cm.DestroyMenu();
	}
}

void CHFVExplorerListView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags ) 
{
	CPoint point;
	int inx;
	RECT rect;

	inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_SELECTED );
	if( inx >= 0 && nChar == VK_RETURN ) {
		if(m_cl->GetItemRect( inx, &rect, LVIR_BOUNDS )) {
			point.x = (rect.left + rect.right) / 2;
			point.y = (rect.top + rect.bottom) / 2;
			ClientToScreen( &point );
			OnContextMenu( this, point );
		}
	} else {
		CListView::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CHFVExplorerListView::on_properties( int inx ) 
{
	CString fullpath;
	int stype = get_inx_type(inx);
	// int isdir = get_inx_isdir(inx);

	if(stype == LIST_SPEC) return;

	get_path( inx, fullpath );

	if(inx >= 0) {
		if(stype == LIST_FAT) {
			// update_current_view();
		} else {
			char volpath[_MAX_PATH];
			char oldname[_MAX_PATH];
			hfsdirent ent;

			inx2volname( inx, volpath, _MAX_PATH );
			char *path = fullpath.GetBuffer( _MAX_PATH );

			CAskProperties dlg;
			if(0 == HFSIFACE_get_properties( volpath, path, &ent )) {

				// flags
				dlg.m_isdir    =	(ent.flags & HFS_ISDIR) != 0;
				dlg.m_islocked =	(ent.flags & HFS_ISLOCKED) != 0;

				// common
				ent.name[HFS_MAX_FLEN] = 0;
				mac_to_pc_charset( (unsigned char *)ent.name );
				strcpy( oldname, ent.name );
				dlg.m_name = CString( ent.name );
				dlg.m_nodeid	= ent.cnid;
				dlg.m_parentid = ent.parid;
				dlg.m_backup = ent.bkdate;
				dlg.m_created = ent.crdate;
				dlg.m_modified = ent.mddate;

				// finder location
				dlg.m_x = ent.fdlocation.h;
				dlg.m_y = ent.fdlocation.v;

				// flags
				if(dlg.m_isdir) {
					// file
					dlg.m_rsize = 0;
					dlg.m_dsize = 0;
					dlg.m_creator = "";
					dlg.m_type =  "";

					// folder
					dlg.m_valence		= ent.u.dir.valence;
					dlg.m_bottom    = ent.u.dir.rect.bottom;
					dlg.m_left			= ent.u.dir.rect.left;
					dlg.m_rifht 		= ent.u.dir.rect.right;
					dlg.m_top 			= ent.u.dir.rect.top;
				} else {
					// file
					dlg.m_rsize = ent.u.file.rsize;
					dlg.m_dsize = ent.u.file.dsize;
					ent.u.file.creator[4] = 0;
					ent.u.file.type[4] = 0;
					dlg.m_creator = CString( ent.u.file.creator );
					dlg.m_type =  CString( ent.u.file.type );

					// folder
					dlg.m_valence		= 0;
					dlg.m_bottom    = 0;
					dlg.m_left			= 0;
					dlg.m_rifht 		= 0;
					dlg.m_top 			= 0;
				}

				// fdflags
				dlg.m_isindesktop = (ent.fdflags & HFS_FNDR_ISONDESK) != 0;
				dlg.m_hasbeeninited = (ent.fdflags & HFS_FNDR_HASBEENINITED) != 0;
				dlg.m_isinvisible  = (ent.fdflags & HFS_FNDR_ISINVISIBLE) != 0;
				dlg.m_hasbundle = (ent.fdflags & HFS_FNDR_HASBUNDLE) != 0;
				dlg.m_hascolors = (ent.fdflags & HFS_FNDR_COLOR) != 0;
				dlg.m_hascustomicons = (ent.fdflags & HFS_FNDR_HASCUSTOMICON) != 0;
				dlg.m_hasnoinits = (ent.fdflags & HFS_FNDR_HASNOINITS) != 0;
				dlg.m_isalias = (ent.fdflags & HFS_FNDR_ISALIAS) != 0;
				dlg.m_isnamelocked = (ent.fdflags & HFS_FNDR_NAMELOCKED) != 0;
				dlg.m_isshared = (ent.fdflags & HFS_FNDR_ISSHARED) != 0;
				dlg.m_isstationery = (ent.fdflags & HFS_FNDR_ISSTATIONERY) != 0;
				dlg.m_reqswitchlaunch = (ent.fdflags & HFS_FNDR_REQUIRESSWITCHLAUNCH) != 0;
				dlg.m_reserved = (ent.fdflags & HFS_FNDR_RESERVED) != 0;
				dlg.m_reservedcolors = (ent.fdflags & HFS_FNDR_COLORRESERVED) != 0;

				if(dlg.DoModal() == IDOK) {

					// flags
					if(dlg.m_isdir) 
						ent.flags |= HFS_ISDIR;
					else
						ent.flags &= ~HFS_ISDIR;
					if(dlg.m_islocked) 
						ent.flags |= HFS_ISLOCKED;
					else
						ent.flags &= ~HFS_ISLOCKED;

					if(dlg.m_isdir) {
						// folder
						ent.u.dir.valence = dlg.m_valence;
						ent.u.dir.rect.bottom = dlg.m_bottom;
						ent.u.dir.rect.left = dlg.m_left;
						ent.u.dir.rect.right = dlg.m_rifht;
						ent.u.dir.rect.top = dlg.m_top;
					} else {
						// file
						ent.u.file.rsize = dlg.m_rsize;
						ent.u.file.dsize = dlg.m_dsize;
						strncpy( ent.u.file.type, dlg.m_type.GetBuffer(100), 4 );
						ent.u.file.type[4] = 0;
						strncpy( ent.u.file.creator, dlg.m_creator.GetBuffer(100), 4 );
						ent.u.file.creator[4] = 0;
					}

					// common
					strncpy( ent.name, dlg.m_name.GetBuffer(100), HFS_MAX_FLEN );
					ent.name[HFS_MAX_FLEN] = 0;
					ent.cnid = dlg.m_nodeid;
					ent.parid = dlg.m_parentid;
					ent.bkdate = dlg.m_backup;
					ent.crdate = dlg.m_created;
					ent.mddate = dlg.m_modified;

					// finder location
					ent.fdlocation.h = dlg.m_x;
					ent.fdlocation.v = dlg.m_y;

					// fdflags
					if(dlg.m_isindesktop) 
						ent.fdflags |= HFS_FNDR_ISONDESK;
					else
						ent.fdflags &= ~HFS_FNDR_ISONDESK;
					if(dlg.m_hasbeeninited) 
						ent.fdflags |= HFS_FNDR_HASBEENINITED;
					else
						ent.fdflags &= ~HFS_FNDR_HASBEENINITED;
					if(dlg.m_isinvisible) 
						ent.fdflags |= HFS_FNDR_ISINVISIBLE;
					else
						ent.fdflags &= ~HFS_FNDR_ISINVISIBLE;
					if(dlg.m_hasbundle) 
						ent.fdflags |= HFS_FNDR_HASBUNDLE;
					else
						ent.fdflags &= ~HFS_FNDR_HASBUNDLE;
					if(dlg.m_hascolors) 
						ent.fdflags |= HFS_FNDR_COLOR;
					else
						ent.fdflags &= ~HFS_FNDR_COLOR;
					if(dlg.m_hascustomicons) 
						ent.fdflags |= HFS_FNDR_HASCUSTOMICON;
					else
						ent.fdflags &= ~HFS_FNDR_HASCUSTOMICON;
					if(dlg.m_hasnoinits) 
						ent.fdflags |= HFS_FNDR_HASNOINITS;
					else
						ent.fdflags &= ~HFS_FNDR_HASNOINITS;
					if(dlg.m_isalias) 
						ent.fdflags |= HFS_FNDR_ISALIAS;
					else
						ent.fdflags &= ~HFS_FNDR_ISALIAS;
					if(dlg.m_isnamelocked) 
						ent.fdflags |= HFS_FNDR_NAMELOCKED;
					else
						ent.fdflags &= ~HFS_FNDR_NAMELOCKED;
					if(dlg.m_isshared) 
						ent.fdflags |= HFS_FNDR_ISSHARED;
					else
						ent.fdflags &= ~HFS_FNDR_ISSHARED;
					if(dlg.m_isstationery) 
						ent.fdflags |= HFS_FNDR_ISSTATIONERY;
					else
						ent.fdflags &= ~HFS_FNDR_ISSTATIONERY;
					if(dlg.m_reqswitchlaunch) 
						ent.fdflags |= HFS_FNDR_REQUIRESSWITCHLAUNCH;
					else
						ent.fdflags &= ~HFS_FNDR_REQUIRESSWITCHLAUNCH;
					if(dlg.m_reserved) 
						ent.fdflags |= HFS_FNDR_RESERVED;
					else
						ent.fdflags &= ~HFS_FNDR_RESERVED;
					if(dlg.m_reservedcolors) 
						ent.fdflags |= HFS_FNDR_COLORRESERVED;
					else
						ent.fdflags &= ~HFS_FNDR_COLORRESERVED;

					// Name must be changed separately.
					if(strcmp( oldname, ent.name ) != 0) {
						// do_rename: takes PC names
						if(!do_rename( inx, oldname, ent.name, 0))
						{
							AfxMessageBox( "Could not rename the file or folder." );
							return;
						} else {
							char *p = strrchr(path,':');
							if(!p) return; // impossible
							strcpy( p+1, ent.name );
						}
					}

					// HFSIFACE_set_properties: takes PC path
					if(0 == HFSIFACE_set_properties( volpath, path, &ent )) {
						update_floppy_view(volpath);
					} else {
						AfxMessageBox( "Failed to set item properties" );
					}
				}
			} else {
				AfxMessageBox( "Cannot get item properties" );
			}
		}
	}
}

#define MAXCMDS 40


void CHFVExplorerListView::save_icon_command( int inx )
{
	CString path;

	CFileDialog dlg( FALSE, _T("ICO"), path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Icon Files|*.ico|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		path = dlg.GetPathName();
		if(!save_icon( inx, (char *)(LPCSTR)path, FALSE )) {
			CString cs = "Failed to save the icon to file \"" + CString(path) + ".\"";
			AfxMessageBox( cs );
		}
	}
}

BOOL CHFVExplorerListView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int handled = 1;
	UINT cmd = LOWORD(wParam);
	int inx;

	switch( cmd ) {
		case ID_OWN_SHORTCUT_COLOR_NONE:
			set_label_color( FINDER_COLOR_NONE );
			break;
		case ID_OWN_SHORTCUT_COLOR_ESSENTIAL:
			set_label_color( FINDER_COLOR_ESSENTIAL );
			break;
		case ID_OWN_SHORTCUT_COLOR_HOT:
			set_label_color( FINDER_COLOR_HOT );
			break;
		case ID_OWN_SHORTCUT_COLOR_IN_PROGRESS:
			set_label_color( FINDER_COLOR_IN_PROGRESS );
			break;
		case ID_OWN_SHORTCUT_COLOR_COOL:
			set_label_color( FINDER_COLOR_COOL );
			break;
		case ID_OWN_SHORTCUT_COLOR_PERSONAL:
			set_label_color( FINDER_COLOR_PERSONAL );
			break;
		case ID_OWN_SHORTCUT_COLOR_PROJECT_1:
			set_label_color( FINDER_COLOR_PROJECT_1 );
			break;
		case ID_OWN_SHORTCUT_COLOR_PROJECT_2:
			set_label_color( FINDER_COLOR_PROJECT_2 );
			break;
		case ID_OWN_TOGGLE_LABEL_COLORS:
			toggle_label_colors();
			break;
		case ID_OWN_FOLDER_OPEN:
			inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
			if(inx >= 0) {
				activate_list_item( inx, FALSE, FALSE );
			}
			break;
		case ID_OWN_FILE_OPEN:
			// OnSysKeyDown( VK_RETURN, 0, 0 );
			inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
			if(inx >= 0) {
				activate_list_item( inx, FALSE, FALSE );
			}
			break;
		case ID_OWN_FOLDER_PROPERTIES:
		case ID_OWN_FILE_PROPERTIES:
			inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
			if(inx >= 0) {
				on_properties( inx );
			}
			break;
		case ID_OWN_MAP_THIS:
			inx = m_cl->GetNextItem( -1, LVNI_SELECTED );
			if(inx >= 0) {
				map_selected_item();
			}
			break;
		case ID_OWN_DELETE:
			inx = m_cl->GetNextItem( -1, LVNI_SELECTED );
			if(inx >= 0) {
				try_delete_selected_items( TRUE );
			}
			break;
		case ID_OWN_SAVE_ICON:
			inx = m_cl->GetNextItem( -1, LVNI_SELECTED );
			if(inx >= 0) {
				save_icon_command(inx);
			}
			break;
		case ID_OWN_NEW_FOLDER:
			new_folder();
			break;
		case ID_OWN_PROGRAM_OPTIONS:
			{
			CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
			((CMainFrame *)pApp->m_pMainWnd)->OnProgramProperties();
		  // ((CHFVExplorerDoc*)m_pDocument)->open_sub_directory( id, name );
			}
			break;
		case ID_OWN_WITH_LIST:
			break;
		case ID_OWN_SHORTCUT_CREATE_E_DOS:
		case ID_OWN_SHORTCUT_CREATE_E_WIN32:
		case ID_OWN_SHORTCUT_CREATE_VMAC:
		case ID_OWN_SHORTCUT_CREATE_SS:
		case ID_OWN_SHORTCUT_CREATE_FUSION:
		case ID_OWN_SHORTCUT_WINDOWS:
			inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
			if(inx >= 0) {
				activate_list_item( inx, -(int)cmd, TRUE );
			}
			break;
		default:
			// if( cmd >= ID_OWN_WITH_MAC_FIRST && cmd <= ID_OWN_WITH_MAC_FIRST+MAXCMDS) {
			if( cmd >= ID_OWN_WITH_MAC_FIRST && cmd <= ID_OWN_WITH_MAC_FIRST+5) {
				inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
				if(inx >= 0) {
					switch(cmd) {
						case ID_OWN_WITH_MAC_FIRST:
							activate_list_item( inx, -ID_OWN_SHORTCUT_CREATE_E_DOS, FALSE );
							break;
						case ID_OWN_WITH_MAC_FIRST+1:
							activate_list_item( inx, -ID_OWN_SHORTCUT_CREATE_E_WIN32, FALSE );
							break;
						case ID_OWN_WITH_MAC_FIRST+2:
							activate_list_item( inx, -ID_OWN_SHORTCUT_CREATE_VMAC, FALSE );
							break;
						case ID_OWN_WITH_MAC_FIRST+3:
							activate_list_item( inx, -ID_OWN_SHORTCUT_CREATE_SS, FALSE );
							break;
						case ID_OWN_WITH_MAC_FIRST+4:
							activate_list_item( inx, -ID_OWN_SHORTCUT_CREATE_FUSION, FALSE );
							break;
						case ID_OWN_WITH_MAC_FIRST+5:
							activate_list_item( inx, -ID_OWN_SHORTCUT_WINDOWS, FALSE );
							break;
					}
				}
			} else if( cmd >= ID_OWN_WITH_PC_FIRST && cmd <= ID_OWN_WITH_PC_FIRST+MAXCMDS) {
			} else {
				handled = 0;
			}
	}
	if(handled) 
		return(1);
	else
		return CListView::OnCommand(wParam, lParam);
}

RGBQUAD rgbStd256[] =
{
	{   0,  0,  0, 0 },	{   0,  0,128, 0 },	{   0,128,  0, 0 },	{   0,128,128, 0 },
	{ 128,  0,  0, 0 },	{ 128,  0,128, 0 },	{ 128,128,  0, 0 },	{ 192,192,192, 0 },
	{ 192,220,192, 0 },	{ 240,202,166, 0 },	{ 238,238,238, 0 },	{ 221,221,221, 0 },
	{ 204,204,204, 0 },	{ 187,187,187, 0 },	{ 170,170,170, 0 },	{ 153,153,153, 0 },
	{ 136,136,136, 0 },	{ 119,119,119, 0 },	{ 102,102,102, 0 },	{  85, 85, 85, 0 },
	{  68, 68, 68, 0 },	{  51, 51, 51, 0 },	{  34, 34, 34, 0 },	{  17, 17, 17, 0 },
	{ 204,255,255, 0 },	{ 153,255,255, 0 },	{ 102,255,255, 0 },	{  51,255,255, 0 },
	{ 255,204,255, 0 },	{ 204,204,255, 0 },	{ 153,204,255, 0 },	{ 102,204,255, 0 },
	{  51,204,255, 0 },	{   0,204,255, 0 },	{ 255,153,255, 0 },	{ 204,153,255, 0 },
	{ 153,153,255, 0 },	{ 102,153,255, 0 },	{  51,153,255, 0 },	{   0,153,255, 0 },
	{ 255,102,255, 0 },	{ 204,102,255, 0 },	{ 153,102,255, 0 },	{ 102,102,255, 0 },
	{  51,102,255, 0 },	{   0,102,255, 0 },	{ 255, 51,255, 0 },	{ 204, 51,255, 0 },
	{ 153, 51,255, 0 },	{ 102, 51,255, 0 },	{  51, 51,255, 0 },	{   0, 51,255, 0 },
	{ 204,  0,255, 0 },	{ 153,  0,255, 0 },	{ 102,  0,255, 0 },	{  51,  0,255, 0 },
	{ 255,255,204, 0 },	{ 204,255,204, 0 },	{ 153,255,204, 0 },	{ 102,255,204, 0 },
	{  51,255,204, 0 },	{   0,255,204, 0 },	{ 255,204,204, 0 },	{ 153,204,204, 0 },
	{ 102,204,204, 0 },	{  51,204,204, 0 },	{   0,204,204, 0 },	{ 255,153,204, 0 },
	{ 204,153,204, 0 },	{ 153,153,204, 0 },	{ 102,153,204, 0 },	{  51,153,204, 0 },
	{   0,153,204, 0 },	{ 255,102,204, 0 },	{ 204,102,204, 0 },	{ 153,102,204, 0 },
	{ 102,102,204, 0 },	{  51,102,204, 0 },	{   0,102,204, 0 },	{ 255, 51,204, 0 },
	{ 204, 51,204, 0 },	{ 153, 51,204, 0 },	{ 102, 51,204, 0 },	{  51, 51,204, 0 },
	{   0, 51,204, 0 },	{ 255,  0,204, 0 },	{ 204,  0,204, 0 },	{ 153,  0,204, 0 },
	{ 102,  0,204, 0 },	{  51,  0,204, 0 },	{ 255,255,153, 0 },	{ 204,255,153, 0 },
	{ 153,255,153, 0 },	{ 102,255,153, 0 },	{  51,255,153, 0 },	{   0,255,153, 0 },
	{ 255,204,153, 0 },	{ 204,204,153, 0 },	{ 153,204,153, 0 },	{ 102,204,153, 0 },
	{  51,204,153, 0 },	{   0,204,153, 0 },	{ 255,153,153, 0 },	{ 204,153,153, 0 },
	{ 102,153,153, 0 },	{  51,153,153, 0 },	{   0,153,153, 0 },	{ 255,102,153, 0 },
	{ 204,102,153, 0 },	{ 153,102,153, 0 },	{ 102,102,153, 0 },	{  51,102,153, 0 },
	{   0,102,153, 0 },	{ 255, 51,153, 0 },	{ 204, 51,153, 0 },	{ 153, 51,153, 0 },
	{ 102, 51,153, 0 },	{  51, 51,153, 0 },	{   0, 51,153, 0 },	{ 255,  0,153, 0 },
	{ 204,  0,153, 0 },	{ 153,  0,153, 0 },	{ 102,  0,153, 0 },	{  51,  0,153, 0 },
	{ 255,255,102, 0 },	{ 204,255,102, 0 },	{ 153,255,102, 0 },	{ 102,255,102, 0 },
	{  51,255,102, 0 },	{   0,255,102, 0 },	{ 255,204,102, 0 },	{ 204,204,102, 0 },
	{ 153,204,102, 0 },	{ 102,204,102, 0 },	{  51,204,102, 0 },	{   0,204,102, 0 },
	{ 255,153,102, 0 },	{ 204,153,102, 0 },	{ 153,153,102, 0 },	{ 102,153,102, 0 },
	{  51,153,102, 0 },	{   0,153,102, 0 },	{ 255,102,102, 0 },	{ 204,102,102, 0 },
	{ 153,102,102, 0 },	{  51,102,102, 0 },	{   0,102,102, 0 },	{ 255, 51,102, 0 },
	{ 204, 51,102, 0 },	{ 153, 51,102, 0 },	{ 102, 51,102, 0 },	{  51, 51,102, 0 },
	{   0, 51,102, 0 },	{ 255,  0,102, 0 },	{ 204,  0,102, 0 },	{ 153,  0,102, 0 },
	{ 102,  0,102, 0 },	{  51,  0,102, 0 },	{ 255,255, 51, 0 },	{ 204,255, 51, 0 },
	{ 153,255, 51, 0 },	{ 102,255, 51, 0 },	{  51,255, 51, 0 },	{   0,255, 51, 0 },
	{ 255,204, 51, 0 },	{ 204,204, 51, 0 },	{ 153,204, 51, 0 },	{ 102,204, 51, 0 },
	{  51,204, 51, 0 },	{   0,204, 51, 0 },	{ 255,153, 51, 0 },	{ 204,153, 51, 0 },
	{ 153,153, 51, 0 },	{ 102,153, 51, 0 },	{  51,153, 51, 0 },	{   0,153, 51, 0 },
	{ 255,102, 51, 0 },	{ 204,102, 51, 0 },	{ 153,102, 51, 0 },	{ 102,102, 51, 0 },
	{  51,102, 51, 0 },	{   0,102, 51, 0 },	{ 255, 51, 51, 0 },	{ 204, 51, 51, 0 },
	{ 153, 51, 51, 0 },	{ 102, 51, 51, 0 },	{   0, 51, 51, 0 },	{ 255,  0, 51, 0 },
	{ 204,  0, 51, 0 },	{ 153,  0, 51, 0 },	{ 102,  0, 51, 0 },	{  51,  0, 51, 0 },
	{ 204,255,  0, 0 },	{ 153,255,  0, 0 },	{ 102,255,  0, 0 },	{  51,255,  0, 0 },
	{ 255,204,  0, 0 },	{ 204,204,  0, 0 },	{ 153,204,  0, 0 },	{ 102,204,  0, 0 },
	{  51,204,  0, 0 },	{ 255,153,  0, 0 },	{ 204,153,  0, 0 },	{ 153,153,  0, 0 },
	{ 102,153,  0, 0 },	{   0,  0,238, 0 },	{   0,  0,221, 0 },	{   0,  0,204, 0 },
	{   0,  0,187, 0 },	{   0,  0,170, 0 },	{   0,  0,153, 0 },	{   0,  0,136, 0 },
	{   0,  0,119, 0 },	{   0,  0,102, 0 },	{   0,  0, 85, 0 },	{   0,  0, 68, 0 },
	{   0,  0, 51, 0 },	{   0,  0, 34, 0 },	{   0,  0, 17, 0 },	{   0,238,  0, 0 },
	{   0,221,  0, 0 },	{   0,204,  0, 0 },	{   0,187,  0, 0 },	{   0,170,  0, 0 },
	{   0,153,  0, 0 },	{   0,136,  0, 0 },	{   0,119,  0, 0 },	{   0,102,  0, 0 },
	{   0, 85,  0, 0 },	{   0, 68,  0, 0 },	{   0, 51,  0, 0 },	{   0, 34,  0, 0 },
	{   0, 17,  0, 0 },	{ 238,  0,  0, 0 },	{ 221,  0,  0, 0 },	{ 204,  0,  0, 0 },
	{ 187,  0,  0, 0 },	{ 170,  0,  0, 0 },	{ 153,  0,  0, 0 },	{ 136,  0,  0, 0 },
	{ 119,  0,  0, 0 },	{ 102,  0,  0, 0 },	{  85,  0,  0, 0 },	{  68,  0,  0, 0 },
	{  51,  0,  0, 0 },	{  34,  0,  0, 0 },	{ 240,251,255, 0 },	{ 164,160,160, 0 },
	{ 128,128,128, 0 },	{   0,  0,255, 0 },	{   0,255,  0, 0 },	{   0,255,255, 0 },
	{ 255,  0,  0, 0 },	{ 255,  0,255, 0 },	{ 255,255,  0, 0 },	{ 255,255,255, 0 }
};					   

RGBQUAD mac256[] =
{
	{ 255,255,255, 0 },	{ 253,203,153, 0 },	{ 253,203,153, 0 },	{ 253,203, 53, 0 },
	{ 253,203, 53, 0 },	{ 253,203,  0, 0 },	{ 203,150,160, 0 },	{ 203,203,203, 0 },
	{ 210,153,153, 0 },	{ 225,152, 55, 0 },	{ 230,152, 55, 0 },	{ 208,150,  0, 0 },
	{ 204,152,165, 0 },	{ 217,152,152, 0 },	{ 208,152,153, 0 },	{ 223,152, 53, 0 },
	{ 223,152, 55, 0 },	{ 208,151,  0, 0 },	{ 203, 52,106, 0 },	{ 202, 51,107, 0 },
	{ 204, 51,108, 0 },	{ 204, 51, 53, 0 },	{ 203, 50, 51, 0 },	{ 205, 95,  3, 0 },
	{ 203, 50,105, 0 },	{ 204, 50,105, 0 },	{ 203, 50,105, 0 },	{ 203, 51, 51, 0 },
	{ 204, 51, 52, 0 },	{ 203, 96,  0, 0 },	{ 175,  0,103, 0 },	{ 200,  0,102, 0 },
	{ 200,  0,102, 0 },	{ 200,  0, 51, 0 },	{ 201,  0, 52, 0 },	{ 203, 40, 18, 0 },
	{ 205,205,205, 0 },	{ 203,203,152, 0 },	{ 203,203,152, 0 },	{ 203,203, 51, 0 },
	{ 203,203, 50, 0 },	{ 203,203,  0, 0 },	{ 199,152,176, 0 },	{ 203,203,203, 0 },
	{ 185,185,185, 0 },	{ 205,151, 53, 0 },	{ 205,152, 54, 0 },	{ 203,153,  0, 0 },
	{ 200,152,175, 0 },	{ 193,193,193, 0 },	{ 203,153,153, 0 },	{ 203,153, 53, 0 },
	{ 203,153, 55, 0 },	{ 203,153,  0, 0 },	{ 153, 49,103, 0 },	{ 203, 51,112, 0 },
	{ 203, 51,111, 0 },	{ 203, 65, 63, 0 },	{ 203, 63, 63, 0 },	{ 206, 87,  5, 0 },
	{ 153, 48,104, 0 },	{ 203, 51,113, 0 },	{ 203, 51,106, 0 },	{ 203, 52, 54, 0 },
	{ 204, 52, 57, 0 },	{ 203, 86,  5, 0 },	{ 153,  0,102, 0 },	{ 167,  0,101, 0 },
	{ 170,  0,101, 0 },	{ 195,  0, 53, 0 },	{ 196,  0, 51, 0 },	{ 202,  0, 18, 0 },
	{ 153,203,205, 0 },	{ 148,182,103, 0 },	{ 147,181,103, 0 },	{ 150,174, 43, 0 },
	{ 151,173, 46, 0 },	{ 153,201,  0, 0 },	{ 106,152,204, 0 },	{ 190,190,190, 0 },
	{ 150,150,150, 0 },	{ 147,151, 57, 0 },	{ 147,152, 52, 0 },	{ 152,153,  0, 0 },
	{ 106,152,203, 0 },	{ 149,152,155, 0 },	{ 152,152,152, 0 },	{ 145,151, 65, 0 },
	{ 145,152, 64, 0 },	{ 150,153,  2, 0 },	{ 103, 50,121, 0 },	{ 118, 51,118, 0 },
	{ 124, 51,124, 0 },	{ 140, 63, 63, 0 },	{ 140, 51, 54, 0 },	{ 153, 71,  0, 0 },
	{ 102, 50,104, 0 },	{ 117, 52,118, 0 },	{ 120, 50,119, 0 },	{ 143, 51, 53, 0 },
	{ 141, 51, 55, 0 },	{ 152, 70,  2, 0 },	{ 103,  0, 99, 0 },	{ 103,  0,101, 0 },
	{ 103,  0,100, 0 },	{ 150,  0, 69, 0 },	{ 150,  0, 68, 0 },	{ 152,  0,  0, 0 },
	{ 103,169,170, 0 },	{ 102,152,101, 0 },	{ 103,152,101, 0 },	{ 102,152,  0, 0 },
	{ 101,152,  2, 0 },	{ 146,164,  0, 0 },	{ 103,152,205, 0 },	{ 100,152,154, 0 },
	{ 101,153,153, 0 },	{ 102,152, 56, 0 },	{ 102,151, 55, 0 },	{ 101,152,  0, 0 },
	{ 103,152,204, 0 },	{ 101,152,154, 0 },	{ 101,152,154, 0 },	{ 102,151, 55, 0 },
	{ 102,151, 55, 0 },	{ 101,151,  0, 0 },	{ 101, 50,105, 0 },	{ 101, 52,140, 0 },
	{ 101, 51,142, 0 },	{ 102,102,102, 0 },	{ 100, 52, 54, 0 },	{ 100, 69,  0, 0 },
	{ 101, 49,109, 0 },	{ 102, 50,143, 0 },	{ 101, 51,145, 0 },	{ 101, 52, 54, 0 },
	{  52, 52, 52, 0 },	{ 101, 72,  0, 0 },	{  90,  0, 93, 0 },	{  92,  0, 93, 0 },
	{  91,  0, 90, 0 },	{ 101,  0, 57, 0 },	{ 101,  0, 58, 0 },	{ 101,  0,  0, 0 },
	{ 101,153,156, 0 },	{  53,152, 99, 0 },	{  52,152, 97, 0 },	{ 100,151,  4, 0 },
	{  99,151,  4, 0 },	{ 102,151,  0, 0 },	{  51,151,204, 0 },	{  51,152,153, 0 },
	{  52,152,153, 0 },	{  51,152, 52, 0 },	{  51,151, 52, 0 },	{  69,152, 18, 0 },
	{  51,151,205, 0 },	{  52,152,152, 0 },	{  51,153,152, 0 },	{  51,152, 51, 0 },
	{  52,151, 52, 0 },	{  64,152, 15, 0 },	{  51, 50,105, 0 },	{  52, 50,143, 0 },
	{  51, 51,148, 0 },	{  51, 50, 54, 0 },	{  51, 49, 51, 0 },	{  51, 62,  0, 0 },
	{  52, 50,112, 0 },	{  51, 51,146, 0 },	{  52, 50,146, 0 },	{  51, 49, 51, 0 },
	{  52, 50, 52, 0 },	{  50, 70,  0, 0 },	{  50,  0, 75, 0 },	{  50,  0, 83, 0 },
	{  50,  0, 80, 0 },	{  50,  0, 65, 0 },	{  50,  0, 65, 0 },	{  50,  0,  0, 0 },
	{  65,152,152, 0 },	{  50,151, 54, 0 },	{  50,153, 56, 0 },	{  50,152, 45, 0 },
	{  50,152, 47, 0 },	{ 103,151,  0, 0 },	{   0,151,204, 0 },	{  16,150,146, 0 },
	{  15,151,146, 0 },	{   0,110, 51, 0 },	{   0,100, 51, 0 },	{  50,152, 50, 0 },
	{   0,151,204, 0 },	{   0,150,146, 0 },	{   0,150,150, 0 },	{   0,101, 51, 0 },
	{   0,102, 51, 0 },	{  51,152, 50, 0 },	{  42, 49,104, 0 },	{   0, 51,148, 0 },
	{   0, 51,146, 0 },	{   0, 55, 55, 0 },	{   0, 55, 55, 0 },	{   0,100, 50, 0 },
	{  46, 50,103, 0 },	{   0, 51,147, 0 },	{   0, 49,146, 0 },	{   0, 52, 55, 0 },
	{  50, 50, 50, 0 },	{   0,100, 50, 0 },	{  50,  0, 70, 0 },	{   0,  0, 70, 0 },
	{   0,  0, 70, 0 },	{   0,  0, 65, 0 },	{   0,  0, 65, 0 },	{ 203, 40, 20, 0 },
	{ 200,  0, 15, 0 },	{ 193,  0, 23, 0 },	{ 195,  0, 27, 0 },	{ 153,  0,  3, 0 },
	{ 102,  0,  0, 0 },	{  80,  0,  0, 0 },	{  55,  0,  0, 0 },	{   0,  0,  0, 0 },
	{   0,  0,  0, 0 },	{ 101,152,  0, 0 },	{ 101,151,  0, 0 },	{  52,152, 49, 0 },
	{  51,153, 48, 0 },	{  50,152, 48, 0 },	{   0,100, 49, 0 },	{   0,100, 49, 0 },
	{   0,100, 50, 0 },	{   0,  0,  0, 0 },	{   0,  0,  0, 0 },	{  50,  0, 70, 0 },
	{  50,  0, 70, 0 },	{   0,  0, 70, 0 },	{   0,  0, 70, 0 },	{   5,  0, 70, 0 },
	{   0,  0, 65, 0 },	{   0,  0, 65, 0 },	{   0,  0, 65, 0 },	{   0,  0,  0, 0 },
	{   0,  0,  0, 0 },	{ 210,210,210, 0 },	{ 190,190,190, 0 },	{ 167,167,167, 0 },
	{ 150,150,150, 0 },	{ 110,110,110, 0 },	{  85, 85, 85, 0 },	{  72, 72, 72, 0 },
	{  60, 60, 60, 0 },	{  40, 40, 40, 0 },	{  20, 20, 20, 0 },	{   0,  0,  0, 0 }
};					   



void CHFVExplorerListView::CreatePalette()
{
	UINT i;

	init_map_palette();

	if (m_Palette.m_hObject != NULL)
		m_Palette.DeleteObject();

	// memcpy( m_rgbPalette, rgbStd256, sizeof(RGBQUAD) * 256 );
	memcpy( m_rgbPalette, exact_mac256, sizeof(RGBQUAD) * 256 );

	
	LPLOGPALETTE lpLogPal;
	lpLogPal = (LPLOGPALETTE) new BYTE[sizeof(LOGPALETTE) + ((255) * sizeof(PALETTEENTRY))];
	lpLogPal->palVersion = 0x0300;
	lpLogPal->palNumEntries = 256;

	for (i = 0; i < 256; i++)
	{
		lpLogPal->palPalEntry[i].peRed = m_rgbPalette[i].rgbRed;
		lpLogPal->palPalEntry[i].peGreen = m_rgbPalette[i].rgbGreen;
		lpLogPal->palPalEntry[i].peBlue = m_rgbPalette[i].rgbBlue;
		lpLogPal->palPalEntry[i].peFlags = 0;
	}

	VERIFY( m_Palette.CreatePalette( lpLogPal ) );
	delete [] (BYTE *)lpLogPal;
}

CPalette* CHFVExplorerListView::GetPalette()
{
	if (m_Palette.m_hObject != NULL)
		return &m_Palette;
	else
		return NULL;
}

BOOL CHFVExplorerListView::OnQueryNewPalette() 
{
	if (GetPalette() == NULL)
		return FALSE;

	return FALSE;

	CClientDC dc(this);
	CPalette* pOldPalette = dc.SelectPalette(&m_Palette,
		GetCurrentMessage()->message == WM_PALETTECHANGED);

	UINT nChanged = dc.RealizePalette();
	dc.SelectPalette(pOldPalette, TRUE);

	if (nChanged == 0)
		return FALSE;

	Invalidate(FALSE);

	return TRUE;
}

void CHFVExplorerListView::OnPaletteChanged(CWnd* pFocusWnd) 
{
	if (pFocusWnd == this || IsChild(pFocusWnd))
		return;
	OnQueryNewPalette();
}

void CHFVExplorerListView::_OnPaletteChanged(CWnd* pFocusWnd) 
{
	OnPaletteChanged(pFocusWnd);
}

BOOL CHFVExplorerListView::_OnQueryNewPalette() 
{
	return(OnQueryNewPalette());
}

/*
void CHFVExplorerListView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	LONG rstyle, style = GetWindowLong(m_cl->m_hWnd, GWL_STYLE);

	UINT nFormat;
	int icon_dim;

	rstyle = style & LVS_TYPEMASK;

	switch( rstyle) {
		case LVS_LIST:
			nFormat = DT_LEFT;
			rstyle = LVS_LIST;
			icon_dim = 16;
			break;
		case LVS_SMALLICON:
			nFormat = DT_LEFT;
			rstyle = LVS_SMALLICON;
			icon_dim = 16;
			break;
		case LVS_REPORT:
			nFormat = DT_LEFT;
			rstyle = LVS_REPORT;
			icon_dim = 16;
			break;
		case LVS_ICON:
			nFormat = DT_CENTER | DT_WORDBREAK;
			rstyle = LVS_ICON;
			icon_dim = 32;
			break;
	}

	HDC hdc = dc.GetSafeHdc();
	// SetBkColor( hdc, m_bkcolor );
	SetBkMode( hdc, TRANSPARENT );

	// ::SelectObject( hdc, m_hfont );

	int i, count = m_cl->GetItemCount();
	RECT r;

	CPalette* pOldPalette = dc.SelectPalette(&m_Palette,FALSE);
	UINT nChanged = dc.RealizePalette();
	
	for(i=0; i<count; i++) {
		long dat;
		HICON hicon;
		CRect result_rect;
		CString text;

		dat = m_cl->GetItemData( i );
		hicon = get_lparam_icon32(dat);
		m_cl->GetItemRect( i, &r, LVIR_ICON );

		int x = r.left + ((r.right - r.left) - icon_dim) / 2;
		r.left = x;
		r.right = r.left + icon_dim;

		int y = r.top + ((r.bottom - r.top) - icon_dim) / 2;
		r.top = y;
		r.bottom = r.top + icon_dim;

		if(result_rect.IntersectRect(&dc.m_ps.rcPaint,&r)) {
			if(hicon) {
				dc.DrawIcon( r.left, r.top, hicon );
			} else {
				if(rstyle == LVS_ICON) {
					// m_pimagelist
				} else {
					// m_pimagelistSmall.Get
				}
			}
		}

		m_cl->GetItemRect( i, &r, LVIR_LABEL );
		if(result_rect.IntersectRect(&dc.m_ps.rcPaint,&r)) {
			if(rstyle == LVS_REPORT) {
				int j, w, x = 0;
				for(j=0; j<NCOLUMNS; j++) {
					w = m_cl->GetColumnWidth( j );
					text = m_cl->GetItemText( i, j );
					r.left = x;
					r.right = r.left + w;
					dc.DrawText( text, text.GetLength(), &r, nFormat );
					x += w;
				}
			} else {
				text = m_cl->GetItemText( i, 0 );
				dc.DrawText( text, text.GetLength(), &r, nFormat );
			}
		}
	}

	dc.SelectPalette(pOldPalette, TRUE);
}

void CHFVExplorerListView::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	RECT r;
	int item;

	CDC *dc = m_cl->GetDC();
	if(pNMListView->uOldState != pNMListView->uNewState) {
		item = pNMListView->iItem;
		m_cl->GetItemRect( item, &r, LVIR_LABEL );
		dc->InvertRect( &r );
	}
	m_cl->ReleaseDC(dc);
	
	*pResult = 0;
}

typedef struct _NM_LISTVIEW
{
    NMHDR   hdr;
    int     iItem;
    int     iSubItem;
    UINT    uNewState;
    UINT    uOldState;
    UINT    uChanged;
    POINT   ptAction;
    LPARAM  lParam;
} NM_LISTVIEW, FAR *LPNM_LISTVIEW;

BOOL CHFVExplorerListView::OnEraseBkgnd(CDC* pDC) 
{
	CBrush cb;
	RECT r;
	cb.CreateSolidBrush( m_bkcolor );
	m_cl->GetClientRect(&r);
	pDC->FillRect( &r, &cb );
	return(0);
	// return CListView::OnEraseBkgnd(pDC);
}
*/

void CHFVExplorerListView::OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
	m_move_type = 0;
}

void CHFVExplorerListView::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint			ptItem, ptAction, ptImage;
	NM_LISTVIEW		*pnmListView = (NM_LISTVIEW *)pNMHDR;

	ASSERT(!m_dragging);

	m_move_type = 0;
	set_cursor_by_type(m_move_type);

	m_dragging = 1;
	m_iItemDrop = -1;
	m_TreeiItemDrop = 0;
	m_save_tree_selection = 0;

	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	pApp->m_tree->enable_selchange(0);

	m_iItemDrag = pnmListView->iItem;
	ptAction = pnmListView->ptAction;
	m_cl->GetItemPosition(m_iItemDrag, &ptItem);  // ptItem is relative to (0,0) and not the view origin
	m_cl->GetOrigin(&m_ptOrigin);
	m_drag_list = m_cl->CreateDragImage(m_iItemDrag, &ptImage);
	m_sizeDelta = ptAction - ptImage;   // difference between cursor pos and image pos
	m_ptHotSpot = ptAction - ptItem + m_ptOrigin;  // calculate hotspot for the cursor
	// m_drag_list->SetDragCursorImage(0, m_ptHotSpot);  // define the hot spot for the new cursor image
	// m_drag_list->DragShowNolock(TRUE);  // lock updates and show drag image
	m_drag_list->BeginDrag(0, CPoint(0, 0));
	ptAction -= m_sizeDelta;
	m_cl->ClientToScreen(&ptAction);
	m_drag_list->DragEnter(0, ptAction);
	m_drag_list->DragMove(ptAction);
	m_cl->SetCapture();
}

void CHFVExplorerListView::set_drop_target( int iItem, int onoff )
{
	CRect rect;
	CDC *dc;
	dc = m_cl->GetDC();
	if(dc) {
		if(m_cl->GetItemRect( iItem, &rect, LVIR_ICON )) {
			int w = rect.right - rect.left;
			if(w > systemiconsize) {
				rect.left += ((w - systemiconsize) >> 1);
				rect.right = rect.left + systemiconsize;
			}
			dc->InvertRect( &rect );
		}
		if(m_cl->GetItemRect( iItem, &rect, LVIR_LABEL )) 
			dc->InvertRect( &rect );
		m_cl->ReleaseDC(dc);
	}
}

void CHFVExplorerListView::set_tree_drop_target( HTREEITEM item, int onoff )
{
	CHFVExplorerApp	*pApp;
	CTreeCtrl *tp;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	tp = &pApp->m_tree->GetTreeCtrl();

	if(onoff) {
		if(!m_save_tree_selection) {
			m_save_tree_selection = tp->GetSelectedItem();
		}
		tp->SelectItem( item );
	} else {
		tp->SelectItem( m_save_tree_selection );
	}
}

void CHFVExplorerListView::ensure_pane( int request_col )
{
	CHFVExplorerApp	*pApp;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	pApp->m_pre->set_pane( request_col );
}

int CHFVExplorerListView::tree_view_hit_test( CPoint pt ) 
{
	CHFVExplorerApp	*pApp;
	CTreeCtrl *tp;
	HTREEITEM item;
	CPoint treepoint;
	UINT Flags;
	int ret = 0;

	pApp = (CHFVExplorerApp *)AfxGetApp();
	tp = &pApp->m_tree->GetTreeCtrl();
	treepoint = pt;
	tp->ScreenToClient(&treepoint);

	item = tp->HitTest( treepoint, &Flags );
	if(Flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
		if(item != m_TreeiItemDrop) {
			m_drag_list->DragLeave(0);
			ensure_pane(PANE_TREE);
			/*
			if(m_TreeiItemDrop != 0) {
				set_tree_drop_target( m_TreeiItemDrop, 0 );
			}
			*/
			m_TreeiItemDrop = item;
			set_tree_drop_target( m_TreeiItemDrop, 1 );
			ret = 1;
		}
	} else if(m_TreeiItemDrop != 0) {
		m_drag_list->DragLeave(0);
		ensure_pane(PANE_TREE);
		set_tree_drop_target( m_TreeiItemDrop, 0 );
		m_TreeiItemDrop = 0;
		ret = 1;
	}
	return(ret);
}

void CHFVExplorerListView::set_cursor_by_type( int control )
{
	if(control) {
		if(m_copy_cursor) SetCursor( m_copy_cursor );
	} else {
		if(m_move_cursor) SetCursor( m_move_cursor );
	}
}

void CHFVExplorerListView::check_move_type( 
	int control,
	int shift
) {
	int is_copy;

	// This is the Explorer-style determination.
	if(control) {
		is_copy = 1;
	} else if(shift) {
		is_copy = 0;
	} else {
		is_copy = 0;
		if(m_TreeiItemDrop) {
			CHFVExplorerApp	*pApp;
			CTreeCtrl *tc;
			int srcvol, destvol;
			int srctype, desttype;
			pApp = (CHFVExplorerApp *)AfxGetApp();
			tc = pApp->m_tree->m_ct;
			destvol = get_lparam_volinx( tc->GetItemData(m_TreeiItemDrop) );
			srcvol = get_lparam_volinx( m_cl->GetItemData(m_iItemDrag) );
			desttype = get_lparam_type( tc->GetItemData(m_TreeiItemDrop) );
			srctype = get_lparam_type( m_cl->GetItemData(m_iItemDrag) );
			if(desttype != srctype) {
				is_copy = 1;
			} else {
				if(destvol != srcvol) is_copy = 1;
			}
		}
	}
	if( m_move_type != is_copy ) {
		m_move_type = is_copy;
		set_cursor_by_type(is_copy);
	}
}

void CHFVExplorerListView::OnMouseMove(UINT nFlags, CPoint point)
{
	// long		lStyle;
	int			iItem;
	CPoint screenpoint, screenhitpoint;

	// lStyle = GetWindowLong(m_cl->m_hWnd, GWL_STYLE);
	// lStyle &= LVS_TYPEMASK;  // drag will do different things in list and report mode
	if (m_dragging) {
		screenpoint = point;
		m_cl->ClientToScreen(&screenpoint);
		screenhitpoint = screenpoint;
		screenpoint -= m_sizeDelta;
		m_drag_list->DragMove(screenpoint);  // move the image
		if ((iItem = m_cl->HitTest(point)) != -1 && iItem != m_iItemDrag) {
			if(iItem != m_iItemDrop) {
				m_drag_list->DragLeave(0);
				ensure_pane(PANE_LIST);
				if(m_iItemDrop >= 0) {
					set_drop_target( m_iItemDrop, 0 );
				}
				m_iItemDrop = iItem;
				set_drop_target( m_iItemDrop, 1 );
				m_drag_list->DragEnter(0, screenpoint);
			}
		} else if(m_iItemDrop >= 0) {
			m_drag_list->DragLeave(0);
			ensure_pane(PANE_LIST);
			set_drop_target( m_iItemDrop, 0 );
			m_iItemDrop = -1;
			m_drag_list->DragEnter(0, screenpoint);
		}
		if(tree_view_hit_test(screenhitpoint)) {
			m_drag_list->DragEnter(0, screenpoint);
		}
		check_move_type( nFlags & MK_CONTROL, nFlags & MK_SHIFT );
	}

	CListView::OnMouseMove(nFlags, point);
}

int CHFVExplorerListView::drop_list_to_tree( int item, HTREEITEM htree, int action )
{
	DWORD ldat;

	if(item < 0 || htree == 0) return(0);
	
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	if(htree == pApp->m_tree->m_ct->GetSelectedItem()) {
		if(action == ACTION_COPY) {
			AfxMessageBox( "Cannot copy, the destination folder is the same as the souce folder." );
		} else {
			AfxMessageBox( "Cannot move, the destination folder is the same as the souce folder." );
		}
		return(0);
	}

	ldat = m_cl->GetItemData( item );
	return(copy_item_to_dir( item, htree, action, 0, 0, 0, 0, 0 ));
}

void CHFVExplorerListView::OnButtonUp(CPoint point)
{
	CString creator;
	int action, update_finfo;

	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();

	update_finfo = ((CHFVExplorerApp *)AfxGetApp())->m_mac_window_mode;

	if(m_move_type) 
		action = ACTION_COPY;
	else
		action = ACTION_MOVE;

	m_shutup_for_now = FALSE;

	BOOL prevTimer = pApp->EnableTimer( FALSE );

	if (m_dragging) {
		long		lStyle;
		CString		cstr;

		::ReleaseCapture();
		SetCursor( LoadCursor( 0, IDC_ARROW ) );

		lStyle = GetWindowLong(m_cl->m_hWnd, GWL_STYLE) & LVS_TYPEMASK; 
		m_dragging = FALSE;
		m_drag_list->DragLeave(0);
		m_drag_list->EndDrag();

		if(m_iItemDrop >= 0) {
			set_drop_target( m_iItemDrop, 0 );
		}
		if(m_TreeiItemDrop != 0) {
			set_tree_drop_target( m_TreeiItemDrop, 0 );
		}
		ensure_pane(PANE_LIST);

		if(m_iItemDrop >= 0) {
			DWORD dw_drag, dw_drop;
			int drag_is_dir, drop_is_dir;
			
			m_iItemDrag = -1;
			while( (m_iItemDrag = m_cl->GetNextItem(m_iItemDrag,LVNI_SELECTED)) >= 0 ) {
				dw_drag = m_cl->GetItemData( m_iItemDrag );
				dw_drop = m_cl->GetItemData( m_iItemDrop );
				drag_is_dir = get_lparam_isdir( dw_drag );
				drop_is_dir = get_lparam_isdir( dw_drop );

				if(drop_is_dir) {
					if(!do_file_to_folder( m_iItemDrag, m_iItemDrop, action )) {
						break;
					}
				} else {
					creator = m_cl->GetItemText( m_iItemDrop, FIELD_CREATOR );
					do_open_with( m_iItemDrag, m_iItemDrop, creator ); // ?? explore folder
					break;
				}
			}
		} else if(m_TreeiItemDrop != 0) {
			m_iItemDrag = -1;
			while( (m_iItemDrag = m_cl->GetNextItem(m_iItemDrag,LVNI_SELECTED)) >= 0 ) {
				if(!drop_list_to_tree( m_iItemDrag, m_TreeiItemDrop, action )) {
					break;
				}
			}
		} else {
			CRect cr;
			m_cl->GetClientRect( &cr );
			if(cr.PtInRect(point)) {
				if (lStyle == LVS_ICON || lStyle == LVS_SMALLICON) {
					CPoint p2 = point, p3, p4;
					p2 -= m_ptHotSpot;  // the icon should be drawn exactly where the image is
					p2 += m_ptOrigin;
					m_cl->GetItemPosition( m_iItemDrag, &p3 );
					m_iItemDrag = -1;
					while( (m_iItemDrag = m_cl->GetNextItem(m_iItemDrag,LVNI_SELECTED)) >= 0 ) {
						m_cl->GetItemPosition( m_iItemDrag, &p4 );
						p4 = p4 - p3 + p2;
						m_cl->SetItemPosition(m_iItemDrag, p4);
						if(update_finfo) update_finder_position(m_iItemDrag, p4);
					}
				}
			} else { // somewhere else
			}
		}

		m_TreeiItemDrop = 0;
		m_iItemDrop = -1;

		pApp->m_tree->enable_selchange(1);
	}
	pApp->EnableTimer( prevTimer );
}

void CHFVExplorerListView::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp(point);
	CListView::OnLButtonUp(nFlags, point);
}

void CHFVExplorerListView::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	if( (pNMListView->uNewState & LVIS_FOCUSED) &&
		  (pNMListView->uOldState & LVIS_FOCUSED) == 0) {
		int item = pNMListView->iItem;
		if((item >= 0) && m_cl) {
			DWORD dat = m_cl->GetItemData( item );
			if(dat) {
				CHFVExplorerApp	*pApp;
				pApp = (CHFVExplorerApp *)AfxGetApp();
				if(pApp && pApp->m_pre) {
					pApp->m_pre->set_file( (list_lparam_struct *)dat );
				}
			}
		}
	}
	*pResult = 0;
}

void CHFVExplorerListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int inx, handled = 0;
	switch( nChar ) {
		case VK_ESCAPE:
			empty_clipboard();
			handled = 1;
			break;
		case VK_BACK:
			go_up( 0 );
			handled = 1;
			break;
		case VK_RETURN:
			inx = m_cl->GetNextItem( -1, LVNI_ALL | LVNI_FOCUSED );
			if(inx >= 0) {
				int ctrl = GetAsyncKeyState( VK_CONTROL ) & 0x80000000ul;
				activate_list_item( inx, ctrl != 0, FALSE );
			}
			handled = 1;
			break;
		case VK_DELETE:
			inx = m_cl->GetNextItem( -1, LVNI_SELECTED );
			if(inx >= 0) {
				try_delete_selected_items( TRUE );
			}
			handled = 1;
			break;
		case VK_F4:
			inx = m_cl->GetNextItem( -1, LVNI_SELECTED );
			if(inx >= 0) {
				map_selected_item();
			}
			handled = 1;
			break;
		case VK_TAB:
			CHFVExplorerApp	*pApp;
			pApp = (CHFVExplorerApp *)AfxGetApp();
			pApp->m_pre->set_pane( PANE_TREE );
			// pApp->m_pre->set_pane_index( PANE_TREE );
			handled = 1;
			break;
		case VK_SHIFT:
		case VK_CONTROL:
			if (m_dragging) {
				// For immediate response.
				int ctrl = GetAsyncKeyState( VK_CONTROL ) & 0x80000000ul;
				int shift = GetAsyncKeyState( VK_SHIFT ) & 0x80000000ul;
				check_move_type( ctrl, shift );
			}
			handled = 1;
			break;
	}
	if(!handled) CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CHFVExplorerListView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = 0;

	if(m_editing_item == pDispInfo->item.iItem) {
		if(pDispInfo->item.pszText) {
			if(!*pDispInfo->item.pszText) {
				strcpy(pDispInfo->item.pszText,m_editing_item_name);
			} else {
				if(strcmp(m_editing_item_name,pDispInfo->item.pszText) != 0) {
					char lname[50];
					strncpy(lname,pDispInfo->item.pszText,HFS_MAX_FLEN);
					lname[HFS_MAX_FLEN] = 0;
					if(do_rename( 
						m_editing_item, 
						m_editing_item_name,
						lname, 1))
					{
						*pResult = 1;
					}
				}
			}
		}
	}
}

void CHFVExplorerListView::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	m_editing_item = pDispInfo->item.iItem;
	lstrcpy( 
			m_editing_item_name, 
			m_cl->GetItemText( m_editing_item, 0 ) 
	);
	
	*pResult = 0;
}

void CHFVExplorerListView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch( nChar ) {
		case VK_SHIFT:
		case VK_CONTROL:
			if (m_dragging) {
				// For immediate response.
				int ctrl = GetAsyncKeyState( VK_CONTROL ) & 0x80000000ul;
				int shift = GetAsyncKeyState( VK_SHIFT ) & 0x80000000ul;
				check_move_type( ctrl, shift );
			}
			break;
	}
	CListView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CHFVExplorerListView::update_floppy_view( char *volpath )
{
	CHFVExplorerDoc *pdoc;

	pdoc = (CHFVExplorerDoc*)m_pDocument;
	pdoc->set_floppy_volume_dirty(volpath);
}

void CHFVExplorerListView::on_volume_changed( int volinx )
{
	/*
	CString currdir;
	int mac;

	currdir = ((CHFVExplorerDoc*)m_pDocument)->get_directory( 0, &mac );
	if(currdir != "") {
		((CHFVExplorerDoc*)m_pDocument)->my_chdir( currdir, mac );
	}
	*/
}

// Kicked out.
/*
DWORD CHFVExplorerListView::stupid_get_date( int inx, int whichone )
{
	CString fullpath;
	char volpath[_MAX_PATH];
	char *path;
	hfsdirent ent;
	DWORD result = 0;

	if(inx >= 0 && get_inx_type(inx) == LIST_HFS) {
		get_path( inx, fullpath );
		inx2volname( inx, volpath, _MAX_PATH );
		path = fullpath.GetBuffer( _MAX_PATH );
		if(0 == HFSIFACE_get_properties( volpath, path, &ent )) {
			if(whichone == FIELD_CREATED) {
				result = ent.crdate;
			} else {
				result = ent.mddate;
			}
		}
		fullpath.ReleaseBuffer();
	}
	return(result);
}
*/

typedef struct {
	CString *cs;
	DWORD save_data;
} SortString;

int CALLBACK clCompareFunc(
	LPARAM lParam1, 
	LPARAM lParam2, 
  LPARAM lParamSort
)
{
	BOOL is_ascending = (lParamSort);

	if(!is_ascending) {
		LPARAM temp = lParam1;
		lParam1 = lParam2;
		lParam2 = temp;
	}
	// Sort strings
	if(lParam1 == 0 && lParam2 == 0) return(0);
	if(lParam1 == 0) return(-1);
	if(lParam2 == 0) return(1);
	// Compiler optimizes the variables away.
	SortString *s1 = (SortString *)lParam1, *s2 = (SortString *)lParam2;
	return( s1->cs->CompareNoCase(*(s2->cs)) );
}

// This one does NOT need to be fast, the sort callback above NEEDS to be fast,
// since this is O(n) but the callback is probably O(n*log(n))
void CHFVExplorerListView::do_report_sort(void) 
{
	int i, count, isdir, space_inx;
	CString *cs, zeroes;
	SortString *ss;

	if( (GetWindowLong(m_cl->m_hWnd,GWL_STYLE) & LVS_REPORT) == 0 ) return;

	zeroes = "000000000000000";

	count = m_cl->GetItemCount();

	for(i=0; i<count; i++) {
		ss = new SortString;
		ss->cs = cs = new CString;
		if(get_inx_type(i) == LIST_SPEC) {
			if(m_sort_is_ascending) {
				*cs = "";
			} else {
				*cs = "~~~~~";
			}
		} else {
			isdir = get_inx_isdir(i);
			*cs = m_cl->GetItemText(i,m_sorted_by_column);
			switch( m_sorted_by_column ) {
				case FIELD_NAME:
				case FIELD_TYPE:
				case FIELD_CREATOR:
				case FIELD_LABEL:
					break;
				case FIELD_SIZE:
					space_inx = cs->Find(' ');
					if(space_inx >= 0) {
						// "KB 0000000123"
						*cs = cs->Mid(space_inx+1) + zeroes.Left(10-space_inx) + cs->Left(space_inx);
					}
					break;
				case FIELD_CREATED:
				case FIELD_MODIFIED:
					// Kicked out.
					/*
					if(cs->GetLength() > 0) {
						if( isalpha(*cs[0]) ) {
							// Mac date
							DWORD date_long;
							// oh wow, what a waste of cpu cycles!
							date_long = stupid_get_date( i, m_sorted_by_column );
							cs->Format( "%09uld", date_long );
						}
					}
					*/
					DWORD dat, lo = 0, hi = 0;
					dat = m_cl->GetItemData(i);
					if(dat) {
						if(m_sorted_by_column == FIELD_CREATED) {
							get_lparam_creation( dat, &lo, &hi );
						} else {
							get_lparam_modification( dat, &lo, &hi );
						}
					}
					cs->Format( "%16u%16u", hi, lo );
					break;
			}

			if(m_sorted_by_column != FIELD_NAME) {
				// Add second sort criteria.
				*cs += m_cl->GetItemText(i,FIELD_NAME);
			}

			if(isdir == m_sort_is_ascending) {
				// force directories at start of the list
				*cs = "A" + *cs;
			} else {
				*cs = "B" + *cs;
			}
		}
		ss->save_data = m_cl->GetItemData(i);
		m_cl->SetItemData(i,(DWORD)ss);
	}

	m_cl->SortItems( clCompareFunc, m_sort_is_ascending );

	for(i=0; i<count; i++) {
		ss = (SortString *)m_cl->GetItemData(i);
		if(ss) {
			m_cl->SetItemData(i,ss->save_data);
			if(ss->cs) delete ss->cs;
			delete ss;
		}
	}
}

void CHFVExplorerListView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if(m_sorted_by_column == pNMListView->iSubItem) {
		// same column, reverse direction
		m_sort_is_ascending = !m_sort_is_ascending;
	} else {
		// column changed.
		m_sorted_by_column = pNMListView->iSubItem;
	}
	do_report_sort();
	*pResult = 0;
}

void CHFVExplorerListView::OnEditLabel() 
{
	int nItem;
	nItem = m_cl->GetNextItem(-1,LVNI_SELECTED);
	if(nItem >= 0) {
		CEdit *dummy = m_cl->EditLabel( nItem );
	}
}

/*
typedef struct tagDRAWITEMSTRUCT {
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
    HWND        hwndItem;
    HDC         hDC;
    RECT        rcItem;
    DWORD       itemData;
} DRAWITEMSTRUCT, NEAR *PDRAWITEMSTRUCT, FAR *LPDRAWITEMSTRUCT;

void CHFVExplorerListView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	int nItem;

	nItem = lpDrawItemStruct->itemID;

	if(nItem >= 0) {
		long dat;
		HICON hicon;
		CRect result_rect;
		CString text;
		RECT r;

		int icon_dim = 32;

		dat = m_cl->GetItemData( nItem );
		hicon = get_lparam_icon32(dat);
		m_cl->GetItemRect( nItem, &r, LVIR_ICON );

		int x = r.left + ((r.right - r.left) - icon_dim) / 2;
		r.left = x;
		r.right = r.left + icon_dim;

		int y = r.top + ((r.bottom - r.top) - icon_dim) / 2;
		r.top = y;
		r.bottom = r.top + icon_dim;

		if(hicon) {
			::DrawIcon( lpDrawItemStruct->hDC, r.left, r.top, hicon );
		}
	}
}

	long lStyleOld;
	lStyleOld = GetWindowLong(m_cl->m_hWnd, GWL_STYLE);
	lStyleOld |= LVS_NOLABELWRAP;
	SetWindowLong(m_cl->m_hWnd, GWL_STYLE, lStyleOld);
*/

void CHFVExplorerListView::empty_clipboard( void )
{
	m_clipboard.RemoveAll();
	m_clipboard_type.RemoveAll();
	m_clipboard_isdir.RemoveAll();
	m_clipboard_volpath.RemoveAll();
	m_clipboard_cutting.RemoveAll();
	m_clipboard_volinx.RemoveAll();
}

BOOL in_string_list( LPCSTR path, CStringArray & arr )
{
	int i, count;

	count = arr.GetSize();
	for(i=0; i<count; i++) {
		if(arr.ElementAt(i).CompareNoCase(path) == 0) return(TRUE);
	}
	return(FALSE);
}

void CHFVExplorerListView::edit_cut_copy( BOOL cutting, BOOL append )
{
	int inx = -1, count = 0, isdir, type, volinx;
	CString fullpath;
	char srcvolpath[_MAX_PATH];

	while( (inx = m_cl->GetNextItem(inx,LVNI_SELECTED)) >= 0 ) count++;

	if(count > 0) {
		inx = -1;
		if(!append) empty_clipboard();
		while( (inx = m_cl->GetNextItem(inx,LVNI_SELECTED)) >= 0 ) {
			type = get_inx_type(inx);
			if(type != LIST_SPEC) {
				if(get_path( inx, fullpath )) {
					if(!in_string_list(fullpath,m_clipboard)) {
						isdir = get_lparam_isdir(m_cl->GetItemData(inx));
						inx2volname( inx, srcvolpath, _MAX_PATH );
						volinx = get_lparam_volinx(m_cl->GetItemData(inx));
						m_clipboard.Add(fullpath);
						m_clipboard_type.Add(type);
						m_clipboard_isdir.Add(isdir);
						m_clipboard_volpath.Add(srcvolpath);
						m_clipboard_cutting.Add(cutting);
						m_clipboard_volinx.Add(volinx);
					}
				}
			}
		}
	}
}

void CHFVExplorerListView::edit_copy(void)
{
	edit_cut_copy( FALSE, FALSE );
}

void CHFVExplorerListView::edit_cut(void)
{
	edit_cut_copy( TRUE, FALSE );
}

void CHFVExplorerListView::edit_copy_append(void)
{
	edit_cut_copy( FALSE, TRUE );
}

void CHFVExplorerListView::edit_cut_append(void)
{
	edit_cut_copy( TRUE, TRUE );
}

void CHFVExplorerListView::edit_paste(void)
{
	int i, j, count, action, fat_changed = 0;
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	HTREEITEM hItem;
	CString cs;
	CTreeCtrl *tc;
	HFVExplorerTreeView *tv;
	BOOL keep_clipboard_content = FALSE;

	m_shutup_for_now = FALSE;

	hItem = pApp->m_tree->GetTreeCtrl().GetSelectedItem();
	count = m_clipboard.GetSize();

	if(hItem && count > 0) {
		tv = pApp->m_tree;
		tc = tv->m_ct;
		BOOL prevTimer = pApp->EnableTimer( FALSE );

		// Must clean up all user errors. This is important particularly for CUT

		char destname[_MAX_PATH];
		CString destdir;
		int ismac;
		DWORD dest_data, dest_volinx, dest_type;

		dest_data = tc->GetItemData( hItem );
		dest_volinx = tv->get_lparam_volinx( dest_data );
		dest_type = tv->get_lparam_type( dest_data );

		destdir = ((CHFVExplorerDoc*)m_pDocument)->get_directory( hItem, &ismac );
		lstrcpy( destname, cs.GetBuffer( _MAX_PATH ) );
		cs.ReleaseBuffer();

		BOOL looping = TRUE;
		int delete_me;
		while(looping) {
			count = m_clipboard.GetSize();
			looping = FALSE;
			delete_me = -1;
			for(i=0; i<count; i++) {

				// Target may not be a subfolder of any item. This is fatal.
				if(dest_type == m_clipboard_type.GetAt(i) && dest_volinx == m_clipboard_volinx.GetAt(i)) {
					if(m_clipboard_isdir.GetAt(i) &&
						 is_subfolder( m_clipboard.ElementAt(i), destname ))
					{
						action = m_clipboard_cutting.GetAt(i) ? ACTION_MOVE : ACTION_COPY;
						CString ActionString = action == ACTION_MOVE ? "move" : "copy";
						AfxMessageBox( 
							"Cannot " + ActionString + " \"" + m_clipboard.ElementAt(i) + "\" to folder \"" + CString(destname) + "\". The destination folder is a subfolder of the source folder."
						);
						// Gíve a chance to paste somewhere else.
						keep_clipboard_content = TRUE;
						goto fatal_exit;
					}
				}
				
				// There should not (Cutting: must not) be duplicates.
				for(j=i+1; j<count; j++) {
					if(m_clipboard_type.GetAt(i) == m_clipboard_type.GetAt(j) && m_clipboard_volinx.GetAt(i) == m_clipboard_volinx.GetAt(j)) {
						if(m_clipboard_isdir.GetAt(i) &&
							 is_subfolder( m_clipboard.ElementAt(i), m_clipboard.ElementAt(j) ))
						{
							delete_me = j;
						} else if(m_clipboard_isdir.GetAt(j) &&
							is_subfolder( m_clipboard.ElementAt(j), m_clipboard.ElementAt(i) ))
						{
							delete_me = i;
						}
						if(delete_me >= 0) {
							m_clipboard.RemoveAt(i);
							m_clipboard_isdir.RemoveAt(i);
							m_clipboard_type.RemoveAt(i);
							m_clipboard_volpath.RemoveAt(i);
							m_clipboard_cutting.RemoveAt(i);
							m_clipboard_volinx.RemoveAt(i);
							looping = TRUE;
							i = j = count;
						}
					}

					// copy_item_to_dir() takes care of the rest of the checks.
				}
			}
		}

		count = m_clipboard.GetSize();
		for(i=0; i<count; i++) {
			cs = m_clipboard.GetAt(i);
			action = m_clipboard_cutting.GetAt(i) ? ACTION_MOVE : ACTION_COPY;
			if(copy_item_to_dir( 
				-1, 
				hItem, 
				action, 
				&cs, 
				m_clipboard_isdir.GetAt(i), 
				m_clipboard_type.GetAt(i), 
				m_clipboard_volpath.GetAt(i),
				m_clipboard_volinx.GetAt(i) ))
			{
				cs = m_clipboard_volpath.GetAt(i);
				update_floppy_view(cs.GetBuffer(_MAX_PATH));
				cs.ReleaseBuffer();
			} else {
				
				// Should we break out...?
				break;

				/*
				AfxMessageBox( 
					"Could not copy or move file \"" + cs + ".",
					MB_OK | MB_ICONSTOP
				);
				*/
			}
		}

fatal_exit:
		pApp->EnableTimer( prevTimer );
		if(tv->get_lparam_type(tc->GetItemData(hItem)) == LIST_FAT) fat_changed = 1;

		if(!keep_clipboard_content) {
			for(i=m_clipboard.GetSize()-1; i>=0; i--) {
				if(m_clipboard_cutting.GetAt(i)) {
					m_clipboard.RemoveAt(i);
					m_clipboard_isdir.RemoveAt(i);
					m_clipboard_type.RemoveAt(i);
					m_clipboard_volpath.RemoveAt(i);
					m_clipboard_cutting.RemoveAt(i);
					m_clipboard_volinx.RemoveAt(i);
				}
			}
		}

		if(fat_changed) update_current_view();
	}
}

void CHFVExplorerListView::edit_select_all(void)
{
	int inx = -1;

	while( (inx = m_cl->GetNextItem(inx,LVNI_ALL)) >= 0 ) {
		m_cl->SetItemState( inx, LVIS_SELECTED, LVIS_SELECTED );
	}
}
