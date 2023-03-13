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
#include "HFVCommandLineInfo.h"

CHFVCommandLineInfo::CHFVCommandLineInfo()
{
}

// "@!window directory"		if child
// "@directory"						if parent

void CHFVCommandLineInfo::ParseParam(
	const char* pszParam, BOOL bFlag, BOOL bLast)
{
	LPSTR s;
	int i;

	if(*pszParam == '@') {
		CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
		pApp->m_opendir = CString(pszParam).Mid( 1 );
		if(pApp->m_opendir.GetAt(0) == '!') {
			s = pApp->m_opendir.GetBuffer(MAX_PATH);
			pApp->m_parent_hwnd = (HWND)atol(&s[1]);
			pApp->m_opendir.ReleaseBuffer();
			i = pApp->m_opendir.Find(' ');
			pApp->m_opendir = pApp->m_opendir.Mid( i+1 );
			pApp->is_desktop_application = 0;
		}
	} else {
		CCommandLineInfo::ParseParam(pszParam,bFlag,bLast);
	}
}
