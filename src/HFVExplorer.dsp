# Microsoft Developer Studio Project File - Name="HFVExplorer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=HFVExplorer - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HFVExplorer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HFVExplorer.mak" CFG="HFVExplorer - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HFVExplorer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "HFVExplorer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HFVExplorer - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /Ot /Oi /Ob2 /D "__EMX__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# SUBTRACT CPP /Og
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x40b /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "HFVExplorer - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "__EMX__" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x40b /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 winmm.lib /nologo /subsystem:windows /debug /machine:I386

!ENDIF 

# Begin Target

# Name "HFVExplorer - Win32 Release"
# Name "HFVExplorer - Win32 Debug"
# Begin Group "Core"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\AskDir.cpp
# End Source File
# Begin Source File

SOURCE=.\AskDump.cpp
# End Source File
# Begin Source File

SOURCE=.\AskNewVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\AskProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\AskVolumeToFloppy.cpp
# End Source File
# Begin Source File

SOURCE=.\aspecial.c
# End Source File
# Begin Source File

SOURCE=.\Bndl.cpp
# End Source File
# Begin Source File

SOURCE=.\cache.cpp
# End Source File
# Begin Source File

SOURCE=.\CFATVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\charset.c
# End Source File
# Begin Source File

SOURCE=.\DynCopyModeSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\eject_nt.cpp
# End Source File
# Begin Source File

SOURCE=.\eject_w95.cpp
# End Source File
# Begin Source File

SOURCE=.\FileTypeMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\floppy.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVCommandLineInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVExplorer.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\HFVExplorer.hpj

!IF  "$(CFG)" == "HFVExplorer - Win32 Release"

# Begin Custom Build - Making help file...
OutDir=.\.\Release
ProjDir=.
TargetName=HFVExplorer
InputPath=.\hlp\HFVExplorer.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(ProjDir)\makehelp.bat"

# End Custom Build

!ELSEIF  "$(CFG)" == "HFVExplorer - Win32 Debug"

# Begin Custom Build - Making help file...
OutDir=.\.\Debug
ProjDir=.
TargetName=HFVExplorer
InputPath=.\hlp\HFVExplorer.hpj

"$(OutDir)\$(TargetName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"$(ProjDir)\makehelp.bat"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HFVExplorer.rc
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerListView.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerTreeView.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerView.cpp
# End Source File
# Begin Source File

SOURCE=.\HFVPreviewView.cpp
# End Source File
# Begin Source File

SOURCE=.\icon.cpp
# End Source File
# Begin Source File

SOURCE=.\mactypes.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\openfile.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage1.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage10.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage2.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage3.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage4.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage5.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage6.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage7.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage8.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsPage9.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\part.cpp
# End Source File
# Begin Source File

SOURCE=.\shell.cpp
# End Source File
# Begin Source File

SOURCE=.\special.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\vmacpatch.c
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\adouble.h
# End Source File
# Begin Source File

SOURCE=.\AskDir.h
# End Source File
# Begin Source File

SOURCE=.\AskDump.h
# End Source File
# Begin Source File

SOURCE=.\AskNewVolume.h
# End Source File
# Begin Source File

SOURCE=.\AskProperties.h
# End Source File
# Begin Source File

SOURCE=.\AskVolumeToFloppy.h
# End Source File
# Begin Source File

SOURCE=.\bndl.h
# End Source File
# Begin Source File

SOURCE=.\CFATVolume.h
# End Source File
# Begin Source File

SOURCE=.\DynCopyModeSelect.h
# End Source File
# Begin Source File

SOURCE=.\eject_w95.h
# End Source File
# Begin Source File

SOURCE=.\FileTypeMapping.h
# End Source File
# Begin Source File

SOURCE=.\floppy.h
# End Source File
# Begin Source File

SOURCE=.\ftypes.h
# End Source File
# Begin Source File

SOURCE=.\HFVCommandLineInfo.h
# End Source File
# Begin Source File

SOURCE=.\HFVExplorer.h
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerDoc.h
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerListView.h
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerTreeView.h
# End Source File
# Begin Source File

SOURCE=.\HFVExplorerView.h
# End Source File
# Begin Source File

SOURCE=.\HFVPreviewView.h
# End Source File
# Begin Source File

SOURCE=.\mactypes.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\openfile.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage1.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage10.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage2.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage3.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage4.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage5.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage6.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage7.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage8.h
# End Source File
# Begin Source File

SOURCE=.\OptionsPage9.h
# End Source File
# Begin Source File

SOURCE=.\OptionsSheet.h
# End Source File
# Begin Source File

SOURCE=.\own_ids.h
# End Source File
# Begin Source File

SOURCE=.\part.h
# End Source File
# Begin Source File

SOURCE=.\shell.h
# End Source File
# Begin Source File

SOURCE=.\special.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\arrowcop.cur
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\HFVExplorer.ico
# End Source File
# Begin Source File

SOURCE=.\res\HFVExplorer.rc2
# End Source File
# Begin Source File

SOURCE=.\res\HFVExplorerDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00006.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00007.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00008.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00009.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00010.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon10.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon11.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon12.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon13.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon14.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon3.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon4.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon5.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon52.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon53.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon54.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon55.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon56.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon57.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon6.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon7.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon8.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon9.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_hfve.ico
# End Source File
# Begin Source File

SOURCE=.\res\label_co.ico
# End Source File
# Begin Source File

SOURCE=.\res\label_ho.ico
# End Source File
# Begin Source File

SOURCE=.\res\label_in.ico
# End Source File
# Begin Source File

SOURCE=.\res\label_pe.ico
# End Source File
# Begin Source File

SOURCE=.\res\label_pr.ico
# End Source File
# Begin Source File

SOURCE=.\res\mainfram.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "Hfs utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hfs\binhex.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\block.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\btree.c
# End Source File
# Begin Source File

SOURCE=.\hfs\copyhfs.c
# End Source File
# Begin Source File

SOURCE=.\hfs\copyin.c
# End Source File
# Begin Source File

SOURCE=.\hfs\copyout.c
# End Source File
# Begin Source File

SOURCE=.\hfs\crc.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\data.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\file.c
# End Source File
# Begin Source File

SOURCE=.\hfs\hcopy.c
# End Source File
# Begin Source File

SOURCE=.\hfs\hcwd.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\hfs.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\low.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\node.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\record.c
# End Source File
# Begin Source File

SOURCE=.\hfs\libhfs\volume.c
# End Source File
# End Group
# Begin Group "Interfaces"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\filehook.c
# End Source File
# Begin Source File

SOURCE=.\hfs\interface.c
# End Source File
# Begin Source File

SOURCE=.\sys\ntcd.cpp
# End Source File
# Begin Source File

SOURCE=.\vxdiface.c
# End Source File
# End Group
# Begin Group "Doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\shipping\Current\Ideas and notes.txt"
# End Source File
# Begin Source File

SOURCE=.\Docs\lock.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\shipping\Current\README.TXT
# End Source File
# End Group
# Begin Source File

SOURCE=C:\WINNT\HFVExplorer.INI
# End Source File
# End Target
# End Project
