BOOL EjectVolume(TCHAR cDriveLetter,BOOL reload);

HANDLE OpenVolume(TCHAR cDriveLetter);
BOOL LockVolume(HANDLE hVolume);
BOOL DismountVolume(HANDLE hVolume);
BOOL PreventRemovalOfVolume(HANDLE hVolume, BOOL fPrevent);
BOOL AutoEjectVolume(HANDLE hVolume,BOOL reload);
BOOL CloseVolume(HANDLE hVolume);
