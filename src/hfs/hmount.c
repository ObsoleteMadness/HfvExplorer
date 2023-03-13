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
# include "suid.h"
# include "hmount.h"

/*
 * NAME:	hmount->main()
 * DESCRIPTION:	implement hmount command
 */
int hmount_main(int argc, char *argv[])
{
  char *path;
  hfsvol *vol;
  hfsvolent ent;
  int partno, result = 0;

  if (argc < 2 || argc > 3)
    {
      fprintf(stderr, "Usage: %s source-path [partition-no]\n", argv0);
      return 1;
    }

  path   = argv[1];
  partno = (argc == 3) ? atoi(argv[2]) : 1;

  suid_enable();
  vol = hfs_mount(path, partno, O_RDWR);
  suid_disable();

  if (vol == 0)
    {
      hfs_perror(path);
      return 1;
    }

  hfs_vstat(vol, &ent);
  hfs_pinfo(&ent);

  if (hcwd_mounted(ent.name, ent.crdate, path, partno) < 0)
    {
      perror("Failed to record mount");
      result = 1;
    }

  if (hfs_umount(vol) < 0 && result == 0)
    {
      hfs_perror("Error closing HFS volume");
      result = 1;
    }

  return result;
}
