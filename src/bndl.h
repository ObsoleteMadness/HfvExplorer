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

#if !defined(_BNDL_H_)
#define _BNDL_H_

#define OSType unsigned long
#define ResType unsigned long
#define INTEGER unsigned short
#define ICONTABLESIZE	1024
#define icn_sharp_hand HICON

#pragma pack(1)
typedef struct
{
  INTEGER local_id;
  INTEGER resource_id;
} local_mapping_t;

typedef struct
{
  ResType code;
  INTEGER n_mappings_minus_1;
  local_mapping_t mapping[1]; /* would like to put 0 here */
} bndl_section_t;

typedef struct
{
  INTEGER local_id;
  icn_sharp_hand icon;
	HANDLE h;
	int iscolor;
	int dim;
	int bytes;
	int small;
	unsigned char *color_icon_mask;
} local_icn_sharp_t;

typedef struct type_creator_link_str
{
  struct type_creator_link_str *next;
  OSType type;
  OSType creator;
  icn_sharp_hand icon16;
  icn_sharp_hand icon32;
} type_creator_link_t;

typedef struct
{
  OSType type;
  INTEGER local_id;
  /* more, but we don't care about it */
} fref_t, **fref_hand;

typedef struct
{
  OSType owner;
  INTEGER id;
  INTEGER n_sections_minus_1;
  bndl_section_t section[1]; /* would like to put 0 here */
} bndl_t;
#pragma pack()

extern void process_bndl (bndl_t **bndl_h, OSType creator);

extern icn_sharp_hand map_type_and_creator (OSType type, OSType creator);

#define COLORICONSIZE 1024
#define BW_ICONSIZE	256

#endif
