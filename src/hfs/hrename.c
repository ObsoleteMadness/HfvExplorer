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

# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# include <errno.h>

# include "hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "glob.h"
# include "hrename.h"

/*
 * NAME:	do_rename()
 * DESCRIPTION:	move/rename files
 */
static
int do_rename(hfsvol *vol, int argc, char *argv[], char *dest)
{
  hfsdirent ent;
  int i, result = 0;

  if (argc > 1 && (hfs_stat(vol, dest, &ent) < 0 ||
		   ! (ent.flags & HFS_ISDIR)))
    {
      ERROR(ENOTDIR, 0);
      hfs_perrorp(dest);

      return 1;
    }

  for (i = 0; i < argc; ++i)
    {
      if (hfs_rename(vol, argv[i], dest) < 0)
	{
	  hfs_perrorp(argv[i]);
	  result = 1;
	}
    }

  return result;
}

/*
 * NAME:	hrename->main()
 * DESCRIPTION:	implement hrename command
 */
int hrename_main(int argc, char *argv[])
{
  mountent *ment;
  hfsvol *vol;
  int fargc;
  char **fargv;
  int result = 0;

  if (argc < 3)
    {
      fprintf(stderr, "Usage: %s hfs-src-path [...] hfs-target-path\n", argv0);
      return 1;
    }

  vol = hfs_remount(ment = hcwd_getvol(-1), O_RDWR);
  if (vol == 0)
    return 1;

  fargv = hfs_glob(vol, argc - 2, &argv[1], &fargc);
  if (fargv == 0)
    {
      fprintf(stderr, "%s: globbing error\n", argv0);
      result = 1;
    }
  else
    result = do_rename(vol, fargc, fargv, argv[argc - 1]);

  if (result == 0)
    {
      char *path;

      path = hfs_cwd(vol);
      if (path == 0)
	{
	  hfs_perror("Can't get current HFS directory path");
	  result = 1;
	}
      else if (hcwd_setcwd(ment, path) < 0)
	{
	  perror("Can't set current HFS directory");
	  result = 1;
	}

      if (path)
	free(path);
    }

  if (hfs_umount(vol) < 0 && result == 0)
    {
      hfs_perror("Error closing HFS volume");
      result = 1;
    }

  if (fargv)
    free(fargv);

  return result;
}
