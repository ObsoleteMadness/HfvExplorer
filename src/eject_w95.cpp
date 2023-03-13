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
 *
 * This file is derived from Microsoft sample code.
 *
 */

// #include "afx.h"
#include "stdafx.h"
#include <winioctl.h>

// Prototypes

extern "C" {

#include "eject_w95.h"
#include <stdio.h>
#include <ctype.h>

//-----------------------------------------------------------------------
// DeviceIoControl infrastructure

#if !defined (VWIN32_DIOC_DOS_IOCTL) 
#define VWIN32_DIOC_DOS_IOCTL      1

typedef struct _DIOC_REGISTERS { 
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS, *PDIOC_REGISTERS;
#endif 

// Intel x86 processor status flags
#define CARRY_FLAG             0x0001

//-----------------------------------------------------------------------
// DOS IOCTL function support

#pragma pack(1) 

// Parameters for locking/unlocking removable media
typedef struct _PARAMBLOCK { 
   BYTE bOperation;
   BYTE bNumLocks;
} PARAMBLOCK, *PPARAMBLOCK; 
#pragma pack() 


//-----------------------------------------------------------------------
// Win95 low-level media unlocking/removal support

BOOL UnlockMedia (HANDLE hVWin32, BYTE bDrive); 
BOOL EjectMedia (HANDLE hVWin32, BYTE bDrive); 

/*-----------------------------------------------------------------------
Ejects media from the specified drive, if the media 
is removable and the device supports software-controlled media removal. 

The command line arguments are: 


   eject drive

For example: 
   eject E:

This code works on Windows 95 only, but does not check for Windows 95. 
-----------------------------------------------------------------------*/ 
BOOL EjectVolumeW95(TCHAR cDriveLetter)
{ 
   HANDLE hVWin32      = INVALID_HANDLE_VALUE;
   BYTE   bDrive;
   BOOL   fDriveLocked = FALSE;
   BOOL   result = FALSE;

   // convert command line arg 1 from a drive letter to a DOS drive
   // number
   bDrive = (toupper (cDriveLetter) - 'A') + 1;

   hVWin32 = OpenVWin32();

   // Make sure no other applications are using the drive.
   fDriveLocked = LockLogicalVolumeW95(hVWin32, bDrive, 0, 0);
   if (!fDriveLocked)
   {
      /* printf("volume %c is in use by another application; therefore, it "
             "cannot be ejected\n", 'A' + bDrive - 1); */
		 if(0) {
			 goto CLEANUP_AND_EXIT_APP;
		 }
   }

   // Make sure there is no software lock keeping the media in the drive.
   if (!UnlockMedia (hVWin32, bDrive))
   {
      // printf("could not unlock media from drive %c:\n", 'A' + bDrive - 1);
      goto CLEANUP_AND_EXIT_APP;
   }

   // Eject the media
	 result = EjectMedia (hVWin32, bDrive);
   if (!result)
   {
      // printf("could not eject media from drive %c:\n", 'A' + bDrive - 1);
   }

CLEANUP_AND_EXIT_APP: 
   if (fDriveLocked)
      UnlockLogicalVolumeW95(hVWin32, bDrive);

   if (hVWin32 != INVALID_HANDLE_VALUE)
      CloseVWin32 (hVWin32);

	 return( result );
} 

/*-----------------------------------------------------------------------
UnlockMedia (hVWin32, bDrive) 
Purpose: 

   Unlocks removable media from the specified drive so that it can be
   ejected.

Parameters: 
   hVWin32
      A handle to VWIN32. Used to issue request to unlock the media.

   bDrive
      The logical drive number to unlock. 0 = default, 1 = A, 2 = B,
      etc.

Return Value: 
   If successful, returns TRUE; if unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/

BOOL UnlockMedia (HANDLE hVWin32, BYTE bDrive)
{ 
   DIOC_REGISTERS regs = {0};
   PARAMBLOCK     unlockParams = {0};
   int   i;
   BOOL  fResult;
   DWORD cb;

   // First, check the lock status. This way, you'll know the number of
   // pending locks you must unlock.

   unlockParams.bOperation = 2;   // return lock/unlock status

   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDrive;
   regs.reg_ECX = MAKEWORD(0x48, 0x08);
   regs.reg_EDX = (DWORD)&unlockParams;

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the unlock succeeded.
   if (fResult)
   {
      /*
         DeviceIoControl succeeded. Now see if the unlock succeeded. It
         succeeded if the carry flag is not set, or if the carry flag is
         set but EAX is 0x01 or 0xB0.

         It failed if the carry flag is set and EAX is not 0x01 or 0xB0.

         If the carry flag is clear, then unlock succeeded. However, you
         don't need to set fResult because it is already TRUE when you get
         in here.

      */
      if (regs.reg_Flags & CARRY_FLAG)
         fResult = (regs.reg_EAX == 0xB0) || (regs.reg_EAX == 0x01);
   }

   if (!fResult) return(FALSE);

   // Now, let's unlock the media for every time it has been locked;
   // this will totally unlock the media.

   for (i = 0; i < unlockParams.bNumLocks; ++i) {
      unlockParams.bOperation = 1;   // unlock the media

      regs.reg_EAX = 0x440D;
      regs.reg_EBX = bDrive;
      regs.reg_ECX = MAKEWORD(0x48, 0x08);
      regs.reg_EDX = (DWORD)&unlockParams;

      fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                                 &regs, sizeof(regs), &regs, sizeof(regs),
                                 &cb, 0);

      // See if DeviceIoControl and the lock succeeded
      fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);
      if (!fResult) break;
   }
   return fResult;
} 

/*-----------------------------------------------------------------------
EjectMedia (hVWin32, bDrive) 
Purpose: 

   Ejects removable media from the specified drive.

Parameters: 
   hVWin32
      A handle to VWIN32. Used to issue request to unlock the media.

   bDrive
      The logical drive number to unlock. 0 = default, 1 = A, 2 = B,
      etc.

Return Value: 
   If successful, returns TRUE; if unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/
BOOL EjectMedia (HANDLE hVWin32, BYTE bDrive)
{ 
   DIOC_REGISTERS regs = {0};
   BOOL  fResult;
   DWORD cb;

   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDrive;
   regs.reg_ECX = MAKEWORD(0x49, 0x08);

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the lock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   return fResult;
} 

/*-----------------------------------------------------------------------
OpenVWin32 () 
Purpose: 

   Opens a handle to VWIN32 that can be used to issue low-level disk I/O
   commands.

Parameters: 
   None.

Return Value: 
   If successful, returns a handle to VWIN32.

   If unsuccessful, return INVALID_HANDLE_VALUE. Call GetLastError() to
   determine the cause of failure.
-----------------------------------------------------------------------*/
HANDLE WINAPI OpenVWin32 (void)
{
   return CreateFile(
			"\\\\.\\vwin32", 
			0, 0, NULL, 0,
      FILE_FLAG_DELETE_ON_CLOSE, NULL
	 );
}

/*-----------------------------------------------------------------------
CloseVWin32 (hVWin32) 
Purpose: 

   Closes the handle opened by OpenVWin32.

Parameters: 
   hVWin32
      An open handle to VWIN32.

Return Value: 
   If successful, returns TRUE. If unsuccessful, returns FALSE. Call
   GetLastError() to determine the cause of failure.
-----------------------------------------------------------------------*/ 
BOOL WINAPI CloseVWin32 (HANDLE hVWin32)
{
   return CloseHandle (hVWin32);
} 

/*-----------------------------------------------------------------------
LockLogicalVolumeW95(hVWin32, bDriveNum, bLockLevel, wPermissions) 
Purpose: 

   Takes a logical volume lock on a logical volume.

Parameters: 
   hVWin32
      An open handle to VWIN32.

   bDriveNum
      The logical drive number to lock. 0 = default, 1 = A:, 2 = B:,
      3 = C:, etc.

   bLockLevel
      Can be 0, 1, 2, or 3. Level 0 is an exclusive lock that can only
      be taken when there are no open files on the specified drive.
      Levels 1 through 3 form a hierarchy where 1 must be taken before
      2, which must be taken before 3.

   wPermissions
      Specifies how the lock will affect file operations when lock levels
      1 through 3 are taken. Also specifies whether a formatting lock
      should be taken after a level 0 lock.

      Zero is a valid permission.

Return Value: 
   If successful, returns TRUE.  If unsuccessful, returns FALSE.
-----------------------------------------------------------------------*/ 
BOOL WINAPI LockLogicalVolumeW95(HANDLE hVWin32, 
                                 BYTE   bDriveNum,
                                 BYTE   bLockLevel,
                                 WORD   wPermissions)
{
   BOOL           fResult;
   DIOC_REGISTERS regs = {0};
   BYTE           bDeviceCat;  // can be either 0x48 or 0x08
   DWORD          cb;

   /*
      Try first with device category 0x48 for FAT32 volumes. If it
      doesn't work, try again with device category 0x08. If that
      doesn't work, then the lock failed.
   */

   bDeviceCat = 0x48;

ATTEMPT_AGAIN: 
   // Set up the parameters for the call.
   regs.reg_EAX = 0x440D;
   regs.reg_EBX = MAKEWORD(bDriveNum, bLockLevel);
   regs.reg_ECX = MAKEWORD(0x4A, bDeviceCat);
   regs.reg_EDX = wPermissions;

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the lock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   // If DeviceIoControl or the lock failed, and device category 0x08
   // hasn't been tried, retry the operation with device category 0x08.
   if (!fResult && (bDeviceCat != 0x08))
   {
      bDeviceCat = 0x08;
      goto ATTEMPT_AGAIN;
   }

   return fResult;
} 

/*-----------------------------------------------------------------------
UnlockLogicalVolumeW95(hVWin32, bDriveNum) 
Purpose: 

   Unlocks a logical volume that was locked with LockLogicalVolumeW95().

Parameters: 
   hVWin32
      An open handle to VWIN32.

   bDriveNum
      The logical drive number to unlock. 0 = default, 1 = A:, 2 = B:,
      3 = C:, etc.

Return Value: 
   If successful, returns TRUE. If unsuccessful, returns FALSE.

Comments: 
   Must be called the same number of times as LockLogicalVolumeW95() to
   completely unlock a volume.

   Only the lock owner can unlock a volume.
-----------------------------------------------------------------------*/ 
BOOL WINAPI UnlockLogicalVolumeW95(HANDLE hVWin32, BYTE bDriveNum)
{ 
   BOOL           fResult;
   DIOC_REGISTERS regs = {0};
   BYTE           bDeviceCat;  // can be either 0x48 or 0x08
   DWORD          cb;

   /* Try first with device category 0x48 for FAT32 volumes. If it
      doesn't work, try again with device category 0x08. If that
      doesn't work, then the unlock failed.
   */

   bDeviceCat = 0x48;

ATTEMPT_AGAIN: 
   // Set up the parameters for the call.
   regs.reg_EAX = 0x440D;
   regs.reg_EBX = bDriveNum;
   regs.reg_ECX = MAKEWORD(0x6A, bDeviceCat);

   fResult = DeviceIoControl (hVWin32, VWIN32_DIOC_DOS_IOCTL,
                              &regs, sizeof(regs), &regs, sizeof(regs),
                              &cb, 0);

   // See if DeviceIoControl and the unlock succeeded
   fResult = fResult && !(regs.reg_Flags & CARRY_FLAG);

   // If DeviceIoControl or the unlock failed, and device category 0x08
   // hasn't been tried, retry the operation with device category 0x08.
   if (!fResult && (bDeviceCat != 0x08))
   {
      bDeviceCat = 0x08;
      goto ATTEMPT_AGAIN;
   }
   return fResult;
} 

} // extern "C"
