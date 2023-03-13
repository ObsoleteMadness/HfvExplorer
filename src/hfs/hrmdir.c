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

# include "hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "glob.h"
# include "hrmdir.h"

/*
 * NAME:	hrmdir->main()
 * DESCRIPTION:	implement hrmdir command
 */
int hrmdir_main(int argc, char *argv[])
{
  hfsvol *vol;
  char **fargv;
  int fargc, i, result = 0;

  if (argc < 2)
    {
      fprintf(stderr, "Usage: %s hfs-path [...]\n", argv0);
      return 1;
    }

  vol = hfs_remount(hcwd_getvol(-1), O_RDWR);
  if (vol == 0)
    return 1;

  fargv = hfs_glob(vol, argc - 1, &argv[1], &fargc);
  if (fargv == 0)
    {
      fprintf(stderr, "%s: globbing error\n", argv0);
      result = 1;
    }
  else
    {
      for (i = 0; i < fargc; ++i)
	{
	  if (hfs_rmdir(vol, fargv[i]) < 0)
	    {
	      hfs_perrorp(fargv[i]);
	      result = 1;
	    }
	}
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
