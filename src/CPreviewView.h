// CPreviewView.h : header file
//

#ifndef _CPREVIEWVIEW_INCLUDED
#define _CPREVIEWVIEW_INCLUDED

// #define PARENTIDFLAG 0xFFFF8000ul

/////////////////////////////////////////////////////////////////////////////
// CPreviewView view

class CPreviewView : public CView
{
protected:
	CPreviewView();
	~CPreviewView();
	DECLARE_DYNCREATE(CPreviewView)

	// palette stuff

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewView)
	protected:
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
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
