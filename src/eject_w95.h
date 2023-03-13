BOOL EjectVolumeW95(TCHAR cDriveLetter);

HANDLE WINAPI OpenVWin32 (void); 
BOOL WINAPI CloseVWin32 (HANDLE hVWin32); 
BOOL WINAPI UnlockLogicalVolumeW95 (HANDLE hVWin32, BYTE bDriveNum); 
BOOL WINAPI LockLogicalVolumeW95 (HANDLE hVWin32, 
                                  BYTE   bDriveNum,
                                  BYTE   bLockLevel,
                                  WORD   wPermissions);
