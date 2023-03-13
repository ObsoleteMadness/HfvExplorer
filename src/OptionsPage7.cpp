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
#include "OptionsPage7.h"
#include "shell.h"
#include "vmacpatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPage7 property page

IMPLEMENT_DYNCREATE(COptionsPage7, CPropertyPage)

COptionsPage7::COptionsPage7() : CPropertyPage(COptionsPage7::IDD)
{
  //{{AFX_DATA_INIT(COptionsPage7)
  m_vmac_path = _T("");
  m_vmac_startup_folder_name = _T("");
  m_vmac_system_file_path = _T("");
  m_launch_method = -1;
  m_use_remover = FALSE;
  //}}AFX_DATA_INIT
}

COptionsPage7::~COptionsPage7()
{
}

void COptionsPage7::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(COptionsPage7)
  DDX_Text(pDX, IDC_COPT7_VMAC_PATH, m_vmac_path);
  DDV_MaxChars(pDX, m_vmac_path, 255);
  DDX_Text(pDX, IDC_COPT7_VMAC_STARTUP_FOLDER_NAME, m_vmac_startup_folder_name);
  DDV_MaxChars(pDX, m_vmac_startup_folder_name, 255);
  DDX_Text(pDX, IDC_COPT7_VMAC_SYSTEM_FILE_PATH, m_vmac_system_file_path);
  DDV_MaxChars(pDX, m_vmac_system_file_path, 255);
  DDX_CBIndex(pDX, IDC_COPT7_LAUNCH_METHOD, m_launch_method);
  DDX_Check(pDX, IDC_USE_REMOVER, m_use_remover);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPage7, CPropertyPage)
  //{{AFX_MSG_MAP(COptionsPage7)
  ON_BN_CLICKED(IDC_COPT7_BROWSE_VMAC_PATH, OnCopt7BrowseVmacPath)
  ON_BN_CLICKED(IDC_COPT7_BROWSE_VMAC_STARTUP_FOLDER_NAME, OnCopt7BrowseVmacStartupFolderName)
  ON_BN_CLICKED(IDC_COPT7_BROWSE_VMAC_SYSTEM_FILE_PATH, OnCopt7BrowseVmacSystemFilePath)
  ON_BN_CLICKED(IDC_COPT7_VMAC_INSTALL, OnCopt7VmacInstall)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPage7 message handlers

void COptionsPage7::OnCopt7BrowseVmacStartupFolderName() 
{
  // HFS browse not yet available
}

void COptionsPage7::OnCopt7BrowseVmacPath() 
{
  GetDlgItemText( IDC_COPT7_VMAC_PATH, m_vmac_path );
  CFileDialog dlg( TRUE, _T("EXE"), m_vmac_path,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("Program Files|*.exe|All Files|*.*||") );
  if(dlg.DoModal() == IDOK) {
    m_vmac_path = dlg.GetPathName();
    SetDlgItemText( IDC_COPT7_VMAC_PATH, m_vmac_path );
  }
}

void COptionsPage7::OnCopt7BrowseVmacSystemFilePath() 
{
  GetDlgItemText( IDC_COPT7_VMAC_SYSTEM_FILE_PATH, m_vmac_system_file_path );
  CFileDialog dlg( TRUE, _T("DSK"), m_vmac_system_file_path,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("HFS volume files|*.dsk;*.hf*|All Files|*.*||") );
  if(dlg.DoModal() == IDOK) {
    m_vmac_system_file_path = dlg.GetPathName();
    SetDlgItemText( IDC_COPT7_VMAC_SYSTEM_FILE_PATH, m_vmac_system_file_path );
  }
}

/*
DrivePath1 = I:\EXECUTOR\System7.hfv
DrivePath2 = I:\EXECUTOR\EXTEMP.hfv
ROMPath1 = vMac.ROM
CurrentROMPath = 1
*/

static int l3exists( char *path )
{
  HANDLE fh;
  WIN32_FIND_DATA FindFileData;
  int ok;

  fh = FindFirstFile( path, &FindFileData );
  ok = fh != INVALID_HANDLE_VALUE;
  if(ok) FindClose( fh );
  return(ok);
}

void COptionsPage7::OnCopt7VmacInstall() 
{
  CHFVExplorerApp *pApp = (CHFVExplorerApp *)AfxGetApp();

  GetDlgItemText( IDC_COPT7_VMAC_PATH, m_vmac_path );
  if(m_vmac_path == "") {
    AfxMessageBox( "Please define the vMac executable file path first." );
    GetDlgItem(IDC_COPT7_VMAC_PATH)->SetFocus();
    return;
  }
  GetDlgItemText( IDC_COPT7_VMAC_SYSTEM_FILE_PATH, m_vmac_system_file_path );
  if(m_vmac_system_file_path == "") {
    AfxMessageBox( "Please define the location of vMac system volume file first." );
    GetDlgItem(IDC_COPT7_VMAC_SYSTEM_FILE_PATH)->SetFocus();
    return;
  }
  GetDlgItemText( IDC_COPT7_VMAC_STARTUP_FOLDER_NAME, m_vmac_startup_folder_name );
  if(m_vmac_startup_folder_name == "") {
    AfxMessageBox( "Please define the HFS startup folder first." );
    m_vmac_startup_folder_name = CString( "System Folder:Startup Items" );
    SetDlgItemText( IDC_COPT7_VMAC_STARTUP_FOLDER_NAME, m_vmac_startup_folder_name );
    GetDlgItem(IDC_COPT7_VMAC_STARTUP_FOLDER_NAME)->SetFocus();
    return;
  }

  char lpszWorkingDirectory[MAX_PATH], *p;
  GetModuleFileName( pApp->m_hInstance, lpszWorkingDirectory, MAX_PATH );
  p = strrchr( lpszWorkingDirectory, '\\' );
  if(p) *p = 0;
  strcat( lpszWorkingDirectory, "\\vMacLink" );

  if(!l3exists(lpszWorkingDirectory)) {
    if(!CreateDirectory(lpszWorkingDirectory,NULL)) {
      CString cs = "Cannot create directory \"" + CString(lpszWorkingDirectory) + "\".";
      AfxMessageBox( cs );
      return;
    }
  }

  LPSTR lpszPathObj = m_vmac_path.GetBuffer( MAX_PATH );
  char lpszPathLink[MAX_PATH];
  LPSTR lpszDesc = "Link to vMac executable file.";
  LPSTR lpszIconFile = lpszPathObj;
  LPSTR lpszArguments = "";
  int showCmd = SW_SHOWNORMAL;
  sprintf( lpszPathLink, "%s\\vMac.lnk", lpszWorkingDirectory );

  HRESULT hs = CreateLink (
    lpszPathObj,
    lpszPathLink, 
    lpszDesc,
    lpszIconFile,
    lpszWorkingDirectory,
    lpszArguments,
    showCmd
  );

  if(hs != 0) {
    CString cs = 
      "Failed to create shortcut file  \"" + 
      CString(lpszPathLink) + "\". The shortcut was supposed to point to vMac target \"" + 
      CString(lpszPathObj) + "\".";
    AfxMessageBox( cs );
    return;
  }

  // god bless the win32 1mb stack
  char vmacdir[MAX_PATH], src[MAX_PATH], dst[MAX_PATH];

  strcpy( vmacdir, lpszPathObj );
  p = strrchr( vmacdir, '\\' ); if(p) *p = 0;
  
  sprintf( src, "%s\\vMac.PRAM", vmacdir );
  sprintf( dst, "%s\\vMac.PRAM", lpszWorkingDirectory );
  if(!CopyFile( src, dst, FALSE )) {
    CString cs = "Failed to copy file  \"" + CString(src) + "\" to \"" + CString(dst) + "\".";
    AfxMessageBox( cs );
    return;
  }
  sprintf( src, "%s\\vMac.ini", vmacdir );
  sprintf( dst, "%s\\vMac.ini", lpszWorkingDirectory );
  if(!CopyFile( src, dst, FALSE )) {
    CString cs = "Failed to copy file  \"" + CString(src) + "\" to \"" + CString(dst) + "\".";
    AfxMessageBox( cs );
    return;
  }

  char *sysBuf = m_vmac_system_file_path.GetBuffer(MAX_PATH);
  if(patch_vmac_ini( dst, vmacdir, sysBuf, NULL )) {
    CString cs = "The directory \"" + CString(lpszWorkingDirectory) + "\" was succesfully prepared.";
    AfxMessageBox( cs );
  }
  m_vmac_system_file_path.ReleaseBuffer();

  m_vmac_path.ReleaseBuffer();
}

void COptionsPage7::OnOK() 
{
  CHFVExplorerApp *pApp = (CHFVExplorerApp *)AfxGetApp();
  char *p, vmacdir[MAX_PATH], dst[MAX_PATH], lpszWorkingDirectory[MAX_PATH];
  GetDlgItemText( IDC_COPT7_VMAC_PATH, m_vmac_path );
  GetDlgItemText( IDC_COPT7_VMAC_SYSTEM_FILE_PATH, m_vmac_system_file_path );
  LPSTR lpszPathObj = m_vmac_path.GetBuffer( MAX_PATH );

  strcpy( vmacdir, lpszPathObj );
  p = strrchr( vmacdir, '\\' ); if(p) *p = 0;

  GetModuleFileName( pApp->m_hInstance, lpszWorkingDirectory, MAX_PATH );
  p = strrchr( lpszWorkingDirectory, '\\' );
  if(p) *p = 0;
  strcat( lpszWorkingDirectory, "\\vMacLink" );
  
  sprintf( dst, "%s\\vMac.ini", lpszWorkingDirectory );

  char *sysBuf = m_vmac_system_file_path.GetBuffer(MAX_PATH);
  patch_vmac_ini( dst, vmacdir, sysBuf, NULL );
  m_vmac_system_file_path.ReleaseBuffer();

  m_vmac_path.ReleaseBuffer();

  CPropertyPage::OnOK();
}
