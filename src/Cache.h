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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CACHE_H_
#define _CACHE_H_
// #define NBLOCKS 500
#define NBLOCKS 1000

typedef struct {
	int inited;
	int res_count;
	int sector_size;
	char *blocks;
	int *block;
	DWORD *LRU;
} cachetype;

void cache_init( cachetype *cptr );
void cache_clear( cachetype *cptr );
void cache_final( cachetype *cptr );
int cache_get( cachetype *cptr, int block, char *buf );
void cache_put( cachetype *cptr, int block, char *buf, int ss );
void cache_remove( cachetype *cptr, int block, int ss );
#endif

#ifdef __cplusplus
} // extern "C"
#endif
