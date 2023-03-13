/*
 * hfsutils - tools for reading and writing Macintosh HFS volumes
 * Copyright (C) 1997 Marcus Better
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

#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#define EXE_NAME "hfsutil.exe"

int main(int argc, char *argv[])
{
  char path[256];

  _wildcard(&argc, &argv);

  /* Search in current dir and then in directories on PATH */
  _searchenv(EXE_NAME, "PATH", path);
  if(path[0] == 0) {
    /* Search directories in HFSUTILS variable */
    _searchenv(EXE_NAME, "HFSUTILS", path);
  }

  if(path[0] == 0) {
    fprintf(stderr, 
	    "Cannot find the hfsutil executable.\n"
	    "Please set the HFSUTIL variable.\n");
    return -1;
  }

  return spawnv(P_WAIT, path, argv);
}
