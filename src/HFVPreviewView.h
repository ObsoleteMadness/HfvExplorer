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

#ifndef _HPREVIEW_H_
#define _HPREVIEW_H_
/////////////////////////////////////////////////////////////////////////////
// CHFVPreviewView view

#include "HFVExplorerListView.h"

#define PANE_TREE			0
#define PANE_LIST			1
#define PANE_PREVIEW	2

class CHFVPreviewView : public CView
{
protected:
	CHFVPreviewView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHFVPreviewView)

// Attributes
public:
	int get_pane_index();

// Operations
public:
	void Initialize();
	void set_pane_index( int inx );
	void set_pane( int inx );
	void set_file( list_lparam_struct *lparam );
	void update();
	void set_show( int onoff );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHFVPreviewView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	list_lparam_struct m_lparam;
	int m_show;
	HFONT m_hfont;
	int m_lineheight;

	void show_pc_info( CDC *pdc, int *px, int *py, CRect *prect );
	void show_mac_info( CDC *pdc, int *px, int *py, CRect *prect );
	void show_contents( CDC *pdc, int *px, int *py, CRect *prect );

	virtual ~CHFVPreviewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CHFVPreviewView)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
