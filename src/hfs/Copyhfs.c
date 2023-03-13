/*
 * hfsutils - tools for reading and writing Macintosh HFS volumes
 * Copyright (C) 1996, 1997 Robert Leslie
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

/* Modified for emx by Marcus Better, July 1997 */

#ifdef WIN32
# include <io.h>
# include <stdio.h>
#else
# include <unistd.h>
#endif

# include <errno.h>

#ifdef __EMX__
# include <sys/types.h>
#endif
# include <sys/stat.h>
# include <fcntl.h>
# include <string.h>

#include "libhfs\internal.h"
#include "libhfs\file.h"

// # include "libhfs\hfs.h"
# include "libhfs\data.h"
# include "copyhfs.h"
#ifdef __EMX__
# include "io.h"
#endif

char *cph_error = "no error";

extern void hfs_perrorp(char *path);

# define MACB_BLOCKSZ	128

/* Copy routines =========================================================== */
static
int do_raw(hfsfile *ifile, hfsfile *ofile)
{
  char buf[HFS_BLOCKSZ * 4];
  long bytes;

  while (1) {
    bytes = hfs_read(ifile, buf, sizeof(buf));
    if (bytes < 0) {
		  cph_error = hfs_error;
			return -1;
		} else if (bytes == 0) {
			break;
		}
    if (hfs_write(ofile, buf, bytes) < 0) {
			cph_error = hfs_error;
			return -1;
		}
  }
  return 0;
}


/* Utility Routines ======================================================== */
static
hfsfile *opensrc(hfsvol *vol, char *srcname, char **dsthint, char *ext)
{
  hfsfile *file;
  hfsdirent ent;
  // static char name[36];
	// LAURI
	static char name[100];

  file = hfs_open(vol, srcname);
  if (file == 0) {
    cph_error = hfs_error;
    return 0;
  }

  if (hfs_fstat(file, &ent) < 0) {
    hfs_close(file);
    cph_error = hfs_error;
    return 0;
  }

  strcpy(name, ent.name);

  if (ext) strcat(name, ext);
  *dsthint = name;

  return file;
}


/*
 * NAME:	opendst()
 * DESCRIPTION:	open the destination file
 */
static
hfsfile *opendst(hfsvol *vol, char *dstname, char *hint,
		 char *type, char *creator)
{
  hfsdirent ent;
  hfsfile *file;
  long cwd;

  if (hfs_stat(vol, dstname, &ent) >= 0 &&
      (ent.flags & HFS_ISDIR))
    {
      cwd = hfs_getcwd(vol);

      if (hfs_setcwd(vol, ent.cnid) < 0)
	{
	  cph_error = hfs_error;
	  return 0;
	}

      dstname = hint;
    }

  hfs_delete(vol, dstname);
  if (hfs_create(vol, dstname, type, creator) < 0)
    {
      cph_error = hfs_error;

      if (dstname == hint)
	hfs_setcwd(vol, cwd);

      return 0;
    }

  file = hfs_open(vol, dstname);
  if (file == 0)
    {
      cph_error = hfs_error;

      if (dstname == hint)
	hfs_setcwd(vol, cwd);

      return 0;
    }

  if (dstname == hint)
    {
      if (hfs_setcwd(vol, cwd) < 0)
	{
	  cph_error = hfs_error;
	  hfs_close(file);
	  return 0;
	}
    }

  return file;
}

/*
 * NAME:	closefiles()
 * DESCRIPTION:	close source and destination files
 */
static
void closefiles(hfsfile *ifile, hfsfile *ofile, int *result)
{
  if (ofile && hfs_close(ofile) < 0 && *result == 0)
    {
      cph_error = hfs_error;
      *result = -1;
    }

  if (hfs_close(ifile) < 0 && *result == 0)
    {
      cph_error = hfs_error;
      *result = -1;
    }
}

/* Interface Routines ====================================================== */

int cphfs_raw( hfsvol *vol1, hfsvol *vol2, char *srcname, char *dstname )
{
  int result = 0;
  hfsfile *ifile;
  hfsfile *ofile;
  char *dsthint;
	unsigned long type, creator;
	char typestr[5], creatorstr[5];
  hfsdirent ent;

	ifile = opensrc( vol1, srcname, &dsthint, 0 );
  if (ifile == 0)
    return -1;


	type = ifile->cat.u.fil.filUsrWds.fdType;
	creator = ifile->cat.u.fil.filUsrWds.fdCreator;

	d_putl( typestr, type );
	typestr[4] = 0;
	d_putl( creatorstr, creator );
	creatorstr[4] = 0;
	

  ofile = opendst( vol2, dstname, dsthint, typestr, creatorstr );
  if (ofile == 0) {
    hfs_close(ifile);
    return -1;
  }


	f_selectfork(ifile, 0);
	f_selectfork(ofile, 0);
  result = do_raw(ifile, ofile);
	if(result == 0) {
		f_selectfork(ifile, 1);
		f_selectfork(ofile, 1);
		result = do_raw(ifile, ofile);
	}

  if (result == 0 && hfs_fstat(ifile, &ent) < 0) {
    cph_error = hfs_error;
    result = -1;
  }

  ent.fdflags &= 
    ~(HFS_FNDR_ISONDESK | HFS_FNDR_HASBEENINITED | HFS_FNDR_ISINVISIBLE);

  if (result == 0 && hfs_fsetattr(ofile, &ent) < 0) {
		cph_error = hfs_error;
		result = -1;
  }

  closefiles(ifile, ofile, &result);

  return result;
}

int do_copyhfshfs( hfsvol *vol1, hfsvol *vol2, char *source, char *dest )
{
  hfsdirent ent;
  int result = 0;

	// Destination must be a directory
  if ( (hfs_stat(vol2, dest, &ent) < 0 || !(ent.flags & HFS_ISDIR))) {
    ERROR(ENOTDIR, 0);
    hfs_perrorp(dest);
    return 1;
  }

	// Source must be a file
  if ( (hfs_stat(vol1, source, &ent) < 0 || (ent.flags & HFS_ISDIR))) {
    ERROR(EISDIR, 0);
    hfs_perrorp(source);
    return 1;
  }

  if (cphfs_raw(vol1, vol2, source, dest) < 0) {
	  ERROR(0, cph_error);
	  hfs_perrorp(source);
	  result = 1;
	}
  return result;
}

// LAURI
// fdata & fres are known to be files and existing
int do_copyindouble( hfsvol *vol, char *fdata, char *fres, char *dest )
{
  hfsdirent ent;
  int result = 0;

	// Destination must be a directory
  if ( (hfs_stat(vol, dest, &ent) < 0 || !(ent.flags & HFS_ISDIR))) {
    ERROR(ENOTDIR, 0);
    hfs_perrorp(dest);
    return 1;
  }

  if (cpi_double( fdata, fres, vol, dest) < 0) {
	  ERROR(0, cph_error);
	  hfs_perrorp(fdata);
	  result = 1;
	}
  return result;
}
