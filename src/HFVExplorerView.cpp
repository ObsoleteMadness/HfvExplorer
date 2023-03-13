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

#include "HFVExplorerDoc.h"
#include "HFVExplorerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerView

IMPLEMENT_DYNCREATE(CHFVExplorerView, CView)

BEGIN_MESSAGE_MAP(CHFVExplorerView, CView)
	//{{AFX_MSG_MAP(CHFVExplorerView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerView construction/destruction

CHFVExplorerView::CHFVExplorerView()
{
	CView::CView();
}

CHFVExplorerView::~CHFVExplorerView()
{
	CView::~CView();
}

BOOL CHFVExplorerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerView drawing

void CHFVExplorerView::OnDraw(CDC* pDC)
{
	CHFVExplorerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerView diagnostics

#ifdef _DEBUG
void CHFVExplorerView::AssertValid() const
{
	CView::AssertValid();
}

void CHFVExplorerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHFVExplorerDoc* CHFVExplorerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHFVExplorerDoc)));
	return (CHFVExplorerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHFVExplorerView message handlers
