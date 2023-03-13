# Microsoft Developer Studio Generated NMAKE File, Based on vMacLink.dsp
!IF "$(CFG)" == ""
CFG=vMacLink - Win32 Debug
!MESSAGE No configuration specified. Defaulting to vMacLink - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "vMacLink - Win32 Release" && "$(CFG)" !=\
 "vMacLink - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "vMacLink - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\vMacLink.exe"

!ELSE 

ALL : "$(OUTDIR)\vMacLink.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\binhex.obj"
	-@erase "$(INTDIR)\block.obj"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\Copyhfs.obj"
	-@erase "$(INTDIR)\copyin.obj"
	-@erase "$(INTDIR)\copyout.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\data.obj"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\filehook.obj"
	-@erase "$(INTDIR)\hcopy.obj"
	-@erase "$(INTDIR)\hcwd.obj"
	-@erase "$(INTDIR)\hfs.obj"
	-@erase "$(INTDIR)\low.obj"
	-@erase "$(INTDIR)\node.obj"
	-@erase "$(INTDIR)\record.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vMacLink.obj"
	-@erase "$(INTDIR)\vmacpatch.obj"
	-@erase "$(INTDIR)\volume.obj"
	-@erase "$(OUTDIR)\vMacLink.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "__EMX__" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)\vMacLink.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vMacLink.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\vMacLink.pdb" /machine:I386 /out:"$(OUTDIR)\vMacLink.exe" 
LINK32_OBJS= \
	"$(INTDIR)\binhex.obj" \
	"$(INTDIR)\block.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\Copyhfs.obj" \
	"$(INTDIR)\copyin.obj" \
	"$(INTDIR)\copyout.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\data.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\filehook.obj" \
	"$(INTDIR)\hcopy.obj" \
	"$(INTDIR)\hcwd.obj" \
	"$(INTDIR)\hfs.obj" \
	"$(INTDIR)\low.obj" \
	"$(INTDIR)\node.obj" \
	"$(INTDIR)\record.obj" \
	"$(INTDIR)\vMacLink.obj" \
	"$(INTDIR)\vmacpatch.obj" \
	"$(INTDIR)\volume.obj"

"$(OUTDIR)\vMacLink.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\vMacLink.exe" "$(OUTDIR)\vMacLink.bsc"

!ELSE 

ALL : "$(OUTDIR)\vMacLink.exe" "$(OUTDIR)\vMacLink.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\binhex.obj"
	-@erase "$(INTDIR)\binhex.sbr"
	-@erase "$(INTDIR)\block.obj"
	-@erase "$(INTDIR)\block.sbr"
	-@erase "$(INTDIR)\btree.obj"
	-@erase "$(INTDIR)\btree.sbr"
	-@erase "$(INTDIR)\Copyhfs.obj"
	-@erase "$(INTDIR)\Copyhfs.sbr"
	-@erase "$(INTDIR)\copyin.obj"
	-@erase "$(INTDIR)\copyin.sbr"
	-@erase "$(INTDIR)\copyout.obj"
	-@erase "$(INTDIR)\copyout.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\data.obj"
	-@erase "$(INTDIR)\data.sbr"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\file.sbr"
	-@erase "$(INTDIR)\filehook.obj"
	-@erase "$(INTDIR)\filehook.sbr"
	-@erase "$(INTDIR)\hcopy.obj"
	-@erase "$(INTDIR)\hcopy.sbr"
	-@erase "$(INTDIR)\hcwd.obj"
	-@erase "$(INTDIR)\hcwd.sbr"
	-@erase "$(INTDIR)\hfs.obj"
	-@erase "$(INTDIR)\hfs.sbr"
	-@erase "$(INTDIR)\low.obj"
	-@erase "$(INTDIR)\low.sbr"
	-@erase "$(INTDIR)\node.obj"
	-@erase "$(INTDIR)\node.sbr"
	-@erase "$(INTDIR)\record.obj"
	-@erase "$(INTDIR)\record.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\vMacLink.obj"
	-@erase "$(INTDIR)\vMacLink.sbr"
	-@erase "$(INTDIR)\vmacpatch.obj"
	-@erase "$(INTDIR)\vmacpatch.sbr"
	-@erase "$(INTDIR)\volume.obj"
	-@erase "$(INTDIR)\volume.sbr"
	-@erase "$(OUTDIR)\vMacLink.bsc"
	-@erase "$(OUTDIR)\vMacLink.exe"
	-@erase "$(OUTDIR)\vMacLink.ilk"
	-@erase "$(OUTDIR)\vMacLink.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "__EMX__" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\vMacLink.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vMacLink.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\binhex.sbr" \
	"$(INTDIR)\block.sbr" \
	"$(INTDIR)\btree.sbr" \
	"$(INTDIR)\Copyhfs.sbr" \
	"$(INTDIR)\copyin.sbr" \
	"$(INTDIR)\copyout.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\data.sbr" \
	"$(INTDIR)\file.sbr" \
	"$(INTDIR)\filehook.sbr" \
	"$(INTDIR)\hcopy.sbr" \
	"$(INTDIR)\hcwd.sbr" \
	"$(INTDIR)\hfs.sbr" \
	"$(INTDIR)\low.sbr" \
	"$(INTDIR)\node.sbr" \
	"$(INTDIR)\record.sbr" \
	"$(INTDIR)\vMacLink.sbr" \
	"$(INTDIR)\vmacpatch.sbr" \
	"$(INTDIR)\volume.sbr"

"$(OUTDIR)\vMacLink.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\vMacLink.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\vMacLink.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\binhex.obj" \
	"$(INTDIR)\block.obj" \
	"$(INTDIR)\btree.obj" \
	"$(INTDIR)\Copyhfs.obj" \
	"$(INTDIR)\copyin.obj" \
	"$(INTDIR)\copyout.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\data.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\filehook.obj" \
	"$(INTDIR)\hcopy.obj" \
	"$(INTDIR)\hcwd.obj" \
	"$(INTDIR)\hfs.obj" \
	"$(INTDIR)\low.obj" \
	"$(INTDIR)\node.obj" \
	"$(INTDIR)\record.obj" \
	"$(INTDIR)\vMacLink.obj" \
	"$(INTDIR)\vmacpatch.obj" \
	"$(INTDIR)\volume.obj"

"$(OUTDIR)\vMacLink.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   rc launcher.rc
	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=$(InputPath)

!ENDIF 


!IF "$(CFG)" == "vMacLink - Win32 Release" || "$(CFG)" ==\
 "vMacLink - Win32 Debug"
SOURCE=..\hfs\binhex.c
DEP_CPP_BINHE=\
	"..\hfs\binhex.h"\
	"..\hfs\crc.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\binhex.obj" : $(SOURCE) $(DEP_CPP_BINHE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\binhex.obj"	"$(INTDIR)\binhex.sbr" : $(SOURCE) $(DEP_CPP_BINHE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\block.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_BLOCK=\
	"..\filehook.h"\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	

"$(INTDIR)\block.obj" : $(SOURCE) $(DEP_CPP_BLOCK) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_BLOCK=\
	"..\filehook.h"\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	

"$(INTDIR)\block.obj"	"$(INTDIR)\block.sbr" : $(SOURCE) $(DEP_CPP_BLOCK)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\btree.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_BTREE=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\node.h"\
	

"$(INTDIR)\btree.obj" : $(SOURCE) $(DEP_CPP_BTREE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_BTREE=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\node.h"\
	

"$(INTDIR)\btree.obj"	"$(INTDIR)\btree.sbr" : $(SOURCE) $(DEP_CPP_BTREE)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\Copyhfs.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_COPYH=\
	"..\hfs\copyhfs.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	

"$(INTDIR)\Copyhfs.obj" : $(SOURCE) $(DEP_CPP_COPYH) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_COPYH=\
	"..\hfs\copyhfs.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	

"$(INTDIR)\Copyhfs.obj"	"$(INTDIR)\Copyhfs.sbr" : $(SOURCE) $(DEP_CPP_COPYH)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\copyin.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_COPYI=\
	"..\adouble.h"\
	"..\hfs\binhex.h"\
	"..\hfs\copyin.h"\
	"..\hfs\crc.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	

"$(INTDIR)\copyin.obj" : $(SOURCE) $(DEP_CPP_COPYI) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_COPYI=\
	"..\adouble.h"\
	"..\hfs\binhex.h"\
	"..\hfs\copyin.h"\
	"..\hfs\crc.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	

"$(INTDIR)\copyin.obj"	"$(INTDIR)\copyin.sbr" : $(SOURCE) $(DEP_CPP_COPYI)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\copyout.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_COPYO=\
	"..\hfs\binhex.h"\
	"..\hfs\copyout.h"\
	"..\hfs\crc.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	

"$(INTDIR)\copyout.obj" : $(SOURCE) $(DEP_CPP_COPYO) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_COPYO=\
	"..\hfs\binhex.h"\
	"..\hfs\copyout.h"\
	"..\hfs\crc.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	

"$(INTDIR)\copyout.obj"	"$(INTDIR)\copyout.sbr" : $(SOURCE) $(DEP_CPP_COPYO)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\crc.c
DEP_CPP_CRC_C=\
	"..\hfs\crc.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) $(DEP_CPP_CRC_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\data.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_DATA_=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	

"$(INTDIR)\data.obj" : $(SOURCE) $(DEP_CPP_DATA_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_DATA_=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	

"$(INTDIR)\data.obj"	"$(INTDIR)\data.sbr" : $(SOURCE) $(DEP_CPP_DATA_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\file.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_FILE_=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\file.obj" : $(SOURCE) $(DEP_CPP_FILE_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_FILE_=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\file.obj"	"$(INTDIR)\file.sbr" : $(SOURCE) $(DEP_CPP_FILE_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\filehook.c
DEP_CPP_FILEH=\
	"..\filehook.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\filehook.obj" : $(SOURCE) $(DEP_CPP_FILEH) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\filehook.obj"	"$(INTDIR)\filehook.sbr" : $(SOURCE) $(DEP_CPP_FILEH)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\hcopy.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_HCOPY=\
	"..\hfs\copyin.h"\
	"..\hfs\copyout.h"\
	"..\hfs\glob.h"\
	"..\hfs\hcopy.h"\
	"..\hfs\hcwd.h"\
	"..\hfs\hfsutil.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\os2lib.h"\
	

"$(INTDIR)\hcopy.obj" : $(SOURCE) $(DEP_CPP_HCOPY) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_HCOPY=\
	"..\hfs\copyin.h"\
	"..\hfs\copyout.h"\
	"..\hfs\glob.h"\
	"..\hfs\hcopy.h"\
	"..\hfs\hcwd.h"\
	"..\hfs\hfsutil.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\os2lib.h"\
	

"$(INTDIR)\hcopy.obj"	"$(INTDIR)\hcopy.sbr" : $(SOURCE) $(DEP_CPP_HCOPY)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\hcwd.c
DEP_CPP_HCWD_=\
	"..\hfs\hcwd.h"\
	"..\hfs\libhfs\hfs.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\hcwd.obj" : $(SOURCE) $(DEP_CPP_HCWD_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\hcwd.obj"	"$(INTDIR)\hcwd.sbr" : $(SOURCE) $(DEP_CPP_HCWD_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\hfs.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_HFS_C=\
	"..\filehook.h"\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	"..\hfs\libhfs\node.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\hfs.obj" : $(SOURCE) $(DEP_CPP_HFS_C) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_HFS_C=\
	"..\filehook.h"\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	"..\hfs\libhfs\node.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\hfs.obj"	"$(INTDIR)\hfs.sbr" : $(SOURCE) $(DEP_CPP_HFS_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\low.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_LOW_C=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	

"$(INTDIR)\low.obj" : $(SOURCE) $(DEP_CPP_LOW_C) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_LOW_C=\
	"..\hfs\libhfs\block.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\file.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	

"$(INTDIR)\low.obj"	"$(INTDIR)\low.sbr" : $(SOURCE) $(DEP_CPP_LOW_C)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\node.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_NODE_=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\node.h"\
	

"$(INTDIR)\node.obj" : $(SOURCE) $(DEP_CPP_NODE_) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_NODE_=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\node.h"\
	

"$(INTDIR)\node.obj"	"$(INTDIR)\node.sbr" : $(SOURCE) $(DEP_CPP_NODE_)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\record.c
DEP_CPP_RECOR=\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\record.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\record.obj" : $(SOURCE) $(DEP_CPP_RECOR) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\record.obj"	"$(INTDIR)\record.sbr" : $(SOURCE) $(DEP_CPP_RECOR)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\vMacLink.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_VMACL=\
	"..\filehook.h"\
	"..\hfs\copyhfs.h"\
	"..\hfs\hcopy.h"\
	"..\hfs\hcwd.h"\
	"..\hfs\hfsutil.h"\
	"..\hfs\hmount.h"\
	"..\hfs\humount.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\suid.h"\
	"..\vmacpatch.h"\
	

"$(INTDIR)\vMacLink.obj" : $(SOURCE) $(DEP_CPP_VMACL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_VMACL=\
	"..\filehook.h"\
	"..\hfs\copyhfs.h"\
	"..\hfs\hcopy.h"\
	"..\hfs\hcwd.h"\
	"..\hfs\hfsutil.h"\
	"..\hfs\hmount.h"\
	"..\hfs\humount.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\suid.h"\
	"..\vmacpatch.h"\
	

"$(INTDIR)\vMacLink.obj"	"$(INTDIR)\vMacLink.sbr" : $(SOURCE) $(DEP_CPP_VMACL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=..\vmacpatch.c
DEP_CPP_VMACP=\
	"..\vmacpatch.h"\
	

!IF  "$(CFG)" == "vMacLink - Win32 Release"


"$(INTDIR)\vmacpatch.obj" : $(SOURCE) $(DEP_CPP_VMACP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"


"$(INTDIR)\vmacpatch.obj"	"$(INTDIR)\vmacpatch.sbr" : $(SOURCE)\
 $(DEP_CPP_VMACP) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\hfs\libhfs\volume.c

!IF  "$(CFG)" == "vMacLink - Win32 Release"

DEP_CPP_VOLUM=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\volume.obj" : $(SOURCE) $(DEP_CPP_VOLUM) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "vMacLink - Win32 Debug"

DEP_CPP_VOLUM=\
	"..\hfs\libhfs\btree.h"\
	"..\hfs\libhfs\data.h"\
	"..\hfs\libhfs\hfs.h"\
	"..\hfs\libhfs\internal.h"\
	"..\hfs\libhfs\low.h"\
	"..\hfs\libhfs\record.h"\
	"..\hfs\libhfs\volume.h"\
	

"$(INTDIR)\volume.obj"	"$(INTDIR)\volume.sbr" : $(SOURCE) $(DEP_CPP_VOLUM)\
 "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

