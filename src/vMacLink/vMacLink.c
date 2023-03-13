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

/*
parse command line
	-> hfv_file, mac_target

mac_target char conversion, ::aa -> mac chars

find HFVExplorer.ini [Setup] vMacSystemFile=I:\EXECUTOR\System7.hfv
	-> system_file

find HFVExplorer.ini [Setup] vMacStartupFolder=System Folder:Startup Items
	-> mac_startup_items_folder
	-> mac_system_folder

if(hfv_file != system_file) patch Moduledir\vMacLink\vMac.ini  
	-> DrivePath2 = hfv_file

copy Moduledir\Launcher Application -> mac_startup_items_folder:Launcher Application

create Launcher Parameters, containing line mac_target
copy Launcher Parameters -> mac_system_folder:Launcher Parameters

ShellExecute Moduledir\vMacLink\vMac.lnk

Change thread priority to idle

Wait 3 seconds

wait until vmac is gone

delete mac_startup_items_folder:Launcher Application
delete mac_system_folder:Launcher Parameters

done.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
// #include "..\hfs\libhfs\hfs.h"
#include "..\hfs\libhfs\internal.h"
#include "..\hfs\hcwd.h"
#include "..\hfs\hfsutil.h"
#include "..\hfs\suid.h"
#include "..\hfs\hmount.h"
#include "..\hfs\humount.h"
#include "..\hfs\hcopy.h"
#include "..\hfs\copyhfs.h"
#include "..\vmacpatch.h"
#include "..\filehook.h"
#include "..\aspecial.h"
#include "alias.h"
#include "reboot.h"

// Quick hack from "volume.c"
void set_invalid_cd_hack( int onoff );

static char *sHFVExplorer_ini = "HFVExplorer.ini";
static char *sCaption = "vMacLink";
static char *sLauncherSourceApplication = "Launcher Application";
static char *sLauncherDestApplication = "HFVExplorer Launcher";
static char *sLauncherParameters = "Launcher Parameters";
static char *sRemoverDestApplication = "HFVExplorer Remover";

HINSTANCE hinst = 0;

enum { LAUNCH_METHOD_ALIAS=0, LAUNCH_METHOD_LAUNCHER };
enum { EMULATOR_VMAC=0, EMULATOR_SS, EMULATOR_FUSION };

char hfv_file[MAX_PATH*2];
char mac_target[MAX_PATH*2];
char system_file[MAX_PATH];
char mac_system_folder[MAX_PATH];
char mac_startup_items_folder[MAX_PATH];
int launch_method = LAUNCH_METHOD_ALIAS;
int m_use_remover = 1;
int m_use_partial_names = FALSE;

// for hfs routines
int crlf_translation = 1;

int emulator = EMULATOR_VMAC;

void hfs_perror(char *msg)
{
	char line[200];
  char *str;

	// if(HFSIFACE_m_silent) return;

  str = strerror(errno);

  if (hfs_error == 0) {
    sprintf(line, "%s: %c%s\n", msg, tolower(*str), str + 1);
  } else {
		if (errno)
			sprintf(line, "%s: %s (%s)\n", msg, hfs_error, str);
    else
			sprintf(line, "%s: %s\n", msg, hfs_error);
  }
	// mac_to_pc_charset(line);
	MessageBox( 0, line, sCaption, MB_OK | MB_ICONSTOP );
}

void hfs_perrorp(char *path)
{
	hfs_perror(path);
}

// DUMMY for copyin
int get_apple_double_type_creator( 
	char *name, 
	char *stype, char *screator,
	unsigned long *pstart,
	unsigned long *plength
) 
{
	return(0);
}

void zerr0( char *arg1 )
{
	MessageBox( 0, arg1, sCaption, MB_OK | MB_ICONSTOP );
}

void zerr1( char *f, char *arg1 )
{
	char buf[500];
	sprintf( buf, f, arg1 );
	zerr0( buf );
}

int set_us_low_priority(void)
{
	HANDLE hThread = GetCurrentThread();
	int result = 0;
	if(hThread) {
		if(SetThreadPriority(hThread,THREAD_PRIORITY_BELOW_NORMAL)) {
			result = 1;
		}
	}
	return(result);
}

void decode_aa( unsigned char *src, unsigned char *dst )
{
	int i, j=0, len;
	len = strlen( src );
	for(i=0; i<len; i++) {
		if(src[i] == ':' && src[i+1] == ':') {
			dst[j] = unhex2( &src[i+2] );
			i += 3; 
		} else {
			dst[j] = src[i];
		}
		j++;
	}
	dst[j] = 0;
}

// G:\C\LP\HFVExplorer\Debug\vMacLink.exe I:\EXECUTOR\MacDisk.hfv?A:MacDisk:LightSpeedC:LightspeedC::aa
int parse_command_line( char *lpCmdLine, int *emulator )
{
	char *p;

	if(strncmp(lpCmdLine,"-SS ",4) == 0) {
		*emulator = EMULATOR_SS;
		lpCmdLine += 4;
	} else 
	if(strncmp(lpCmdLine,"-FU ",4) == 0) {
		*emulator = EMULATOR_FUSION;
		lpCmdLine += 4;
	} else {
		*emulator = EMULATOR_VMAC;
	}

	strcpy( hfv_file, lpCmdLine );
	p = strchr( hfv_file, '?' );
	if(!p) {
		zerr1( "Command line syntax error", lpCmdLine );
		return(0);
	}
	*p = 0;
	p++;
	mac_target[0] = p[0];
	mac_target[1] = p[1];
	decode_aa( &p[2], &mac_target[2] );
	launch_method = (mac_target[0] == 'S') ? LAUNCH_METHOD_ALIAS : LAUNCH_METHOD_LAUNCHER;
	return(1);
}

int read_ini(int emulator)
{
	char *p, *entry1, *entry2, *entry3;

	switch(emulator) {
		case EMULATOR_VMAC:
			entry1 = "vMacSystemFile";
			entry2 = "vMacStartupFolder";
			entry3 = "vMacUseRemover";
			break;
		case EMULATOR_SS:
			entry1 = "ShapeshifterSystemFile";
			entry2 = "ShapeshifterStartupFolder";
			entry3 = "ShapeshifterUseRemover";
			break;
		case EMULATOR_FUSION:
			entry1 = "FusionSystemFile";
			entry2 = "FusionStartupFolder";
			entry3 = "FusionUseRemover";
			if(GetPrivateProfileInt("Setup","RebootWarning",1,sHFVExplorer_ini) != 0) {
				int answer = MessageBox( 0, "Your machine is going to reboot now. Show this warning next time?", sCaption, MB_YESNOCANCEL | MB_ICONQUESTION );
				if(answer == IDCANCEL) return(0);
				if(answer == IDNO) {
					WritePrivateProfileString("Setup","RebootWarning","0",sHFVExplorer_ini);
				}
			}
			break;
	}

	GetPrivateProfileString( 
		"Setup",
		entry1,
		"",
		system_file,
		sizeof(system_file),
		sHFVExplorer_ini
	);
	if(!*system_file) {
		zerr1( "Emulator SystemFile not defined in %s", sHFVExplorer_ini );
		return(0);
	}
	GetPrivateProfileString( 
		"Setup",
		entry2,
		"",
		mac_startup_items_folder,
		sizeof(mac_startup_items_folder),
		sHFVExplorer_ini
	);
	if(!*mac_startup_items_folder) {
		zerr1( "Emulator StartupFolder not defined in %s", sHFVExplorer_ini );
		return(0);
	}
	m_use_remover = GetPrivateProfileInt(
		"Setup",
		entry3,
		1,
		sHFVExplorer_ini
	);
	strcpy( mac_system_folder, mac_startup_items_folder );
	p = strrchr( mac_system_folder, ':' );
	if(!p) {
		zerr1( "The path \"%s\" does not contain a colon", mac_system_folder );
		return(0);
	}
	*p = 0;

	return(1);
}

int ini_patcher_vmac( void )
{
	int ret = 0;
	char vmacini[MAX_PATH], *p;

	if(stricmp(hfv_file,system_file) == 0) {
		// Target volume file is already visible (the boot volume)
		ret = 1;
	} else {
		GetModuleFileName( hinst, vmacini, _MAX_PATH );
		p = strrchr( vmacini, '\\' );
		if(p) *p = 0;
		strcat( vmacini, "\\vMacLink\\vMac.ini" );
		ret = patch_vmac_ini( vmacini, NULL, NULL, hfv_file );
		if(ret) {
			patch_vmac_ini( vmacini, NULL, system_file, NULL );
		}
	}
	return(ret);
}

int ini_patcher_ss(void)
{
	char ss_prefs[_MAX_PATH], f_sys[_MAX_PATH], f_arg[_MAX_PATH], ss_volume[_MAX_PATH], *p;
	int ret = 0;

	GetPrivateProfileString( 
		"Setup",
		"ShapeshifterPreferences",
		"",
		ss_prefs,
		sizeof(ss_prefs),
		sHFVExplorer_ini
	);
	if(!*ss_prefs) {
		zerr0( "ShapeshifterPreferences not defined." );
		return(0);
	}
	GetPrivateProfileString( 
		"Setup",
		"ShapeshifterVolume",
		"",
		ss_volume,
		sizeof(ss_volume),
		sHFVExplorer_ini
	);
	if(!*ss_volume) {
		zerr0( "ShapeshifterVolume not defined." );
		return(0);
	}
	if(ss_volume[strlen(ss_volume)-1] != ':') {
		strcat( ss_volume, ":" );
	}
	strcpy( f_sys, ss_volume );
	p = strrchr( system_file, '\\' );
	if(!p) return(0);
	strcat( f_sys, p+1 );

	strcpy( f_arg, ss_volume );
	p = strrchr( hfv_file, '\\' );
	if(!p) return(0);
	strcat( f_arg, p+1 );

	if(*ss_prefs) {
		if(stricmp(hfv_file,system_file) == 0) {
			ret = patch_ss_prefs( ss_prefs, f_sys, NULL );
		} else {
			ret = patch_ss_prefs( ss_prefs, f_sys, f_arg );
		}
	}
	return(ret);
}

int ini_patcher_fusion(void)
{
	return(1);
}

int ini_patcher( int emulator )
{
	switch(emulator) {
		case EMULATOR_VMAC:
			return(ini_patcher_vmac());
		case EMULATOR_SS:
			return(ini_patcher_ss());
		case EMULATOR_FUSION:
			return(ini_patcher_fusion());
	}
	return(0);
}

int mount_flags( char *path )
{
	path = path;
	return(O_RDWR);
}

int VMACLINK_mount(char *path)
{
  hfsvol *vol;
  hfsvolent ent;
  int partno, result = 0;

  partno = 1;

  // suid_enable();
  vol = hfs_mount(path, partno, mount_flags(path));
  // suid_disable();

  if (vol == 0) {
    hfs_perror(path);
    return 1;
  }

  hfs_vstat(vol, &ent);
  // hfs_pinfo(&ent);

  if (hcwd_mounted(ent.name, ent.crdate, path, partno) < 0) {
    perror("Failed to record mount");
    result = 1;
  }

  if (hfs_umount(vol) < 0 && result == 0) {
    hfs_perror("Error closing HFS volume");
    result = 1;
  }
  return( result );
}

int VMACLINK_umount(char *path)
{
  int vnum;
  mountent *ent;

  for (ent = hcwd_getvol(vnum = 0); ent; ent = hcwd_getvol(++vnum)) {
	  if (strcmp(path, ent->path) == 0) {
		  hcwd_umounted(vnum);
		  return 0;
		}
  }
  fprintf(stderr, "Unknown volume \"%s\"\n", path);
  return 1;
}

hfsvol *hfs_remount(mountent *ment, int flags)
{
  hfsvol *vol;
  hfsvolent vent;

  if (ment == 0) {
    fprintf( stderr, "%s: No volume is current; use `hmount' or `hvol'\n" );
		return 0;
  }

  // suid_enable();
  vol = hfs_mount( ment->path, ment->partno, flags );
  // suid_disable();

  if (vol == 0) {
		hfs_perror(ment->path);
		return 0;
  }
  hfs_vstat(vol, &vent);
  if (strcmp(vent.name, ment->vname) != 0) {
    fprintf(stderr, "Expected volume \"%s\" not found\n",
	    ment->vname);
    fprintf(stderr, "Replace media on %s or use `hmount'\n",
	    ment->path);
    hfs_umount(vol);
    return 0;
  }
  if (hfs_chdir(vol, ment->cwd) < 0) {
    fprintf(stderr, "Current HFS directory \"%s%s:\" no longer exists\n",
	    ment->vname, ment->cwd);
  }
  return vol;
}

static int path2volinx(char *path)
{
  int vnum;
  mountent *ent;

  for (ent = hcwd_getvol(vnum = 0); ent; ent = hcwd_getvol(++vnum)) {
	  if (strcmp(path, ent->path) == 0) {
		  return( vnum );
		}
  }
  return( -1 );
}

// There is no need to free/unlock these things.
int load_resource( 
	HINSTANCE hInstance, 
	char *name,
	char *type,
	char **bytes
)
{
	HRSRC rsrc;
	HGLOBAL hg;
	int count;
	
	count = 0;
	*bytes = 0;

	rsrc = FindResource( hInstance, name, type );
	if(rsrc) {
		count = SizeofResource( hInstance, rsrc );
		hg = LoadResource( hInstance, rsrc );
		if(hg) {
			*bytes = (char *)LockResource( hg );
		}
	}
	if( *bytes == 0 ) count = 0;
	return( count );
}

int hex2bin_buffer( 
	int hex_code_size, 
	unsigned char *hex_code_bytes, 
	int *bin_code_size, 
	unsigned char **bin_code_bytes )
{
	int ret = 0, i, count;
	unsigned char *p;

	count = hex_code_size >> 1;
	p = malloc( count );
	if(p) {
		*bin_code_bytes = p;
		*bin_code_size  = count;
		for(i=0; i<count; i++) {
			 *p = unhex2( hex_code_bytes );
			 p++;
			 hex_code_bytes += 2;
		}
		ret = 1;
	}
	return(ret);
}

int put_mac_data( 
	hfsvol *vol, 
	char *dstname, 
	HFILE hf, 
	char *stext,
	char *binbuf,
	int binsize,
	BOOL rfork,
	char *os_type,
	char *os_creator
)
{
  hfsfile *ofile = 0;
  char buf[HFS_BLOCKSZ * 4];
  long bytes;
	int ret = 0;
	int static_bytes_left = 0;

	hfs_delete(vol, dstname);
  if(hfs_create(vol, dstname, os_type, os_creator) < 0) {
		zerr1( "Cannot create file \"%s\"", dstname );
    return 0;
  }
  ofile = hfs_open(vol, dstname);
  if(ofile == 0) {
		zerr1( "Cannot open file \"%s\"", dstname );
		return 0;
  }
	if(stext) {
		static_bytes_left = strlen(stext);
	}
  if(rfork && hfs_setfork(ofile, 1) < 0) {
		zerr1( "Error selecting resource fork in file \"%s\"", dstname );
	} else {
		while (1) {
			if(binbuf) {
				if(binsize) {
					bytes = binsize > sizeof(buf) ? sizeof(buf) : binsize;
					memcpy( buf, binbuf, bytes );
					binsize -= bytes;
					binbuf += bytes;
				} else {
					bytes = 0;
				}
			} else if(stext) {
				if(static_bytes_left) {
					bytes = static_bytes_left;
					memcpy( buf, stext, bytes );
					static_bytes_left = 0;
				} else {
					bytes = 0;
				}
			} else {
				bytes = _lread( hf, buf, sizeof(buf) );
			}
			if (bytes < 0) {
				zerr0( "Error reading from input file" );
				break;
			} else if (bytes == 0) {
				ret = 1;
				break;
			}
			if(hfs_write(ofile, buf, bytes) < 0) {
				zerr1( "Error writing to file \"%s\", volume probably full", dstname );
				break;
			}
		}
	}
  if(ofile) hfs_close(ofile);
	return(ret);
}

int get_type_creator( hfsvol *vol, char *path, char *type, char *creator )
{
	hfsdirent ent;
	int result = 0;

	if (hfs_stat(vol, path, &ent) >= 0) {
		strncpy( type, ent.u.file.type, 4 );
		strncpy( creator, ent.u.file.creator, 4 );
		type[4] = 0;
		creator[4] = 0;
		result = 1;
	}
	return( result );
}

int change_to_alias( hfsvol *vol, char *path )
{
	hfsdirent ent;
	int result = 0;

	if (hfs_stat(vol, path, &ent) >= 0) {
		ent.fdflags |= HFS_FNDR_ISALIAS;
		if (hfs_setattr(vol, path, &ent) >= 0) {
			result = 1;
		}
	}
	return( result );
}

// copy Moduledir\Launcher Application -> mac_startup_items_folder:Launcher Application
// create Launcher Parameters, containing line mac_target
// copy Launcher Parameters -> mac_system_folder:Launcher Parameters
int patch_volume_file( char *volpath, int bin_code_size, char *bin_code_bytes )
{
	char f1[MAX_PATH], f2[MAX_PATH], pcLauncher[MAX_PATH], *p;
	char macname[MAX_PATH];
	int volinx, result = 0;
	char type[10], creator[10];
  hfsvol *vol = 0;
	int alias_bin_code_size = 0;
	char *alias_bin_code_bytes = 0;
	
	GetModuleFileName( hinst, pcLauncher, _MAX_PATH );
	p = strrchr( pcLauncher, '\\' );
	if(p) *(p+1) = 0;
	strcat( pcLauncher, sLauncherSourceApplication );

	strcpy( macname, &mac_target[2] );

	hfs_error = 0;
	if(launch_method == LAUNCH_METHOD_ALIAS) {
		if(0 == VMACLINK_mount( hfv_file )) {
			volinx = path2volinx(hfv_file);
			if(volinx	>= 0) {
				vol = hfs_remount(hcwd_getvol(volinx), mount_flags(hfv_file));
				if (vol != 0) {
					if(get_type_creator( vol, macname, type, creator )) {
						if(strcmp(type,"APPL") == 0) {
							strcpy( type, "adrp" );
						}
						alias_bin_code_size = make_alias( vol, macname, &alias_bin_code_bytes );
						if(alias_bin_code_size) {
							result = 1;
						}
					}
					hfs_umount(vol);
				}
				VMACLINK_umount( hfv_file );
			}
		}
		if(result == 0) return(0);
		result = 0; // to trap next errors
	}

	hfs_error = 0;
	if(0 == VMACLINK_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				switch(launch_method) {
					case LAUNCH_METHOD_ALIAS:
						sprintf( f1, "%s:%s:%s", vol->mdb.drVN, mac_startup_items_folder, sLauncherDestApplication );
						if(put_mac_data( vol, f1, HFILE_ERROR, NULL, alias_bin_code_bytes,alias_bin_code_size, TRUE, type, creator )) {
							if(change_to_alias( vol, f1 )) {
								result = 1;
							}
						}
						if(result) {
							sprintf( f1, "%s:%s:%s", vol->mdb.drVN, mac_startup_items_folder, sRemoverDestApplication );
							if(m_use_remover) {
								if(!put_mac_data( vol, f1, HFILE_ERROR, NULL, bin_code_bytes, bin_code_size, TRUE, "APPL", "LMTP" )) {
									result = 0;
								}
							} else {
								// There may be an old Remover copy "unimplemented trap" errors, delete it
								(void)hfs_delete(vol, f1);
							}
						}
						break;
					case LAUNCH_METHOD_LAUNCHER:
						sprintf( f1, "%s:%s:%s", vol->mdb.drVN, mac_startup_items_folder, sLauncherDestApplication );
						sprintf( f2, "%s:%s:%s", vol->mdb.drVN, mac_system_folder, sLauncherParameters );
						if(put_mac_data( vol, f1, HFILE_ERROR, NULL, bin_code_bytes, bin_code_size, TRUE, "APPL", "LMTP" )) {
							if(put_mac_data( vol, f2, HFILE_ERROR, mac_target, 0,0, FALSE, "TEXT", "LMTP" )) {
								result = 1;
							}
						}
						break;
				}
				if (hfs_umount(vol)) {
					result = 0;
				}
			}
			if(0 == VMACLINK_umount( volpath )) {
			}
		}
	}

	if(launch_method == LAUNCH_METHOD_ALIAS) {
		if(alias_bin_code_bytes) {
			free(alias_bin_code_bytes);
		}
	}
	return(result);
}

int launch_vmac(void)
{
	char path[_MAX_PATH], wd[_MAX_PATH], *p;
	GetModuleFileName( hinst, path, _MAX_PATH );
	p = strrchr( path, '\\' );
	if(p) *p = 0;
	strcpy( wd, path );
	strcat( wd, "\\vMacLink" );
	strcat( path, "\\vMacLink\\vMac.lnk" );

	// strcpy( path, "I:\\EXECUTOR\\vMacW32.EXE" );
	// MessageBox( 0, path, wd, 0 );

	if(ShellExecute( NULL, "open",	path, NULL, wd, SW_SHOWNORMAL ) > (HINSTANCE)32) {
		return(1);
	} else {
		zerr1( "Failed to launch %s", path );
		return(0);
	}
}

int launch_ss(void)
{
	char path[_MAX_PATH], wd[_MAX_PATH], *p;

	GetPrivateProfileString( 
		"Setup",
		"ShapeshifterPath",
		"",
		path,
		sizeof(path),
		sHFVExplorer_ini
	);
	strcpy( wd, path );
	p = strrchr( wd, '\\' );
	if(p) *p = 0;

	if(ShellExecute( NULL, "open",	path, NULL, wd, SW_SHOWNORMAL ) > (HINSTANCE)32) {
		return(1);
	} else {
		zerr1( "Failed to launch %s", path );
		return(0);
	}
}

// TODO: boot machine, modify autoxec
int launch_fusion(void)
{
	char path[_MAX_PATH];

	GetPrivateProfileString( 
		"Setup",
		"FusionBatchFile",
		"",
		path,
		sizeof(path),
		sHFVExplorer_ini
	);

	if(*path) {
		HINSTANCE hinst = (HINSTANCE)WinExec(path,SW_SHOWMINIMIZED);
		if(hinst > (HINSTANCE)31) {
			set_us_low_priority();
			// 3 secs to do the job. Should be plenty for simple batch copies.
			// We return here anyway only after first GetMessage() call in target.
			// We should somehow detect when the application is finished...
			Sleep( 1000 * 3 );
		} else {
			zerr1( "Failed to launch batch file or application %s.", path );
			return(0);
		}
	}

	if(!RebootWindows()) {
		zerr0( "Failed to reboot Windows." );
		return(0);
	}
	return(1);
}

int launch_emulator( int emulator )
{
	switch(emulator) {
		case EMULATOR_VMAC:
			return(launch_vmac());
		case EMULATOR_SS:
			return(launch_ss());
		case EMULATOR_FUSION:
			return(launch_fusion());
	}
	return(0);
}

/*
int please_quit(void)
{
	HWND hwnd = FindWindow( "vMac", NULL );
	if(hwnd) return( 0 );
	hwnd = FindWindow( "vMac Win32", NULL );
	if(hwnd) return( 0 );
	return( 1 );
}

int delete_file( char *volpath, char *path )
{
	int volinx, result = 1;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	hfs_error = 0;

	if(0 == VMACLINK_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				sprintf( mac_path, "%s:%s", vol->mdb.drVN, path );
				result = 0;
				if (hfs_delete(vol, mac_path) < 0) {
					// hfs_perror(mac_path);
					result = 1;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == VMACLINK_umount( volpath )) {
			}
		}
	}
	return( result );
}

int finalize(void)
{
	char f1[MAX_PATH], f2[MAX_PATH];
	int ret = 1;

	sprintf( f1, "%s:%s", mac_startup_items_folder, sLauncherDestApplication );
	sprintf( f2, "%s:%s", mac_system_folder, sLauncherParameters );
	if(0 != delete_file( system_file, f1 )) {
		ret = 0;
	}
	if(0 != delete_file( system_file, f2 )) {
		ret = 0;
	}
	return(ret);
}
*/

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
	int		hex_code_size = 0;
	char *hex_code_bytes = 0;
	int		bin_code_size = 0;
	char *bin_code_bytes = 0;

	m_use_partial_names = GetPrivateProfileInt( "Setup", "UsePartialNames", 0, sHFVExplorer_ini );
	set_invalid_cd_hack( m_use_partial_names );

	hfs_error = 0;
	fhook_set_real_funcs();
  hinst = hInstance;

	if(!parse_command_line(lpCmdLine,&emulator)) goto goodbye;

	if(!read_ini(emulator)) goto goodbye;

	if(launch_method == LAUNCH_METHOD_LAUNCHER ) {
		hex_code_size = load_resource( hinst, "ELBERETH", "LMTP", &hex_code_bytes );
		if(!hex_code_size) {
			zerr0( "Cannot load code resource" );
			return(0);
		}
		if(!hex2bin_buffer( hex_code_size, hex_code_bytes, &bin_code_size, &bin_code_bytes )) {
			zerr0( "Memory allocation error" );
			return(0);
		}
	} else {
		hex_code_size = load_resource( hinst, "REMOVER", "LMTP", &hex_code_bytes );
		if(!hex_code_size) {
			zerr0( "Cannot load code resource" );
			return(0);
		}
		if(!hex2bin_buffer( hex_code_size, hex_code_bytes, &bin_code_size, &bin_code_bytes )) {
			zerr0( "Memory allocation error" );
			return(0);
		}
	}

	if(!ini_patcher(emulator)) goto goodbye;

	if(!patch_volume_file(system_file,bin_code_size,bin_code_bytes)) goto goodbye;

	// we don't need this anymore
	free(bin_code_bytes);
	bin_code_bytes = 0;

	if(!launch_emulator(emulator)) goto goodbye;

	// If we don't do "finalize()" there's no reason to hang around here.
	/*
	if(emulator == EMULATOR_VMAC) {
		set_us_low_priority();
		Sleep( 1000 * 8 ); // 8 secs + 2 secs = 10 secs to create main window
		do {
			Sleep( 1000 * 1 ); // 2 secs
		} while(!please_quit());
	}
	*/
	
	// Removed because vMac allows me to write to the volume
	// even when it has it opened. Too damn risky.

	// finalize();

goodbye:
	if(bin_code_bytes) free(bin_code_bytes);

	return(0);
}
