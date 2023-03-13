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

#include "stdafx.h"
#include "HFVExplorer.h"
#include "utils.h"

/*
Fun = (hbmMask AND Dest) XOR hbmColor

hbmColor  hbmMask  Dest  (hbmMask AND Dest)  ...XOR hbmColor
--------  -------  ----  ------------------ ----------------
0					0				 0		 0									0
0					0				 1		 0									0
0					1				 0		 0									0
0					1				 1		 1									1
1					0				 0		 0									1
1					0				 1		 0									1
1					1				 0		 0									1
1					1				 1		 1									0

mask = 1 -> keep dest, 
mask = 0 -> hbmColor
*/

enum { ICON_NO_MASK=0, ICON_SIMPLE_MASK, ICON_SMART_MASK };

int icon_mask_type = ICON_SMART_MASK;

// debug related
// int shadow = 0;
int debug = 0;
int systemiconsize = 32;

// Keeping this insanely large hides some bugs in the bundle code
// #define ENOUGH 1500
#define ENOUGH 5000

// Ridiculous? Yup.
static int isWin95 = 1;
static void init_is_Win95(void)
{
	OSVERSIONINFO osv;
	osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx( &osv )) {
		switch( osv.dwPlatformId ) {
			case VER_PLATFORM_WIN32_WINDOWS:
			case VER_PLATFORM_WIN32s:
				isWin95 = 1;
				break;
			case VER_PLATFORM_WIN32_NT:
				isWin95 = 0;
				break;
		}
	}
}
void rotate_buffer( unsigned char *p, int bytes, int small )
{
	int y, w, w2;
	unsigned char *s1;
	unsigned char *s2;
	char line[100];

	w = small ? 16 : 32;
	w2 = w >> 1;
	s1 = p;
	s2 = p + bytes - w;
	for(y=0; y<w2; y++) {
		memcpy( line, s1, w );
		memcpy( s1, s2, w );
		memcpy( s2, line, w );
		s1 += w;
		s2 -= w;
	}
}

void dump_icon_hdc( HDC hDC, HICON hicon )
{
	return;

#ifdef _DEBUG
	static int xx = 0;
	static int yy = 0;
	DrawIcon( hDC, xx, yy, hicon );
	xx+=systemiconsize;
	if(xx > 800) xx=0, yy+=systemiconsize;
#endif
}

void dump_icon( HICON hicon )
{
	return;

#ifdef _DEBUG
	HDC hDC = GetDC(0);
	if(hDC) {
		dump_icon_hdc( hDC, hicon );
		ReleaseDC(0, hDC);
	}
#endif
}

// Explorer-style: if old DOS name: NAME5678.EXT -> Name5678.ext
int is_valid_8_3_name( unsigned char *name )
{
	int body_len, ext_len, len, i;
	unsigned char ch, *p;

	p = (unsigned char *)strchr( (char *)name, '.' );
	len = strlen( (char *)name );
	if(p) {
		body_len = p - name;
		if(body_len > 8) return(0);			   // > 8.		?
		ext_len = len - body_len - 1;
		if(ext_len > 3) return(0);				 // > .3		?
		if(strchr( (char *)p+1, '.' )) return(0);	 // a.b.c   ?
	} else {
		if(len > 8) return(0);
	}
	for(i=0; i<len; i++) {
		ch = name[i];
		if(islower(ch)) return(0);		 // a..z    ?
		if(ch == ' ') return(0);		   // space   ?
	}
	return(1);
}

void dos_lfn( unsigned char *name )
{
	int len, i;

	if(!is_valid_8_3_name(name)) return;
	len = strlen((char *)name);
	name[0] = toupper(name[0]);
	for(i=1; i<len; i++) {
		name[i] = tolower(name[i]);
	}
}

void dos_lfn_fdata( WIN32_FIND_DATA *fd )
{
	int len, i;
	char *name;

	if(!*fd->cAlternateFileName) { // this is enough
	// if(strcmp(fd->cFileName,fd->cAlternateFileName) == 0) {
		name = fd->cFileName;
		len = strlen((char *)name);
		name[0] = toupper(name[0]);
		for(i=1; i<len; i++) {
			name[i] = tolower(name[i]);
		}
	}
}

void long_to_string( unsigned long u, unsigned char *s )
{
	s[0] = (unsigned char)( (u & 0xFF000000ul) >> 24 );
	s[1] = (unsigned char)( (u & 0x00FF0000ul) >> 16 );
	s[2] = (unsigned char)( (u & 0x0000FF00ul) >> 8 );
	s[3] = (unsigned char)( u & 0x000000FFul );
	s[4] = 0;
}

unsigned long string_to_long( unsigned char *s )
{
	unsigned long u;
	u  = s[3];
	u |= s[2] << 8;
	u |= s[1] << 16;
	u |= s[0] << 24;
	return(u);
}

int isdirectory( const CString &fpath )
{
	HANDLE fh;
	int ok;
	WIN32_FIND_DATA FindFileData;
	fh = FindFirstFile( fpath, &FindFileData );
	ok = fh != INVALID_HANDLE_VALUE;
	if(ok) FindClose( fh );
	if(ok) {
		return( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 );
	} else {
		return(0);
	}
}

void get_extension( const char *p, char *extension )
{
	char *s;
	s = strrchr( p, '.' );
	if(s) {
		strcpy( extension, s+1 );
		s = strrchr( extension, '\"' );
		if(s) *s = 0;
	} else {
		*extension = 0;
	}
}

static void expand_wildcards( char *ext, char *tmpl ) 
{
	char *p;
	ULONG i;

	p = strchr( ext, '*' );
	while(p) {
		i = (ULONG)p - (ULONG)ext;
		if(strlen(tmpl) > i) {
			*p = tmpl[i];
		}
		p = strchr( p+1, '*' );
	}
}

int is_extension( char *p, char *ext ) 
{
	char extension[MAX_PATH], ext2[MAX_PATH];
	get_extension( p, extension );
	if(*ext == '.') ext++;
	strcpy( ext2, ext );
	expand_wildcards( ext2, extension );
	return ( stricmp(extension,ext2) == 0 );
}

void silent_close( CFile *fp )
{
	TRY
	{
		fp->Close();
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH
}

void silent_delete( CString &name )
{
	TRY
	{
		CFile::Remove( name );
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH
}

int do_create_file( CFile *fp, CString name )
{
	int retval = 0;

	silent_delete( name );
	TRY
	{
		if(fp->Open( name, 
			CFile::modeReadWrite | CFile::shareExclusive | CFile::modeCreate)) {
			retval = 1;
		}
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH
	return(retval);
}

int do_open_file( CFile *fp, CString name )
{
	int retval = 0;
	TRY
	{
		if(fp->Open( name, CFile::modeRead | CFile::shareDenyNone )) {
			retval = 1;
		}
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH
	return(retval);
}

int do_open_readwrite( CFile *fp, CString name )
{
	int retval = 0;
	TRY
	{
		if(fp->Open( name, CFile::modeReadWrite | CFile::shareDenyNone )) {
			retval = 1;
		}
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH
	return(retval);
}

void negate_buffer( unsigned char *p, int count )
{
	while(count--) {
		// *p = *p ^ 0xFF;
		*p = 255 - *p;
		p++;
	}
}

int is_fat_root_directory( char *dir )
{
	char *p;
	p = strchr( dir, '\\' );
	if(!p) return(0);
	return(p[1] == 0);
}

int get_mac_root_dir( CString name, CString &dir )
{
	int i = name.Find(':');
	if(i < 0) return(0);
	dir = name.Left(i+1);
	return(1);
}

int get_fat_parent_directory( char *dir, char *parent )
{
	int len;
	char *p;

	strcpy( parent, dir );
	len = strlen(parent);
	if(len) {
		if(parent[len-1] == '\\') parent[len-1] = 0;
		p = strrchr( parent, '\\' );
		if(!p) return(0);
		*p = 0;
		if(!strrchr( parent, '\\' )) strcat( parent, "\\" );
		return(1);
	}
	return(0);
}

#define PALVERSION   0x300

int FAR PalEntriesOnDevice(HDC hDC)
{
   int nColors;

   nColors = GetDeviceCaps(hDC, SIZEPALETTE);
   if(!nColors) nColors = GetDeviceCaps(hDC, NUMCOLORS);
   return nColors;
}

HPALETTE FAR GetSystemPalette(void)
{
   HDC hDC;                // handle to a DC
   static HPALETTE hPal = NULL;   // handle to a palette
   HANDLE hLogPal;         // handle to a logical palette
   LPLOGPALETTE lpLogPal;  // pointer to a logical palette
   int nColors;            // number of colors

   hDC = GetDC(NULL);
   if (!hDC)
      return NULL;
   nColors = PalEntriesOnDevice(hDC);   // Number of palette entries

   hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + nColors * sizeof(
             PALETTEENTRY));

   if (!hLogPal)
      return NULL;

   lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);

   lpLogPal->palVersion = PALVERSION;
   lpLogPal->palNumEntries = (WORD)nColors;

   GetSystemPaletteEntries(hDC, 0, nColors, 
                           (LPPALETTEENTRY)(lpLogPal->palPalEntry));

   hPal = CreatePalette(lpLogPal);

   GlobalUnlock(hLogPal);
   GlobalFree(hLogPal);
   ReleaseDC(NULL, hDC);

   return hPal;
}

HPALETTE FAR GetColorPalette( int max_colors )
{
   HDC hDC;                // handle to a DC
   static HPALETTE hPal = NULL;   // handle to a palette
   HANDLE hLogPal;         // handle to a logical palette
   LPLOGPALETTE lpLogPal;  // pointer to a logical palette
   int nColors;            // number of colors

   hDC = GetDC(NULL);
   if (!hDC)
      return NULL;
   nColors = PalEntriesOnDevice(hDC);   // Number of palette entries

	 if(nColors > max_colors) nColors = max_colors;

   hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + nColors * sizeof(
             PALETTEENTRY));

   if (!hLogPal)
      return NULL;

   lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal);

   lpLogPal->palVersion = PALVERSION;
   lpLogPal->palNumEntries = (WORD)nColors;

   GetSystemPaletteEntries(hDC, 0, nColors, 
                           (LPPALETTEENTRY)(lpLogPal->palPalEntry));

   hPal = CreatePalette(lpLogPal);

   GlobalUnlock(hLogPal);
   GlobalFree(hLogPal);
   ReleaseDC(NULL, hDC);

   return hPal;
}

typedef struct {
	WORD    rgbBlue;
	WORD    rgbGreen;
	WORD    rgbRed;
	WORD    rgbReserved;
} WRGBQUAD;

static WRGBQUAD w_exact_mac256[] =
{
{65535   ,   65535 ,  65535  , 0 },
{65535   ,   65535 ,  52428  , 0 },
{65535   ,   65535 ,  39321  , 0 },
{65535   ,   65535 ,  26214  , 0 },
{65535   ,   65535 ,  13107  , 0 },
{65535   ,   65535 ,  0      , 0 },
{65535   ,   52428 ,  65535  , 0 },
{65535   ,   52428 ,  52428  , 0 },
{65535   ,   52428 ,  39321  , 0 },
{65535   ,   52428 ,  26214  , 0 },
{65535   ,   52428 ,  13107  , 0 },
{65535   ,   52428 ,  0      , 0 },
{65535   ,   39321 ,  65535  , 0 },
{65535   ,   39321 ,  52428  , 0 },
{65535   ,   39321 ,  39321  , 0 },
{65535   ,   39321 ,  26214  , 0 },
{65535   ,   39321 ,  13107  , 0 },
{65535   ,   39321 ,  0      , 0 },
{65535   ,   26214 ,  65535  , 0 },
{65535   ,   26214 ,  52428  , 0 },
{65535   ,   26214 ,  39321  , 0 },
{65535   ,   26214 ,  26214  , 0 },
{65535   ,   26214 ,  13107  , 0 },
{65535   ,   26214 ,  0      , 0 },
{65535   ,   13107 ,  65535  , 0 },
{65535   ,   13107 ,  52428  , 0 },
{65535   ,   13107 ,  39321  , 0 },
{65535   ,   13107 ,  26214  , 0 },
{65535   ,   13107 ,  13107  , 0 },
{65535   ,   13107 ,  0      , 0 },
{65535   ,   0     ,  65535  , 0 },
{65535   ,   0     ,  52428  , 0 },
{65535   ,   0     ,  39321  , 0 },
{65535   ,   0     ,  26214  , 0 },
{65535   ,   0     ,  13107  , 0 },
{65535   ,   0     ,  0      , 0 },
{52428   ,   65535 ,  65535  , 0 },
{52428   ,   65535 ,  52428  , 0 },
{52428   ,   65535 ,  39321  , 0 },
{52428   ,   65535 ,  26214  , 0 },
{52428   ,   65535 ,  13107  , 0 },
{52428   ,   65535 ,  0      , 0 },
{52428   ,   52428 ,  65535  , 0 },
{52428   ,   52428 ,  52428  , 0 },
{52428   ,   52428 ,  39321  , 0 },
{52428   ,   52428 ,  26214  , 0 },
{52428   ,   52428 ,  13107  , 0 },
{52428   ,   52428 ,  0      , 0 },
{52428   ,   39321 ,  65535  , 0 },
{52428   ,   39321 ,  52428  , 0 },
{52428   ,   39321 ,  39321  , 0 },
{52428   ,   39321 ,  26214  , 0 },
{52428   ,   39321 ,  13107  , 0 },
{52428   ,   39321 ,  0      , 0 },
{52428   ,   26214 ,  65535  , 0 },
{52428   ,   26214 ,  52428  , 0 },
{52428   ,   26214 ,  39321  , 0 },
{52428   ,   26214 ,  26214  , 0 },
{52428   ,   26214 ,  13107  , 0 },
{52428   ,   26214 ,  0      , 0 },
{52428   ,   13107 ,  65535  , 0 },
{52428   ,   13107 ,  52428  , 0 },
{52428   ,   13107 ,  39321  , 0 },
{52428   ,   13107 ,  26214  , 0 },
{52428   ,   13107 ,  13107  , 0 },
{52428   ,   13107 ,  0      , 0 },
{52428   ,   0     ,  65535  , 0 },
{52428   ,   0     ,  52428  , 0 },
{52428   ,   0     ,  39321  , 0 },
{52428   ,   0     ,  26214  , 0 },
{52428   ,   0     ,  13107  , 0 },
{52428   ,   0     ,  0      , 0 },
{39321   ,   65535 ,  65535  , 0 },
{39321   ,   65535 ,  52428  , 0 },
{39321   ,   65535 ,  39321  , 0 },
{39321   ,   65535 ,  26214  , 0 },
{39321   ,   65535 ,  13107  , 0 },
{39321   ,   65535 ,  0      , 0 },
{39321   ,   52428 ,  65535  , 0 },
{39321   ,   52428 ,  52428  , 0 },
{39321   ,   52428 ,  39321  , 0 },
{39321   ,   52428 ,  26214  , 0 },
{39321   ,   52428 ,  13107  , 0 },
{39321   ,   52428 ,  0      , 0 },
{39321   ,   39321 ,  65535  , 0 },
{39321   ,   39321 ,  52428  , 0 },
{39321   ,   39321 ,  39321  , 0 },
{39321   ,   39321 ,  26214  , 0 },
{39321   ,   39321 ,  13107  , 0 },
{39321   ,   39321 ,  0      , 0 },
{39321   ,   26214 ,  65535  , 0 },
{39321   ,   26214 ,  52428  , 0 },
{39321   ,   26214 ,  39321  , 0 },
{39321   ,   26214 ,  26214  , 0 },
{39321   ,   26214 ,  13107  , 0 },
{39321   ,   26214 ,  0      , 0 },
{39321   ,   13107 ,  65535  , 0 },
{39321   ,   13107 ,  52428  , 0 },
{39321   ,   13107 ,  39321  , 0 },
{39321   ,   13107 ,  26214  , 0 },
{39321   ,   13107 ,  13107  , 0 },
{39321   ,   13107 ,  0      , 0 },
{39321   ,   0     ,  65535  , 0 },
{39321   ,   0     ,  52428  , 0 },
{39321   ,   0     ,  39321  , 0 },
{39321   ,   0     ,  26214  , 0 },
{39321   ,   0     ,  13107  , 0 },
{39321   ,   0     ,  0      , 0 },
{26214   ,   65535 ,  65535  , 0 },
{26214   ,   65535 ,  52428  , 0 },
{26214   ,   65535 ,  39321  , 0 },
{26214   ,   65535 ,  26214  , 0 },
{26214   ,   65535 ,  13107  , 0 },
{26214   ,   65535 ,  0      , 0 },
{26214   ,   52428 ,  65535  , 0 },
{26214   ,   52428 ,  52428  , 0 },
{26214   ,   52428 ,  39321  , 0 },
{26214   ,   52428 ,  26214  , 0 },
{26214   ,   52428 ,  13107  , 0 },
{26214   ,   52428 ,  0      , 0 },
{26214   ,   39321 ,  65535  , 0 },
{26214   ,   39321 ,  52428  , 0 },
{26214   ,   39321 ,  39321  , 0 },
{26214   ,   39321 ,  26214  , 0 },
{26214   ,   39321 ,  13107  , 0 },
{26214   ,   39321 ,  0      , 0 },
{26214   ,   26214 ,  65535  , 0 },
{26214   ,   26214 ,  52428  , 0 },
{26214   ,   26214 ,  39321  , 0 },
{26214   ,   26214 ,  26214  , 0 },
{26214   ,   26214 ,  13107  , 0 },
{26214   ,   26214 ,  0      , 0 },
{26214   ,   13107 ,  65535  , 0 },
{26214   ,   13107 ,  52428  , 0 },
{26214   ,   13107 ,  39321  , 0 },
{26214   ,   13107 ,  26214  , 0 },
{26214   ,   13107 ,  13107  , 0 },
{26214   ,   13107 ,  0      , 0 },
{26214   ,   0     ,  65535  , 0 },
{26214   ,   0     ,  52428  , 0 },
{26214   ,   0     ,  39321  , 0 },
{26214   ,   0     ,  26214  , 0 },
{26214   ,   0     ,  13107  , 0 },
{26300   ,   4265  ,  486    , 0 },
{13107   ,   65535 ,  65535  , 0 },
{13107   ,   65535 ,  52428  , 0 },
{13107   ,   65535 ,  39321  , 0 },
{13107   ,   65535 ,  26214  , 0 },
{13107   ,   65535 ,  13107  , 0 },
{13107   ,   65535 ,  0      , 0 },
{13107   ,   52428 ,  65535  , 0 },
{13107   ,   52428 ,  52428  , 0 },
{13107   ,   52428 ,  39321  , 0 },
{13107   ,   52428 ,  26214  , 0 },
{13107   ,   52428 ,  13107  , 0 },
{13107   ,   52428 ,  0      , 0 },
{13107   ,   39321 ,  65535  , 0 },
{13107   ,   39321 ,  52428  , 0 },
{13107   ,   39321 ,  39321  , 0 },
{13107   ,   39321 ,  26214  , 0 },
{13107   ,   39321 ,  13107  , 0 },
{13107   ,   39321 ,  0      , 0 },
{13107   ,   26214 ,  65535  , 0 },
{13107   ,   26214 ,  52428  , 0 },
{13107   ,   26214 ,  39321  , 0 },
{13107   ,   26214 ,  26214  , 0 },
{13107   ,   26214 ,  13107  , 0 },
{13107   ,   26214 ,  0      , 0 },
{13107   ,   13107 ,  65535  , 0 },
{13107   ,   13107 ,  52428  , 0 },
{13107   ,   13107 ,  39321  , 0 },
{13107   ,   13107 ,  26214  , 0 },
{13107   ,   13107 ,  13107  , 0 },
{15976   ,   14457 ,  2622   , 0 },
{13107   ,   0     ,  65535  , 0 },
{13107   ,   0     ,  52428  , 0 },
{13107   ,   0     ,  39321  , 0 },
{13107   ,   0     ,  26214  , 0 },
{13107   ,   0     ,  13107  , 0 },
{13107   ,   0     ,  0      , 0 },
{0       ,   65535 ,  65535  , 0 },
{0       ,   65535 ,  52428  , 0 },
{0       ,   65535 ,  39321  , 0 },
{0       ,   65535 ,  26214  , 0 },
{0       ,   65535 ,  13107  , 0 },
{0       ,   65535 ,  0      , 0 },
{0       ,   52428 ,  65535  , 0 },
{0       ,   52428 ,  52428  , 0 },
{0       ,   52428 ,  39321  , 0 },
{0       ,   52428 ,  26214  , 0 },
{0       ,   52428 ,  13107  , 0 },
{0       ,   52428 ,  0      , 0 },
{0       ,   40000 ,  65535  , 0 },
{0       ,   39321 ,  52428  , 0 },
{0       ,   39321 ,  39321  , 0 },
{0       ,   39321 ,  26214  , 0 },
{0       ,   39321 ,  13107  , 0 },
{0       ,   39321 ,  0      , 0 },
{0       ,   26214 ,  65535  , 0 },
{0       ,   26214 ,  52428  , 0 },
{0       ,   26214 ,  39321  , 0 },
{0       ,   26214 ,  26214  , 0 },
{0       ,   26214 ,  13107  , 0 },
{0       ,   26214 ,  0      , 0 },
{0       ,   13107 ,  65535  , 0 },
{0       ,   13107 ,  52428  , 0 },
{0       ,   13107 ,  39321  , 0 },
{0       ,   13107 ,  26214  , 0 },
{0       ,   13107 ,  13107  , 0 },
{0       ,   13107 ,  0      , 0 },
{0       ,   0     ,  65535  , 0 },
{0       ,   0     ,  52428  , 0 },
{0       ,   0     ,  39321  , 0 },
{0       ,   0     ,  26214  , 0 },
{0       ,   0     ,  13107  , 0 },
{61183   ,   2079  ,  4938   , 0 },
{56797   ,   0     ,  0      , 0 },
{48059   ,   0     ,  0      , 0 },
{43690   ,   0     ,  0      , 0 },
{34952   ,   0     ,  0      , 0 },
{30583   ,   0     ,  0      , 0 },
{21845   ,   0     ,  0      , 0 },
{17476   ,   0     ,  0      , 0 },
{8738    ,   0     ,  0      , 0 },
{4369    ,   0     ,  0      , 0 },
{0       ,   61166 ,  0      , 0 },
{0       ,   56797 ,  0      , 0 },
{0       ,   48059 ,  0      , 0 },
{0       ,   43690 ,  0      , 0 },
{0       ,   34952 ,  0      , 0 },
{0       ,   30583 ,  0      , 0 },
{0       ,   21845 ,  0      , 0 },
{0       ,   17476 ,  0      , 0 },
{0       ,   8738  ,  0      , 0 },
{0       ,   4369  ,  0      , 0 },
{0       ,   0     ,  61166  , 0 },
{0       ,   0     ,  56797  , 0 },
{0       ,   0     ,  48059  , 0 },
{0       ,   0     ,  43690  , 0 },
{0       ,   0     ,  34952  , 0 },
{0       ,   0     ,  30583  , 0 },
{0       ,   0     ,  21845  , 0 },
{0       ,   0     ,  17476  , 0 },
{0       ,   0     ,  8738   , 0 },
{0       ,   0     ,  4369   , 0 },
{61166   ,   61166 ,  61166  , 0 },
{56797   ,   56797 ,  56797  , 0 },
{48059   ,   48059 ,  48059  , 0 },
{43690   ,   43690 ,  43690  , 0 },
{34952   ,   34952 ,  34952  , 0 },
{30583   ,   30583 ,  30583  , 0 },
{21845   ,   21845 ,  21845  , 0 },
{17476   ,   17476 ,  17476  , 0 },
{8738    ,   8738  ,  8738   , 0 },
{4369    ,   4369  ,  4369   , 0 },
{0       ,   0     ,  0      , 0 }
};


static int map_palette_inited = 0;
RGBQUAD exact_mac256[256];
extern RGBQUAD rgbStd256[];

void init_map_palette()
{
	int i;
	for(i=0; i<256; i++) {
		/*
		exact_mac256[i].rgbRed = (BYTE) (w_exact_mac256[i].rgbRed >> 8);
		exact_mac256[i].rgbGreen = (BYTE) (w_exact_mac256[i].rgbGreen >> 8);
		exact_mac256[i].rgbBlue = (BYTE) (w_exact_mac256[i].rgbBlue >> 8);
		*/
		// RGBQUAD: blue/green/red
		exact_mac256[i].rgbRed = (BYTE) (w_exact_mac256[i].rgbBlue >> 8);
		exact_mac256[i].rgbGreen = (BYTE) (w_exact_mac256[i].rgbGreen >> 8);
		exact_mac256[i].rgbBlue = (BYTE) (w_exact_mac256[i].rgbRed >> 8);
	}
	map_palette_inited = 1;
	init_is_Win95();
}

int myGetNearestPaletteIndex( 
	COLORREF rgb, 
	LPPALETTEENTRY pale,
	int count )
{
	int dr, dg, db;
	long d, dmin = 0x7FFFFFFF;
	int i, mini = 0;
	for(i=0; i<count; i++) {
		dr = (int)pale[i].peRed - (int)GetRValue(rgb);
		dg = (int)pale[i].peGreen - (int)GetGValue(rgb);
		db = (int)pale[i].peBlue - (int)GetBValue(rgb);
		d = (long)dr*dr + (long)dg*dg + (long)db*db;
		if(d < dmin) {
			dmin = d;
			mini = i;
		}
	}
	return(mini);
}

typedef struct {
	BITMAPINFOHEADER bmiHeader; 
	RGBQUAD          bmiColors[256]; 
} zapper_type;

HBITMAP create_mask_bitmap(int count)
{
	HDC hDC = GetDC(NULL);
	zapper_type bmi;
	char *mask = (char *)malloc( 10000 );

	int dim = count == 1024 ? 32 : 16;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = dim;
  bmi.bmiHeader.biHeight = -dim;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 8;
  bmi.bmiHeader.biCompression = 0;
  bmi.bmiHeader.biSizeImage = count;
  bmi.bmiHeader.biXPelsPerMeter = 0;
  bmi.bmiHeader.biYPelsPerMeter = 0;
  bmi.bmiHeader.biClrUsed = 256;
  bmi.bmiHeader.biClrImportant = 256;
	memcpy( bmi.bmiColors, exact_mac256, sizeof(RGBQUAD) * 256 );
	memset( mask, 255, 10000 );
	HBITMAP bmp = CreateDIBitmap(
    hDC,
    (BITMAPINFOHEADER*)&bmi,
    CBM_INIT,
    mask,
    (BITMAPINFO*)&bmi,
    DIB_RGB_COLORS
   );
	free (mask);
  ReleaseDC(NULL, hDC);
	return(bmp);
}

HBITMAP mac_mask32 = 0;
HBITMAP mac_mask16 = 0;

void delete_mask_bitmaps()
{
	if(mac_mask16) {
		DeleteObject(mac_mask16);
		mac_mask16 = 0;
	}
	if(mac_mask32) {
		DeleteObject(mac_mask32);
		mac_mask32 = 0;
	}
}

static unsigned char *quick_buffer = 0;
static unsigned char *quick_buffer2 = 0;
static int quick_bits;
static int quick_dim;

static int white_index = 0;

#define INDEX(x,y) ( ((y)<<quick_bits) + x )

// this is necessary to avoid leaks from "corners"
#define EMPTY(x,y) (x>=0 && x<quick_dim && y>=0 && y<quick_dim && quick_buffer[INDEX(x,y)]==0)

void flood_fill( int x, int y )
{
	int inx; 

	if(x < 0 || y < 0 || x >= quick_dim || y >= quick_dim) return;
	inx = INDEX(x,y);
	if(quick_buffer2[inx]) return; // if already marked
	if(quick_buffer[inx] != white_index) return; // if not white
	quick_buffer2[inx] = 1;
	// p6 likes loop unrolling
	flood_fill( x+1, y );
	flood_fill( x-1, y );
	flood_fill( x  , y+1 );
	flood_fill( x  , y-1 );
	if(EMPTY(x-1,y) || EMPTY(x,y-1)) flood_fill( x-1, y-1 );
	if(EMPTY(x,y-1) || EMPTY(x+1,y)) flood_fill( x+1, y-1 );
	if(EMPTY(x-1,y) || EMPTY(x,y+1)) flood_fill( x-1, y+1 );
	if(EMPTY(x,y+1) || EMPTY(x+1,y)) flood_fill( x+1, y+1 );
}

int _log2( int x )
{
	int y = 0;
	while(x >>= 1) y++;
	return(y);
}

void prepare_for_smart_mask( 
	unsigned char *mask_bits, 
	int dim, 
	int count )
{
	int x, y;

	quick_buffer = mask_bits;
	quick_dim = dim;
	quick_buffer2 = (unsigned char *)malloc( 10000 );
	quick_bits = _log2(dim);
	memset( quick_buffer2, 0, count );

	int lim = dim-1;
	// +2 below gains speed with very theoritical possibility
	// of failure
	for(y=1; y<lim; y+=2) flood_fill( 0, y );
	for(y=1; y<lim; y+=2) flood_fill( dim-1, y );
	for(x=1; x<lim; x+=2) flood_fill( x, 0 );
	for(x=1; x<lim; x+=2) flood_fill( dim-1, x );

	for(y=0; y<count; y++) {
		if(quick_buffer2[y]) {
			mask_bits[y] = white_index;
		} else {
			mask_bits[y] = 255-white_index;
		}
	}
	free(quick_buffer2);
}

void make_mask( 
	unsigned char *original, 
	unsigned char *mask_bits, 
	int count,
	int dim )
{
	int input, output, x, y, bit, nbyt;
	unsigned char byt;
	
	memcpy( mask_bits, original, count );
	negate_buffer( (unsigned char *)original, count );
	
	if(icon_mask_type == ICON_SMART_MASK) {
		prepare_for_smart_mask( mask_bits, dim, count );
	}

	// b&w mask build
	input = 0;
	output = 0;
	nbyt = dim >> 3;
	for(y=0; y<dim; y++) {
		for(x=0; x<nbyt; x++) {
			byt = 0;
			for(bit=0; bit<8; bit++) {
				if(mask_bits[input] == white_index) { // if white
					byt |= ((unsigned char)128>>bit);
				} else {
					original[input] = 255 - original[input];
				}
				input++;
			}
			mask_bits[output++] = byt;
		}
		// if(isWin95 == 0 && nbyt == 2) output += 2;
	}
}

// WORD aligned mask -> DWORD aligned mask
void przuksis( unsigned char *a, unsigned char *b )
{
	int y;
	for(y=0; y<16; y++) {
		*b++ = *a++;
		*b++ = *a++;
		*b++ = 0;
		*b++ = 0;
	}
}

// creates icon from raw data.
// raw data is imported/exported in p,
// mask data is in cim.
HICON map_colors( unsigned char *p, int small, int count, unsigned char **cim )
{
	int dim = small ? 16 : 32;
	HICON hicon = 0;
	ICONINFO pico;
	HDC hDC;
	HBITMAP bmp = 0;
	zapper_type bmi;
	zapper_type bmi_mask;
	HBITMAP bmp_mask = 0;
	unsigned char *mask_bits = 0;
	HWND hwnd = 0;
	unsigned char *aligned_mask = 0;

	hDC = GetDC(hwnd);
	if(!hDC) return(0);

	if(!map_palette_inited) init_map_palette();

	// if(shadow) {
	//   shadow++; // for easy breakpoint
	// }
	if(!*cim) {
		if(icon_mask_type != ICON_NO_MASK) {
			mask_bits = (unsigned char *)malloc( ENOUGH );
			if(!mask_bits) {
				ReleaseDC(hwnd, hDC);
				return(0);
			}
		}
	}
	if(!mac_mask32) {
		mac_mask32 = create_mask_bitmap(1024);
		mac_mask16 = create_mask_bitmap(256);
	}

  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = dim;
  bmi.bmiHeader.biHeight = -dim;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 8;
  bmi.bmiHeader.biCompression = 0;
  bmi.bmiHeader.biSizeImage = count;
  bmi.bmiHeader.biXPelsPerMeter = 0;
  bmi.bmiHeader.biYPelsPerMeter = 0;
  bmi.bmiHeader.biClrUsed = 0;
  bmi.bmiHeader.biClrImportant = 0;
	memcpy( bmi.bmiColors, exact_mac256, sizeof(RGBQUAD) * 256 );

	memcpy( &bmi_mask, &bmi, sizeof(zapper_type) );
  bmi_mask.bmiHeader.biBitCount = 1;
  bmi_mask.bmiHeader.biSizeImage = count;  // count >> 3;

  bmi_mask.bmiHeader.biClrUsed = 2;
  bmi_mask.bmiHeader.biClrImportant = 2;

	if(isWin95) {
		bmi_mask.bmiColors[0].rgbRed = 255;
		bmi_mask.bmiColors[0].rgbGreen = 255;
		bmi_mask.bmiColors[0].rgbBlue = 255;
		bmi_mask.bmiColors[1].rgbRed = 0;
		bmi_mask.bmiColors[1].rgbGreen = 0;
		bmi_mask.bmiColors[1].rgbBlue = 0;
	  bmi_mask.bmiHeader.biSizeImage = count >> 3;
	} else {
		bmi_mask.bmiColors[0].rgbRed = 0;
		bmi_mask.bmiColors[0].rgbGreen = 0;
		bmi_mask.bmiColors[0].rgbBlue = 0;
		bmi_mask.bmiColors[1].rgbRed = 255;
		bmi_mask.bmiColors[1].rgbGreen = 255;
		bmi_mask.bmiColors[1].rgbBlue = 255;
	  bmi_mask.bmiHeader.biSizeImage = 0;
	  bmi_mask.bmiHeader.biSizeImage = count >> 2;
	}
	
	if(!*cim) {
		if(icon_mask_type != ICON_NO_MASK) {
			make_mask( p, mask_bits, count, dim );
		}
	}
	bmp = CreateDIBitmap(
    hDC, (BITMAPINFOHEADER*)&bmi, CBM_INIT, p,
    (BITMAPINFO*)&bmi, DIB_RGB_COLORS );
	if(icon_mask_type != ICON_NO_MASK) {
		if(*cim) {
			mask_bits = *cim;
		} else {
			*cim = mask_bits;
		}

		if(isWin95) {
			bmp_mask = CreateDIBitmap( hDC,
				(BITMAPINFOHEADER*)&bmi_mask,
				0, // we cannot use CBM_INIT & mask_bits here because of 
				0, // bug in CreateDIBitmap
				(BITMAPINFO*)&bmi_mask,
				DIB_RGB_COLORS
			 );
			LONG result = SetBitmapBits( bmp_mask, count, mask_bits );
		} else {
			if(small) {
				aligned_mask = (unsigned char *)malloc( ENOUGH );
				if(aligned_mask) {
					przuksis( mask_bits, aligned_mask );
					bmp_mask = CreateDIBitmap( hDC,
						(BITMAPINFOHEADER*)&bmi_mask,
						CBM_INIT,
						aligned_mask,
						(BITMAPINFO*)&bmi_mask,
						DIB_RGB_COLORS
				  );
				}
			} else {
				bmp_mask = CreateDIBitmap( hDC,
					(BITMAPINFOHEADER*)&bmi_mask,
					CBM_INIT,
					mask_bits,
					(BITMAPINFO*)&bmi_mask,
					DIB_RGB_COLORS
				);
			}
		}
		pico.hbmMask = bmp_mask;
	} else {
	  pico.hbmMask = small ? mac_mask16 : mac_mask32;
	}
  pico.fIcon = 1;
  pico.yHotspot = pico.xHotspot = dim>>1;
  pico.hbmColor = bmp;

	hicon = CreateIconIndirect( &pico );

#ifdef _DEBUG
	if(!small) {
		dump_icon_hdc( hDC, hicon );
	}
#endif
	
	if(aligned_mask) free( aligned_mask );
	if(bmp) DeleteObject( bmp );
	if(bmp_mask) DeleteObject( bmp_mask );
	if(mask_bits && !*cim) free( mask_bits );
  ReleaseDC(hwnd, hDC);
	return(hicon);
  
	/*
	HDC hDC = GetDC(NULL);
  GetSystemPaletteEntries( hDC, 0, 256, pale );
	for(i=0; i<count; i++) {
		j = (int)p[i];
		rgb = RGB( 
					exact_mac256[j].rgbRed,
					exact_mac256[j].rgbGreen,
					exact_mac256[j].rgbBlue );
		p[i] = myGetNearestPaletteIndex( rgb, pale, 256 );
	}
  ReleaseDC(NULL, hDC);
	return;
	*/

	/*
	hpal = GetColorPalette(256);

	for(i=0; i<count; i++) {
		j = (int)p[i];
		rgb = RGB( 
					exact_mac256[j].rgbRed,
					exact_mac256[j].rgbGreen,
					exact_mac256[j].rgbBlue );
		p[i] = GetNearestPaletteIndex( hpal, rgb );
	}
  DeleteObject(hpal);
	*/
}

/*
void correct_colors( unsigned char *p, int count )
{
	PALETTEENTRY pale[256];
	int i, j;
	COLORREF rgb;
	HDC hDC;

	HPALETTE hpal = GetColorPalette(256);
	for(i=0; i<count; i++) {
		j = (int)p[i];
		rgb = RGB( 
					exact_mac256[j].rgbRed,
					exact_mac256[j].rgbGreen,
					exact_mac256[j].rgbBlue );
		p[i] = GetNearestPaletteIndex( hpal, rgb );
	}
	white_index = GetNearestPaletteIndex( hpal, COLORREF(255,255,255) );
  DeleteObject(hpal);
	white_index = 255;
	return;

	hDC = GetDC(NULL);
  GetSystemPaletteEntries( hDC, 0, 256, pale );
	for(i=0; i<count; i++) {
		j = (int)p[i];
		rgb = RGB( 
					exact_mac256[j].rgbRed,
					exact_mac256[j].rgbGreen,
					exact_mac256[j].rgbBlue );
		p[i] = myGetNearestPaletteIndex( rgb, pale, 256 );
	}
	white_index = myGetNearestPaletteIndex( COLORREF(255,255,255), pale, 256 );
	white_index = 255;
  ReleaseDC(NULL, hDC);
}

void make_mask2( 
	unsigned char *original, 
	unsigned char *mask_bits, 
	zapper_type *pz, 
	int count,
	int dim )
{
	int input, output, x, y, bit, nbyt;
	unsigned char byt;

	correct_colors( original, count );
	memcpy( mask_bits, original, count );
	// negate_buffer( (unsigned char *)original, count );

  pz->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  pz->bmiHeader.biWidth = dim;
  pz->bmiHeader.biHeight = -dim;
  pz->bmiHeader.biPlanes = 1;
  pz->bmiHeader.biBitCount = 1;
  pz->bmiHeader.biCompression = 0;
  pz->bmiHeader.biSizeImage = dim * dim / 8;
  pz->bmiHeader.biXPelsPerMeter = 0;
  pz->bmiHeader.biYPelsPerMeter = 0;
  pz->bmiHeader.biClrUsed = 2;
  pz->bmiHeader.biClrImportant = 2;
	memcpy( pz->bmiColors, rgbStd256, sizeof(RGBQUAD) * 256 );

	pz->bmiColors[0].rgbRed = 0;
	pz->bmiColors[0].rgbGreen = 0;
	pz->bmiColors[0].rgbBlue = 0;
	pz->bmiColors[1].rgbRed = 255;
	pz->bmiColors[1].rgbGreen = 255;
	pz->bmiColors[1].rgbBlue = 255;
	
	pz->bmiColors[0] = rgbStd256[0];
	pz->bmiColors[1] = rgbStd256[1];

	if(icon_mask_type == ICON_SMART_MASK) {
		prepare_for_smart_mask( mask_bits, dim, count );
	}

	input = 0;
	output = 0;
	nbyt = dim >> 3;
	for(y=0; y<dim; y++) {
		for(x=0; x<nbyt; x++) {
			byt = 0;
			for(bit=0; bit<8; bit++) {
				if(mask_bits[input++] == white_index) { // if white
					byt |= ((unsigned char)128>>bit);
				}
			}
			mask_bits[output++] = byt;
		}
	}
}
*/

#ifdef __cplusplus
extern "C" {
#endif

DWORD __Ttim = 0;
HCURSOR __TScurs = 0;

void _end_time_consuming( HCURSOR hcursor )
{
	if(hcursor) {
		::SetCursor( hcursor );
	}
}

HCURSOR _start_time_consuming(int update)
{
	HCURSOR cu;
	cu = ::LoadCursor( 0, IDC_WAIT );
	if(cu) {
		cu = ::SetCursor( cu );
	}
	if(update) {
		CHFVExplorerApp	*pApp;
		pApp = (CHFVExplorerApp *)AfxGetApp();
		pApp->m_list->UpdateWindow();
	}
	return(cu);
}

int set_file_size( char *path, DWORD new_size )
{
	CFile f;
	int result = 0;

	if( do_open_readwrite( &f, path ) ) {
		TRY
		{
			f.SetLength( new_size );
			result = 1;
		}
		CATCH( CFileException, e )
		{
		}
		END_CATCH
		silent_close( &f );
	}
	return(result);
}

BOOL is_floppy_by_char( char letter )
{
	char rootdir[20];

	letter = toupper(letter);
	wsprintf( rootdir, "%c:\\", letter );
	return( (letter == 'A' || letter == 'B') && GetDriveType(rootdir) == DRIVE_REMOVABLE );
}

BOOL is_floppy_by_index( int index )
{
	return( is_floppy_by_char( (char)( 'A' + index ) ) );
}

BOOL can_have_exclusive_access( LPCSTR path )
{
	BOOL result = FALSE;
	CFile f;

	if(f.Open( path, CFile::modeRead | CFile::shareExclusive )) {
		f.Close();
		result = TRUE;
	}
	return(result);
}

#ifdef __cplusplus
}
#endif
