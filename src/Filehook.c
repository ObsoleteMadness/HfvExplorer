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

#include <sys/types.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define _FHOOK_INTERNAL_
#include "filehook.h"

/* 
	Hook:
		hfs\libhfs\hfs.c
		hfs\libhfs\block.c

	Do not hook:
		hfs\copyin.c
		hfs\copyout.c
		hfs\binhex.c

	Missing:
		function "stat".
		Just make sure to give file name which does not exist.
*/

ftype_stat  fhook_stat  = stat;
ftype_open  fhook_open  = open;
ftype_close fhook_close = close;
ftype_read  fhook_read  = read;
ftype_write fhook_write = write;
ftype_lseek fhook_lseek = lseek;

static ftype_stat  fake_stat = 0;
static ftype_open  fake_open = 0;
static ftype_close fake_close = 0;
static ftype_read  fake_read = 0;
static ftype_write fake_write = 0;
static ftype_lseek fake_lseek = 0;

static void fhook_init_fsystem( void )
{
}

void fhook_set_real_funcs( void )
{
	fhook_stat =  stat;
	fhook_open =  open;
	fhook_close = close;
	fhook_read =  read;
	fhook_write = write;
	fhook_lseek = lseek;
}

void fhook_set_fake_funcs( void )
{
	fhook_stat =  fake_stat;
	fhook_open =  fake_open;
	fhook_close = fake_close;
	fhook_read =  fake_read;
	fhook_write = fake_write;
	fhook_lseek = fake_lseek;
	fhook_init_fsystem();
}

void fhook_init_fake_funcs(
	ftype_stat  f_stat,
	ftype_open  f_open,
	ftype_close f_close,
	ftype_read  f_read,
	ftype_write f_write,
	ftype_lseek f_lseek
)
{
	fake_stat =  f_stat;
	fake_open =  f_open;
	fake_close = f_close;
	fake_read =  f_read;
	fake_write = f_write;
	fake_lseek = f_lseek;
}
