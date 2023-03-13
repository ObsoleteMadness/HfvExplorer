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

# include <stdio.h>
# include <string.h>
# include <fcntl.h>

# include "hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "hvol.h"
#ifdef __EMX__
# include "os2lib.h"
#endif

/*
 * NAME:	hvol->main()
 * DESCRIPTION:	implement hvol command
 */
int hvol_main(int argc, char *argv[])
{
  int vnum;
  mountent *ment;
  hfsvol *vol;
  hfsvolent vent;

  if (argc > 2)
    {
      fprintf(stderr, "Usage: %s [volume-name-or-path]\n", argv0);
      return 1;
    }

  if (argc == 1)
    {
      int output = 0, header = 0;

      ment = hcwd_getvol(-1);
      if (ment)
	{
	  printf("Current volume is mounted from %s\n", ment->path);
	  output = 1;

	  vol = hfs_remount(ment, O_RDWR);
	  if (vol)
	    {
	      hfs_vstat(vol, &vent);
	      hfs_pinfo(&vent);
	      hfs_umount(vol);
	    }
	}

      for (vnum = 0; ; ++vnum)
	{
	  mountent *ent;

	  ent = hcwd_getvol(vnum);
	  if (ent == 0)
	    break;

	  if (ent == ment)
	    continue;

	  if (header == 0)
	    {
	      printf("%s volumes:\n", ment ? "\nOther known" : "Known");
	      header = 1;
	    }

	  if (ent->partno <= 1)
	    printf("  %-20s     %s\n", ent->path, ent->vname);
	  else
	    printf("  %-20s %2d  %s\n", ent->path, ent->partno, ent->vname);

	  output = 1;
	}

      if (output == 0)
	printf("No known volumes; use `hmount' to introduce new volumes\n");

      return 0;
    }

  for (ment = hcwd_getvol(vnum = 0); ment; ment = hcwd_getvol(++vnum))
    {
      if (strcmp(argv[1], ment->path) == 0 ||
	  strcasecmp(argv[1], ment->vname) == 0)
	{
	  hfsvol *vol;

	  printf("Current volume is mounted from %s\n", ment->path);

	  hcwd_setvol(vnum);
	  vol = hfs_remount(ment, O_RDWR);
	  if (vol == 0)
	    return 1;

	  hfs_vstat(vol, &vent);
	  hfs_pinfo(&vent);

	  if (hfs_umount(vol) < 0)
	    {
	      hfs_perror("Error closing HFS volume");
	      return 1;
	    }

	  return 0;
	}
    }

  fprintf(stderr, "%s: Unknown volume \"%s\"\n", argv0, argv[1]);

  return 1;
}
