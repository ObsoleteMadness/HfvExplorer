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
# include <string.h>
# include <stdlib.h>
# include <fcntl.h>

# include "hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "glob.h"
# include "hattrib.h"

/*
 * NAME:	usage()
 * DESCRIPTION:	display usage message
 */
static
int usage(void)
{
  fprintf(stderr,
	  "Usage: %s [-t TYPE] [-c CREA] [-|+i] [-|+l] hfs-path [...]\n",
	  argv0);

  return 1;
}

/*
 * NAME:	hattrib->main()
 * DESCRIPTION:	implement hattrib command
 */
int hattrib_main(int argc, char *argv[])
{
  char *type = 0, *crea = 0;
  int invis = 0, lock = 0;
  hfsvol *vol;
  int fargc;
  char **fargv;
  int i, result = 0;

  for (i = 1; i < argc; ++i)
    {
      switch (argv[i][0])
	{
	case '-':
	  switch (argv[i][1])
	    {
	    case 't':
	      type = argv[++i];

	      if (type == 0)
		return usage();

	      if (strlen(type) != 4)
		{
		  fprintf(stderr, "%s: file type must be 4 characters\n",
			  argv0);
		  return 1;
		}
	      continue;

	    case 'c':
	      crea = argv[++i];

	      if (crea == 0)
		return usage();

	      if (strlen(crea) != 4)
		{
		  fprintf(stderr, "%s: file creator must be 4 characters\n",
			  argv0);
		  return 1;
		}
	      continue;

	    case 'i':
	      invis = -1;
	      continue;

	    case 'l':
	      lock = -1;
	      continue;

	    default:
	      return usage();
	    }
	  break;

	case '+':
	  switch (argv[i][1])
	    {
	    case 'i':
	      invis = 1;
	      continue;

	    case 'l':
	      lock = 1;
	      continue;

	    default:
	      return usage();
	    }
	  break;
	}

      break;
    }

  if (argc - i == 0)
    return usage();

  if (i == 1)
    {
      fprintf(stderr, "%s: no attributes specified\n", argv0);
      return 1;
    }

  vol = hfs_remount(hcwd_getvol(-1), O_RDWR);
  if (vol == 0)
    return 1;

  fargv = hfs_glob(vol, argc - i, &argv[i], &fargc);
  if (fargv == 0)
    {
      fprintf(stderr, "%s: globbing error\n", argv0);
      result = 1;
    }
  else
    {
      for (i = 0; i < fargc; ++i)
	{
	  hfsdirent ent;

	  if (hfs_stat(vol, fargv[i], &ent) < 0)
	    {
	      hfs_perrorp(fargv[i]);
	      result = 1;
	    }
	  else
	    {
	      if (! (ent.flags & HFS_ISDIR))
		{
		  if (type)
		    memcpy(ent.u.file.type, type, 4);
		  if (crea)
		    memcpy(ent.u.file.creator, crea, 4);
		}

	      if (invis < 0)
		ent.fdflags &= ~HFS_FNDR_ISINVISIBLE;
	      else if (invis > 0)
		ent.fdflags |= HFS_FNDR_ISINVISIBLE;

	      if (lock < 0)
		ent.flags &= ~HFS_ISLOCKED;
	      else if (lock > 0)
		ent.flags |= HFS_ISLOCKED;

	      if (hfs_setattr(vol, fargv[i], &ent) < 0)
		{
		  hfs_perrorp(fargv[i]);
		  result = 1;
		}
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
