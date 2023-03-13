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
#include "HFVExplorerTreeView.h"
#include "HFVExplorerDoc.h"
#include "HFVPreviewView.h"
#include "special.h"
#include "own_ids.h"
#include "MainFrm.h"
#include "utils.h"

extern "C" {
#include "floppy.h"
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HFVExplorerTreeView

IMPLEMENT_DYNCREATE(HFVExplorerTreeView, CTreeView)

#define LISTICONCOUNT 11

void HFVExplorerTreeView::set_style( long style )
{
	long lStyleOld;

	lStyleOld = GetWindowLong(m_ct->m_hWnd, GWL_STYLE);

	lStyleOld &= ~TVS_DISABLEDRAGDROP;
	lStyleOld &= ~TVS_SHOWSELALWAYS;
	lStyleOld &= ~TVS_EDITLABELS;

	lStyleOld |= TVS_HASLINES;
	lStyleOld |= TVS_HASBUTTONS;
	lStyleOld |= TVS_LINESATROOT;
	
	lStyleOld |= style;

	SetWindowLong(m_ct->m_hWnd, GWL_STYLE, lStyleOld);
}


void HFVExplorerTreeView::Initialize()
{
	CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
	if(pApp->get_font()) {
		::SendMessage( 
			m_ct->m_hWnd, 
			WM_SETFONT,
			(WPARAM)pApp->get_font(),
			MAKELPARAM(TRUE,0) );
	}

	tree_clear();
	ASSERT(m_pimagelistSmall != NULL);
	m_ct->SetImageList( m_pimagelistSmall, TVSIL_NORMAL );
}

LPARAM HFVExplorerTreeView::make_lparam( 
	int type,
	int volinx, 
	long id,
	LPSTR path )
{
	lparam_struct *p = new lparam_struct;
	if(!p) return(0);
	m_pointer_array.Add( (DWORD)p );
	p->type = type;
	p->volinx = volinx;
	p->id = id;
	if(path && *path) {
		strncpy( p->path, path, MAX_PATH-1 );
		p->path[MAX_PATH-1] = 0;
	} else {
		p->path[0] = 0;
	}
	return( (LPARAM)p );
}

int HFVExplorerTreeView::get_lparam_volinx( LPARAM lparam )
{
	lparam_struct *p = (lparam_struct *)lparam;
	if(!p) return(-1);
	// if(p->type != LIST_HFS) return(-1);
	return( p->volinx );
}

int HFVExplorerTreeView::get_lparam_id( LPARAM lparam )
{
	lparam_struct *p = (lparam_struct *)lparam;
	if(!p) return(0);
	// if(p->type != LIST_HFS) return(-1);
	return( p->id );
}

int HFVExplorerTreeView::get_lparam_type( LPARAM lparam )
{
	lparam_struct *p = (lparam_struct *)lparam;
	if(!p) return(LIST_UNKNOWN);
	return( p->type );
}

int HFVExplorerTreeView::get_htree_type( HTREEITEM htree )
{
	DWORD dat = m_ct->GetItemData( htree );
	if(!dat) return(LIST_UNKNOWN);
	return(get_lparam_type( dat ));
}

int HFVExplorerTreeView::get_lparam_path( LPARAM lparam, LPSTR lpstr, int maxl )
{
	lparam_struct *p = (lparam_struct *)lparam;

	*lpstr = 0;
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

void HFVExplorerTreeView::destroy_pointers()
{
	int i, count = m_pointer_array.GetSize();
	DWORD dw;
	lparam_struct *p;

	for(i=0; i<count; i++) {
		dw = m_pointer_array.GetAt( i );
		if(dw) {
			p = (lparam_struct *)dw;
			delete p;
		}
	}
	m_pointer_array.RemoveAll();
}

HTREEITEM HFVExplorerTreeView::tree_insert( 
	HTREEITEM root_item,
	int volinx,
	LPSTR name, 
	unsigned long id,
	unsigned long parent_id )
{
	HTREEITEM hItem, h;
	TV_INSERTSTRUCT ins;

	h = find_item( volinx, root_item, id );
	
	if( h ) return(h);
	h = find_item( volinx, root_item, parent_id );

	// if( !h ) h = m_root_item;

	ins.hParent = h;
	// ins.hInsertAfter = TVI_SORT;
	ins.hInsertAfter = TVI_LAST;
	ins.item.iImage = 0;
	ins.item.iSelectedImage = 0;
	ins.item.pszText = name;
	ins.item.lParam = make_lparam( LIST_HFS, volinx, id, 0 );
	// ins.item.state = TVIS_EXPANDED;
	ins.item.state = 0;
	ins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	hItem = m_ct->InsertItem( &ins );

	// m_ct->SetItemState( h, TVIS_EXPANDED, TVIS_EXPANDED );
	// m_ct->SelectItem( h );

	return(hItem);
}

HTREEITEM HFVExplorerTreeView::tree_fat_insert( 
	HTREEITEM root, 
	int volinx, 
	LPSTR name, 
	LPSTR fpath )
{
	TV_INSERTSTRUCT ins;

	HTREEITEM htree;
	htree = tree_find_fat( root, fpath );
	if(htree) return(htree);
	
	ins.hParent = root;
	ins.hInsertAfter = TVI_SORT; // TVI_LAST;
	ins.item.iImage = 11;
	ins.item.iSelectedImage = 11;
	ins.item.pszText = name;
	ins.item.lParam = make_lparam( LIST_FAT, volinx, 0, fpath );
	// ins.item.state = TVIS_EXPANDED;
	ins.item.state = 0;
	ins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	return( m_ct->InsertItem( &ins ) );
}

void HFVExplorerTreeView::select_by_id( HTREEITEM root_item, int volinx, unsigned long id )
{
	HTREEITEM h;
	if(id) {
		h = find_item( volinx, root_item, id );
		if(h) {
			m_ct->SelectItem( h );
		}
	}
}
void HFVExplorerTreeView::expand_dir( 
	HTREEITEM root_item,
	int volinx, 
	unsigned long parent ) 
{
	HTREEITEM h;
	h = find_item( volinx, root_item, parent );
	if(h) {
		m_ct->Expand( h, TVE_EXPAND );
	}
}

HTREEITEM HFVExplorerTreeView::find_item( 
	int volinx, HTREEITEM root, unsigned long id )
{
	HTREEITEM h, r;
	unsigned long id2;
	int vol2;

	int loopguard = 0;

	if(!root) return(0);
	id2 = get_lparam_id( m_ct->GetItemData( root ) );
	vol2 = get_lparam_volinx( m_ct->GetItemData( root ) );
	if( id2 == id && volinx == vol2 ) return(root);
	if( m_ct->ItemHasChildren(root) ) {
		h = m_ct->GetChildItem( root );
		if(h != 0 && (DWORD)h != 0xFFFFFFFF) {
			r = find_item( volinx, h, id );
			if(r) return(r);
			h = m_ct->GetNextSiblingItem( h );
			while(h) {
				r = find_item( volinx, h, id );
				if(r) return(r);
				h = m_ct->GetNextSiblingItem( h );
				if(loopguard++ > 1000) break;
			}
		}
	}
	return(0);
}

// pc version
HTREEITEM HFVExplorerTreeView::tree_find_fat( HTREEITEM root, LPSTR path )
{
	HTREEITEM h, r;
	int len1, len2;
	char p[MAX_PATH];
	long dat;

	int loopguard = 0;

	if(!root) return(0);
	
	dat = m_ct->GetItemData( root );
	if(!dat) return(0);

	if(!get_lparam_path( dat, p, MAX_PATH-1 )) return(0);
	if( stricmp(path,p) == 0 ) return(root);

	if( m_ct->ItemHasChildren(root) ) {
		len1 = strlen(path);
		len2 = strlen(p);

		if(len2 >= len1) return(0);
		if(strnicmp(p,path,min(len1,len2)) != 0) return(0);

		// now it's ok substring

		h = m_ct->GetChildItem( root );
		r = tree_find_fat( h, path );
		if(r) return(r);
		h = m_ct->GetNextSiblingItem( h );
		while(h) {
			r = tree_find_fat( h, path );
			if(r) return(r);
			h = m_ct->GetNextSiblingItem( h );
			if(loopguard++ > 1000) break;
		}
	}
	return(0);
}

unsigned long HFVExplorerTreeView::get_parent_id( 
	HTREEITEM root_item,
	int volinx, 
	unsigned long id )
{
	HTREEITEM h;
	unsigned long newid = 0;

	h = find_item( volinx, root_item, id );
	if(h) {
		h = m_ct->GetParentItem( h );
		if( h ) {
			newid = get_lparam_id( m_ct->GetItemData( h ) );
		}
	}
	return(newid);
}

CString HFVExplorerTreeView::get_directory_name( 
	HTREEITEM root_item,
	int volinx, 
	unsigned long id )
{
	CString name, sname;
	HTREEITEM h;

	int loopguard = 0;
	
	name = "";
	h = find_item( volinx, root_item, id );
	if(h) {
		while( h ) {
			sname = m_ct->GetItemText( h );
			if( name == "" ) {
				name = sname;
			} else {
				name = sname + ":" + name;
			}
			h = m_ct->GetParentItem( h );
			if(loopguard++ > 5000) break;
		}
	}
	return(name);
}

HTREEITEM HFVExplorerTreeView::initialize_volume( 
	int volinx,
	CString root_name, 
	unsigned long id,
	int is_floppy,
	int is_cd,
	int is_hd,
	int is_removable )
{
	TV_INSERTSTRUCT ins;
	LPSTR r;
	HTREEITEM item;
	
	r = root_name.GetBuffer(MAX_PATH-1);
	ins.hParent = NULL;
	// ins.hInsertAfter = TVI_SORT;
	ins.hInsertAfter = TVI_LAST;
	if(is_floppy) {
		ins.item.iImage = 6;
	} else if(is_cd) {
		ins.item.iImage = 9;
	} else if(is_hd) {
		if(is_removable) {
			ins.item.iImage = 18;
		} else {
			ins.item.iImage = 7;
		}
	} else {
		ins.item.iImage = 3;
	}
	ins.item.iSelectedImage = ins.item.iImage;
	ins.item.pszText = r;
	ins.item.lParam = make_lparam( LIST_HFS, volinx, id, 0 );
	ins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	item = m_ct->InsertItem( &ins );
	root_name.ReleaseBuffer();

	// m_ct->Expand( root_item, TVE_EXPAND );
	// m_ct->SelectItem( root_item );

	return(item);
}

HTREEITEM HFVExplorerTreeView::initialize_fat_volume( 
	int volinx,
	CString *plabel, 
	UINT type,
	CString *ppath,
	BOOL is_floppy
)
{
	TV_INSERTSTRUCT ins;
	LPSTR r, p;
	HTREEITEM item;

	r = plabel->GetBuffer(MAX_PATH-1);
	p = ppath->GetBuffer(MAX_PATH-1);
	ins.hParent = NULL;
	ins.hInsertAfter = TVI_LAST;
	switch( type ) {
		case DRIVE_REMOVABLE: 
			if(is_floppy)
				ins.item.iImage = 13; 
			else
				ins.item.iImage = 19;
			break;
		case DRIVE_FIXED:			
			ins.item.iImage = 14; 
			break;
		case DRIVE_REMOTE:		
			ins.item.iImage = 15; 
			break;
		case DRIVE_CDROM:			
			ins.item.iImage = 16; 
			break;
		case DRIVE_RAMDISK:		
			ins.item.iImage = 17; 
			break;
	}
	
	ins.item.iSelectedImage = ins.item.iImage;
	ins.item.pszText = r;
	ins.item.lParam = make_lparam( LIST_FAT, volinx, 0, p );
	ins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	item = m_ct->InsertItem( &ins );
	plabel->ReleaseBuffer();
	ppath->ReleaseBuffer();
	return(item);
}

int HFVExplorerTreeView::initialize_tree()
{
	tree_clear();
	set_style( 0 );

	m_ct->SetImageList( m_pimagelistSmall, TVSIL_NORMAL );

	/*
	TV_INSERTSTRUCT ins;
	ins.hParent = NULL;
	ins.hInsertAfter = TVI_SORT;
	ins.item.iImage = 5;
	ins.item.iSelectedImage = 5;
	ins.item.pszText = configurations_name;
	ins.item.lParam = make_lparam( LIST_SPEC, -1, configurations_id, 0 );
	ins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | TVIF_PARAM;
	//m_control_item = 
			m_ct->InsertItem( &ins );
	*/

	return(1);
}

void HFVExplorerTreeView::tree_delete_images()
{
	// what about icons created?
	m_ct->SetImageList( 0, 0 );
}

void HFVExplorerTreeView::enable_selchange(int onoff)
{
	m_selchange_enabled = onoff;
}

int HFVExplorerTreeView::tree_clear()
{
	m_selchange_enabled = 0;
	m_ct->SetImageList( 0, 0 );
	if(this && IsWindow( this->m_hWnd )) {
		m_ct->DeleteAllItems();
	}
	return(1);
}

HFVExplorerTreeView::HFVExplorerTreeView()
{
	CHFVExplorerApp	*pApp;

	m_ct = &this->GetTreeCtrl();

	m_selchange_enabled = 0;

	pApp = (CHFVExplorerApp *)AfxGetApp();

	::SendMessage( 
		m_ct->m_hWnd, 
		WM_SETFONT,
		(WPARAM)pApp->get_font(),
		MAKELPARAM(TRUE,0) );

	m_pimagelistSmall = new CImageList();

	ASSERT(m_pimagelistSmall != NULL);

	// for version 1.2.7 fixed TRUE -> ILC_MASK|ILC_COLOR24
	m_pimagelistSmall->Create(16, 16, ILC_MASK|ILC_COLOR24, LISTICONCOUNT, LISTICONCOUNT);

	// m_pimagelistSmall->Create(16, 16, TRUE, LISTICONCOUNT, LISTICONCOUNT);

	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON1));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON2));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON3));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON4));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON5));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON6));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON51));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON52));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON53));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON54));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON55));

	// index==11
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON1_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON4_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON51_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON52_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON53_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON54_YELLOW));
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON55_YELLOW));

	// index==18
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON56));
	// index==19
	m_pimagelistSmall->Add(pApp->LoadIcon(IDI_ICON56_YELLOW));
}

HFVExplorerTreeView::~HFVExplorerTreeView()
{
	destroy_pointers();
	m_selchange_enabled = 0;
	if(m_pimagelistSmall) {
		delete m_pimagelistSmall;
		m_pimagelistSmall = 0;
	}

	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();
	pApp->m_tree = 0;
}

BEGIN_MESSAGE_MAP(HFVExplorerTreeView, CTreeView)
	//{{AFX_MSG_MAP(HFVExplorerTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HFVExplorerTreeView diagnostics

#ifdef _DEBUG
void HFVExplorerTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void HFVExplorerTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// HFVExplorerTreeView message handlers

void HFVExplorerTreeView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem;
	unsigned long id = 0;
	CString name;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	int volinx, type;

	if(!m_selchange_enabled) return;

	enable_selchange(0);

	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();

	hItem = pNMTreeView->itemNew.hItem;
	if( hItem ) {
		DWORD dw = m_ct->GetItemData( hItem );
		if(dw) {
			type = get_lparam_type( dw );
			switch(type) {
				case LIST_HFS:
				case LIST_SPEC:
					id = get_lparam_id( dw );
					volinx = get_lparam_volinx( m_ct->GetItemData( hItem ) );
					name = m_ct->GetItemText( hItem );
					if(id != 0 && name != "" && m_pDocument) {
						((CHFVExplorerDoc*)m_pDocument)->open_sub_directory( volinx, id, "" );
					}
					break;
				case LIST_FAT:
					char path[MAX_PATH];
					if(get_lparam_path( dw, path, MAX_PATH-1 ) && m_pDocument) {
						volinx = get_lparam_volinx( dw );
						if(volinx >= 0) {
							((CHFVExplorerDoc*)m_pDocument)->open_fat_directory( volinx, path, 1 );
							if(pApp->m_disable_autofloppy == 0) {
								if(is_floppy_by_char(*path)) {
									if(is_hfs_floppy_present(toupper(*path) - 'A')) {
										((CHFVExplorerDoc*)m_pDocument)->CheckNewFloppy( toupper(*path) - 'A' );
									}
								}
							}
						}
					}
					break;
			}
		}
	}

	enable_selchange(1);

	*pResult = 0;
}

void HFVExplorerTreeView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	HTREEITEM hItem;
	int ok = 0;

	if( nChar == VK_RETURN ) {
		hItem = m_ct->GetSelectedItem();
		if( hItem ) {
			ok = 1,
			m_ct->Expand( hItem, TVE_TOGGLE );
		}
	} else if( nChar == VK_TAB ) {
		CHFVExplorerApp	*pApp;
		pApp = (CHFVExplorerApp *)AfxGetApp();
		// pApp->m_pre->set_pane_index( PANE_LIST );
		pApp->m_pre->set_pane( PANE_LIST );
		ok = 1;
	}
	if(!ok)	CTreeView::OnChar(nChar, nRepCnt, nFlags);
}

BOOL HFVExplorerTreeView::is_cd_item( HTREEITEM hItem )
{
	DWORD dw;
	BOOL result = FALSE;
	int volinx;

	if(!hItem) return(FALSE);
	dw = m_ct->GetItemData( hItem );
	if(dw) {
		switch(get_lparam_type( dw )) {
			case LIST_HFS:
				volinx = get_lparam_volinx( dw );
				if(((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_is_cd) result = TRUE;
				break;
			case LIST_FAT:
				char path[MAX_PATH];
				if(get_lparam_path( dw, path, MAX_PATH-1 )) {
					path[3] = 0;
					if(GetDriveType( path ) == DRIVE_CDROM) result = TRUE;
				}
				break;
		}
	}
	return(result);
}

void HFVExplorerTreeView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// long id, dat;
	CMenu cm, cpop;
	HTREEITEM hItem;
	int items_inserted = 0;

	return;

	hItem = m_ct->GetSelectedItem();
	if(cm.CreatePopupMenu()) {
		if( is_cd_item(hItem) ) {
			cm.AppendMenu( MF_STRING | MF_ENABLED, ID_OWN_EJECT_MEDIA, "&Eject CD" );
			items_inserted++;
		}
		if(items_inserted) {
			// cm.AppendMenu( MF_SEPARATOR | MF_DISABLED | MF_GRAYED, 0, "" );
			if(cm.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this )) {
			}
		}
		cm.DestroyMenu();
	}
}

BOOL HFVExplorerTreeView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int handled = 1;
	UINT cmd = LOWORD(wParam);
	int volinx, type;
	CString *pcs;
	HTREEITEM hItem;
	DWORD dw;

	switch( cmd ) {
		// NOT USED
		case ID_OWN_EJECT_MEDIA:
			hItem = m_ct->GetSelectedItem();
			if( is_cd_item(hItem) ) {
				dw = m_ct->GetItemData( hItem );
				if(dw) {
					type = get_lparam_type( dw );
					volinx = get_lparam_volinx( dw );
					if(type==LIST_HFS) {
						pcs = &((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_file_name;
					} else {
						pcs = &((CHFVExplorerDoc*)m_pDocument)->m_fats[volinx].m_volume_name;
					}
					if(*pcs != "") {
						eject_media( pcs->GetAt(0), 0 );
						handled = 1;
					}
				}
			}
			break;
		default:
			handled = 0;
	}
	if(handled) 
		return(1);
	else
		return CTreeView::OnCommand(wParam, lParam);
}

void HFVExplorerTreeView::delete_subdir_contents( HTREEITEM root )
{
	HTREEITEM h;
	int loopguard = 0;

	if(!root) return;
	while( m_ct->ItemHasChildren(root) ) {
		h = m_ct->GetChildItem( root );
		delete_subdir_contents( h );
		if(!m_ct->DeleteItem( h )) break;
	}
}

void HFVExplorerTreeView::on_volume_changed( int volinx )
{
	CString currdir;
	int mac;
	HTREEITEM root;
	CHFVExplorerApp	*pApp;

	pApp = (CHFVExplorerApp *)AfxGetApp();

	root = ((CHFVExplorerDoc*)m_pDocument)->m_volumes[volinx].m_rootitem;
	currdir = ((CHFVExplorerDoc*)m_pDocument)->get_directory( 0, &mac );
	pApp->m_tree->enable_selchange(0);
	delete_subdir_contents( root );
	pApp->m_tree->enable_selchange(1);
	if(currdir != "") {
		((CHFVExplorerDoc*)m_pDocument)->my_chdir( currdir, mac );
	}
}

void HFVExplorerTreeView::mode_changed( void )
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
	if(hItem) {
		GetTreeCtrl().SelectItem( 0 );
		GetTreeCtrl().EnsureVisible( hItem );
		GetTreeCtrl().SelectItem( hItem );
	}
}

/*
void HFVExplorerTreeView::delete_by_path( LPCSTR path )
{
	HTREEITEM htree;

	htree = tree_find_fat( m_volumes[pitem->volinx].m_rootitem, path );
	if(htree) {
		m_ct->DeleteItem(htree);
	}
}
*/
