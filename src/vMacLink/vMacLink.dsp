# Microsoft Developer Studio Project File - Name="vMacLink" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=vMacLink - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vMacLink.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vMacLink.mak" CFG="vMacLink - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vMacLink - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "vMacLink - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vMacLink - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "__EMX__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "__EMX__" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Launcher.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE=$(InputPath)
PreLink_Cmds=rc launcher.rc
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "vMacLink - Win32 Release"
# Name "vMacLink - Win32 Debug"
# Begin Group "hfs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\hfs\binhex.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\block.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\btree.c
# End Source File
# Begin Source File

SOURCE=..\hfs\Copyhfs.c
# End Source File
# Begin Source File

SOURCE=..\hfs\copyin.c
# End Source File
# Begin Source File

SOURCE=..\hfs\copyout.c
# End Source File
# Begin Source File

SOURCE=..\hfs\crc.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\data.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\file.c
# End Source File
# Begin Source File

SOURCE=..\hfs\hcopy.c
# End Source File
# Begin Source File

SOURCE=..\hfs\hcwd.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\hfs.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\low.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\node.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\record.c
# End Source File
# Begin Source File

SOURCE=..\hfs\libhfs\volume.c
# End Source File
# End Group
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\filehook.c
# End Source File
# End Group
# Begin Group "Mac resources"

# PROP Default_Filter "*.bin"
# Begin Source File

SOURCE=.\lmtp1.bin
# End Source File
# Begin Source File

SOURCE=.\lmtp2.bin
# End Source File
# End Group
# Begin Group "Core"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\alias.c
# End Source File
# Begin Source File

SOURCE=..\aspecial.c
# End Source File
# Begin Source File

SOURCE=..\charset.c
# End Source File
# Begin Source File

SOURCE=.\Launcher.rc

!IF  "$(CFG)" == "vMacLink - Win32 Release"

!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\reboot.c
# End Source File
# Begin Source File

SOURCE=.\vMacLink.c
# End Source File
# Begin Source File

SOURCE=..\vmacpatch.c
# End Source File
# End Group
# End Target
# End Project
