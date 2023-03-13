/*
 * HFVExplorer
 * Copyright (C) 1997-1998 by Anygraaf Oy
 * Author: Lauri Pesonen, email: lpesonen@clinet.fi or lauri.pesonen@anygraaf.fi
 * Interface to cdenable.vxd driver.
 * See the .h file for usage
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

#include <stdio.h>
#include <windows.h>
#include "vxdiface.h"

#define CVXD_APIFUNC_1 1
#define CVXD_APIFUNC_2 2
#define CVXD_APIFUNC_3 3
#define CVXD_APIFUNC_4 4
#define CVXD_APIFUNC_5 5
#define CVXD_APIFUNC_6 6

static HANDLE get_enable( void )
{
	HANDLE hCVxD = 0;

  hCVxD = CreateFile( "\\\\.\\CDENABLE.VXD",0,0,0,CREATE_NEW,FILE_FLAG_DELETE_ON_CLOSE,0);
	return(hCVxD);
}

int VxdReadCdSectors( int drive, ULONG LBA, int count, char *buf )
{
  HANDLE      hCVxD = 0;
  DWORD       cbBytesReturned;
  DWORD       dwErrorCode;
  DWORD       ReqInfo[4];
	DWORD       ReplyInfo[4];
	int					bytes_read = 0;
	static int shutup = 0;

	char msg[256];

  hCVxD = get_enable();

  if ( hCVxD == INVALID_HANDLE_VALUE ) {
    dwErrorCode = GetLastError();
    if ( dwErrorCode == ERROR_NOT_SUPPORTED )
    {
      sprintf(msg,"Unable to open CDENABLE.VXD, \n device does not support DeviceIOCTL");
    } else {
      sprintf(
				msg,
				"Unable to open CDENABLE.VXD, Error code: %lx\r\n"
				"Please make sure that you copied the file to Windows\\System directory", 
				dwErrorCode);
    }
		if(!shutup) 
			MessageBox( 
				0, 
				msg, 
				"Installation error", 
				MB_OK | MB_ICONEXCLAMATION );
		shutup = 1;
  } else {
		ReqInfo[0] = drive;
		ReqInfo[1] = LBA;
		ReqInfo[2] = count;
		ReqInfo[3] = (DWORD)buf;
		if ( DeviceIoControl(hCVxD, CVXD_APIFUNC_1,
						(LPVOID)ReqInfo, sizeof(ReqInfo),
						(LPVOID)ReplyInfo, sizeof(ReplyInfo),
						&cbBytesReturned, NULL) )
		{
			bytes_read = ReplyInfo[0];
		} else {
			sprintf(msg,"Device does not support the requested API\n");
			if(!shutup) MessageBox( 0, msg, 0, 0 );
			shutup = 1;
		}
    CloseHandle(hCVxD);
  }
  return(bytes_read);
}

int VxdReadHdSectors( int drive, ULONG LBA, int count, char *buf )
{
  HANDLE      hCVxD = 0;
  DWORD       cbBytesReturned;
  DWORD       dwErrorCode;
  DWORD       ReqInfo[4];
	DWORD       ReplyInfo[4];
	int					bytes_read = 0;
	static int shutup = 0;

	char msg[512];

  hCVxD = get_enable();

  if ( hCVxD == INVALID_HANDLE_VALUE ) {
    dwErrorCode = GetLastError();
    if ( dwErrorCode == ERROR_NOT_SUPPORTED )
    {
      sprintf(msg,"Unable to open CDENABLE.VXD, \n device does not support DeviceIOCTL");
    } else {
      sprintf(
				msg,
				"Unable to open CDENABLE.VXD, Error code: %lx\r\n"
				"Please make sure that you copied the file to Windows\\System directory", 
				dwErrorCode);
    }
		if(!shutup) 
			MessageBox( 
				0, 
				msg, 
				"Installation error", 
				MB_OK | MB_ICONEXCLAMATION );
		shutup = 1;
  } else {
		ReqInfo[0] = drive;
		ReqInfo[1] = LBA;
		ReqInfo[2] = count;
		ReqInfo[3] = (DWORD)buf;
		if ( DeviceIoControl(hCVxD, CVXD_APIFUNC_5,
						(LPVOID)ReqInfo, sizeof(ReqInfo),
						(LPVOID)ReplyInfo, sizeof(ReplyInfo),
						&cbBytesReturned, NULL) )
		{
			bytes_read = ReplyInfo[0];
		} else {
			sprintf(
				msg,
				"Device does not support the requested API. Please make sure you copied the latest CDENABLE.VXD to the \"\\Windows\\System\" folder. If you did this already, reboot the computer."
				);
			if(!shutup) MessageBox( 0, msg, 0, 0 );
			shutup = 1;
		}
    CloseHandle(hCVxD);
  }
  return(bytes_read);
}

int VxdWriteHdSectors( int drive, ULONG LBA, int count, char *buf )
{
  HANDLE      hCVxD = 0;
  DWORD       cbBytesReturned;
  DWORD       dwErrorCode;
  DWORD       ReqInfo[4];
	DWORD       ReplyInfo[4];
	int					bytes_written = 0;
	static int shutup = 0;

	char msg[256];

  hCVxD = get_enable();

  if ( hCVxD == INVALID_HANDLE_VALUE ) {
    dwErrorCode = GetLastError();
    if ( dwErrorCode == ERROR_NOT_SUPPORTED )
    {
      sprintf(msg,"Unable to open CDENABLE.VXD, \n device does not support DeviceIOCTL");
    } else {
      sprintf(
				msg,
				"Unable to open CDENABLE.VXD, Error code: %lx\r\n"
				"Please make sure that you copied the file to Windows\\System directory", 
				dwErrorCode);
    }
		if(!shutup) 
			MessageBox( 
				0, 
				msg, 
				"Installation error", 
				MB_OK | MB_ICONEXCLAMATION );
		shutup = 1;
  } else {
		ReqInfo[0] = drive;
		ReqInfo[1] = LBA;
		ReqInfo[2] = count;
		ReqInfo[3] = (DWORD)buf;
		if ( DeviceIoControl(hCVxD, CVXD_APIFUNC_6,
						(LPVOID)ReqInfo, sizeof(ReqInfo),
						(LPVOID)ReplyInfo, sizeof(ReplyInfo),
						&cbBytesReturned, NULL) )
		{
			bytes_written = ReplyInfo[0];
		} else {
			sprintf(msg,"Device does not support the requested API\n");
			if(!shutup) MessageBox( 0, msg, 0, 0 );
			shutup = 1;
		}
    CloseHandle(hCVxD);
  }
  return(bytes_written);
}

int VxdPatch( int onoff )
{
  HANDLE      hCVxD = 0;
	int					func = 0;
  DWORD       cbBytesReturned;
  DWORD       ReqInfo[2];
	DWORD       ReplyInfo[2];

	int ok = 0;

	static DWORD patch = 0;
	static DWORD sr2 = 0;

  hCVxD = get_enable();
  if ( hCVxD != INVALID_HANDLE_VALUE ) {
		func = onoff ? CVXD_APIFUNC_3 : CVXD_APIFUNC_4;
		if(!onoff) {
			ReqInfo[0] = patch;
			ReqInfo[1] = sr2;
		}
		(void)DeviceIoControl(
						hCVxD, 
						func,
						(LPVOID)ReqInfo, sizeof(ReqInfo),
						(LPVOID)ReplyInfo, sizeof(ReplyInfo),
						&cbBytesReturned, NULL);
		if(onoff) {
			patch = ReplyInfo[0];
			sr2 = ReplyInfo[1];
			if(patch) ok = 1;
		}
    CloseHandle(hCVxD);
  }
	return(ok);
}
