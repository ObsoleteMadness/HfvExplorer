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
# include <string.h>
# include <fcntl.h>

# include "hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "glob.h"
# include "hcd.h"

/*
 * NAME:	hcd->main()
 * DESCRIPTION:	implement hcd command
 */
int hcd_main(int argc, char *argv[])
{
  mountent *ment;
  hfsvol *vol;
  hfsvolent vent;
  char *path, root[29];
  int fargc;
  char **fargv = 0;
  int result = 0;

  if (argc > 2)
    {
      fprintf(stderr, "Usage: %s [hfs-path]\n", argv0);
      return 1;
    }

  vol = hfs_remount(ment = hcwd_getvol(-1), O_RDONLY);
  if (vol == 0)
    return 1;

  if (argc == 2)
    {
      fargv = hfs_glob(vol, 1, &argv[1], &fargc);
      if (fargv == 0)
	{
	  fprintf(stderr, "%s: globbing error\n", argv0);
	  result = 1;
	}
      else if (fargc != 1)
	{
	  fprintf(stderr, "%s: %s: ambiguous path\n", argv0, argv[1]);
	  result = 1;
	}
      else
	path = fargv[0];
    }
  else
    {
      hfs_vstat(vol, &vent);

      strcpy(root, vent.name);
      strcat(root, ":");
      path = root;
    }

  if (result == 0)
    {
      if (hfs_chdir(vol, path) < 0)
	{
	  hfs_perrorp(path);
	  result = 1;
	}
      else
	{
	  path = hfs_cwd(vol);
	  if (path == 0)
	    {
	      hfs_perror("Can't get new HFS directory path");
	      result = 1;
	    }

	  if (result == 0 && hcwd_setcwd(ment, path) < 0)
	    {
	      perror("Can't set new HFS directory");
	      result = 1;
	    }

	  if (path)
	    free(path);
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
