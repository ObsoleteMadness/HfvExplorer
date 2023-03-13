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

#ifndef _FILEHOOK_H_
#define _FILEHOOK_H_

#ifdef __cplusplus
extern "C" {
#endif

_CRTIMP int __cdecl stat(const char *, struct stat *);
_CRTIMP int __cdecl open(const char *, int, ...);
_CRTIMP int __cdecl close(int);
_CRTIMP int __cdecl read(int, void *, unsigned int);
_CRTIMP int __cdecl write(int, const void *, unsigned int);
_CRTIMP long __cdecl lseek(int, long, int);

typedef int (*ftype_stat)(const char *, struct stat *);
typedef int (*ftype_open)(const char *, int, ...);
typedef int (*ftype_close)(int);
typedef int (*ftype_read)(int, void *, unsigned int);
typedef int (*ftype_write)(int, const void *, unsigned int);
typedef long (*ftype_lseek)(int, long, int);

extern ftype_stat fhook_stat;
extern ftype_open fhook_open;
extern ftype_close fhook_close;
extern ftype_read fhook_read;
extern ftype_write fhook_write;
extern ftype_lseek fhook_lseek;

#ifndef _FHOOK_INTERNAL_
// #define stat  fhook_stat
#define open  fhook_open
#define close fhook_close
#define read  fhook_read
#define write fhook_write
#define lseek fhook_lseek
#endif

void fhook_set_real_funcs( void );
void fhook_set_fake_funcs( void );
void fhook_init_fake_funcs(
	ftype_stat  f_stat,
	ftype_open  f_open,
	ftype_close f_close,
	ftype_read  f_read,
	ftype_write f_write,
	ftype_lseek f_lseek
);

void fhook_set_last_open_file( const char *path );
void fhook_get_last_open_file( char *path, int maxpath );

#ifdef __cplusplus
} // extern "C"
#endif

#endif //_FILEHOOK_H_
