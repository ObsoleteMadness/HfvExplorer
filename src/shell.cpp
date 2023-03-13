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
#include "afxole.h"
#include <winnetwk.H>
#include <shlobj.h>
#include "shell.h"

// My old age and have to learn the f* ole...but I did it

static int l2exists( char *path )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok;

	fh = FindFirstFile( path, &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	if(ok) FindClose( fh );
	return(ok);
}

static void make_full_link( 
	LPSTR lpszFullPathLink, 
	LPSTR lpszPathLink,
	BOOL unique
)
{
	char name[MAX_PATH], index[100], tail[100], *p;
	int i = 1;

	strcat( lpszFullPathLink, "\\" );
	strcat( lpszFullPathLink, lpszPathLink );

	if(!unique) return;

	strcpy( name, lpszFullPathLink );

	while( l2exists(name) && i++ < 100 ) {
		strcpy( name, lpszFullPathLink );
		p = strrchr( name, '.' );
		if(p) {
			strncpy( tail, p, 99 ); 
			tail[99] = 0;
			*p = 0;
		}
		sprintf( index, " (%d)%s", i, tail );
		strcat( name, index );
	}
	strcpy( lpszFullPathLink, name );
}

HRESULT CreateLinkDesktop(
	HWND hWnd,
	LPCSTR lpszPathObj, 
  LPSTR lpszPathLink, 
	LPSTR lpszDesc,
	LPSTR lpszIconFile,
	LPSTR lpszWorkingDirectory,
	LPSTR lpszArguments,
	int showCmd,
	BOOL unique
)
{ 
	char lpszFullPathLink[MAX_PATH];
  LPITEMIDLIST pidl;
  HRESULT hres; 
  LPMALLOC ppMalloc;

	hres = CoInitialize(0);
  if (hres == S_OK) { 
		hres = SHGetSpecialFolderLocation( hWnd, CSIDL_DESKTOP, &pidl );
	  if (hres == S_OK) { 
			if(SHGetPathFromIDList(pidl,lpszFullPathLink)) {
				make_full_link( lpszFullPathLink, lpszPathLink, unique );
				hres = CreateLink( 
					lpszPathObj, 
					lpszFullPathLink, 
					lpszDesc, 
					lpszIconFile, 
					lpszWorkingDirectory, 
					lpszArguments, 
					showCmd
				);
			}
			if(SHGetMalloc( &ppMalloc ) == NOERROR) {
				ppMalloc->Free( pidl );
				ppMalloc->Release();
			}
		}
	}
	CoUninitialize();
	return( hres );
}

HRESULT CreateLink (
	LPCSTR lpszPathObj, 
  LPSTR lpszPathLink, 
	LPSTR lpszDesc,
	LPSTR lpszIconFile,
	LPSTR lpszWorkingDirectory,
	LPSTR lpszArguments,
	int showCmd
) 
{ 
  HRESULT hres; 
  IShellLink *psl; 
  IPersistFile *ppf; 

	hres = CoInitialize(0);

  // Get a pointer to the IShellLink interface. 
  hres = CoCreateInstance(
		CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, 
		IID_IShellLink, (LPVOID FAR *)&psl ); 
  if (hres == S_OK) { 
    psl->SetPath( lpszPathObj ); 
    psl->SetDescription(lpszDesc); 
		psl->SetWorkingDirectory(lpszWorkingDirectory);
		psl->SetArguments(lpszArguments);

		psl->SetIconLocation( lpszIconFile, 0 );
		psl->SetShowCmd(showCmd);

		/*
		SetHotkey
		SetRelativePath
		*/

   // Query IShellLink for the IPersistFile interface for saving the 
   // shortcut in persistent storage. 
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf); 
    if (SUCCEEDED(hres)) { 
      WORD wsz[MAX_PATH]; 
      MultiByteToWideChar( 
						CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH); 
			hres = ppf->Save((LPCOLESTR)wsz, TRUE); 
      ppf->Release(); 
    } 

    psl->Release(); 
  } 
	CoUninitialize();
  return( hres );
} 
