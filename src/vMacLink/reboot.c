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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "reboot.h"

static int get_os( void )
{
  OSVERSIONINFO osv;

  osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if(GetVersionEx( &osv )) {
    return(osv.dwPlatformId);
  }
  return(VER_PLATFORM_WIN32_WINDOWS);
}

int adjust_privileges( void )
{
  HANDLE h;
  TOKEN_PRIVILEGES priv;

  if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&h)) return(0);
  LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&priv.Privileges[0].Luid);
  priv.PrivilegeCount = 1;  // privilege count
  priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  AdjustTokenPrivileges(h,FALSE,&priv,0,(PTOKEN_PRIVILEGES)NULL,0);
  if(GetLastError() != ERROR_SUCCESS) return(0);
  return(1);
}

int RebootWindows( void )
{
  int os = get_os(), retval = 1;

  if(os == VER_PLATFORM_WIN32_WINDOWS) {
    if(!ExitWindowsEx(EWX_REBOOT,0)) {
      retval = 0;
    }
  } else if(os == VER_PLATFORM_WIN32_NT) {
    (void)adjust_privileges();
    if(!ExitWindowsEx(EWX_REBOOT,0)) {
      retval = 0;
    }
  } else { // What? VER_PLATFORM_WIN32s ?
    retval = 0;
  }
  return(retval);
}
