HFV/DSK Explorer v. 1.3.0 -- a tool for vMac and Executor(DOS/Win32) users.
Experimental freeware software.


Disclaimer
----------
  THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
  IN HOPE THAT IT WILL BE USEFUL BUT WITH NO GUARANTEES WHATSOEVER.
  ANYGRAAF DISCLAIMS ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
  INCLUDING THE WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE. IN NO EVENT SHALL ANYGRAAF BE LIABLE FOR ANY DAMAGES
  WHATSOEVER INCLUDING DIRECT, INDIRECT, INCIDENTAL, CONSEQUENTIAL, 
  LOSS OF BUSINESS PROFITS OR SPECIAL DAMAGES. USE AT YOUR OWN RISK. 



Installation of HFVExplorer
---------------------------
  1. Extract all of the files to the directory of your choice.
     HFVExplorer initially shows all HFV and DSK files from the directory
     of the executable, so it is convenient to place them into a directory
     where you have your HFV and DSK files.

  2. CDENABLE.VXD is needed under Windows 95 to give access to
     HFS/hybrid CD:s. Must be *moved* to "\Windows\System" directory.

     CDENABLE.SYS is needed under Windows NT 4.0 to give access to
     HFS/hybrid CD:s. Must be *moved* to "\Winnt\System32\Drivers" directory.
     After moving the driver, the program must be run once with system administrator
     rights. Later normal user rights suffice.

  3. Start HFVExplorer and go to the options dialog (View menu).
     Check all of the property pages to suit your needs.




Features of HFVExplorer
-----------------------
- A read/write, Explorer-like view to ARDI HFV files and
  vMac DSK files (the same thing essentially).
- A tree representation of the corresponding HFS file structure.
- Direct access to floppies, works in both Windows 95 and NT 4.0.
- Direct access to CDs, works under Windows 95, 98, NT 4, NT 5.
- You can launch the applications or documents by double-clicking them.
- After you have run the program once, you can start it by
  double-clicking HFV file name.
- Drag&drop a HFV file to open it.
- Drag&drop file copy/move. If target volume is the same as source,
  the file will be moved, otherwise it will copied. Override by
  holding down Shift or Control keys - again, just like in Explorer.
  Drop item on top of the "up" button and it goes to parent directory.
- Rename by clicking the highlighted name, or by pressing F2.
  Just like in Windows Explorer.
- Delete by pressing the "Delete" button.
- Create new folder: right mouse click in an empty window area.
- Press "Tab" to switch between list and tree views.
- Press F5 to update screen, for example, after inserting new floppy disk.
- Enter or double-click opens Application/Document/Folder.
- Format floppies. File menu.
- Commands to dump HFV/DSK files to floppies, and create them from
  floppies.
- Create and format new HFS/DSK files. File menu.
- AppleDouble support. Special characters in names not handled.
- Automatic type conversions when moving between FAT and HFS.
  This code is directly from HFS tools.
  fat -> hfs:
    *.bin (Mac Binary II) -> data & resource forks
    *.hqx (binhex) -> data & resource forks
  hfs -> fat:
    data & resource forks -> *.bin (Mac Binary II)
  txt, c, h:
    CR/LF conversion, file name extension
- File copy mode selection for HFS -> FAT and FAT -> HFS.
  Selected from Options dialog (new property page).
  Maybe in future there will be file type mapping.
- Multiple item selection -- move, copy, delete.
  Works as in Explorer (or in any multi-selection list control).
  Shift-click, control-click or drag a marquee.
- "Create shortcut" command implemented for Executor/DOS,
  Executor/Win32 and vMac, including a launcher app for vMac.
  Places an icon on the Windows desktop, pointing to either a document 
  or an application. Automatic Mac icon extraction. 
  If you want the special characters  to be shown in icon names, 
  change the desktop icon font (Control Panel/Display/
  Appearance/Icon "Font") to something else than MS Sans Serif, 
  perhaps Arial, which may be ugly but has more visible characters.
  Icons and parameter  files are placed into subdirectory "Links".
  Helper directory "vMacLink" contains a link to a vMac installation.
- "Open with" supports the tree emulators (E/DOS, E/Win32, vMac).
  *** IMPORTANT *** : DO NOT try to copy/move/delete/create new files 
  in HFVExplorer when the emulator is running. MacOS doesn't like
  anything happening behind its back. The file accesses are not properly
  synchronized, and if unlucky, you may suffer data loss.
  HFVExplorer tries to protect you from this, but its protection
  schemes are far from perfect. It takes care that its own data
  structures are always in sync, but the emulators don't do this.
  After the emulator terminates, HFVExplorer checks if it needs to
  update the file structures and if so, it does this. That's the
  reason why it takes a while when emulator terminates -- if one bit
  of data has changed in a volume file, the only safe way is to
  reload the whole volume.
  Pressing return or double-clicking now opens Executor/Win32 by default.
  You can change this behavior from the options dialog.
- You should normally use the alias launching method with vMac,
  and only try the Apple events method if this fails. Executor users
  should not be concerned with the launching method since appropriate
  way is selected automatically. Just make sure that you don't
  claim in the setup dialog that your Windows Executor is DOS
  or vice versa -- this information have to be correct.
- Possibility to hide/show "invisible" files.
- Automatic file type mappings. Mappings are easily exported and imported.


CD support:
-----------
ISO9660 CD:s of course. HFS. Hybrid HFS/ISO. No High Sierra.
ISO with Apple extensions is shown as DOS CD, with no additional
resources, that is, icons and types/creators.

CDENABLE.VXD is needed under Windows 95 to give access to
HFS/hybrid CD:s. Must be in either Windows or Windows\System directory.

When HFVExplorer is running, DOS Executor is also able to see HFS
CD:s, using protected mode drivers. No MSCDEX is needed.
If you close HFVExplorer, Executor will continue to see the
current CD, but will not see the next one inserted.

NEW note: released a separate utility, CDENABLE.EXE,
to allow people who do not need HFVExplorer to use
protected mode drivers in Executor.
This file is now shipped with Executor and is not available
from me any more (to avoid unnecessary version conflicts).

TECHNICAL NOTE: the way CDEnable.vxd is implemented is incompatible
with future Windows versions. It works (at least) with original
Windows 95, OEM Service Release 2 (OSR2, "Windows 95 B") and
Windows 98 release build.

CD auto insert notification supported, just pop in the CD and it
should appear in the tree view, if it contains recognizable volume.
In some machines, the auto-insertion/arrival messages
take long time to reach applications. HFVExplorer can't
help it, press F5 if you don't want to wait.

Starting from version 1.1.1i, full CD support under Windows NT 4.0, too.
CDENABLE.SYS must be copied to your System32\Drivers directory.
After copying the driver, the program must be run once with system
administrator rights. Later normal user rights are enough.

Hook1508 released. A tool to help DOS Executor see the HFS (non-hybrid)
CD's under Windows NT 4.0. Not included in this archive.
If you intend to use Hook1508 with HFVExplorer shortcuts and/or
launching, you should either create a .bat or modify AUTOXEC.NT,
or make a custom version AUTOXEC.NT. The procedure is described
in Hook1508 read me file.


Tips:
-----
Use shortest possible paths. Application launching
imposes some limits to path lengths. There is nothing
I can do for it. It depends on system and the program to
be launched only.

All icons from the icon cache file HFVICONS.DAT are always 
loaded into the memory when HFVExplorer starts. You may save 
some memory and time by deleting the icon cache file HFVICONS.DAT 
once the startup gets intolerably slow.


Platforms:
----------
Microsoft Windows 95, Windows NT 4.0, Windows NT 5.0 beta 1 and beta 2, 
Windows 98.



Availability:
-------------
http://www.nic.fi/~lpesonen/HFVExplorer/

Could be in a subdirectory below. File
is named for example "Hdexp130.zip" for
version 1.3.0, etc.

There is no web page, just the zip file.


Documentation:
--------------
This file.


License:
--------
Freeware. See the disclaimer.


Credits:
--------
- Marcus Better, for "HFS tools". Much better than
  my own HFS code. Probably I will discard my
  own code altogether.
  HFS tools can be found:
  http://www.student.nada.kth.se/~f96-bet/hfsutils/
  email: Marcus.Better@abc.se
  Originally Copyright (C) 1996, 1997 by Robert Leslie, email: rob@mars.org
- ARDI (http://www.ardi.com/). Executor is great. HFV Executor uses a snippet
  from ARDI Browser code; resource bundle hash table.
- vMac team (http://www.vmac.org/). vMac is great, too.
- Paul Hargrove. HFS CD partition table handling was taken
  from Paul's Linux sources. GNU GPL as well.
- Mr. Noda (noda@lyra.vega.or.jp) for his help in Japanese language support.
- Peter van Sebille; vxd sector read routine is inspired by his
  Win95 Linux FSD.
- Everybody else I forgot to mention.

  GNU GPL (General Public License) now. See "Copying".
  Source code available by request, email: lpesonen@nic.fi



Copyrights
----------
TODO: put here a comprehensive copyright list (with urls).

Anygraaf
Apple
ARDI (Executor)
Bauer, Christian (Shapeshifter)
Better, Marcus
Cummins, Philip (vMac) (and other members)
Hargrove, Paul 
Leslie, Robert
Microcode solutions (Fusion)
Microsoft
Ortmann, Mathias (WinUAE)
Schmidt, Bernd (UAE)

- more?



E-mail:
------
"lpesonen@nic.fi" or "lauri.pesonen@anygraaf.fi"



History:
--------
Things fixed/changed (oldest first):


Versions 1.1.0-1.0.4
--------------------
- 32 MB limit in HFV files fixed
  (Misunderstanding between logical sector size and
  physical sector size).

- Better support for special characters when launching
  applications. Requires Executor version 2.0n or later.

- Default viewer for types "TEXT", "ttro" and "ttxt
  is now "System:Shareware:Tex-Edit:Tex-Edit"

- 1.0.1 crashed when opened dos volumes.
  (Visual Studio 5.0 optimization bug)

- AppleDouble launching: new versions of Executor
  seem to want drive letter to be lowercase,
  otherwise you get always browser only.
  Both programs changed to respect this feature.

- Old: lists *.HFV. Now: *.HFV and *.DSK

- about box: version number -> 1.0.4



Version 1.1.0:
--------------
- Copy / Move files or folders
    - Drag & drop: drop to tree view (left pane of the
      window), or a folder in the right pane.
    - No multi-selection so far.
    - Control & shift à la Windows Explorer
      to force copy or move; otherwise auto-selection
        mode. That is, move inside same volume, copy
        if volume changes.
        Cursor shows a "+" when copy will be performed.
    - can copy between fat/hfs
    - auto type convert when copying between fat & hfs:
        fat -> hfs:
            *.bin (Mac Binary II) -> data & resource forks
            *.hqx (binhex) -> data & resource forks
        hfs -> fat:
            data & resource forks -> *.bin (Mac Binary II)
        txt, c, h:
          CR/LF conversion, file name extension
        Conversion code directly from HFS tools.
        Will be customized later.
    - drag on top of "Up" button -> copy/move to parent directory

- Direct access to floppies. After floppy change, press F5
  to force update of the screen.

- delete file / folder (all contents!), asks for confirmation

- create folder: right mouse click context menu.

- rename file / folder: click the (already) selected name
  under icon -> wait 1 sec -> edit the name. Just like
  in Explorer.

- Format New Volume
    - Sizes the file, creates HFS structures and Desktop
    - menu command
    - create New document with Explorer context menu.
      double-click -> will ask name/size and format.
    - if you want to format a floppy, select from
      combo the appropriate drive

- Now contains *two* HFS routine sets - my own and
  Hfsutils' (see credits).
  I will eventually discard my own code.

- "File" menu contains two commands to dump a volume
  to a floppy / create a volume from a floppy.

- HFS code now taken from Hfsutils by Marcus Better:
  http://www.student.nada.kth.se/~f96-bet/hfsutils/
  email: Marcus.Better@abc.se
  Originally Copyright (C) 1996, 1997 by Robert Leslie, email: rob@mars.org

- GNU GPL (General Public License) now. See "Copying".
  Source code available by request, email: lpesonen@nic.fi

- MFC42.DLL not needed anymore.



Version 1.1.1:
--------------
- No fatal bugs reported since 1.1.0, so I assume that disk
  writing functions are ok.

- Deleting from fat volume was broken (simply didn't delete!)
  in 1.1.0, fixed.

- Fixed a bug that caused HFS -> HFS copied files
  to be shown with standard icons only.
  (Failed to clear the "inited" finder flag after copying)

- Creating own "Desktop" (after format) was a bad idea.
  Does not work, but prevents Finder from copying stuff
  to Desktop later. Removed.
  Erroneous Desktops should be deleted. Affects
  vMac only, Executor seems not use Desktop.

- CD support. HFS cd's and hybrid ISO/HFS.
    - No support for High Sierra.
    - No support for ISO with Apple Extensions. They are
      shown as FAT volumes, with icon information missing.
    - CD auto notification supported, just pop in the CD
      and it should appear in the tree view.
      In some machines, the auto-insertion/arrival messages
      take long time to reach applications. There
      is nothing HFVExplorer can do about it, press
      F5 if you don't want to wait.
  - Small memory leak (custom icons).
    Windows 95:
      - Seems to work ok now. Requires that CDENABLE.VXD is copied to
        your Windows directory. System dir is ok too, as any directory
        within path. Do NOT copy to HFVExplorer directory.
      - ALSO when HFVExplorer is running, DOS Executor (and any other
        program) is able to see cd's, too - and using protected mode
        drivers! No MSCDEX required. In some machines, you had to boot
    to MS-DOS mode to see CD drive.
    NT 4.0:
      More limited so far, does not open all cd's.

- AppleDouble now supported when copying/deleting files
  to HFS volumes. From HFS volumes creates MacBinary II.

- About box: credits edited. Version updated 1.1.1

- DSK filter added to "Open" dialog.

- Some missing tool tips added.

- NEW icon in tool bar opens now the Format dialog.

- New way to create volumes: in Windows Explorer,
  right-click and select New/HFVExp Document.
  Creates empty file, double-click it to open
  it in HFV Explorer -> will ask volume name and size.

- Option dialog: first page is now functional.



Version 1.1.1b & c
------------------
- Attempted to fix cache bug.
- Bundle bit was lost when copying HFS -> HFS
  causing missing icons.

Version 1.1.1d
--------------
- Fixed a bug in reading from CD.
  (caused missing icons and the previous "Known bug: sometimes
  has problems opening items in the root directory of the CD."


Version 1.1.1e
--------------
- Fixed a bug in reading from CD.
  Failed to mount some CD's with partition
  located at very high addresses, near end of disk.


Version 1.1.1f
--------------
- Following performance improvements:
  - Read-ahead code for CD's.
  - Larger cache.
  - Does not update screen so often any more.
    Could be still better.
- Memory leaks fixed.


Version 1.1.1g
--------------
- Properties dialog. Click file/folder
  with right mouse button (or Alt-Enter).
  Select properties. Be careful in what you change.
- Name locked items cannot be renamed any more, without
  removing the lock first (properties).
- Tool bar buttons to eject/reload all CD trays.


Version 1.1.1h
--------------
- Failed to mount hybrid CD's with boot block zeroed out.
  Fixed. For example, Executor 2 CD.


Version 1.1.1i
--------------
- Support for pure Mac CD's under Windows NT.
  CDENABLE.SYS must be copied to your System32\Drivers
  directory. After copying the driver, the program
  must be run once with system administrator rights.
  Later normal user rights suffice.


Version 1.2.0
-------------
- Faster directory browsing. Especially large volumes
  and directories work better, and CD's. The performance
  hit was tremendous if the volume had files more than
  a specific threshold.

- Multiple item selection -- move, copy, delete.
  Works as in Explorer (or in any multi-selection list control).

- File copy mode selection for HFS -> FAT and FAT -> HFS.
  Selected from Options dialog (new property page).
  Maybe in future there will be file type mapping.

- "Create shortcut" command implemented for Executor/DOS,
  Executor/Win32 and vMac, including a launcher app for vMac,
  and an automatic alias creator.
  Places an icon on the Windows desktop, pointing to either a document
  or an application. Automatic Mac icon extraction. 
  If you want the special characters to be shown in icon names, 
  change the desktop icon font (Control Panel/Display/
  Appearance/Icon "Font") to something else than MS Sans Serif, 
  perhaps Arial, which may be ugly but has more visible characters.
  Icons and parameter files are placed into subdirectory "Links".
  Helper directory "vMacLink" contains a link to the vMac installation.
  Click the button "Install files required by vMac" to set up this
  directory.

- "Open with" supports the tree emulators (E/DOS, E/Win32, vMac).
  *** IMPORTANT *** : DO NOT try to copy/move/delete/create new files 
  in HFVExplorer when the emulator is running. MacOS doesn't like
  anything happening behind its back. The file accesses are not properly
  synchronized, and if unlucky, you may suffer data loss.
  HFVExplorer tries to protect you from this, but its protection
  schemes are far from perfect. It takes care that its own data
  structures are always in sync, but the emulators don't do this.
  After the emulator terminates, HFVExplorer checks if it needs to
  update the file structures and if so, it does this. That's the
  reason why it takes a while when emulator terminates -- if one bit
  of data has changed in a volume file, the only safe way is to
  reload the whole volume.

- You should normally use the alias launching method with vMac,
  and only try Apple events method if this fails. Executor users
  should not be concerned with the launching method since appropriate
  way is selected automatically. Just make sure that you don't
  claim in the setup dialog that your Windows Executor is DOS
  or vice versa -- this information has to be correct.

- Icon corruption bug fixed. To get rid of the old
  corrupted icons, the icon cache file "HFVICONS.DAT" must be deleted.
  HFVExplorer will ask you whether it's ok to delete this file.
  After deleting the file, you may notice that some icons,
  typically various preference files but some documents as well,
  do not have proper icons. This is normal, since HFVExplorer 
  does not load icons from MacOS Desktop file or Desktop database, 
  but uses its own system. The icons will eventually reappear after
  you browse through the folder containing the creator applications
  which has the bundle resources and icons.

- Resolves folder custom icons.

- Resolves (some) alias types icons.

- HFS and FAT default icons are now in different colors.
  The new plain icons are best shown if you have window color something
  different than white (Control Panel/Display/Appearance/item "Window")

- Folder "Icon\x00D" file is copied out to FAT volumes as "Icon~"
  This file contains the custom icons of a folder, if any.

- CD's were mounted as R/W file systems, causing strange error
  messages even when copying succeeded.

- Accelerator F2 to edit file name.

- Obsolete tool buttons removed, new buttons added
  to read a volume file from floppy and write one to a floppy.

- Some more tool tips added.

- Possibility to limit the time spent in creating DOS icons.
  In options dialog.

- Mac vs. ANSI character set fixes. Some characters may still
  be incorrect, but see the note in "Create shortcut" above.

- Uses Arial font in most places now. Yes I know it's ugly,
  but MS Sans Serif does not have some important special
  characters.

- Possibility to hide/show "invisible" files.

- Pressing return or double-clicking now opens Executor/Win32 by default.
  You can change this behavior from the options dialog.

- Changing of "is alias" enabled in properties dialog.

- Many miscellaneous bug fixes.
  
- Hook1508 -- a tool to help DOS Executor see the HFS (non-hybrid) 
  CD's under Windows NT 4.0. Not included in this archive.
  If you intend to use Hook1508 with HFVExplorer shortcuts and/or
  launching, you should either create a .bat or modify AUTOXEC.NT,
  or make a custom version AUTOXEC.NT. The procedure is described
  in Hook1508 read me file.


Version 1.2.0 -- additon to same version
-------------
- "Icon\x00D" is now treated as invisible file even if the 
  corresponding flag is not set.

- Explained the netbios problem under NT.


Version 1.2.1
-------------
- Configurable font and point size (Options/General)
- Remembers window position and size
- Mac floppies are now detected when "A:\" is clicked.
- Bugs in emulator launching code fixed
- Minor cosmetic fixes
- List view now always shows the selection
- Some special characters fixed in AppleDouble names
- CD support for Windows 98, was previously a separate download.
  (Check the size of \Windows\System\cdenable.vxd -- should be 10400 bytes.
  Some users experienced file corruption when downloading the separate vxd.)


Version 1.2.2
-------------
- Japanese font support, thanks to Mr. Noda (noda@lyra.vega.or.jp).
- Japanese users please check the option "Font character set is SJIS (Japanese)"
  in "General" page of the options, change Font (for example "MS gothic")
  and restart HFVExplorer.


Version 1.2.3
-------------
- File type conversion table DOS extension <--> MacOS type/creator.
  See "Options/Copying Files/Raw copy file type mappings".
  Click the "?" button for help.
  Initially, the program comes with no preset mappings, but once you
  have typed in some they can be easily shared with other people with
  export/import commands. 
- New right-click popup menu command to prefill the mapping dialog
  with currently selected item(s), either DOS or HFS files will do.
  Function key F4 does the same thing.
- Copying files between HFS and DOS does not change ' ' <-> '_' any more.
  Some other character mapping fixes in file names. 


Version 1.2.4
-------------
- File type conversion table (1.2.3): fixed a bug which caused types and
  creators that contain special characters to be ignored.

- In report (details) list view, columns may now be sorted by clicking
  the column header. Click again to reverse the order (asdending/descending).

- Creating new volume file is much faster now.

- Yet another experimental feature: "Finder view mode" tool button. 
  Shows the files and folders in original positions and uses Finder "magic rectangle"
  to reposition moved icons. Use a small font point size.

  Works under any System version, but positioning new items works only under
  System 7.1.1 or later. 

  Note that Finder relocates items only when it "sees" them; if some application
  (including Finder itself; i.e. "Recent" folders) has installed new files 
  and you have never opened the window in Finder, the files may be layered
  on top of each other. HFVExplorer tries to place them "somewhere".

  If you copy new files into a folder and do not move them in "Finder view mode",
  they may not be shown exactly in same positions in Finder and HFVExplorer.
  This is because HFVExplorer does not try to mimic the Finder automatic
  placement function. Either move them in HFVExplorer or visit the folder in Finder.

  "Go up..." button is not available in Finder view, there is no good place for it.
  Use the new keyboard shortcut Backspace to move upwards (used to work when the 
  focus was in tree view, now works always).

  (Check "http://developer.apple.com/technotes/tb/tb_42.html" for a discussion
  on the problems when positioning Finder icons.)



Version 1.2.5 (in-house)
------------------------
- DOS report view said that all folders have "0 items". Fixed.
- AppleDouble report view displayed an incorrect number for folder item count. Fixed.
- A message "Failed to set item properties" was generated every time
  when trying to move within a window in read-only media, when "Finder view"
  was selected. Fixed.
- Launch support for WinUAE/Shapeshifter (please see the bug list).
- Launch support for Fusion demo (not completed).



Version 1.2.6
-------------
- Sorting by HFS creation/modification date was intolerably slow. Fixed.
- Some icons crashed the program. Now they shouldn't any more, but they
  are shown garbled. Not perfect, but will do for now.
- Some popup menu reorderings.
- Fixed some problems with WinUAE/Shapeshifter launching.
- Launch support for Fusion finished.
- Alias launching for vMac, Fusion and ShapeShifter: optionally, installs
  a tiny Mac app into Startup folder (HFVExplorer Remover). It's only purpose
  is to wait in background for 5 seconds and then delete the alias.
  This is safer than the old method which deleted the alias under Windows.
  Check "Remove alias" in the corresponding emulator dialog to activate this feature.
- Apple events launching should not be used any more.



Version 1.2.6 (addition)
------------------------
- Updated bug list.



Version 1.2.7 (icon facelift release)
-------------------------------------
- Compressed icons are not shown any more (result was garbage).
  Generic icons are used instead. Decompression algorithm is not documented, see:
  http://developer.apple.com/technotes/tb/tb_555.html
- Icons 16 colors -> 256 colors. Both in list view and shortcuts.
  Requires "thousands of colors" display mode at minimum. 15, 16, 24 and 32 bits supported.
  To show the icons correctly in Windows desktop, enable in Control panel: 
  "Display" / "Plus!" / "Show icons using all possible colors".
- "Save icon..." command (right click menu), works for folders too.
- Finder label colors. Displayed as superimposed colored pedestals right below 
  the icons. Not perfect (will probably change later), but the way MacOS shows 
  the labels of colored icons is (arguably) even worse.
  Right click items to change labels.
  Uses default colors and texts (Red=Hot, Orange=Essential etc).



Version 1.2.8
-------------
- It turned out that Toast allows writing of CD's with very long volume
  names. Although it correctly cuts the name at 27 characters in MDB, 
  the entry in Catalog File is longer which causes problems for
  HFVExplorer. To mount such CD's, edit the file "HFVExplorer.ini" in
  your Windows directory as shown below:

  [Setup]
  UsePartialNames=1 ; default value is 0

  If there is entry "UsePartialNames" already, change the value to be "1",
  otherwise create new entry.

  This has the effect that any requests to the hfsutils core are made
  with partial file paths instead of full file paths:
     ":MyFolder:Myfile.xxx"
  instead of
     "MyMacVolume:MyFolder:Myfile.xxx"
  Once I'm sure that this works always, I may turn it on by default.

- Support for volume files named as HFx (where x is anything).



Version 1.2.9 (bug fixes)
-------------------------
- Fixed some special character issues when copying files
  between HFS and FAT file systems.



Version 1.2.10 (bug fixes)
--------------------------
- Folder custom icons were lost when copying from HFS -> FAT and back again.
- A bug in deletion fixed (HFS & FAT); sometimes, one had to try delete twice
  before a nested folder was completely deleted.
- If a folder name had any special characters, then the creation of a subfolder
  below it failed (when copying a deep  directory structure). 
  This was a new bug in 1.2.9.



Version 1.2.11 (clipboard release)
----------------------------------
- "Cut", "Copy" & "Paste" -- these work mostly like in Explorer.
- "Cut Append" & "Copy Append" -- the selected items are appended to the
  clipboard file list. You can even mix "Cut Append" and "Copy Append"
  commands so that one "Paste" command copies some files and cuts some.
- Escape empties the clipboard ("Cut" and "Copy" also empty it before a refill)
- Added more checks for illegal copy attempts. The destination folder may 
  not be a subfolder of the source folder, etc.
- "Select All" command. Control-A.
- When moving directory structures between HFS and FAT, not all files
  were deleted but empty directories was left behind. Fixed.
- The default action changed when the "A:\" or "B:\" floppy symbols are clicked.
  Starting from version 1.2.1, Mac floppies were detected too, but this
  is not the case any more. There seems to be some very nasty bug related
  to this feature, and therefore it's only accessible by manually editing
  HFVExplorer.ini. If you really want that feature back, change the following:

      [Setup]
      DisableAutoFloppy=0

  Normally, you should press F5 to refresh the screen after
  inserting/changing/removing floppies.



Version 1.2.12
--------------
- Tool buttons for "Cut Append" & "Copy Append".
- One comment was left out about 1.2.11 "Append" command
  description: note that you can append from different 
  folders, even from different volumes and medias (but do 
  not change the floppy or cd when collecting files).



Version 1.2.13
--------------
- Launching Mac documents with Windows applications (New "emulator").
  The documents are copied to the Windows temporary folder and opened.
  When the application (not document) is closed, the user is prompted
  whether to update the changes, if there are any. 
  Multiple documents can be open at the same time. The changes are tracked
  file by file.

  If the original document does not have an extension, a file type
  mapping must be created; a dialog box will open automatically.

  Under Windows NT, launching to 16 bit Windows apps does not 
  work properly. The document opens ok, but HFVExplorer does not
  recognize when the application is terminated.

- Default emulator (after first time installation) is now the one 
  described above.

- Copying HFS->FAT: file type mappings are not enforced
  any more if the path already contains an(y) extension.



Version 1.3.0 (beta 1 - beta 3: in-house)


Version 1.3.0
-------------
- Support for Mac formatted removable media (Zip and others).
  No ASPI manager needed. For Windows 9x, cdenable.vxd must 
  be updated. For Windows NT, no driver update is needed.
  The removable drive must have a drive letter in Windows.
  If you use any HFS file system driver (MacOpener or others),
  do not try to modify the media simultaneously in HFVExplorer
  and via a drive letter.
  Auto-insert notification works with Zip disks under Windows 9x.
  Under Windows NT, you need to use Refresh command (F5) after
  changing media.

- Write-back cache code for all writable media. Consequently,
  some performance boost in floppy write throughput.

- None of the newly supported media can be formatted, yet.

- Some enhancements in read ahead cache code.

- Fixed CD eject problems (HP7110e & NT, maybe some others)

- CD eject and reload now manipulate the selected drive only.

- Eject and reload work for removable disks too, if possible
  (for example, Zip's can only eject).

- New accelerators: Ctrl-E to eject selected media, Ctrl-R to reload it.

- Attempted to fix a conflict between Norton AV 5.0 under
  Windows 9x (reportedly freezed when accessing floppy drive 
  with no floppy disk present).

- Fixed a crash when ejecting, and the selected item
  was not a drive (i.e. the "Configurations" icon).

- Removed the "Configurations" icon from the tree view.
  It had no function (never had).

- Fixed strange error messages when copying from Zip drive
  to elsewhere. Strangely enough, HFVExplorer actually copied
  allright, but all the same believed that there was an error
  an reported nonsense things. The same bug once pestered my
  CD code in its infancy, and now the same bug reappeared here.

- Removed (undocumented) hard disk support. I have no chance
  to test it properly now, and I should before I let anyone else 
  try it out.

- Fixed a bug in floppy writing code.

- Removable media and floppy write code was disabled in beta 2,
  should work now.

- Fixed a bug in "New folder" code.

- Supports HPBurner. Menu command and tool button for burning cd.
  You'll need HPBurner.exe to write to CDR/CDRW media.
   ONLY works with HP CD-Writer+ 7100 (and compatibles).

- Fixed reload media crash.

- execut95.exe is not supported any more and was removed from the 
  distribution archive.




Version 1.3.1
-------------
Fixed some cd burning problems.




Bug list
--------
- Copying when the file already exists: behaves badly,
  does not ask for confirmation to overwrite. If you copy
  a file and there is a *folder* with same name -> not
  even error message is given, and the copying fails.
  As for now, I decided that this is a "feature" :)

- In report view, file dates seem to be wrong (again/still?).
  I thought that it was fixed. oh well.

- Properties and file mappings with F4 does not work for AppleDouble files.

- Root folder of DOS volumes: two first items are overlapped (in some systems).
  Could be a bug in Common dialog control libraries, since not all systems
  have this problem.

- AppleDouble launching for Executor DOS works only with short file names.
  Not much I can do for it.

- vMac launch has problems under NT 5 beta 2. Seems to be a beta bug,
  cannot resolve shortcuts properly.

- Finder view mode: in some systems, the scroll bars always appear and
  the window is scrolled a few pixels. Very annoying. Strangely enough, 
  I cannot repeat this on my dev machine. I can repeat it with my backup 
  machine so there's hope to get it eventually fixed. Could be a bug in MFC 
  libraries which is already fixed.

- Options dialog: some "browse" buttons outsmart themselves when a directory
  name contains spaces.

- Shapeshifter launching crashes sometimes if "Remove alias" is on. Hard to fix
  since it can be repeated by manually launching two aliases in SS within
  narrow enough time frame. If this happens to you, do not use "Remove alias".



Wish list
---------
- Support for Services for Macintosh (SFM) under Windows NT.
  There is now minimal support (showing icons), copying is not yet implemented.

- Unimplemented: "Find"

- Unimplemented: Preview window (I forgot the whole thing...)

- If you open a second browser window (Control-double
  click to folder, control-enter) it can't access HFS 
  floppies (under NT at least). "Second browser" is quite
  useless feature anyway, until one can drop between views.

- OLE drag and drop.

- Expanding Finder view mode to a full-blown Finder replacement???
  This would require kicking out the MFC CListView code, it's too restricted.

- Icon cache code should be rewritten.

- Should upgrage to Rob's latest hfsutils source tree (w/ blessed folder).
  Would allow making bootable volume file.

- Should follow folder aliases ('fdrp'), or launch them.

- HFS volume browse dialog (no big deal)

- After delete, tree view still shows the deleted folders...

- Copy progress dialog

- F4 should have menu equivalent; and the right-click commands
  should be in menu too.

- What should be done when copying FAT files starting with a dot.
  Traditionally Mac driver names only, Apple says that no normal
  file name should start with a dot.

- Compressed Mac resources.

- PC link floppies.

- Perhaps copying files should always update the icon locations;
  trying to keep the original order, but avoiding any overlaps.
  It is confusing that (in Finder view mode) you need to move
  the icons again in order to permanently record the positions,
  so that real Finder honours them.

- Copyright list



3/12/1999
Lauri Pesonen
lpesonen@nic.fi
