/*
 * HFVExplorer
 * Copyright (C) 1997-1998 by Anygraaf Oy
 * Author: Lauri Pesonen, email: lpesonen@clinet.fi or lauri.pesonen@anygraaf.fi
 * This is the HFVExplorer interface to the hfsutils package.
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ERROR
#endif

# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <string.h>

# include "libhfs\hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "suid.h"
# include "hmount.h"
# include "humount.h"
# include "hcopy.h"
# include "copyhfs.h"
#include "..\charset.h"
#include "..\timer.h"
#include "..\floppy.h"
#include "..\cache.h"

#include "..\MacTypes.h"
#include "..\itemtype.h"

#include "interface.h"
#include "copyin.h"
#include "copyout.h"
#include "..\tmap.h"

int set_file_size( char *path, DWORD new_size );

static char HFSIFACE_copyout_mode = 'a';
static char HFSIFACE_copyin_mode = 'a';
static char HFSIFACE_copyout_ad = 1;

/*
static char *dir_hack( char *path )
{
#ifdef DIRHACK_ENABLED
	p = strchr( path, ':' );
	if(p) {
		return(p);
	} else {
		return(path);
	}
#else
	return(path);
#endif
}
*/

int hfs_readdir2(
	hfsdir *dir, 
	hfsdirent *ent,
  CatKeyRec *pkey,
  CatDataRec *pdata
);

static int HFSIFACE_m_silent = 0;
static HWND error_hwnd = 0;
char *argv0 = "HFVExplorer";

int crlf_translation = 1;

int mount_flags( char *path )
{
	char buffer[10];
	int flags;

	wsprintf( buffer, "%c:\\", *path );
	if(GetDriveType(buffer) == DRIVE_CDROM) {
		flags = 0;
	} else {
		flags = O_RDWR;
	}
	return(flags);
}

void make_companion_name( char *target, char *source, int fork )
{
	char *p1, *p2;

	strcpy( target, source );
	p1 = strrchr( target, '\\' );
	if(!p1) return;
	p1++;

	p2 = strrchr( source, '\\' );
	if(!p2) return;
	p2++;
	if(*p2 == '%') p2++;

	if(fork) *p1++ = '%';
	strcpy( p1, p2 );
}

void hfs_pinfo(hfsvolent *ent)
{
	char buf[500];
	char line[200];

	return;

	*buf = 0;
  sprintf(line,"Volume name is \"%s\"%s\n", ent->name, (ent->flags & HFS_ISLOCKED) ? " (locked)" : "");
	strcat( buf, line );
  sprintf(line,"Volume was created on %s", ctime(&ent->crdate));
	strcat( buf, line );
  sprintf(line,"Volume was last modified on %s", ctime(&ent->mddate));
	strcat( buf, line );
  sprintf(line,"Volume has %lu bytes free\n", ent->freebytes);
	strcat( buf, line );
	MessageBox( error_hwnd, buf, "HFS", 0 );
}

int myDeleteFile(char *path)
{
	char buf[500];
	strcpy( buf, "Now deleting file " );
	strcat( buf, path );
	MessageBox( error_hwnd, buf, "HFS", 0 );
	return(DeleteFileA(path));
}

// #define DeleteFile myDeleteFile

void hfs_perrorp(char *path)
{
	hfs_perror(path);
}

void hfs_perror(char *msg)
{
	char line[200];
  char *str;

	if(HFSIFACE_m_silent) return;

  str = strerror(errno);

  if (hfs_error == 0) {
    sprintf(line, "%s: %s: %c%s\n", argv0, msg, tolower(*str), str + 1);
  } else {
		if (errno)
			sprintf(line, "%s: %s: %s (%s)\n", argv0, msg, hfs_error, str);
    else
			sprintf(line, "%s: %s: %s\n", argv0, msg, hfs_error);
  }
	mac_to_pc_charset(line);
	MessageBox( error_hwnd, line, "Error", MB_OK | MB_ICONSTOP );
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

int HFSIFACE_mount(char *path)
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
  hfs_pinfo(&ent);

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

int HFSIFACE_umount(char *path)
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

void hfs_tester(void)
{
	char vol[200];

	lstrcpy( vol, "I:\\EXECUTOR\\test.hfv" );

	HFSIFACE_init(0);

	/*
	if(0 == HFSIFACE_copy_HFS_HFS( 
		vol, 
		"Games #3:Colony:CData:map.1",
		vol ,
		"Games #3:Colony:CData:Uusi koe"
	)) 
	{
		hfs_perror("HFSIFACE_copy_HFS_HFS failed");
	}
	*/
	if(0 != HFSIFACE_delete( vol, ":Colony:CData:map.1" )) 
	{
	}
}

void HFSIFACE_internal_init( void )
{
	hfs_error = 0;
  // curvol = -1; //static in hcwd, initialized already.
}

void HFSIFACE_init( HWND hwnd )
{
	// _fmode = _O_BINARY;
	if(hwnd) error_hwnd = hwnd;
	HFSIFACE_internal_init();
	// We don't call hcwd_init(). There is no "current volume"
	// nor "current directory".
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

int HFSIFACE_delete( char *volpath, char *path )
{
	int volinx, result = 0;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_path, path );
	pc_to_mac_charset(mac_path);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if (hfs_delete(vol, mac_path) < 0) {
					hfs_perror(mac_path);
					result = 1;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return( result );
}

int HFSIFACE_mkdir( char *volpath, char *path )
{
	int volinx, result = 0;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_path, path );
	pc_to_mac_charset(mac_path);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if (hfs_mkdir(vol, mac_path) < 0) {
					hfs_perror(mac_path);
					result = 1;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return( result );
}

int do_delete_directory( hfsvol *vol, char *path )
{
  hfsdir *dir;
	hfsdirent ent;
	char new_source[_MAX_PATH];

	dir = hfs_opendir( vol, path );
	if(dir == 0) {
		hfs_perror(path);
		return(-1);
	} else {
		while (hfs_readdir(dir, &ent) >= 0) {
			sprintf( new_source, "%s:%s", path, ent.name );
			if(ent.flags & HFS_ISDIR) {
				if(do_delete_directory( vol, new_source ) != 0) {
					break;
				}
			} else {
				if (hfs_delete(vol, new_source) < 0) {
					hfs_perror(new_source);
					break;
				}
			}

			// NEW: since we deleted from the volume, the hfsutils iteration
			// structures are not in sync anymore. Must close and reopen.
			hfs_closedir(dir);
			dir = hfs_opendir( vol, path );
			if(dir == 0) {
				hfs_perror(path);
				return(-1);
			}

		}
		hfs_closedir(dir);
	}
	return(hfs_rmdir(vol, path));
}

int HFSIFACE_rmdir( char *volpath, char *path )
{
	int volinx, result = 0;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_path, path );
	pc_to_mac_charset(mac_path);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if(do_delete_directory( vol, mac_path ) != 0) {
					result = 1;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return( result );
}

int HFSIFACE_rename( char *volpath, char *oldname, char *newname )
{
	int volinx, result = 0;
  hfsvol *vol;
	char mac_oldname[_MAX_PATH];
	char mac_newname[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_oldname, oldname );
	strcpy( mac_newname, newname );
	pc_to_mac_charset(mac_oldname);
	pc_to_mac_charset(mac_newname);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if (hfs_rename(vol, mac_oldname, mac_newname) < 0) {
					hfs_perror(mac_oldname);
					result = 1;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return( result );
}

int fat_copy( char *source, char *target )
{
	char ttarget[_MAX_PATH];
	DWORD attr;
	int len;
	char *p;

	attr = GetFileAttributes( target );
	if(attr == 0xFFFFFFFF || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		return(1);
	} else {
		strcpy( ttarget, target );
		len = strlen( ttarget );
		if( len > 0 && ttarget[len-1] != '\\' ) {
			strcat( ttarget, "\\" );
		}
		p = strrchr( source, '\\' );
		if(p) {
			strcat( ttarget, p+1 );
		} else {
			return(1);
		}
		// should check existence here
		return( CopyFile( source, ttarget, FALSE ) == 0 );
	}
}

// easy since we know that this is full path
static int is_hfs_volume( char *s )
{
	int len;

	len = strlen(s);
	if(len < 4) return(0);

	if(s[len-4] == '.') {
		if(toupper(s[len-3]) == 'H' &&
		   toupper(s[len-2]) == 'F' /* &&
		   toupper(s[len-1]) == 'V' */ ) return(1);
		if(toupper(s[len-3]) == 'D' &&
		   toupper(s[len-2]) == 'S' &&
		   toupper(s[len-1]) == 'K') return(1);
	}
	return(0);
}

int build_target_dir_name( 
	int is_source_hfs, 
	int is_target_hfs, 
	char *tdir, 
	char *source, 
	char *target )
{
	int len;
	char slet;
	char tlet[2];
	char *p, *start_convert;

	slet = is_source_hfs ? ':' : '\\';

	tlet[0] = is_target_hfs ? ':' : '\\';
	tlet[1] = 0;

	strcpy( tdir, target );
	len = strlen( target );
	start_convert = tdir + len;
	if( len > 0 && target[len-1] != *tlet ) {
		strcat( tdir, tlet );
		start_convert++;
	}
	p = strrchr( source, slet );
	if(!p) return(1);
	strcat( tdir, p+1 );

	if(is_source_hfs && !is_target_hfs) {
		int i, nlen = strlen(tdir);
		if(strcmp(start_convert,"Icon\x00D") == 0) {
			start_convert[4] = '~';
		} else {
			mac_to_pc_charset( (unsigned char *)start_convert );
			for (i = 0; i<nlen; i++) {
				switch (tdir[i]) {
					case '/':
					case '*':
					case '?':
					case '\"':
					case '<':
					case '>':
					case '|':
						tdir[i] = '-';
						break;
				}
			}
		}
	} else if(!is_source_hfs && is_target_hfs) {
		int nlen = strlen(tdir);
		if(strcmp(start_convert,"Icon~") == 0) {
			start_convert[4] = '\x00D';
		} else {
			pc_to_mac_charset( (unsigned char *)start_convert );
		}
	}
	return(0);
}

static void prepare_for_copyin_raw( char *source, char *target )
{
	int use_default = 1;
	char extension[100], type[100], creator[100], *p;
	int strip;

	p = strrchr( source, '.' );
	if(p && (strlen(p) < 30)) {
		strcpy( extension, p+1 );
		if( tmap_fat2hfs( extension, type, creator, &strip ) ) {
			use_default = 0;
		}
	}
	if(use_default) {
		copyin_set_raw_param(0,0,0);
	} else {
		copyin_set_raw_param(type,creator,strip);
	}
}

// no dot in extension
int has_extension( char *target, char *extension )
{
	int tlen, ext_len, retval = 0;

	ext_len = strlen(extension);
	tlen = strlen(target);
	// +1 to avoid ".txt"
	if(tlen > ext_len+1 && target[tlen-ext_len-1] == '.') {
		if(stricmp(&target[tlen-ext_len],extension) == 0) {
			retval = 1;
		}
	}
	return(retval);
}

static void modify_extension( char *target, char *extension )
{
	char *old_extension;

	if(*extension == '.') extension++; // known state now

	old_extension = strrchr( target, '.' );
	if(!old_extension || strlen(old_extension) > 6) {

	// if(!has_extension(target,extension)) {
		char *p = strrchr( target, '.' );
		if(p) {
			p++;
			strcpy( p, extension );
		} else {
			strcat( target, "." );
			strcat( target, extension );
		}
	}
}

static void prepare_for_copyout_raw( hfsvol *vol, char *source, char *target, char *extension )
{
	hfsdirent ent;
	
	if (hfs_stat(vol, source, &ent) >= 0) {
		if( (ent.flags & HFS_ISDIR) == 0 ) {
			ent.u.file.creator[4] = 0;
			ent.u.file.type[4] = 0;
			if( tmap_hfs2fat( ent.u.file.type, ent.u.file.creator, extension ) ) {
				// ok signaled by *extension != 0
			}
		}
	}
}

int HFSIFACE_copy_one_file( 
	int is_source_hfs,
	int is_target_hfs,
	hfsvol *vol1, 
	hfsvol *vol2,
	char *source, 
	char *target,
	int move_if_possible, 
	int *pwas_moved
)
{
	int result = 0;
  hfsvol *tvol = 0;
	char new_source[_MAX_PATH];
	char tmp[_MAX_PATH];
	char tmp2[_MAX_PATH];
	int fork;

	if(vol2 == 0) tvol = vol1; else tvol = vol2;

	if(is_source_hfs && is_target_hfs && vol1 == tvol && move_if_possible) {
		if(0 == build_target_dir_name( 
			is_source_hfs, is_target_hfs,
			new_source, source, target ))
		{
			if (hfs_rename(vol1, source, new_source) >= 0) {
				*pwas_moved = 1;
				return(0);
			}
		}
	}

	if(is_source_hfs) {
		if(is_target_hfs) {
			// HFS -> HFS
			result = do_copyhfshfs( vol1, tvol, source, target );
			if(result != 0) {
				if(0 == build_target_dir_name( 
					1, 1, new_source, source, target ))
				{
					hfs_delete(tvol, new_source);
				}
			} else if(move_if_possible) {
				if(hfs_delete(vol1, source) == 0) *pwas_moved = 1;
			}
		} else {
			char extension[100];
			// HFS -> FAT
			// if AppleDouble ? Should we split?

			// careful not to change the parameter -- necessary... ?
			strcpy( tmp, target );
			*extension = 0;
			if(HFSIFACE_copyout_mode == 'r' || HFSIFACE_copyout_mode == 'R') {
				//ZAP
				prepare_for_copyout_raw( vol1, source, tmp, extension );
			}

			result = do_copyout( vol1, 1, &source, tmp, HFSIFACE_copyout_mode );
			if(result != 0) {
				if(0 == build_target_dir_name( 1, 0, new_source, source, tmp )) {
					DeleteFile(new_source);
				}
			} else {
				if(*extension && 0 == build_target_dir_name( 1, 0, new_source, source, tmp )) {
					strcpy( tmp2, new_source );
					modify_extension( tmp2, extension );
					if(stricmp(tmp2,new_source) != 0) {
						DeleteFile( tmp2 );
						if(MoveFile( new_source, tmp2 )) {
							copyout_set_last_open_file(tmp2);
						}
					}
				}
				if(move_if_possible) {
					if(hfs_delete(vol1, source) == 0) *pwas_moved = 1;
				}
			}
		}
	} else {
		if(is_target_hfs) {
			// FAT -> HFS
			// if AppleDouble ?
			char mode = HFSIFACE_copyin_mode;
			make_companion_name( tmp, source, 0 );
			make_companion_name( tmp2, source, 1 );
			if(HFSIFACE_copyout_ad) {
				if(exists(tmp) && exists(tmp2)) mode = '&';
			}
			if(mode == '&')
				result = do_copyindouble( vol2, tmp, tmp2, target );
			else {
				strcpy( tmp, target );
				if(mode == 'r' || mode == 'R') {
					prepare_for_copyin_raw( source, tmp );
				}
				result = do_copyin( vol2, 1, &source, tmp, mode );
			}
			if(result != 0) {
				// new_source REUSED!
				if(0 == build_target_dir_name( 0, 1, new_source, source, tmp ))
				{
					hfs_delete(vol2, new_source);
				}
			} else if(move_if_possible) {
				if(mode == '&') {
					for(fork=0; fork<2; fork++) {
						make_companion_name( tmp, source, fork );
						if(DeleteFile(tmp)) *pwas_moved = 1;
					}
				} else {
					if(DeleteFile(source)) *pwas_moved = 1;
				}
			}
		} else {
			// FAT -> FAT
			for(fork=0; fork<2 && result==0; fork++) {
				make_companion_name( tmp, source, fork );
				if(exists(tmp)) {
					result = fat_copy( tmp, target );
					if(result != 0) {
						if(0 == build_target_dir_name( 
							0, 0, new_source, tmp, target ))
						{
							DeleteFile(new_source);
						}
					} else if(move_if_possible) {
						if(DeleteFile(tmp)) *pwas_moved = 1;
					}
				}
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return(result);
}

int HFSIFACE_dir_copy( 
	int is_source_hfs,
	int is_target_hfs,
	hfsvol *vol1, 
	hfsvol *vol2,
	char *source, 
	char *target,
	int move_if_possible,
	int *pwas_moved
)
{
	int isdir, result = 0;
	char tdir[_MAX_PATH];
	char new_source[_MAX_PATH];
	char tmp[_MAX_PATH];
	char tmp2[_MAX_PATH];

	result = build_target_dir_name( 
		is_source_hfs, is_target_hfs, tdir, source, target );

	if(result != 0) {
		hfs_perror(source);
		return(result);
	}

	if(is_target_hfs) {
		if (hfs_mkdir(vol2, tdir) < 0) {
			hfs_perror(tdir);
			result = 1;
		}
	} else {
		if(!CreateDirectory(tdir,NULL)) {
			if(!HFSIFACE_m_silent) {
				if(exists(tdir)) {
					MessageBox( error_hwnd, "The directory already exists.", tdir, MB_OK | MB_ICONSTOP );
				} else {
					MessageBox( error_hwnd, "Could not create the directory.", tdir, MB_OK | MB_ICONSTOP );
				}
			}
			result = 1;
		}
	}
	if(result == 0) {
		if(is_source_hfs) {
      hfsdir *dir;
			hfsdirent ent;
			dir = hfs_opendir( vol1, source );
			if(dir == 0) {
				hfs_perrorp(source);
			} else {
				while (hfs_readdir(dir, &ent) >= 0) {
					UPDATE_TIME_CONSUMING(0);
					if(ent.flags & HFS_ISDIR) {
						sprintf( new_source, "%s:%s", source, ent.name );
						result = HFSIFACE_dir_copy( 
							is_source_hfs, is_target_hfs, vol1, vol2,
							new_source, tdir,
							move_if_possible, pwas_moved);
						if(result != 0) break;
					} else {
						sprintf( new_source, "%s:%s", source, ent.name );
						result = HFSIFACE_copy_one_file( 
								is_source_hfs, is_target_hfs, vol1, vol2,
								new_source, tdir,
								move_if_possible, pwas_moved);
						if(result != 0) break;
					}
				}
				hfs_closedir(dir);
			}
		} else {
			HANDLE fh;
			WIN32_FIND_DATA FindFileData;
			int ok;
			char mask[_MAX_PATH];

			sprintf( mask, "%s\\*.*", source );
			fh = FindFirstFile( mask, &FindFileData );
			ok = fh != INVALID_HANDLE_VALUE;
			while(ok) {
				UPDATE_TIME_CONSUMING(0);

				sprintf( new_source, "%s\\%s", source, FindFileData.cFileName );

				if(is_target_hfs && HFSIFACE_copyout_ad && *FindFileData.cFileName != '.') {
					make_companion_name( tmp, new_source, 0 );
					make_companion_name( tmp2, new_source, 1 );
					if(exists(tmp) && exists(tmp2) && *FindFileData.cFileName == '%') {
						goto go_get_next; // oh, dirty
					}
				}
			
				isdir = (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
				if(isdir) {
					if(*FindFileData.cFileName != '.') {
						result = HFSIFACE_dir_copy( 
							is_source_hfs, is_target_hfs, vol1, vol2,
							new_source, tdir,
							move_if_possible, pwas_moved);
						if(result != 0) break;
					}
				} else {
					result = HFSIFACE_copy_one_file( 
							is_source_hfs, is_target_hfs, vol1, vol2,
							new_source, tdir,
							move_if_possible, pwas_moved);
					if(result != 0) break;
				}
go_get_next:
				ok = FindNextFile( fh, &FindFileData );
			}
			if(fh != INVALID_HANDLE_VALUE) FindClose( fh );
		}
	}

	if(result == 0 && move_if_possible != 0) {
		// Oh. What a bug.
		// if(is_target_hfs) {
		if(is_source_hfs) {
			if (hfs_rmdir(vol1, source) >= 0) {
				*pwas_moved = 1;
			}
		} else {
			if(RemoveDirectory(source)) {
				*pwas_moved = 1;
			}
		}
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return(result);
}

int HFSIFACE_copy( 
	char *sourcevol, 
	char *source, 
	char *targetvol, 
	char *target,
	int is_dir_copy,
	int move_if_possible,
	int *pwas_moved )
{
	int volinx1 = -1, volinx2 = -1, result = 0;
	int mounted1 = 0, mounted2 = 0;
  hfsvol *vol1 = 0;
  hfsvol *vol2 = 0;
	int is_source_hfs, is_target_hfs;
	
	HFSIFACE_internal_init();

	*pwas_moved = 0;

	is_source_hfs = is_hfs_volume( sourcevol );
	is_target_hfs = is_hfs_volume( targetvol );

	if(is_source_hfs) {
		pc_to_mac_charset(source); //XXX
		if(strchr(source,':') == 0) strcat( source, ":" );
	}

	if(is_target_hfs) {
		pc_to_mac_charset(target); //XXX
		if(strchr(target,':') == 0) strcat( target, ":" );
	}

	if(is_source_hfs) {
		result = 1;
		if(0 == HFSIFACE_mount( sourcevol )) {
			mounted1 = 1;
			volinx1 = path2volinx( sourcevol );
			if(volinx1 >= 0) {
				int flags;
				if(is_target_hfs && strcmp(sourcevol,targetvol) == 0) {
					flags = mount_flags(sourcevol);
				} else {
					flags = 0;
				}
				vol1 = hfs_remount(hcwd_getvol(volinx1), flags);
				if (vol1 != 0) {
					result = 0;
				}
			}
		}
	}

	if(result == 0) {
		if(is_target_hfs && strcmp(sourcevol,targetvol) != 0) {
			result = 1;
			if(0 == HFSIFACE_mount( targetvol )) {
				mounted2 = 1;
				volinx2 = path2volinx( targetvol );
				if(volinx2 >= 0) {
					vol2 = hfs_remount(hcwd_getvol(volinx2), mount_flags(targetvol));
					if (vol2 != 0) {
						result = 0;
					}
				}
			}
		}
	}

	if(result == 0) {

		// Try to move only inside same HFS volume.
		if(is_target_hfs) {
			if(strcmp(sourcevol,targetvol) != 0) {
				// move_if_possible = 0;
			}
		} else {
			// move_if_possible = 0;
		}

		if(is_dir_copy) {
			START_TIME_CONSUMING(1000);
			result = HFSIFACE_dir_copy( 
				is_source_hfs,
				is_target_hfs,
				vol1, vol2,
				source, target,
				move_if_possible,
				pwas_moved );
			END_TIME_CONSUMING;
		} else {
			// single file copy
			result = HFSIFACE_copy_one_file( 
					is_source_hfs, 
					is_target_hfs, 
					vol1, vol2, source, target,
					move_if_possible,
					pwas_moved );
		}
	}

	if(mounted1) {
		if(vol1 >= 0) {
			if (hfs_umount(vol1) < 0) {
				hfs_perror("Error closing HFS volume");
				result = 1;
			}
		}
		if(0 == HFSIFACE_umount( sourcevol )) {
		}
	}
	if(mounted2) {
		if(vol2 >= 0) {
			if (hfs_umount(vol2) < 0) {
				hfs_perror("Error closing HFS volume");
				result = 1;
			}
		}
		if(0 == HFSIFACE_umount( targetvol )) {
		}
	}

	if(is_source_hfs) {
		mac_to_pc_charset(source);
	}
	if(is_target_hfs) {
		mac_to_pc_charset(target);
	}

	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}

	return( result );
}

// Not used since 1version 1.2.4, too slow
int fill_with_nulls( HANDLE hf, long size )
{
	char buf[8192];
	int numbytes, bytes_written;

	memset( buf, 0, sizeof(buf) );
	while(size > 0) {
		UPDATE_TIME_CONSUMING(0);
		numbytes = sizeof(buf);
		if(numbytes > size) numbytes = size;
		if(!WriteFile( hf, buf, numbytes, &bytes_written, NULL ) || numbytes != bytes_written) {
			return(0);
		}
		size -= bytes_written;
	}
	return(1);
}

int init_format( char *volpath, long size )
{
	HANDLE hf;
	int result = 0;

	if(exists(volpath)) {
		(void)DeleteFile( volpath );
	}
	if(!exists(volpath)) {
		hf = CreateFile( 
			volpath,
			GENERIC_READ | GENERIC_WRITE,
			0, // not shared now
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL );
		if(hf != INVALID_HANDLE_VALUE) {
			/*
			if(fill_with_nulls( hf, size )) {
				result = 1;
			}
			*/
			CloseHandle( hf );
			result = set_file_size( volpath, size );

			if(result == 0 && exists(volpath)) {
				hfs_error = "No room.";
				(void)DeleteFile( volpath );
			}
		}
	}
	return(result);
}

unsigned char desktopData[384] = {
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x32, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0B, 0x0A, 0x46, 0x69, 0x6E, 0x64, 0x65, 0x72, 0x20, 0x31, 0x2E, 0x30, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x32, 0x00, 0x00, 0x53, 0x54, 0x52, 
	0x20, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0xFF, 0xFF, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	} ;

static char *desktopname = "Desktop";
void make_desktop( char *volpath )
{
	int volinx, result = 0;
  hfsvol *vol;
  hfsfile *file;
  hfsdirent ent;
	
	HFSIFACE_internal_init();

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
			  hfs_delete(vol,desktopname); // obsolete
			  if (hfs_create( vol, desktopname, "FNDR", "ERIK" ) >= 0) {
					file = hfs_open(vol, desktopname);
					if(file != 0) {
						if (hfs_setfork(file, 1) < 0) {}
						if (hfs_write(file, desktopData, sizeof(desktopData)) < 0) {
							result = -1;
						} else {
							if (hfs_fstat(file, &ent) < 0) {
								result = -1;
							}
							ent.fdflags = HFS_FNDR_NAMELOCKED	| HFS_FNDR_ISINVISIBLE;
							// ent.crdate = 0;
							// ent.mddate = 0;
							// ent.bkdate = 0;
							if (result == 0 && hfs_fsetattr(file, &ent) < 0) {
								result = -1;
							}
						}
						hfs_close(file);
						if(result != 0) hfs_delete(vol,desktopname);
					}
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}
	if(result != 0) {
		hfs_perror("Could not create desktop");
	}
}

int HFSIFACE_format( 
	char *volpath, 
	char *vname, 
	long size,
	BOOL initialize
)
{
	int result = 0;
	int partno = 0;
	char mac_vname[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_vname, vname );
	pc_to_mac_charset(mac_vname);

	START_TIME_CONSUMING(100);
	if(!initialize || init_format(volpath,size)) {
		SetCursor( LoadCursor(NULL,IDC_WAIT) );
		if (hfs_format(volpath, partno, mac_vname) < 0) {
			END_TIME_CONSUMING;
		  hfs_perror(volpath);
		} else {
			// make_desktop(volpath);
			END_TIME_CONSUMING;
		}
		SetCursor( LoadCursor(NULL,IDC_ARROW) );
	} else {
		END_TIME_CONSUMING;
		hfs_perror("Could not format volume");
	}
	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}
	return( result );
}

int HFSIFACE_silencer( int shutup )
{
	int prev_shutup = HFSIFACE_m_silent;
	HFSIFACE_m_silent = shutup;
	return(prev_shutup);
}

void HFSIFACE_set_modeout( int outmode )
{
	switch(outmode) {
		case 0:
			HFSIFACE_copyout_mode = 'a';
			break;
		case 1:
			HFSIFACE_copyout_mode = 'm';
			break;
		case 2:
			HFSIFACE_copyout_mode = 'b';
			break;
		case 3:
			HFSIFACE_copyout_mode = 't';
			break;
		case 4:
			HFSIFACE_copyout_mode = 'r';
			break;
		case 5:
			HFSIFACE_copyout_mode = 'R';
			break;
		case 6:
			HFSIFACE_copyout_mode = 'x';
			break;
		default:
			HFSIFACE_copyout_mode = 'a';
			break;
	}
}

void HFSIFACE_set_modein( int inmode, int appledouble )
{
	switch(inmode) {
		case 0:
			HFSIFACE_copyin_mode = 'a';
			break;
		case 1:
			HFSIFACE_copyin_mode = 'm';
			break;
		case 2:
			HFSIFACE_copyin_mode = 'b';
			break;
		case 3:
			HFSIFACE_copyin_mode = 't';
			break;
		case 4:
			HFSIFACE_copyin_mode = 'r';
			break;
		case 5:
			HFSIFACE_copyin_mode = 'R';
			break;
		case 6:
			HFSIFACE_copyin_mode = 'x';
			break;
		default:
			HFSIFACE_copyin_mode = 'a';
			break;
	}
	HFSIFACE_copyout_ad = appledouble;
}

void HFSIFACE_set_copy_modes( 
	int outmode, int inmode, int appledouble
)
{
	HFSIFACE_set_modeout( outmode );
	HFSIFACE_set_modein( inmode, appledouble );
}

int HFSIFACE_get_properties( char *volpath, char *path, hfsdirent *pent )
{
	int volinx, result = 1;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_path, path );
	pc_to_mac_charset(mac_path);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if (hfs_stat(vol, mac_path, pent) >= 0) {
					result = 0;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}
	return( result );
}

int HFSIFACE_set_properties( char *volpath, char *path, hfsdirent *pent )
{
	int volinx, result = 1;
  hfsvol *vol;
	char mac_path[_MAX_PATH];
	
	HFSIFACE_internal_init();
	strcpy( mac_path, path );
	pc_to_mac_charset(mac_path);

	if(0 == HFSIFACE_mount( volpath )) {
		volinx = path2volinx(volpath);
		if(volinx	>= 0) {
			vol = hfs_remount(hcwd_getvol(volinx), mount_flags(volpath));
			if (vol != 0) {
				if (hfs_setattr(vol,mac_path, pent) >= 0) {
					result = 0;
				}
				if (hfs_umount(vol) < 0 && result == 0) {
					hfs_perror("Error closing HFS volume");
					result = 1;
				}
			}
			if(0 == HFSIFACE_umount( volpath )) {
			}
		}
	}
	if(!writeback_flush_all()) {
		hfs_perror("Error writing to HFS volume");
		result = 1;
	}
	return( result );
}

// Speedup. This family is NOT reentrant!!!!
static hfsvol *ff_vol = 0;
static int ff_mounted = 0;
static int ff_mounted2 = 0;
static char ff_path[_MAX_PATH];
static char ff_mac_path[_MAX_PATH];
static hfsdir *ff_dir = 0;

static int pIcon13catdatarecValid = 0;
static CatDataRec Icon13catdatarec;

void ff_cleanup( void )
{
	if(ff_dir) {
		hfs_closedir(ff_dir);
		ff_dir = 0;
	}
	if(ff_mounted2) {
		if (hfs_umount(ff_vol) < 0) {
			// hfs_perror("Error closing HFS volume");
		}
		ff_mounted2 = 0;
	}
	if(ff_mounted) {
		if(0 == HFSIFACE_umount( ff_path )) {
		}
		ff_mounted = 0;
	}
}

CatDataRec *HFSIFACE_get_last_Icon13( void )
{
	if(pIcon13catdatarecValid) {
		return(&Icon13catdatarec);
	} else {
		return(0);
	}
}

static void check_for_icon13( hfs_item_type *pitem )
{
	char sub_path[_MAX_PATH];
	hfsdir *sub_dir;
	hfsdirent ent;
	CatKeyRec catkeyrec;

	sprintf( sub_path, "%s%s:", ff_mac_path, pitem->catkeyrec.ckrCName );
	sub_dir = hfs_opendir( ff_vol, sub_path );
	if(sub_dir) {
		while(hfs_readdir2(sub_dir, &ent, &catkeyrec, &Icon13catdatarec) >= 0) {
			if(strcmp(catkeyrec.ckrCName,"Icon\x00D") == 0) {
				pIcon13catdatarecValid = 1;
				break;
			}
		}
		hfs_closedir(sub_dir);
	}
}

int HFSIFACE_get_dir_next( hfs_item_type *pitem )
{
	int result = 1, looping = 1;
	hfsdirent ent;
	
	HFSIFACE_internal_init();

	pIcon13catdatarecValid = 0;

	// Loop to skip deleted items
	while(looping) {
		looping = 0;
		if(hfs_readdir2(ff_dir, &ent, &pitem->catkeyrec, &pitem->catdatarec) >= 0) {
			if(pitem->catkeyrec.ckrKeyLen == 0) {
				looping = 1;
			} else if(pitem->catdatarec.cdrType == cdrDirRec) {
				// While we are at it, check the "Icon13"
				check_for_icon13( pitem );
			}
			result = 0;
		}	else {
			result = 1;
		}
	}

	if(result != 0) ff_cleanup();
	return( result );
}

void HFSIFACE_get_dir_flush( void )
{
	ff_cleanup();
}

int HFSIFACE_get_dir_first( char *volpath, char *path, hfs_item_type *pitem )
{
	int volinx, result = 1;
	
	if(ff_dir) return(-1);

	HFSIFACE_internal_init();

	strcpy( ff_path, volpath );
	strcpy( ff_mac_path, path );
	pc_to_mac_charset( ff_mac_path );

	if(0 == HFSIFACE_mount( ff_path )) {
		ff_mounted = 1;
		volinx = path2volinx(ff_path);
		if(volinx	>= 0) {
			ff_vol = hfs_remount(hcwd_getvol(volinx), mount_flags(ff_path));
			if (ff_vol != 0) {
				ff_mounted2 = 1;
				ff_dir = hfs_opendir( ff_vol, ff_mac_path );
				if(ff_dir == 0) {
					hfs_perror(path);
					result = -1;
				} else {
					result = HFSIFACE_get_dir_next( pitem );
				}
			}
		}
	}
	if(result != 0) ff_cleanup();
	return( result );
}

// dir_name does not end with a colon.
BOOL HFSIFACE_exists_split( int volinx, char *volpath, char *dir_name, char *name )
{
	BOOL retval = FALSE;
	hfs_item_type item;
	int ret;
	char d[_MAX_PATH];

	memset( &item, 0, sizeof(item) );
	item.volinx = volinx;
	item.id = 0; // not needed

	strcpy( d, dir_name );
	strcat( d, ":" );

	ret = HFSIFACE_get_dir_first( volpath, d, &item );
	while (ret == 0) {
		mac_to_pc_charset( item.catkeyrec.ckrCName );
		if(stricmp((char *)item.catkeyrec.ckrCName,name) == 0) {
			retval = TRUE;
			break;
		}
		ret = HFSIFACE_get_dir_next( &item );
	}
	HFSIFACE_get_dir_flush();

	return(retval);
}
