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

// #include "afx.h"
#include "stdafx.h"
#include <winioctl.h>
#include "hfs\libhfs\hfs.h"
#include "hfs\interface.h"
#include "vxdiface.h"
#include <mmsystem.h>

extern "C" {
#include "floppy.h"
#include "part.h"
#include "utils.h"
#include "sys\ntcd.h"

#define _FHOOK_INTERNAL_
#include "filehook.h"

#include "eject_nt.h"
#include "eject_w95.h"


int w95_patch_CDVSD = 1;
int w95_is_patched = 0;

// For win95.
// For this to work properly, CDVSD must patched.
// #define USE_OLD_THUNKS

// For NT
#define USE_SYS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifdef USE_OLD_THUNKS
// HFV32DLL.DLL!GETSECTORS
HMODULE thunk32inst = 0;
extern "C" {
BOOL (FAR PASCAL *get_95_cd_sectors) (BYTE,DWORD,WORD,LPBYTE) = 0;
}
#endif


// local forwards
void cd_do_final( int drive );
int do_hd_read( HANDLE h, int drive, ULONG LBA, int count, char *buf );
int do_wb_write( HANDLE h, int drive, ULONG LBA, int count, char *buf, int is_floppy );
static void writeback_final( void );
static void writeback_init( void );
int nt_floppy_write( HANDLE hfloppy, ULONG LBA, int count, char *buf );
int nt_hd_write( HANDLE h, ULONG LBA, int count, char *buf );
int w95_floppy_write( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf );
int w95_hd_write( HANDLE h, int drive, ULONG LBA, int count, char *buf );

#define FLOPPY_ALIGN_MEMORY_SIZE 512
#define CD_ALIGN_MEMORY_SIZE 2048
#define HD_ALIGN_MEMORY_SIZE 512

/*
#define FILE_DEVICE_CD_ROM      2
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define METHOD_BUFFERED         0
#define FILE_READ_ACCESS        1
*/


// #include "ntddcdrm.h"
#define IOCTL_CDROM_BASE                 FILE_DEVICE_CD_ROM
#define IOCTL_CDROM_UNLOAD_DRIVER        CTL_CODE(IOCTL_CDROM_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS)
//
// CDROM Audio Device Control Functions
//
#define IOCTL_CDROM_READ_TOC         CTL_CODE(IOCTL_CDROM_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_CONTROL      CTL_CODE(IOCTL_CDROM_BASE, 0x000D, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_PLAY_AUDIO_MSF   CTL_CODE(IOCTL_CDROM_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_SEEK_AUDIO_MSF   CTL_CODE(IOCTL_CDROM_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_STOP_AUDIO       CTL_CODE(IOCTL_CDROM_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_PAUSE_AUDIO      CTL_CODE(IOCTL_CDROM_BASE, 0x0003, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RESUME_AUDIO     CTL_CODE(IOCTL_CDROM_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_VOLUME       CTL_CODE(IOCTL_CDROM_BASE, 0x0005, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_SET_VOLUME       CTL_CODE(IOCTL_CDROM_BASE, 0x000A, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_READ_Q_CHANNEL   CTL_CODE(IOCTL_CDROM_BASE, 0x000B, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_GET_LAST_SESSION CTL_CODE(IOCTL_CDROM_BASE, 0x000E, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RAW_READ         CTL_CODE(IOCTL_CDROM_BASE, 0x000F, METHOD_OUT_DIRECT,  FILE_READ_ACCESS)
#define IOCTL_CDROM_DISK_TYPE        CTL_CODE(IOCTL_CDROM_BASE, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CDROM_GET_DRIVE_GEOMETRY CTL_CODE(IOCTL_CDROM_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS)
//
// The following device control codes are common for all class drivers.  The
// functions codes defined here must match all of the other class drivers.
//
// Warning: these codes will be replaced in the future with the IOCTL_STORAGE
// codes included below
//
#define IOCTL_CDROM_CHECK_VERIFY    CTL_CODE(IOCTL_CDROM_BASE, 0x0200, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_MEDIA_REMOVAL   CTL_CODE(IOCTL_CDROM_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_EJECT_MEDIA     CTL_CODE(IOCTL_CDROM_BASE, 0x0202, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_LOAD_MEDIA      CTL_CODE(IOCTL_CDROM_BASE, 0x0203, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RESERVE         CTL_CODE(IOCTL_CDROM_BASE, 0x0204, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_RELEASE         CTL_CODE(IOCTL_CDROM_BASE, 0x0205, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_FIND_NEW_DEVICES CTL_CODE(IOCTL_CDROM_BASE, 0x0206, METHOD_BUFFERED, FILE_READ_ACCESS)


static int m_silent = 0;

// Record floppy and cd characteristics. These
// do not change during session.
flcd_device_type flcds[MAX_DEVICES];

#define MY_FLOPPY_HANDLE_A 0xBADBABE

#define IS_OUR_FLOPPY(h) ((h) >= MY_FLOPPY_HANDLE_A && (h) < MY_FLOPPY_HANDLE_A+MAX_DEVICES)
#define GETDRINX(h) ((h)-MY_FLOPPY_HANDLE_A)

static char *sector_buffer = 0;

// Low level routines:
// count must be full sectors everywhere
// & base addresses must be aligned to sector boundaries.

// VER_PLATFORM_WIN32_WINDOWS, VER_PLATFORM_WIN32_NT 
#define VER_UNDEFINED 123456
DWORD win_os = VER_UNDEFINED;

// common for both platforms
// static HANDLE hfloppy = 0;

// LBA = (C1 * H0 + H1) * S0 + S1 - 1
// 1.44 floppy: C,H,S = 80,2,18

// Win95 vxd things
#define VWIN32_DIOC_DOS_IOCTL		1
#define VWIN32_DIOC_DOS_INT25		2
#define VWIN32_DIOC_DOS_INT26		3
#define VWIN32_DIOC_DOS_INT13		4
#define VWIN32_DIOC_DOS_DRIVEINFO	6

/*
VWIN32_DIOC_DOS_DRIVEINFO (6) 	Performs Interrupt 21h Function 730X commands. This value is supported in Windows 95 OEM Service Release 2 and later. 
VWIN32_DIOC_DOS_INT13 (4) 	Performs Interrupt 13h commands 
VWIN32_DIOC_DOS_INT25 (2) 	Performs the Absolute Disk Read command (Interrupt 25h) 
VWIN32_DIOC_DOS_INT26 (3) 	Performs the Absolute Disk Write command (Interrupt 25h) 
VWIN32_DIOC_DOS_IOCTL (1) 	Performs the specified MS-DOS device I/O control function (Interrupt 21h Function 4400h through 4411h) 
*/

typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;


// there is a trouble reading more than 8 sectors 
// from floppy - reason unknown
#define FLOPPY_READ_AHEAD_SECTORS 1
#define CD_READ_AHEAD_SECTORS 32
#define HD_READ_AHEAD_SECTORS 128

// int floppy_read_ahead_sectors = FLOPPY_READ_AHEAD_SECTORS;
// int cd_read_ahead_sectors = CD_READ_AHEAD_SECTORS;



// Floppy, HD & Removable writeback cache code. Assumes that only one
// device is written between writeback_flush_all() calls.
// Nice performance boost, but be careful when changing things.

#define HD_WRITEBACK_MAX_COUNT 65536

#define NT_FLOPPY_WRITEBACK_MAX_COUNT 65536
#define W95_FLOPPY_WRITEBACK_MAX_COUNT 512

#define WRITEBACK_MAX_COUNT (HD_WRITEBACK_MAX_COUNT)

ULONG writeback_next_LBA = (ULONG)-1;
ULONG writeback_first_LBA = 0;
ULONG writeback_count = 0;
char *writeback_buffer = 0;
HANDLE writeback_handle = 0;
int writeback_drive = 0;
int writeback_is_floppy;

static void writeback_final( void )
{
	if(writeback_buffer) VirtualFree( writeback_buffer, 0, MEM_RELEASE  ); 
}

static void writeback_init( void )
{
	writeback_buffer = (char *)VirtualAlloc( 
					NULL, 
					WRITEBACK_MAX_COUNT,
					MEM_RESERVE | MEM_COMMIT, 
					PAGE_READWRITE
	);
}

ULONG writeback_flush( HANDLE h, int drive )
{
	int retval = 0;

	if(writeback_count) {
		if(win_os == VER_PLATFORM_WIN32_NT) {
			if(writeback_is_floppy) {
				retval = nt_floppy_write( h, writeback_first_LBA, writeback_count, writeback_buffer );
			} else {
				retval = nt_hd_write( h, writeback_first_LBA, writeback_count, writeback_buffer );
			}
		} else {
			if(writeback_is_floppy) {
				retval = w95_floppy_write( h, drive, writeback_first_LBA, writeback_count, writeback_buffer );
			} else {
				retval = w95_hd_write( h, drive, writeback_first_LBA, writeback_count, writeback_buffer );
			}
		}
	}
	if((ULONG)retval != writeback_count) {
		int i = retval;
	}
	writeback_next_LBA = (ULONG)-1;
	writeback_count = 0;
	return(retval);
}

BOOL writeback_flush_all(void)
{
	ULONG oldcount = writeback_count;

	return( oldcount == writeback_flush( writeback_handle, writeback_drive ) );
}

int do_wb_write( HANDLE h, int drive, ULONG LBA, int count, char *buf, int is_floppy )
{
/*
	int retval = 0;
	retval = nt_hd_write( h, LBA, count, buf );
	return(retval);
*/
	int retval = 0;
	ULONG oldcount;

	if(drive == 9) {
		retval = 1;
		retval = 0;
	}

	if(writeback_next_LBA == LBA && drive == writeback_drive) {
		ULONG maxbytes;
		if(is_floppy) {
			if(win_os == VER_PLATFORM_WIN32_WINDOWS)
				maxbytes = W95_FLOPPY_WRITEBACK_MAX_COUNT;
			else
				maxbytes = NT_FLOPPY_WRITEBACK_MAX_COUNT;
		} else {
			maxbytes = HD_WRITEBACK_MAX_COUNT;
		}
		if(writeback_count + count <= maxbytes) {
			memcpy( &writeback_buffer[writeback_count], buf, count );
			writeback_next_LBA += (ULONG)count;
			writeback_count += count;
			return( count );
		}
	}

	oldcount = writeback_count;
	if(writeback_flush( writeback_handle, writeback_drive ) == oldcount) {
		retval = count;
	}

	memcpy( writeback_buffer, buf, count );
	writeback_is_floppy = is_floppy;
	writeback_first_LBA = LBA;
	writeback_next_LBA = LBA + (ULONG)count;
	writeback_count = count;
	writeback_handle = h;
	writeback_drive = drive;

	return(retval);
}
//// writeback cache code end


static void record_geometry( int drive, int C, int H, int S, int sector_size )
{
	flcds[drive].m_C0 = C;
	flcds[drive].m_H0 = H;
	flcds[drive].m_S0 = S;
	flcds[drive].m_H0_S0 = H * S;
	flcds[drive].sector_size = sector_size;
}

void floppy_get_geometry( 
	int drive,
	int *pC, int *pH, int *pS, int *psector_size
)
{
	*pC = flcds[drive].m_C0;
	*pH = flcds[drive].m_H0;
	*pS = flcds[drive].m_S0;
	*psector_size = flcds[drive].sector_size;
}

HANDLE w95_floppy_init( int drive )
{
	// HANDLE hfloppy = 0;

  HANDLE hVWin32 = OpenVWin32();
	if(!LockLogicalVolumeW95(hVWin32, drive + 1, 0, 0)) {
		CloseVWin32(hVWin32);
		return(0);
	}

	/*
  CloseVWin32(hVWin32);

	hfloppy = CreateFile("\\\\.\\vwin32",
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
    OPEN_EXISTING,
    FILE_FLAG_DELETE_ON_CLOSE,
    NULL
	);
	if( hfloppy == INVALID_HANDLE_VALUE ) {
		hfloppy = 0;
	}
	return(hfloppy);
	*/

  return(hVWin32);
}

int w95_reset_controller( HANDLE hfloppy, int drive )
{
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;

	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	memset( &reg, 0, sizeof(reg) );
	reg.reg_EAX = 0;
	reg.reg_EDX = drive;
	reg.reg_Flags = 0x0001;     

	fResult = DeviceIoControl(
		hfloppy, 
		VWIN32_DIOC_DOS_INT13,
		&reg, sizeof(reg), 
		&reg, sizeof(reg), 
		&cb, 0
	);
	if(!fResult || (reg.reg_Flags & 1)) {
		if(!m_silent) AfxMessageBox( "Cannot reset floppy controller" );
		fResult = 0;
	}
	return( fResult );
}

int w95_floppy_read( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf )
{
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;
	int i, sect_count, sectors_read, C, H, S;
	int H0, S0, H0S0;

	H0 = flcds[drive].m_H0;
	S0 = flcds[drive].m_S0,
	H0S0 = H0 * S0;



	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);


	// return( VxdReadHdSectors( drive, LBA, count, buf ) );


	sect_count = count / flcds[drive].sector_size;

	LBA /= (ULONG)flcds[drive].sector_size;

	C = LBA / H0S0;
	H = (LBA - C * H0S0) / S0;
	S = (LBA - (C * H0 + H) * S0) + 1;
	
	for( i=0; i<3; i++ ) {
		memset( &reg, 0, sizeof(reg) );
		reg.reg_EAX = 0x0200 | (sect_count & 0x7F); // max 128
		reg.reg_EBX = (DWORD)buf;
		reg.reg_ECX = (S & 0x3F) | 
									( (C & 0xFF) << 8) |
									( ((C >> 8) & 0x3) << 6 );
		reg.reg_EDX = drive | (H << 8);
		reg.reg_Flags = 0x0001;     

		fResult = DeviceIoControl(
			hfloppy,
			VWIN32_DIOC_DOS_INT13,
			&reg, sizeof(reg), 
			&reg, sizeof(reg), 
			&cb, 0
		);

		sectors_read = (reg.reg_EAX & 0xFF);

		if(!fResult || (reg.reg_Flags & 1) || sectors_read != sect_count) {
			fResult = FALSE;
			w95_reset_controller( hfloppy, drive );
		} else {
			break;
		}
	}
	return( sectors_read * flcds[drive].sector_size );
}

int w95_floppy_write( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf )
{
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;
	int sect_count, sectors_written, C, H, S;
	int H0, S0, H0S0;


	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);


	// return( VxdWriteHdSectors( drive, LBA, count, buf ) );


	H0 = flcds[drive].m_H0;
	S0 = flcds[drive].m_S0,
	H0S0 = H0 * S0;

	sect_count = count / flcds[drive].sector_size;

	LBA /= (ULONG)flcds[drive].sector_size;

	C = LBA / H0S0;
	H = (LBA - C * H0S0) / S0;
	S = (LBA - (C * H0 + H) * S0) + 1;

	char *buf2 = (char *)VirtualAlloc( NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	if(!buf2) return(0);
	memcpy( buf2, buf, count );

	memset( &reg, 0, sizeof(reg) );
	reg.reg_EAX = 0x0300 | (sect_count & 0x7F); // max 128
	reg.reg_EBX = (DWORD)buf2;
	reg.reg_ECX = (S & 0x3F) | 
								( (C & 0xFF) << 8) |
								( ((C >> 8) & 0x3) << 6 );
	reg.reg_EDX = drive | (H << 8);
	reg.reg_Flags = 0x0001;     

	fResult = DeviceIoControl(
		hfloppy,
		VWIN32_DIOC_DOS_INT13,
		&reg, sizeof(reg), 
		&reg, sizeof(reg), 
		&cb, 0
	);

	sectors_written = (reg.reg_EAX & 0xFF);

	if(!fResult || (reg.reg_Flags & 1) || sectors_written != sect_count) {
		w95_reset_controller( hfloppy, drive );
		fResult = FALSE;
	}

	VirtualFree( buf2, 0, MEM_RELEASE  ); 

	if(fResult) {
		return( sectors_written * flcds[drive].sector_size );
	} else {
		return( 0 );
	}
}

HANDLE nt_floppy_init( int drive )
{
	char dnamebuf[64];
	char dname[64];
	char dnamel[64];
	int rc;
	HANDLE hfloppy = 0;

	if(drive >= MAX_FLOPPIES) return(0);

	sprintf(dname,"HfvDsk_%c",(char)('A' + drive));
	sprintf(dnamel,"\\Device\\Floppy%d",drive);

	if ( QueryDosDevice(dname, dnamebuf, sizeof(dnamebuf)) == 0) {
		rc = GetLastError();
		if ( rc == ERROR_FILE_NOT_FOUND ) {
			if ( DefineDosDevice(DDD_RAW_TARGET_PATH, dname, dnamel) == 0 ) {
				return( 0);
			}
		} else {
			return(0);
		}
	}
	sprintf(dnamebuf,"\\\\.\\%s", dname);
	hfloppy = CreateFile(
		dnamebuf,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		/* | FILE_SHARE_WRITE */
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_RANDOM_ACCESS, // | FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS
		NULL
	);
	if( hfloppy == INVALID_HANDLE_VALUE ) {
		hfloppy = 0;
	}
	return(hfloppy);
}

int nt_floppy_read( HANDLE hfloppy, ULONG LBA, int count, char *buf )
{
	DWORD bytes_read;
	OVERLAPPED overl;
	char *aux_buffer = 0;
	char *rd_buffer;

	if(!hfloppy) return(0);

	if(count == 0) return(0);

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (FLOPPY_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		rd_buffer = aux_buffer;
	} else {
		rd_buffer = buf;
	}

	overl.Offset = LBA;
	overl.OffsetHigh = 0;
	overl.hEvent = NULL;

  if (!ReadFile (hfloppy, rd_buffer, count, &bytes_read, &overl)) {
		bytes_read = count = 0;
	}
	if(aux_buffer) {
		if(count) memcpy( buf, rd_buffer, count );
		VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	}
	return(bytes_read);
}

int nt_floppy_write( HANDLE hfloppy, ULONG LBA, int count, char *buf )
{
	DWORD bytes_written;
	OVERLAPPED overl;
	char *aux_buffer = 0;

	if(!hfloppy) return(0);
	if(count == 0) return(0);

	overl.Offset = LBA;
	overl.OffsetHigh = 0;
	overl.hEvent = NULL;

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (FLOPPY_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		memcpy( aux_buffer, buf, count );
		buf = aux_buffer;
	}
	
  if (WriteFile (hfloppy, buf, count, &bytes_written, &overl) &&
			(int)bytes_written == count)
	{
		;
	} else {
		bytes_written = 0;
	}
	if(aux_buffer) VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	return(bytes_written);
}

void init_os( void )
{
	OSVERSIONINFO osv;

	if(win_os != VER_UNDEFINED) return;

	win_os = VER_PLATFORM_WIN32_WINDOWS;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx( &osv )) {
		if(osv.dwPlatformId == VER_PLATFORM_WIN32s) {
			AfxMessageBox( "Cannot run on Win32s" );
		} else {
			win_os = osv.dwPlatformId;
		}
	}
}

static HANDLE xt_floppy_init( int drive )
{
	if(is_floppy_by_index(drive)) {
		if(win_os == VER_PLATFORM_WIN32_NT) {
			return(nt_floppy_init(drive));
		} else {
			return(w95_floppy_init(drive));
		}
	} else {
		return(0);
	}
}

int w95_get_floppy_geometry( HANDLE hfloppy, int drive )
{
	DWORD cb;
	BOOL fResult = 0;
	DIOC_REGISTERS reg;
	char *disk_base_table;
	int C, H, S, sector_size;

	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	memset( &reg, 0, sizeof(reg) );
	reg.reg_EAX = 0x0800;
	reg.reg_EDX = drive;
	reg.reg_Flags = 0x0001;     

	fResult = DeviceIoControl(
		hfloppy,
		VWIN32_DIOC_DOS_INT13,
		&reg, sizeof(reg), 
		&reg, sizeof(reg), 
		&cb, 0
	);

	// Clearer this way...
	C = ((reg.reg_ECX >> 8) & 0xFF) | 
	 	  (((reg.reg_ECX >> 6) & 0x3) << 8);
	H = (reg.reg_EDX >> 8) & 0xFF;
	S = reg.reg_ECX & 0x3F; 

	// convert from max values to counts
	C++;
	H++;
	// S is ok (1-based)

	sector_size = 512;
	disk_base_table = (char *)reg.reg_EDI;
	if(disk_base_table && !IsBadReadPtr(disk_base_table,11)) {
		int bps_code = (int)disk_base_table[3];
		// sanity check
		if(bps_code >= 0 && bps_code <= 5) {
			sector_size = (128 << bps_code);
		}
	}

	if(!fResult || (reg.reg_Flags & 1)) {
		fResult = FALSE;
		w95_reset_controller( hfloppy, drive );
		record_geometry( drive, 80, 2, 18, 512 );
	} else {
	  record_geometry( drive, C, H, S, sector_size );
	}
	return(fResult);
}

int nt_get_floppy_geometry( HANDLE hfloppy, int drive )
{
	DISK_GEOMETRY geom;
	DWORD cb;
	BOOL fResult = 0;

	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	fResult = DeviceIoControl(
		hfloppy, 
		IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0, 
		&geom, sizeof(DISK_GEOMETRY), 
		&cb, 0
	);
	if(fResult) {
		record_geometry(
			drive,
			geom.Cylinders.u.LowPart,
			geom.TracksPerCylinder,
			geom.SectorsPerTrack,
			geom.BytesPerSector
		);
	} else {
		record_geometry( drive, 80, 2, 18, 512 );
	}
	return(fResult);
}

int get_floppy_geometry( HANDLE hfloppy, int drive )
{
	int ok;

	if(flcds[drive].is_inited) return(1);

	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	if(win_os == VER_PLATFORM_WIN32_NT) {
		ok = nt_get_floppy_geometry(hfloppy,drive);
		if(ok) flcds[drive].is_inited = 1;
		return(ok);
	} else {
		ok = w95_get_floppy_geometry(hfloppy,drive);
		if(ok) flcds[drive].is_inited = 1;
		return(ok);
	}
}

void w95_floppy_final( int drive, HANDLE hfloppy )
{
	/*
  HANDLE hVWin32 = OpenVWin32();
  UnlockLogicalVolumeW95( hVWin32, drive + 1 );
  CloseVWin32(hVWin32);

	if(hfloppy) {
		CloseHandle(hfloppy);
	}
	*/
  UnlockLogicalVolumeW95( hfloppy, drive + 1 );
  CloseVWin32(hfloppy);
}

void nt_floppy_final( HANDLE hfloppy )
{
	if(hfloppy) {
		CloseHandle(hfloppy);
	}
}

void floppy_do_final( int drive )
{
	if(drive >= MAX_FLOPPIES) return;

	if(win_os == VER_PLATFORM_WIN32_NT) {
		nt_floppy_final(flcds[drive].handle);
	} else {
		w95_floppy_final(drive,flcds[drive].handle);
	}
	cache_clear( &flcds[drive].cache );
}

void floppy_final( int drive )
{
	if(drive >= MAX_FLOPPIES) return;

	if(!flcds[drive].handle) return;

	if(--flcds[drive].refcount > 0) return;
	flcds[drive].refcount = 0; // superfluous
	floppy_do_final(drive);
	flcds[drive].handle = 0;
}

HANDLE floppy_init( int drive, BOOL check_geometry )
{
	HANDLE hFloppy = 0;
	BOOL fResult;

	if(drive >= MAX_FLOPPIES) return(0);

	if(flcds[drive].handle) {
		flcds[drive].refcount++;
		return(flcds[drive].handle);
	}
	hFloppy = xt_floppy_init(drive);
	if(hFloppy) {
		flcds[drive].handle = hFloppy;
		flcds[drive].refcount++;
		if(check_geometry) {
			fResult = get_floppy_geometry(hFloppy,drive);
			if(!fResult) {
				if(!m_silent) AfxMessageBox( "Cannot get disk geometry" );
				floppy_final(drive);
				hFloppy = 0;
			}
		}
	}
	return(hFloppy);
}

int do_floppy_read( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf )
{
	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	if(win_os == VER_PLATFORM_WIN32_NT) {
		return(nt_floppy_read( hfloppy, LBA, count, buf ));
	} else {
		return(w95_floppy_read( hfloppy, drive, LBA, count, buf ));
	}
}

int floppy_read( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf )
{
	ULONG l1, l2, cc;
	int i, c_count, ss, nblocks, s_inx, got_bytes, first_block;
	int ok_bytes = 0;
	cachetype *cptr = &flcds[drive].cache;
	char *ptr, *ttptr = 0, *tmpbuf = 0;
	int do_read_ahead = 1;

	if(hfloppy == 0 || count <= 0) return(0);
	if(drive >= MAX_FLOPPIES) return(0);

try_again_with_no_readahead:

	ss = flcds[drive].sector_size;
	l1 = (LBA / ss) * ss;
	l2 = ((LBA+count-1+ss)/ss) * ss;
	cc = l2-l1;
	nblocks = cc / ss;
	first_block = LBA / ss;

	ptr = buf;
	s_inx = LBA-l1;
	c_count = ss - s_inx;
	if(c_count > count) c_count = count;
	for(i=0; i<nblocks; i++) {
		if(cache_get( cptr, first_block+i, sector_buffer )) {
			memcpy( ptr, sector_buffer+s_inx, c_count );
			ok_bytes += c_count;
			ptr += c_count;
			s_inx = 0;
			c_count = ss;
			if(c_count > count-ok_bytes) c_count = count-ok_bytes;
		} else {
			break;
		}
	}

	if(i != nblocks && count != ok_bytes) {
		int bytes_left, blocks_left, alignedleft;

		bytes_left = count - ok_bytes;
		blocks_left = nblocks - i;

		// NEW read ahead code:
		if(do_read_ahead) {
			if(blocks_left < FLOPPY_READ_AHEAD_SECTORS) {
				nblocks += (FLOPPY_READ_AHEAD_SECTORS - blocks_left);
				blocks_left = FLOPPY_READ_AHEAD_SECTORS;
			}
		}
		
		alignedleft = blocks_left*ss;

		tmpbuf = (char *)VirtualAlloc( 
				NULL, alignedleft, 
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(tmpbuf) {
			got_bytes = do_floppy_read( hfloppy, drive, 
						(first_block+i)*ss, alignedleft, tmpbuf );
			if(got_bytes != alignedleft) {
				// should never happen, but it does ...
				// Maybe we are formatting, try to remove read-ahead first
				if(do_read_ahead) {
					do_read_ahead = 0;
					ok_bytes = 0;
					VirtualFree( tmpbuf, 0, MEM_RELEASE  ); 
					goto try_again_with_no_readahead;
				}
				if(got_bytes < 0) got_bytes = 0;
				if(c_count > got_bytes) c_count = got_bytes;
				if(c_count > 0) {
					ttptr = tmpbuf;
					memcpy( ptr, ttptr+s_inx, c_count );
					ok_bytes += c_count;
				}
				VirtualFree( tmpbuf, 0, MEM_RELEASE  ); 
				return(ok_bytes); 
			}
			ttptr = tmpbuf;
			for( ; i<nblocks; i++ ) {
				if(c_count > 0) {
					memcpy( ptr, ttptr+s_inx, c_count );
					ok_bytes += c_count;
					ptr += c_count;
					s_inx = 0;
					c_count = ss;
					if(c_count > count-ok_bytes) c_count = count-ok_bytes;
				}
				cache_put( cptr, first_block+i, ttptr, ss );
				ttptr += ss;
			}
			VirtualFree( tmpbuf, 0, MEM_RELEASE  ); 
		}
	}

	return(ok_bytes);
}

int floppy_write( HANDLE hfloppy, int drive, ULONG LBA, int count, char *buf )
{
	ULONG l1, l2, cc;
	int nblocks, ss, i, first_block;
	int ok_bytes = 0;
	cachetype *cptr = &flcds[drive].cache;
	char *ttptr = 0;
	int bytes_written;

	if(hfloppy == 0 || count <= 0) return(0);
	if(drive >= MAX_FLOPPIES) return(0);

	bytes_written = do_wb_write( hfloppy, drive, LBA, count, buf, TRUE );

	/*
	if(win_os == VER_PLATFORM_WIN32_NT) {
		bytes_written = nt_floppy_write( hfloppy, LBA, count, buf );
	} else {
		bytes_written = w95_floppy_write( hfloppy, drive, LBA, count, buf );
	}
	*/

	count = bytes_written;

	ss = flcds[drive].sector_size;
	l1 = (LBA / ss) * ss;
	l2 = ((LBA+count-1+ss)/ss) * ss;
	cc = l2-l1;
	nblocks = cc / ss;
	first_block = LBA / ss;

	if(l1 < LBA) cache_remove( cptr, first_block-1, ss );
	if(l2 > LBA+count) cache_remove( cptr, first_block+nblocks, ss );
	ttptr = buf;
	for(i=0; i<nblocks; i++) {
		cache_put( cptr, first_block+i, ttptr, ss );
		ttptr += ss;
	}

	return(bytes_written);
}


////////////////////////////////////////////////////////////
long get_floppy_size( HANDLE hfloppy, int drive )
{
	if(!hfloppy || drive >= MAX_FLOPPIES) return(0);

	if(hfloppy) {
		return( flcds[drive].m_C0 * flcds[drive].m_H0 * 
						flcds[drive].m_S0 * flcds[drive].sector_size );
	} else {
		return(0);
	}
}

void get_floppy_volume_file_name( int drive, char *name )
{
	sprintf( name, "%c:\\NO~1_2~3.DSK", (char)('A'+drive) );
}

void get_hd_volume_file_name( int drive, char *name )
{
	sprintf( name, "%c:\\HP~1_2~3.DSK", (char)('A'+drive) );
}

static int my_stat( const char *filename, struct stat *pstat )
{
	int drive;
	char name[_MAX_PATH];

	for(drive=0; drive<MAX_FLOPPIES; drive++) {
		if(is_floppy_by_index( drive )) {
			get_floppy_volume_file_name( drive, name );
			if(stricmp( name, filename ) == 0) {
				return( -1 );
			}
		}
	}
	for(drive=2; drive<MAX_DEVICES; drive++) {
		get_cd_volume_file_name( drive, name );
		if(stricmp( name, filename ) == 0) {
				return( -1 );
		}
	}
	for(drive=2; drive<MAX_DEVICES; drive++) {
		get_hd_volume_file_name( drive, name );
		if(stricmp( name, filename ) == 0) {
				return( -1 );
		}
	}
	return(stat(filename,pstat));
}

/* RETURNS: -1 if error
errno:
EACCES Tried to open read-only file for writing, or file's sharing mode does not allow specified operations, or given path is directory
EEXIST _O_CREAT and _O_EXCL flags specified, but filename already exists
EINVAL Invalid oflag or pmode argument 
EMFILE No more file handles available (too many open files)
ENOENT
*/
static int my_open( const char *filename, int oflag, .../*int pmode*/ )
{
	int drive;
	char name[_MAX_PATH];

	for(drive=0; drive<MAX_FLOPPIES; drive++) {
		if(is_floppy_by_index( drive )) {
			get_floppy_volume_file_name( drive, name );
			if(stricmp( name, filename ) == 0) {
				flcds[drive].seekpoint = 0;
				flcds[drive].handle = floppy_init( drive, TRUE );
				flcds[drive].type = FLTYPE_FLOPPY;
				if(flcds[drive].handle == 0) {
					// errno = 
					return( -1 );
				}
				return( MY_FLOPPY_HANDLE_A + drive );
			}
		}
	}
	for(drive=2; drive<MAX_DEVICES; drive++) {
		get_cd_volume_file_name( drive, name );
		if(stricmp( name, filename ) == 0) {
			flcds[drive].seekpoint = 0;
			flcds[drive].handle = cd_init( drive, TRUE );
			flcds[drive].type = FLTYPE_CD;
			if(flcds[drive].handle == 0) {
				// errno = 
				return( -1 );
			}
			return( MY_FLOPPY_HANDLE_A + drive );
		}
	}
	for(drive=2; drive<MAX_DEVICES; drive++) {
		get_hd_volume_file_name( drive, name );
		if(stricmp( name, filename ) == 0) {
			flcds[drive].seekpoint = 0;
			flcds[drive].handle = hd_init( drive, TRUE );
			flcds[drive].type = FLTYPE_HD;
			if(flcds[drive].handle == 0) {
				// errno = 
				return( -1 );
			}
			return( MY_FLOPPY_HANDLE_A + drive );
		}
	}
	// pmode ignored.
	return(open(filename,oflag));
}

static int my_close( int handle )
{
	int drive;

	if(IS_OUR_FLOPPY(handle)) {
		drive = GETDRINX(handle);
		if(flcds[drive].handle) {
			if(flcds[drive].type == FLTYPE_FLOPPY) {
				floppy_final(drive);
			} else if(flcds[drive].type == FLTYPE_HD) {
				hd_final(drive);
			} else {
				cd_final(drive);
			}
		}
		return(0);
	} else {
		return(close(handle));
	}
}

//RETURNS: -1 if error
//errno: EBADF
static int my_read( int handle, void *buffer, unsigned int count )
{
	int ok, drive, bytes_read = -1;

	if(!IS_OUR_FLOPPY(handle))
		return(read( handle, buffer, count ));

	drive = GETDRINX(handle);
	if(!flcds[drive].handle) return(-1);

	if(flcds[drive].type == FLTYPE_FLOPPY) {
		ok = floppy_read( flcds[drive].handle, drive, 
				flcds[drive].seekpoint, count, (char *)buffer );
	} else if(flcds[drive].type == FLTYPE_HD) {
		ok = cd_read( flcds[drive].handle, drive, 
				flcds[drive].seekpoint, count, (char *)buffer, FALSE );
	} else {
		ok = cd_read( flcds[drive].handle, drive, 
				flcds[drive].seekpoint, count, (char *)buffer, TRUE );
	}

	if(!ok) {
		bytes_read = -1;
		// errno = 
	} else {
		bytes_read = count;
		flcds[drive].seekpoint += bytes_read;
	}
	return(bytes_read);
}

//RETURNS: -1 if error
// errno:
// EBADF: handle is invalid or the file is not opened for writing
// ENOSPC: no enough space
static int my_write( int handle, const void *buffer, unsigned int count )
{
	int ok, drive, bytes_written = -1;

	if(!IS_OUR_FLOPPY(handle)) 
		return(write( handle, buffer, count ));

	drive = GETDRINX(handle);
	if(!flcds[drive].handle) return(-1);

	if(flcds[drive].type == FLTYPE_FLOPPY) {
		ok = floppy_write( flcds[drive].handle, drive, flcds[drive].seekpoint, count, (char *)buffer );
	} else if(flcds[drive].type == FLTYPE_HD) {
		ok = hd_write( flcds[drive].handle, drive, flcds[drive].seekpoint, count, (char *)buffer );
	} else {
		ok = 0; // r/o device, no support for CDR/CDRW/DVD-ROM/DVD-RAM
	}
	if(!ok) {
		bytes_written = -1;
		// errno = 
	} else {
		bytes_written = count;
		flcds[drive].seekpoint += bytes_written;
	}
	return(bytes_written);
}

//RETURNS: -1 if error
/*
errno:
EBADF: invalid handle
EINVAL: bad arguments
*/
static long my_lseek( int handle, long offset, int origin )
{
	long sz;
	int drive;

	if(!IS_OUR_FLOPPY(handle)) 
		return(lseek( handle, offset, origin ));

	drive = GETDRINX(handle);
	if(!flcds[drive].handle) return(-1);

	if(flcds[drive].type == FLTYPE_FLOPPY) {
		sz = get_floppy_size(flcds[drive].handle,drive);
	} else if(flcds[drive].type == FLTYPE_HD) {
		sz = get_hd_size(flcds[drive].handle,drive);
	} else {
		sz = get_cd_size(flcds[drive].handle,drive);
	}

	switch(origin) {
		case SEEK_CUR:
			flcds[drive].seekpoint += offset;
			break;
		case SEEK_END:
			flcds[drive].seekpoint = sz + offset; // TEST: WAS: - offset
			break;
		case SEEK_SET:
			flcds[drive].seekpoint = offset;
			break;
	}
	
	if(flcds[drive].seekpoint < 0 || flcds[drive].seekpoint >= sz) {
		// flcds[drive].seekpoint = -1;
		return(-1);
	}

	return(flcds[drive].seekpoint);
}

#ifdef USE_OLD_THUNKS
static char *sthunklibname = "HFV32DLL.DLL";
#endif

void floppy_module_global_init( void )
{
	int i;
	CWinApp	*pApp;

	writeback_init();

	init_os();

	pApp = (CWinApp *)AfxGetApp();
	// set_cd_instance( pApp->m_hInstance );

#ifdef USE_OLD_THUNKS
	thunk32inst = LoadLibrary( sthunklibname );
	if(thunk32inst) {
	  get_95_cd_sectors = 
				(BOOL (FAR PASCAL *)(BYTE,DWORD,WORD,LPBYTE))
							GetProcAddress(thunk32inst,"GETSECTORS");
		if(!get_95_cd_sectors) {
			if(win_os == VER_PLATFORM_WIN32_WINDOWS) {
				AfxMessageBox( "Cannot find GETSECTORS from library " + CString(sthunklibname) + " (required to access CD-ROM)" );
			}
		}
	} else {
		if(win_os == VER_PLATFORM_WIN32_WINDOWS) {
			AfxMessageBox( "Cannot find library " + CString(sthunklibname) + " (required to access CD-ROM)" );
		}
	}
#else
	if(win_os == VER_PLATFORM_WIN32_WINDOWS) {
		if(w95_patch_CDVSD) {
			VxdPatch( 1 );
			w95_is_patched = 1;
		}
	}
#endif

#ifdef USE_SYS
	if(win_os == VER_PLATFORM_WIN32_NT) {
		CdenableSysInstallStart();
	}
#endif

	if(!sector_buffer) {
		sector_buffer = (char *)VirtualAlloc( 
					NULL, 8192, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	}
	for(i=0; i<MAX_DEVICES; i++) {
		memset( &flcds[i], 0, sizeof(flcd_device_type) );
		cache_init( &flcds[i].cache );
	}
	HFSIFACE_init(0);
	fhook_init_fake_funcs(  
			my_stat,
			my_open,
			my_close, 
			my_read, 
			my_write, 
			my_lseek
	);
	fhook_set_fake_funcs();
}

void floppy_module_global_final( void )
{
	int i;

	if(sector_buffer) {
		VirtualFree( sector_buffer, 0, MEM_RELEASE  ); 
		sector_buffer = 0;
	}
	for(i=0; i<MAX_DEVICES; i++) {
		cache_final( &flcds[i].cache );
		if(flcds[i].handle) {
			if(flcds[i].type == FLTYPE_FLOPPY) {
				floppy_do_final(i);
			} else {
				cd_do_final(i);
			}
			flcds[i].seekpoint = 0;
			flcds[i].handle = 0;
			flcds[i].refcount = 0;
		}
	}
#ifdef USE_OLD_THUNKS
	if(thunk32inst) {
		FreeLibrary( thunk32inst );
		thunk32inst = 0;
	}
#else
	if(win_os == VER_PLATFORM_WIN32_WINDOWS) {
		if(w95_is_patched) {
			VxdPatch( 0 );
			w95_is_patched = 0;
		}
	}
#endif

#ifdef USE_SYS
	if(win_os == VER_PLATFORM_WIN32_NT) {
		// CdenableSysStopRemove();
	}
#endif
	writeback_final();
}

void set_patch_option( int patch )
{
	if(w95_patch_CDVSD == patch) return;

	w95_patch_CDVSD = patch;

	if(w95_patch_CDVSD) {
		VxdPatch( 1 );
		w95_is_patched = 1;
	} else {
		VxdPatch( 0 );
		w95_is_patched = 0;
	}
}

int silencer( int shutup )
{
	int prev = m_silent;
	m_silent = shutup;
	HFSIFACE_silencer( shutup );
	return(prev);
}

int is_any_floppy_present( int drive )
{
	int result = 0;
	HANDLE hFloppy;
	int prev_silence;

	if(drive >= MAX_FLOPPIES) return(0);
	if(!is_floppy_by_index(drive)) return(0);

	prev_silence = silencer(1);
	HFSIFACE_init(0);
	hFloppy = floppy_init( drive, FALSE );
	if(hFloppy) {
		result = 1;
		floppy_final( drive );
	}
	silencer(prev_silence);
	return(result);
}

int is_hfs_floppy_present( int drive )
{
	int result = 0;
	char volpath[100];
	HANDLE hFloppy;
	int prev_silence;

//int ReadCdSectors( int drive, int LBA, int count, char *buf );
	// vxdiface_test();

	if(drive >= MAX_FLOPPIES) return(0);
	if(!is_floppy_by_index(drive)) return(0);

	prev_silence = silencer(1);
	HFSIFACE_init(0);
	hFloppy = floppy_init( drive, TRUE );
	if(hFloppy) {
		get_floppy_volume_file_name( drive, volpath );
		if(0 == HFSIFACE_mount( volpath )) {
			(void)HFSIFACE_umount( volpath );
			result = 1;
		}
		floppy_final( drive );
	}
	silencer(prev_silence);
	return(result);
}

int get_floppy_max_size( int drive )
{
	int sz = 0;
	HANDLE hFloppy;

	if(!is_floppy_by_index(drive)) return(0);

	hFloppy = floppy_init( drive, TRUE );
	if(hFloppy) {
		sz = get_floppy_size( hFloppy, drive );
		floppy_final( drive );
	}
	return(sz);
}

void clear_cache_by_drive_index( int drive )
{
	cache_clear( &flcds[drive].cache );
}

#include "hfs\interface.h"


void tstflp( void )
{
	int was_moved, drive = 0;
	char fakename[100];
	HANDLE hFloppy;

	hFloppy = floppy_init( drive, TRUE );
	if(hFloppy) {
		get_floppy_volume_file_name( drive, fakename );
		HFSIFACE_copy( 
			fakename,
			"Games #2:Dark Castle:Castle Preferences", 
			"J:\\", 
			"J:\\",
			0,
			1,
			&was_moved );
		floppy_final( drive );
	}
}

int exists( const char *path )
{
	HANDLE fh;
	WIN32_FIND_DATA FindFileData;
	int ok;

	fh = FindFirstFile( path, &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	if(ok) FindClose( fh );
	return(ok);
}

/////////////////////////////////////////////////////////////////////
HANDLE w95_cd_init( int drive )
{
	return( (HANDLE)2718 );
}

void w95_cd_final( HANDLE hcd )
{
	hcd = hcd;
}

#ifdef USE_OLD_THUNKS
int w95_cd_read( HANDLE hcd, int drive, ULONG LBA, int count, char *buf )
{
	if(!get_95_cd_sectors) return(0);

	if( get_95_cd_sectors( 
		'A' + drive, 
    LBA / flcds[drive].sector_size,
		count / flcds[drive].sector_size,
		(unsigned char*)buf ))
	{
		return( count );
	} else {
		return( 0 );
	}
	return(0);
}
#else
int w95_cd_read( HANDLE hcd, int drive, ULONG LBA, int count, char *buf )
{
	return( VxdReadCdSectors( drive, LBA, count, buf ) );
}
#endif

int w95_get_cd_geometry( HANDLE hcd, int drive )
{
	DISK_GEOMETRY geom;

	// DAMNDAMNDAMNDAMNDAMN
	// These must be LARGE ENOUGH, otherwise
	// they don't matter!
	geom.Cylinders.u.LowPart = 119;
	geom.TracksPerCylinder	 = 64;
	// geom.SectorsPerTrack		 = 32;
	geom.SectorsPerTrack		 = 100;
	geom.BytesPerSector			 = 2048;

	record_geometry(
		drive,
		geom.Cylinders.u.LowPart,
		geom.TracksPerCylinder,
		geom.SectorsPerTrack,
		geom.BytesPerSector
	);

#ifdef USE_OLD_THUNKS
	char buf[2048];
	if(!w95_cd_read( hcd, drive, 16*2048, 2048, buf )) 
		return(0);
#endif

	return(1);
}
/////////////////////////////////////////////////////////////////////

static int cd_indexes[MAX_DEVICES];
static int cd_free_index = 0;

int cd_letter2inx( int drive )
{
	if( cd_indexes[drive] != 0 ) {
		return(cd_indexes[drive]-1);
	}
	return(cd_free_index);
}

void cd_remember_index( int drive )
{
	cd_indexes[drive] = ++cd_free_index;
}

HANDLE nt_cd_init_1( int drive )
{
	char dnamebuf[64];
	char dname[64];
	char dnamel[64];
	int rc, cd_index;
	HANDLE hcd = 0;

	cd_index = cd_letter2inx( drive );

	sprintf(dname,"HfvCD_%d",cd_index);
	// sprintf(dnamel,"\\Device\\Cdrom%d",drive);
	sprintf(dnamel,"\\Device\\Cdrom%d",cd_index);

	if ( QueryDosDevice(dname, dnamebuf, sizeof(dnamebuf)) == 0) {
		rc = GetLastError();
		if ( rc == ERROR_FILE_NOT_FOUND ) {
			if ( DefineDosDevice(DDD_RAW_TARGET_PATH, dname, dnamel) == 0 ) {
				return( 0);
			}
			cd_remember_index( drive );
		} else {
			return(0);
		}
	}
	sprintf(dnamebuf,"\\\\.\\%s", dname);
	hcd = CreateFile(
		dnamebuf,
		GENERIC_READ /* | GENERIC_WRITE */,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		/* | FILE_SHARE_WRITE */
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY, // | FILE_FLAG_NO_BUFFERING | FILE_FLAG_RANDOM_ACCESS
		NULL
	);
	if( hcd == INVALID_HANDLE_VALUE ) {
		hcd = 0;
	}
	return(hcd);
}

HANDLE nt_cd_init_2( int drive )
{
	char dname[64];
	HANDLE hcd = 0;

	sprintf(dname,"\\\\.\\%c:",(char)('A' + drive));

	hcd = CreateFile (dname, GENERIC_READ,
                   FILE_SHARE_READ|FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   NULL);
	if( hcd == INVALID_HANDLE_VALUE ) {
		hcd = 0;
	}
	return(hcd);
}

static int nt_use_dos_devices = 0;

HANDLE nt_cd_init( int drive )
{
#ifdef USE_SYS
	return(nt_cd_init_2(drive));
#else
	if(nt_use_dos_devices) {
		return(nt_cd_init_1(drive));
	}	else {
		return(nt_cd_init_2(drive));
	}
#endif
}

void nt_cd_final( HANDLE hcd )
{
	if(hcd) {
		CloseHandle(hcd);
	}
}

int nt_cd_read_1( HANDLE hcd, ULONG LBA, int count, char *buf )
{
	DWORD bytes_read;
	OVERLAPPED overl;
	char *aux_buffer = 0;
	char *rd_buffer;

	if(!hcd) return(0);

	if(count == 0) return(0);

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (CD_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		rd_buffer = aux_buffer;
	} else {
		rd_buffer = buf;
	}

	overl.Offset = LBA;
	overl.OffsetHigh = 0;
	overl.hEvent = NULL;

  if (!ReadFile (hcd, rd_buffer, count, &bytes_read, &overl)) {
		bytes_read = count = 0;
	}
	if(aux_buffer) {
		if(bytes_read) memcpy( buf, rd_buffer, bytes_read );
		VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	}
	return(bytes_read);
}

typedef enum _TRACK_MODE_TYPE {
    YellowMode2,
    XAForm2,
    CDDA
} TRACK_MODE_TYPE, *PTRACK_MODE_TYPE;

typedef struct __RAW_READ_INFO {
    LARGE_INTEGER DiskOffset;
    ULONG    SectorCount;
    TRACK_MODE_TYPE TrackMode;
} RAW_READ_INFO, *PRAW_READ_INFO;

int nt_cd_read_2( HANDLE hcd, ULONG LBA, int count, char *buf )
{
	DWORD bytes_read;
	char *aux_buffer = 0;
	char *rd_buffer;

	if(!hcd) return(0);

	if(count == 0) return(0);

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (CD_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		rd_buffer = aux_buffer;
	} else {
		rd_buffer = buf;
	}

  if(SetFilePointer ( hcd, LBA, NULL, FILE_BEGIN ) == 0xFFFFFFFF) {
		bytes_read = count = 0;
	} else if (!ReadFile (hcd, rd_buffer, count, &bytes_read, NULL)) {
		bytes_read = count = 0;
	}
	if(aux_buffer) {
		if(bytes_read) memcpy( buf, rd_buffer, bytes_read );
		VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	}
	return(bytes_read);
}

int nt_cd_read( HANDLE hcd, ULONG LBA, int count, char *buf )
{
#ifdef USE_SYS
	return(CdenableSysReadCdBytes( hcd, LBA, count, buf ));
#else
	if(nt_use_dos_devices) {
		return(nt_cd_read_1(hcd,LBA,count,buf));
	}	else {
		return(f(hcd,LBA,count,buf));
	}
#endif
}

int nt_get_cd_geometry( HANDLE hcd, int drive )
{
	DISK_GEOMETRY geom;
  // PREVENT_MEDIA_REMOVAL pmrLockCDROM;
	// DWORD cb;
	BOOL fResult = 0;

	if(!hcd) return(0);

	// DAMNDAMNDAMNDAMNDAMN
	// These must be LARGE ENOUGH, otherwise
	// they don't matter!
	geom.Cylinders.u.LowPart = 119;
	geom.TracksPerCylinder	 = 64;
	// geom.SectorsPerTrack		 = 32;
	geom.SectorsPerTrack		 = 100;
	geom.BytesPerSector			 = 2048;
	fResult = 1;

	/*
  fResult = DeviceIoControl(
		hcd, 
		IOCTL_CDROM_GET_DRIVE_GEOMETRY,
    NULL, 0, 
		&geom, sizeof(geom),
		&cb, 0
	);
	if(!fResult) {
		geom.Cylinders.u.LowPart = 119;
		geom.TracksPerCylinder	 = 64;
		geom.SectorsPerTrack		 = 100;
		geom.BytesPerSector			 = 2048;
	}
	// geom.MediaType == RemovableMedia
	*/

	record_geometry(
		drive,
		geom.Cylinders.u.LowPart,
		geom.TracksPerCylinder,
		geom.SectorsPerTrack,
		geom.BytesPerSector
	);
	return(fResult);
}
///////////////////////////////////////////////////////////////


int get_cd_geometry( HANDLE hcd, int drive )
{
	int ok;

	if(flcds[drive].is_inited) return(1);

	if(!hcd) return(0);
	if(win_os == VER_PLATFORM_WIN32_NT) {
		ok = nt_get_cd_geometry(hcd,drive);
		if(ok) flcds[drive].is_inited = 1;
		return(ok);
	} else {
		ok = w95_get_cd_geometry(hcd,drive);
		if(ok) flcds[drive].is_inited = 1;
		return(ok);
	}
}

static HANDLE xt_cd_init( int drive )
{
	if(win_os == VER_PLATFORM_WIN32_NT) {
		return(nt_cd_init(drive));
	} else {
		return(w95_cd_init(drive));
	}
}

HANDLE cd_init( int drive, BOOL check_geometry )
{
	HANDLE hcd = 0;
	BOOL fResult;

	if(flcds[drive].handle) {
		flcds[drive].refcount++;
		return(flcds[drive].handle);
	}
	hcd = xt_cd_init(drive);
	if(hcd) {
		flcds[drive].handle = hcd;
		flcds[drive].refcount++;
		if(check_geometry) {
			fResult = get_cd_geometry(hcd,drive);
			if(!fResult) {
				if(!m_silent) AfxMessageBox( "Cannot get CD geometry" );
				cd_final(drive);
				hcd = 0;
			}
		}
	}
	return(hcd);
}

void cd_do_final( int drive )
{
	if(win_os == VER_PLATFORM_WIN32_NT) {
		nt_cd_final(flcds[drive].handle);
	} else {
		w95_cd_final(flcds[drive].handle);
	}
	cache_clear( &flcds[drive].cache );
}

void cd_final( int drive )
{
	if(!flcds[drive].handle) return;

	if(--flcds[drive].refcount > 0) return;
	flcds[drive].refcount = 0; // superfluous
	cd_do_final(drive);
	flcds[drive].handle = 0;
}

long get_cd_size( HANDLE hcd, int drive )
{
	if(hcd) {
		return( flcds[drive].m_C0 * flcds[drive].m_H0 * 
						flcds[drive].m_S0 * flcds[drive].sector_size );
	} else {
		return(0);
	}
}


//////////////////// CD LOGGING ////////////////////
static int cd_log_enabled = 0;
HANDLE cd_log_handle = 0;

int cd_restore_enabled = 0;
HANDLE cd_log_restore_handle = 0;

char cdrestore_name[_MAX_PATH];

void cd_log_enable( int enable, char *path )
{
	cd_log_enabled = enable;
	if(enable) {
		if(!cd_log_handle) {
			cd_log_handle = CreateFile( path, GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, CREATE_ALWAYS, 0, NULL );	
			if( cd_log_handle == INVALID_HANDLE_VALUE ) {
				cd_log_handle = 0;
				AfxMessageBox( "Could not create file " + CString(path) );
			} else {
				AfxMessageBox( "Will log 1 MB of CD traffic to file " + CString(path) );
			}
		}
	} else {
		if(cd_log_handle) {
			CloseHandle(cd_log_handle);
			cd_log_handle = 0;
		}
	}
}

void cd_log_restore_enable( int enable, char *path )
{
	cd_restore_enabled = enable;
	if(enable) {
		strcpy( cdrestore_name, path );
	} else {
		if(cd_log_restore_handle) {
			CloseHandle(cd_log_restore_handle);
			cd_log_restore_handle = 0;
			AfxMessageBox( "Stopped restore." );
		}
	}
}

void cd_log_write( HANDLE hcd, int drive, ULONG LBA, int count, char *buf, int retval )
{
	ULONG pos, bytes_written = 0;
	char header[512];

	if(cd_log_enabled && cd_log_handle) {
		memset( header, 0, sizeof(header) );
		wsprintf( 
			header, 
			"Handle: 0x%08x, drive: 0x%08x, LBA: 0x%08x, count: 0x%08x, buf: 0x%08x, returned: 0x%08x",
			(int)hcd, (int)drive, (int)LBA, (int)count, 
			(int)buf, (int)retval );
		WriteFile(cd_log_handle, header, sizeof(header), &bytes_written, NULL);
		WriteFile(cd_log_handle, buf, count, &bytes_written, NULL);
		pos = SetFilePointer ( cd_log_handle, 0, NULL, FILE_CURRENT );
		if(pos >= 1024*1024) {
			CloseHandle(cd_log_handle);
			cd_log_handle = 0;
			AfxMessageBox( "Log file closed, got 1MB" );
		}
	}
}

void cd_log_restore( HANDLE hcd, int drive, ULONG LBA, int count, char *buf, int *pretval )
{
	ULONG read_bytes;
	char header[512];

	if(!cd_log_restore_handle) {
		cd_log_restore_handle =  CreateFile( 
			cdrestore_name, GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL );	
		if( cd_log_restore_handle == INVALID_HANDLE_VALUE ) {
			cd_log_restore_handle = 0;
			AfxMessageBox( "Could not open file " + CString(cdrestore_name) );
		} else {
			AfxMessageBox( "Restoring cd from log " + CString(cdrestore_name) );
		}
	}
	if(cd_log_restore_handle) {
	  SetFilePointer ( cd_log_restore_handle, 0, NULL, FILE_BEGIN );
		for(;;) {
			if(!ReadFile(cd_log_restore_handle, header, 512, &read_bytes, NULL)) {
				*pretval = 0;
				break;
			}
			if(read_bytes != 512) {
				*pretval = 0;
				break;
			}
			int countnew;
			ULONG LBAnew;

			LBAnew = strtoul( &header[44],0,0 );
			countnew = (int)strtoul( &header[63],0,0 );
			if(LBAnew == LBA && countnew == count) {
				if(!ReadFile(cd_log_restore_handle, buf, count, &read_bytes, NULL)) {
					*pretval = 0;
					break;
				} else {
					*pretval = count;
					break;
				}
			}
		}
	}
}
//////////////////// CD LOGGING ////////////////////


int do_cd_read( HANDLE hcd, int drive, ULONG LBA, int count, char *buf )
{
	int retval;

	if(cd_restore_enabled) {
		cd_log_restore( hcd, drive, LBA, count, buf, &retval );
		return(retval);
	}

	if(win_os == VER_PLATFORM_WIN32_NT) {
		retval = nt_cd_read( hcd, LBA, count, buf );
	} else {
		retval = w95_cd_read( hcd, drive, LBA, count, buf );
	}
	if(cd_log_enabled) cd_log_write( hcd, drive, LBA, count, buf, retval );
	return(retval);
}

int cd_read( HANDLE hcd, int drive, ULONG LBA, int count, char *buf, BOOL is_cd )
{
	ULONG l1, l2, cc;
	int i, c_count, got_bytes, nblocks, s_inx, ss, first_block;
	int ok_bytes = 0;
	cachetype *cptr = &flcds[drive].cache;
	char *ptr, *ttptr = 0, *tmpbuf;

	if(hcd == 0 || count <= 0) return(0);

	LBA += (ULONG)flcds[drive].volume_start;

	ss = flcds[drive].sector_size;
	l1 = (LBA / ss) * ss;
	l2 = ((LBA+count-1+ss)/ss) * ss;
	cc = l2-l1;
	nblocks = cc / ss;
	first_block = LBA / ss;

	ptr = buf;
	s_inx = LBA-l1;
	c_count = ss - s_inx;
	if(c_count > count) c_count = count;
	for(i=0; i<nblocks; i++) {
		if(cache_get( cptr, first_block+i, sector_buffer )) {
			memcpy( ptr, sector_buffer+s_inx, c_count );
			ok_bytes += c_count;
			ptr += c_count;
			s_inx = 0;
			c_count = ss;
			if(c_count > count-ok_bytes) c_count = count-ok_bytes;
		} else {
			break;
		}
	}

	if(i != nblocks && count != ok_bytes) {
		int bytes_left, blocks_left, alignedleft;

		bytes_left = count - ok_bytes;
		blocks_left = nblocks - i;

		// NEW read ahead code:
		int ahead = is_cd ? CD_READ_AHEAD_SECTORS : HD_READ_AHEAD_SECTORS;
		if(blocks_left < ahead) {
			nblocks += (ahead - blocks_left);
			blocks_left = ahead;
		}

		alignedleft = blocks_left*ss;

		tmpbuf = (char *)VirtualAlloc( 
				NULL, alignedleft, 
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(tmpbuf) {
			if(is_cd) {
				got_bytes = do_cd_read( hcd, drive, 
							(first_block+i)*ss, alignedleft, tmpbuf );
			} else {
				got_bytes = do_hd_read( hcd, drive, 
							(first_block+i)*ss, alignedleft, tmpbuf );
			}
			if(got_bytes != alignedleft) {
				// should never happen
				// Yes it does ...
				if(got_bytes < 0) got_bytes = 0;
				if(c_count > got_bytes) c_count = got_bytes;
				if(c_count > 0) {
					ttptr = tmpbuf;
					memcpy( ptr, ttptr+s_inx, c_count );
					ok_bytes += c_count;
				}
				VirtualFree( tmpbuf, 0, MEM_RELEASE  ); 
				return(ok_bytes); 
			}
			ttptr = tmpbuf;
			for( ; i<nblocks; i++ ) {
				if(c_count > 0) {
					memcpy( ptr, ttptr+s_inx, c_count );
					ok_bytes += c_count;
					ptr += c_count;
				}
				s_inx = 0;
				c_count = ss;
				if(c_count > count-ok_bytes) c_count = count-ok_bytes;
				cache_put( cptr, first_block+i, ttptr, ss );
				ttptr += ss;
			}
			VirtualFree( tmpbuf, 0, MEM_RELEASE  ); 
		}
	}

	return(ok_bytes);
}

void get_cd_volume_file_name( int drive, char *name )
{
	sprintf( name, "%c:\\NP~1_2~3.DSK", (char)('A'+drive) );
}

/*
The reason for this code:
- MSCDEX for Win95 refuses to load sectors below 0x10.
*/
int desperate_try_mount_cd( HANDLE hcd, int drive )
{
	int block, b, subblocks, read_bytes, result=0;
	char *buf = 0;

// 95 mscdex can't handle more in single read
#define ALLOCMEMSZ 32768
#define MAJOR_BLOCKS 40
	// mem mus be aligned to flcds[drive].sector_size
	subblocks = ALLOCMEMSZ/512;

	flcds[drive].volume_start = 0;

	buf = (char *)VirtualAlloc( 
				NULL, 
				ALLOCMEMSZ,
				MEM_RESERVE | MEM_COMMIT, 
				PAGE_READWRITE );
	if(!buf) return(0);

	for(block=0; block<MAJOR_BLOCKS; block++) {
		read_bytes = cd_read( hcd, drive, block*ALLOCMEMSZ, ALLOCMEMSZ, buf, TRUE );
		if(read_bytes == ALLOCMEMSZ) {
			char *ptr = buf;
			for( b=0; b<subblocks; b++) {
				if(ptr[0] == 0x42 && ptr[1] == 0x44 &&
					 ptr[36] > 0 && ptr[36] <= 27) {
					// 0x14600;
					flcds[drive].volume_start = block*ALLOCMEMSZ + b*512;
					if(flcds[drive].volume_start >= 0x400) {
						flcds[drive].volume_start -= 0x400;
						// Point to "LK", not "DB"
						// Note: the entire boot block may be zeroed out.
						result = 1;
						break;
					}
				}
				ptr += 512;
			}
			if(result) break;
		}
	}
	VirtualFree( buf, 0, MEM_RELEASE  ); 

#undef ALLOCMEMSZ
#undef MAJOR_BLOCKS

	return(result);
}

int decent_try_mount_cd( HANDLE hcd, int drive )
{
	long start;

	flcds[drive].volume_start = 0;
	if(get_mdb_block_address( hcd, drive, &start, TRUE )) {
		flcds[drive].volume_start = start;
		return(1);
	} else {
		flcds[drive].volume_start = 0;
		return(0);
	}
}

int try_mount_cd( HANDLE hcd, int drive )
{
	if(decent_try_mount_cd(hcd,drive)) {
		return(1);
	} else {
		// This should not be needed anymore ... oh but it is!
		return(desperate_try_mount_cd(hcd,drive));
	}
}

int is_hfs_cd_present( int drive )
{
	int result = 0;
	char volpath[100];
	HANDLE hcd;
	int prev_silence;

	prev_silence = silencer(1);
	HFSIFACE_init(0);
	hcd = cd_init( drive, TRUE );
	if(hcd) {
		get_cd_volume_file_name( drive, volpath );
		result = try_mount_cd( hcd, drive );
		cd_final( drive );
	}
	silencer(prev_silence);
	return(result);
}

void dump_cd( 
	HANDLE hcd, 
	int drive, 
	char *path,
	ULONG start,
	long blocks
)
{
#ifndef USE_OLD_THUNKS
	if(win_os == VER_PLATFORM_WIN32_WINDOWS) {
		if(!w95_is_patched) {
			VxdPatch( 1 );
			w95_is_patched = 1;
		}
	}
#endif
	int i; 
	ULONG LBA = 0;
	HANDLE hf;
	char *buf = 0;
	DWORD bytes_read;
	int chunk, nIter, ok = 0;
	int save = flcds[drive].volume_start;

	if(exists(path)) DeleteFile( path );
	hf = CreateFile( path, GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, 0, NULL );	
	if( hf == INVALID_HANDLE_VALUE ) {
		AfxMessageBox( "Cannot create file " + CString(path) );
		return;
	}
	flcds[drive].volume_start = 0;
	chunk = 2048;
	nIter = blocks;
	LBA = start;
	buf = (char *)VirtualAlloc( 
			NULL, chunk, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	if(buf) {
		ok = 1;
		for( i=0; i<nIter; i++ ) {
			if(cd_read( hcd, drive, LBA, chunk, buf, TRUE ) != chunk) {
				AfxMessageBox( "Error reading from cd." );
				ok = 0;
				break;
			}
			if (!WriteFile(hf, buf, chunk, &bytes_read, NULL) ) {
				AfxMessageBox( "Error writing to file " + CString(path) );
				ok = 0;
				break;
			}
			LBA += (ULONG)chunk;
		}
		// free(buf);
		VirtualFree( buf, 0, MEM_RELEASE  ); 
	}
	CloseHandle(hf);
	if(!ok) {
		if(exists(path)) DeleteFile( path );
	}
	flcds[drive].volume_start = save;
}

extern "C" {
void dump_first_cd( 
	char *path,
	ULONG start,
	long blocks )
{
	HANDLE hcd;
	// char name[_MAX_PATH];
	char rootdir[20], letter;
	int i;

	for( letter = 'C'; letter <= 'Z'; letter++ ) {
		i = (int)( letter - 'A' );
		wsprintf( rootdir, "%c:\\", letter );
		if(GetDriveType( rootdir ) == DRIVE_CDROM) {
			if(is_hfs_cd_present(i)) {
				hcd = cd_init( i, TRUE );
				if(hcd) {
					dump_cd( hcd, i, path, start, blocks );
					cd_final( i );
				}
			}
		}
	}
}
}

// Hard disk support start

/*
HANDLE w95_hd_init( int drive )
{
	HANDLE h = 0;
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;

	h = CreateFile("\\\\.\\vwin32",
    GENERIC_READ,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
    OPEN_EXISTING,
    FILE_FLAG_DELETE_ON_CLOSE,
    NULL
	);
	if( h == INVALID_HANDLE_VALUE ) h = 0;

	if(drive == 9) {
		int x = drive;
	}

	if(h) {
		memset( &reg, 0, sizeof(reg) );
		reg.reg_EAX = 0x440D; // generic IOCTL
		reg.reg_EBX = drive+1; // bl==DriveNum, bh==0 for level 0 lock
		reg.reg_ECX = 0x084A; // device category==8, Lock Logical Volume==4A
		fResult = DeviceIoControl(
			h, 
			VWIN32_DIOC_DOS_IOCTL,
			&reg, sizeof(reg), 
			&reg, sizeof(reg), 
			&cb, 0
		);
		if(!fResult || (reg.reg_Flags & 1)) {
			if(!m_silent) AfxMessageBox( "Could not obtain level 0 lock." );
			fResult = 0;
		} else {
#if 0
			memset( &reg, 0, sizeof(reg) );
			reg.reg_EAX = 0x710D; // Reset Drive
			reg.reg_EDX = drive+1; // DriveNum
			reg.reg_ECX = 1; // Flag; flush buffers and invalidate cache.
			fResult = DeviceIoControl(
				h, 
				VWIN32_DIOC_DOS_IOCTL, // int 21h ?
				&reg, sizeof(reg), 
				&reg, sizeof(reg), 
				&cb, 0
			);
			if(!fResult || (reg.reg_Flags & 1)) {
				if(!m_silent) AfxMessageBox( "Could not flush file system buffers." );
				// fResult = 0;
			}
#endif
		}
	}
	if(fResult == 0 && h != 0) {
		CloseHandle(h);
		h = 0;
	}
	return(h);
}

void w95_hd_final( HANDLE h, int drive )
{
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;

	if(drive == 9) {
		int x = drive;
	}

	if(h) {
		memset( &reg, 0, sizeof(reg) );
		reg.reg_EAX = 0x440D; // generic IOCTL
		reg.reg_EBX = drive+1; // DriveNum
		reg.reg_ECX = 0x086A; // device category==8, Unlock Logical Volume==6A
		fResult = DeviceIoControl(
			h, 
			VWIN32_DIOC_DOS_IOCTL,
			&reg, sizeof(reg), 
			&reg, sizeof(reg), 
			&cb, 0
		);
#if 0
		if(!fResult || (reg.reg_Flags & 1)) {
			if(!m_silent) AfxMessageBox( "Could not release level 0 lock." );
			fResult = 0;
		}
#endif
		CloseHandle(h);
	}
}
*/

HANDLE w95_hd_init( int drive )
{
  int bDrive = drive + 1;
  HANDLE hVWin32 = OpenVWin32();

	if(!LockLogicalVolumeW95(hVWin32, bDrive, 0, 0)) {
	  CloseVWin32(hVWin32);
		return(0);
	}
  CloseVWin32(hVWin32);

	// Bogus handle, never used.
	return( (HANDLE)2719 );
}

void w95_hd_final( HANDLE hcd, int drive )
{
  int bDrive = drive + 1;
  HANDLE hVWin32 = OpenVWin32();

	hcd = hcd;
  UnlockLogicalVolumeW95( hVWin32, bDrive );
  CloseVWin32(hVWin32);
}

int w95_hd_read( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	return( VxdReadHdSectors( drive, LBA, count, buf ) );
}

int w95_hd_write( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	return( VxdWriteHdSectors( drive, LBA, count, buf ) );
}
/*
int w95_hd_read( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	int result = 0;
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;

	if(h) {
		memset( &reg, 0, sizeof(reg) );
		reg.reg_EAX = drive;
		reg.reg_ECX = count / flcds[drive].sector_size;
		reg.reg_EDX = LBA / flcds[drive].sector_size;
		reg.reg_EBX = (ULONG)buf;
		fResult = DeviceIoControl(
			h, 
			VWIN32_DIOC_DOS_INT25,
			&reg, sizeof(reg), 
			&reg, sizeof(reg), 
			&cb, 0
		);
		if(!fResult || (reg.reg_Flags & 1)) {
			if(!m_silent) AfxMessageBox( "Failed to read from volume." );
			fResult = 0;
		} else {
			result = count;
		}
	}
	return(result);
}

int w95_hd_write( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	int result = 0;
	DIOC_REGISTERS reg;
	BOOL fResult = 0;
	DWORD cb;

	if(h) {
		memset( &reg, 0, sizeof(reg) );
		reg.reg_EAX = drive;
		reg.reg_ECX = count / flcds[drive].sector_size;
		reg.reg_EDX = LBA / flcds[drive].sector_size;
		reg.reg_EBX = (ULONG)buf;
		fResult = DeviceIoControl(
			h, 
			VWIN32_DIOC_DOS_INT26,
			&reg, sizeof(reg), 
			&reg, sizeof(reg), 
			&cb, 0
		);
		if(!fResult || (reg.reg_Flags & 1)) {
			if(!m_silent) AfxMessageBox( "Failed to write to volume." );
			fResult = 0;
		} else {
			result = count;
		}
	}
	return(result);
}
*/

long get_hd_size( HANDLE h, int drive )
{
	if(h) {
		return( flcds[drive].m_C0 * flcds[drive].m_H0 * 
						flcds[drive].m_S0 * flcds[drive].sector_size );
	} else {
		return(0);
	}
}

int get_hd_geometry( HANDLE h, int drive )
{
	DISK_GEOMETRY geom;
	BOOL fResult = 0;

	if(!h) return(0);

	geom.Cylinders.u.LowPart = 256;
	geom.TracksPerCylinder	 = 256;
	geom.SectorsPerTrack		 = 63;
	geom.BytesPerSector			 = FLOPPY_ALIGN_MEMORY_SIZE;
	fResult = 1;

	record_geometry(
		drive,
		geom.Cylinders.u.LowPart,
		geom.TracksPerCylinder,
		geom.SectorsPerTrack,
		geom.BytesPerSector
	);
	return(fResult);
}

void nt_hd_final( HANDLE h )
{
	if(h) {
		DWORD dwBytesReturned = 0;
		DeviceIoControl(
			h,
			FSCTL_UNLOCK_VOLUME,
			NULL, 0,
			NULL, 0,
			&dwBytesReturned,
			NULL
		);
		CloseHandle(h);
	}
}

void hd_do_final( int drive )
{
	if(win_os == VER_PLATFORM_WIN32_NT) {
		nt_hd_final(flcds[drive].handle);
	} else {
		w95_hd_final(flcds[drive].handle,drive);
	}
	cache_clear( &flcds[drive].cache );
}

void hd_final( int drive )
{
	if(!flcds[drive].handle) return;

	if(--flcds[drive].refcount > 0) return;
	flcds[drive].refcount = 0; // superfluous
	hd_do_final(drive);
	flcds[drive].handle = 0;
}

/*
int nt_hd_read( HANDLE h, ULONG LBA, int count, char *buf )
{
	return(CdenableSysReadCdBytes( h, LBA, count, buf ));
}

int nt_hd_write( HANDLE h, ULONG LBA, int count, char *buf )
{
	return(CdenableSysWriteCdBytes( h, LBA, count, buf ));
}
*/

int nt_hd_read( HANDLE hcd, ULONG LBA, int count, char *buf )
{
	DWORD bytes_read;
	OVERLAPPED overl;
	char *aux_buffer = 0;
	char *rd_buffer;

	if(!hcd) return(0);

	if(count == 0) return(0);

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (HD_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		rd_buffer = aux_buffer;
	} else {
		rd_buffer = buf;
	}

	overl.Offset = LBA;
	overl.OffsetHigh = 0;
	overl.hEvent = NULL;

  if (!ReadFile (hcd, rd_buffer, count, &bytes_read, &overl)) {
		bytes_read = count = 0;
	}
	if(aux_buffer) {
		if(bytes_read) memcpy( buf, rd_buffer, bytes_read );
		VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	}
	return(bytes_read);
}

int nt_hd_write( HANDLE hcd, ULONG LBA, int count, char *buf )
{
	DWORD bytes_written;
	OVERLAPPED overl;
	char *aux_buffer = 0;
	char *rd_buffer;

	if(!hcd) return(0);

	if(count == 0) return(0);

	// If not aligned to sector boundary -> must use another buffer
	if( (unsigned long)buf & (HD_ALIGN_MEMORY_SIZE-1) ) {
		aux_buffer = (char *)VirtualAlloc( 
				NULL, count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
		if(!aux_buffer) return(0);
		rd_buffer = aux_buffer;
		memcpy( rd_buffer, buf, count );
	} else {
		rd_buffer = buf;
	}

	overl.Offset = LBA;
	overl.OffsetHigh = 0;
	overl.hEvent = NULL;

  if (!WriteFile (hcd, rd_buffer, count, &bytes_written, &overl)) {
		if(GetLastError() == ERROR_WRITE_PROTECT) {
			AfxMessageBox( "The media is write protected." );
		}
		bytes_written = count = 0;
	}

	/*
  if(SetFilePointer ( hcd, LBA, NULL, FILE_BEGIN ) == 0xFFFFFFFF) {
		bytes_written = count = 0;
  } else if (!WriteFile (hcd, rd_buffer, count, &bytes_written, NULL)) {
		if(GetLastError() == ERROR_WRITE_PROTECT) {
			AfxMessageBox( "The media is write protected." );
		}
		bytes_written = count = 0;
	}
	*/

	if(aux_buffer) {
		VirtualFree( aux_buffer, 0, MEM_RELEASE  ); 
	}
	return(bytes_written);
}

int do_hd_read( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	int retval;

	if(win_os == VER_PLATFORM_WIN32_NT) {
		retval = nt_hd_read( h, LBA, count, buf );
	} else {
		retval = w95_hd_read( h, drive, LBA, count, buf );
	}
	return(retval);
}

int hd_write( HANDLE h, int drive, ULONG LBA, int count, char *buf )
{
	ULONG l1, l2, cc;
	int i, ss, nblocks, first_block;
	int ok_bytes = 0;
	cachetype *cptr = &flcds[drive].cache;
	char *ttptr = 0;
	int bytes_written;

	if(h == 0 || count <= 0) return(0);

	LBA += flcds[drive].volume_start;

	bytes_written = do_wb_write( h, drive, LBA, count, buf, FALSE );

	count = bytes_written;

	ss = flcds[drive].sector_size;
	l1 = (LBA / ss) * ss;
	l2 = ((LBA+count-1+ss)/ss) * ss;
	cc = l2-l1;
	nblocks = cc / ss;
	first_block = LBA / ss;

	if(l1 < LBA) cache_remove( cptr, first_block-1, ss );
	if(l2 > LBA+count) cache_remove( cptr, first_block+nblocks, ss );
	ttptr = buf;
	for(i=0; i<nblocks; i++) {
		cache_put( cptr, first_block+i, ttptr, ss );
		ttptr += ss;
	}

	return(bytes_written);
}

HANDLE nt_hd_init( int drive )
{
	HANDLE h = 0;
	char dname[64];
	char dnamebuf[256];

	*dname = 0;

	sprintf(dname,"%c:",(char)('A' + drive));

	if ( QueryDosDevice(dname, dnamebuf, sizeof(dnamebuf)) == 0) {
	  return(0);
	} else {
		char *h, *p;
		h = strstr( dnamebuf, "\\Harddisk" );
		p = strstr( dnamebuf, "\\Partition1" );
		if(h == 0 || p == 0) return(0);
		h += 9;
		int inx = atoi(h);
		sprintf( dnamebuf, "\\\\.\\PHYSICALDRIVE%d", inx );
	}


	h = CreateFile( 
		dnamebuf, 
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, 
		OPEN_EXISTING, 
		FILE_FLAG_NO_BUFFERING,
		0 
	);	
	if( h == INVALID_HANDLE_VALUE ) {
		h = CreateFile( 
			dnamebuf, 
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			0, 
			OPEN_EXISTING, 
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
			0 
		);
		if( h == INVALID_HANDLE_VALUE ) {
			h = 0;
		} else {
			AfxMessageBox( "Could not get read/write access to the drive " + CString(dname) + ", mounting read-only." );
		}
	}
	if(h) {
		DWORD dwBytesReturned = 0;
		if(!DeviceIoControl(h,
			FSCTL_LOCK_VOLUME,
			NULL, 0,
			NULL, 0,
			&dwBytesReturned,
			NULL))
		{
			CloseHandle(h);
			h = 0;
		}
	}

	return(h);
}




#if 0
HANDLE nt_hd_init( int drive )
{
	char dname[64];
	HANDLE h = 0;
	/*
	DWORD flags = 
			FILE_ATTRIBUTE_NORMAL | 
			FILE_FLAG_RANDOM_ACCESS | 
			FILE_FLAG_NO_BUFFERING | 
			FILE_FLAG_WRITE_THROUGH;
	*/
	
	// DWORD flags = FILE_ATTRIBUTE_NORMAL;
	// DWORD flags = FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED;

	DWORD flags = 0;

	sprintf(dname,"\\\\.\\%c:",(char)('A' + drive));
	h = CreateFile( 
		dname, 
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		0, 
		OPEN_EXISTING, 
		flags,
		0 
	);	

	if( h == INVALID_HANDLE_VALUE ) {
		// debug
		h = CreateFile( 
			dname, 
			GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			0, 
			OPEN_EXISTING, 
			flags,
			0 
		);	
		if( h != INVALID_HANDLE_VALUE ) {
			AfxMessageBox( "Could not get read/write access to the drive " + CString(dname) + ", mounting read-only." );
		}
	}

	if( h == INVALID_HANDLE_VALUE ) {
		h = 0;
	}
	if(h) {
		DWORD dwBytesReturned = 0;
		if(!DeviceIoControl(h,
			FSCTL_LOCK_VOLUME,
			NULL, 0,
			NULL, 0,
			&dwBytesReturned,
			NULL))
		{
			CloseHandle(h);
			h = 0;
		}
	}
	return(h);
}
#endif

static HANDLE xt_hd_init( int drive )
{
	if(win_os == VER_PLATFORM_WIN32_NT) {
		return(nt_hd_init(drive));
	} else {
		return(w95_hd_init(drive));
	}
}

HANDLE hd_init( int drive, BOOL check_geometry )
{
	HANDLE h = 0;
	BOOL fResult;

	if(flcds[drive].handle) {
		flcds[drive].refcount++;
		return(flcds[drive].handle);
	}
	h = xt_hd_init(drive);
	if(h) {
		flcds[drive].handle = h;
		flcds[drive].refcount++;
		if(check_geometry) {
			fResult = get_hd_geometry(h,drive);
			if(!fResult) {
				if(!m_silent) AfxMessageBox( "Cannot get HD geometry" );
				hd_final(drive);
				h = 0;
			}
		}
	}
	return(h);
}

int desperate_try_mount_hd( HANDLE h, int drive )
{
	int block, b, subblocks, read_bytes, result=0;
	char *buf = 0;

// 95 mscdex can't handle more in single read
#define ALLOCMEMSZ 32768
#define MAJOR_BLOCKS 1
// #define MAJOR_BLOCKS 40
	// mem mus be aligned to flcds[drive].sector_size
	subblocks = ALLOCMEMSZ/512;

	flcds[drive].volume_start = 0;

	buf = (char *)VirtualAlloc( 
				NULL, 
				ALLOCMEMSZ,
				MEM_RESERVE | MEM_COMMIT, 
				PAGE_READWRITE );
	if(!buf) return(0);

	for(block=0; block<MAJOR_BLOCKS; block++) {
		read_bytes = cd_read( h, drive, block*ALLOCMEMSZ, ALLOCMEMSZ, buf, FALSE );
		if(read_bytes == ALLOCMEMSZ) {
			char *ptr = buf;
			for( b=0; b<subblocks; b++) {
				if(ptr[0] == 0x42 && ptr[1] == 0x44 &&
					 ptr[36] > 0 && ptr[36] <= 27) {
					// 0x14600;
					flcds[drive].volume_start = block*ALLOCMEMSZ + b*512;
					if(flcds[drive].volume_start >= 0x400) {
						flcds[drive].volume_start -= 0x400;
						// Point to "LK", not "DB"
						// Note: the entire boot block may be zeroed out.
						result = 1;
						break;
					}
				}
				ptr += 512;
			}
			if(result) break;
		}
	}
	VirtualFree( buf, 0, MEM_RELEASE  ); 

#undef ALLOCMEMSZ
#undef MAJOR_BLOCKS

	return(result);
}

int try_mount_hd( HANDLE h, int drive )
{
	long start;

	flcds[drive].volume_start = 0;
	if(get_mdb_block_address( h, drive, &start, FALSE )) {
		flcds[drive].volume_start = start;
		return(1);
	} else if(desperate_try_mount_hd( h, drive )) {
		return(1);
	} else {
		return(0);
	}
}

int is_hfs_hd_present( int drive )
{
	int result = 0;
	char volpath[100];
	HANDLE h;
	int prev_silence;

	prev_silence = silencer(1);
	HFSIFACE_init(0);
	h = hd_init( drive, TRUE );
	if(h) {
		get_hd_volume_file_name( drive, volpath );
		result = try_mount_hd( h, drive );
		hd_final( drive );
	}
	silencer(prev_silence);
	return(result);
}

} // extern "C"

#ifdef _STORAGE
		{ //TESTTEST
		RAW_READ_INFO tinfo;
		DWORD cb, fResult;

		tinfo.TrackMode = YellowMode2; 
		memset( &tinfo, 0, sizeof(RAW_READ_INFO) );
		tinfo.DiskOffset.u.LowPart = LBA;
		tinfo.SectorCount = count/2048;
		fResult = DeviceIoControl(
			hcd, 
			IOCTL_CDROM_RAW_READ,
			&tinfo, sizeof(RAW_READ_INFO), 
			rd_buffer, count,
			&cb, 0);

		tinfo.TrackMode = XAForm2;
		memset( &tinfo, 0, sizeof(RAW_READ_INFO) );
		tinfo.DiskOffset.u.LowPart = LBA;
		tinfo.SectorCount = count/2048;
		fResult = DeviceIoControl(
			hcd, 
			IOCTL_CDROM_RAW_READ,
			&tinfo, sizeof(RAW_READ_INFO), 
			rd_buffer, count,
			&cb, 0);

		tinfo.TrackMode = CDDA;
		memset( &tinfo, 0, sizeof(RAW_READ_INFO) );
		tinfo.DiskOffset.u.LowPart = LBA;
		tinfo.SectorCount = count/2048;
		fResult = DeviceIoControl(
			hcd, 
			IOCTL_CDROM_RAW_READ,
			&tinfo, sizeof(RAW_READ_INFO), 
			rd_buffer, count,
			&cb, 0);

	}
#endif

int eject_media_old( char letter, int reload )
{
	MCIERROR err = 0;
	MCIDEVICEID id;
	MCI_OPEN_PARMS op;
	char name[_MAX_PATH];

	sprintf( name, "%c:", letter );
	memset( &op, 0, sizeof(op) );
	op.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
	op.lpstrElementName = (LPCSTR)name;
	
	err = mciSendCommand( NULL, MCI_OPEN, MCI_OPEN_TYPE_ID | MCI_OPEN_TYPE, (DWORD)&op );
	if(err == 0) {
		id = op.wDeviceID;
		// id = MCI_ALL_DEVICE_ID;
		err = mciSendCommand( id, MCI_STOP, 0, 0 );
		// ignore STOP error
		if(reload) {
			err = mciSendCommand( id, MCI_SET, MCI_SET_DOOR_CLOSED, 0 );
		} else {
			err = mciSendCommand( id, MCI_SET, MCI_SET_DOOR_OPEN, 0 );
		}
		/*err = */ mciSendCommand( id, MCI_CLOSE, MCI_WAIT, 0 );
	}
	return( (err==0) );
}

int eject_media( char letter, int reload )
{
	int fResult = 0;
	int drive = toupper(letter) - 'A';

	if( win_os == VER_PLATFORM_WIN32_NT ) {
		// if(!reload) cd_final( drive );
		fResult = EjectVolume(letter,reload);
	} else {
		if(reload) {
			HANDLE hVWin32 = OpenVWin32();
			if(!LockLogicalVolumeW95(hVWin32, drive + 1, 0, 0)) {
				CloseVWin32(hVWin32);
				return(0);
			}
			CloseVWin32(hVWin32);
			fResult = eject_media_old( letter, TRUE );
			hVWin32 = OpenVWin32();
			UnlockLogicalVolumeW95( hVWin32, drive + 1 );
			CloseVWin32(hVWin32);
		} else {
			fResult = EjectVolumeW95(letter);
		}
	}
	return( fResult );
}

/*
Eject:
------
mciSendCommandA( MCI_ALL_DEVICE_ID, MCI_STOP, 
00000001  00000808  00000000  0064F810  

MCI_STATUS
00000001  00000814  00000100  0064F7EC  

MCI_SET
00000001  0000080D  00000100  0064F808  

Reload:
-------
MCI_STOP
00000001  00000808  00000000  0064F810  

MCI_STATUS
00000001  00000814  00000100  0064F7EC  

MCI_SET
00000001  0000080D  00000200  0064F808  

	MCI_STATUS_PARMS spar;
			memset( &spar, 0, sizeof(spar) );
			err = mciSendCommand( id, MCI_STATUS, MCI_SET_DOOR_OPEN, (DWORD)&spar );
			if(err == 0) {

	HANDLE h;
	char buf[1024];
	
	int nt_cd_read_2( HANDLE hcd, ULONG LBA, int count, char *buf );

	h = nt_open_logical( 'j' - 'a' );
	if( h != INVALID_HANDLE_VALUE ) {
		if(nt_cd_read_2( h, 1024, 1024, buf )) {
			buf[0] = 'B';
			DWORD written, seeked;

			seeked = SetFilePointer ( h, 0, NULL, FILE_END );

			if(SetFilePointer ( h, 1024, NULL, FILE_BEGIN ) != 0xFFFFFFFF) {
				if(WriteFile( h, buf, 1024, &written, NULL)) {
					buf[0] = 'B';
				}
			}
		}
		CloseHandle(h);
	}
*/
