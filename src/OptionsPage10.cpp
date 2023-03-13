// OptionsPage10.cpp : implementation file
//

#include "stdafx.h"
#include "hfvexplorer.h"
#include "OptionsPage10.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage10 dialog

IMPLEMENT_DYNCREATE(COptionsPage10, CPropertyPage)

COptionsPage10::COptionsPage10() : CPropertyPage(COptionsPage10::IDD)
{
	//{{AFX_DATA_INIT(COptionsPage10)
	m_hpburner_path = _T("");
	//}}AFX_DATA_INIT
}

COptionsPage10::~COptionsPage10()
{
}

void COptionsPage10::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPage10)
	DDX_Text(pDX, IDC_COPT10_HPBURNER_PATH, m_hpburner_path);
	DDV_MaxChars(pDX, m_hpburner_path, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage10, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPage10)
	ON_BN_CLICKED(IDC_COPT10_HPBURNER_BROWSE, OnCopt10HpburnerBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage10 message handlers

void COptionsPage10::OnCopt10HpburnerBrowse() 
{
	UpdateData(TRUE);
	CFileDialog dlg( TRUE, _T("EXE"), m_hpburner_path,
				OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
				_T("Program Files|*.exe|All Files|*.*||") );
	if(dlg.DoModal() == IDOK) {
		m_hpburner_path = dlg.GetPathName();
		UpdateData(FALSE);
	}
}
