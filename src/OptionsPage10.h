#if !defined(AFX_OPTIONSPAGE10_H__CB6179E3_D83E_11D2_A8C8_00201881A006__INCLUDED_)
#define AFX_OPTIONSPAGE10_H__CB6179E3_D83E_11D2_A8C8_00201881A006__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OptionsPage10.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsPage10 dialog

class COptionsPage10 : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsPage10)

// Construction
public:
	COptionsPage10();
	~COptionsPage10();

// Dialog Data
	//{{AFX_DATA(COptionsPage10)
	enum { IDD = IDD_OPTIONS_PAGE_10 };
	CString	m_hpburner_path;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsPage10)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionsPage10)
	afx_msg void OnCopt10HpburnerBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSPAGE10_H__CB6179E3_D83E_11D2_A8C8_00201881A006__INCLUDED_)
