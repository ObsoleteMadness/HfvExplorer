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
# include <stdlib.h>
# include <string.h>
# include <time.h>
# include <errno.h>
# include <ctype.h>

# include "libhfs\hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "suid.h"
# include "version.h"

# include "hattrib.h"
# include "hcd.h"
# include "hcopy.h"
# include "hdel.h"
# include "hformat.h"
# include "hls.h"
# include "hmkdir.h"
# include "hmount.h"
# include "hpwd.h"
# include "hrename.h"
# include "hrmdir.h"
# include "humount.h"
# include "hvol.h"

char *argv0, *bargv0;

/*
 * NAME:	main()
 * DESCRIPTION:	program entry dispatch
 */
int main(int argc, char *argv[])
{
  int i;

  struct {
    char *name;
    int (*func)(int, char *[]);
  } list[] = {
    { "hattrib", hattrib_main },
    { "hcd",     hcd_main     },
    { "hcopy",   hcopy_main   },
    { "hdel",    hdel_main    },
    { "hdir",    hls_main     },
    { "hformat", hformat_main },
    { "hls",     hls_main     },
    { "hmkdir",  hmkdir_main  },
    { "hmount",  hmount_main  },
    { "hpwd",    hpwd_main    },
    { "hrename", hrename_main },
    { "hrmdir",  hrmdir_main  },
    { "humount", humount_main },
    { "hvol",    hvol_main    },
    { 0,         0            }
  };

  suid_init(0);

  if (argc == 2)
    {
      if (strcmp(argv[1], "--version") == 0)
	{
#ifdef __EMX__
	  printf("%s\n%s\n", VERSION, COPYRIGHT);
#else
	  printf("%s - %s\n", VERSION, COPYRIGHT);
#endif
	  printf("`%s --license' for licensing information.\n", argv[0]);
	  return 0;
	}
      else if (strcmp(argv[1], "--license") == 0)
	{
	  printf("\n%s", LICENSE);
	  return 0;
	}
    }

  argv0 = argv[0];

#ifdef __EMX__
  strlwr(argv0);
  bargv0 = strrchr(argv0, '\\');
#else
  bargv0 = strrchr(argv0, '/');
#endif  
  if (bargv0 == 0)
    bargv0 = argv0;
  else
    ++bargv0;
#ifdef __EMX__
  _remext(bargv0);
#endif

  for (i = 0; list[i].name; ++i)
    {
      if (strcmp(bargv0, list[i].name) == 0)
	{
	  int result;

	  if (hcwd_init() < 0)
	    {
	      perror("Failed to initialize HFS working directories");
	      return 1;
	    }

	  result = list[i].func(argc, argv);

	  if (hcwd_finish() < 0)
	    {
	      perror("Failed to save working directory state");
	      return 1;
	    }

	  return result;
	}
    }

  fprintf(stderr, "%s: Unknown operation `%s'\n", argv0, bargv0);
  return 1;
}

/*
 * NAME:	hfs->perror()
 * DESCRIPTION:	output an HFS error
 */
void hfs_perror(char *msg)
{
  char *str = strerror(errno);

  if (hfs_error == 0)
    fprintf(stderr, "%s: %s: %c%s\n", argv0, msg, tolower(*str), str + 1);
  else
    {
      if (errno)
	fprintf(stderr, "%s: %s: %s (%s)\n", argv0, msg, hfs_error, str);
      else
	fprintf(stderr, "%s: %s: %s\n", argv0, msg, hfs_error);
    }
}

/*
 * NAME:	hfs_perrorp()
 * DESCRIPTION:	output an HFS error for a pathname
 */
void hfs_perrorp(char *path)
{
  char *str = strerror(errno);

  if (hfs_error == 0)
    fprintf(stderr, "%s: \"%s\": %c%s\n", argv0, path, tolower(*str), str + 1);
  else
    {
      if (errno)
	fprintf(stderr, "%s: \"%s\": %s (%s)\n", argv0, path, hfs_error, str);
      else
	fprintf(stderr, "%s: \"%s\": %s\n", argv0, path, hfs_error);
    }
}

/*
 * NAME:	hfs->remount()
 * DESCRIPTION:	mount a volume as though it were still mounted
 */
hfsvol *hfs_remount(mountent *ment, int flags)
{
  hfsvol *vol;
  hfsvolent vent;

  if (ment == 0)
    {
      fprintf(stderr, "%s: No volume is current; use `hmount' or `hvol'\n",
	      argv0);
      return 0;
    }

  suid_enable();
  vol = hfs_mount(ment->path, ment->partno, flags);
  suid_disable();

  if (vol == 0)
    {
      hfs_perror(ment->path);
      return 0;
    }

  hfs_vstat(vol, &vent);

  if (strcmp(vent.name, ment->vname) != 0)
    {
      fprintf(stderr, "%s: Expected volume \"%s\" not found\n",
	      argv0, ment->vname);
      fprintf(stderr, "%s: Replace media on %s or use `hmount'\n",
	      argv0, ment->path);

      hfs_umount(vol);
      return 0;
    }

  if (hfs_chdir(vol, ment->cwd) < 0)
    {
      fprintf(stderr, "%s: Current HFS directory \"%s%s:\" no longer exists\n",
	      argv0, ment->vname, ment->cwd);
    }

  return vol;
}

/*
 * NAME:	hfs->pinfo()
 * DESCRIPTION:	print information about a volume
 */
void hfs_pinfo(hfsvolent *ent)
{
  printf("Volume name is \"%s\"%s\n", ent->name,
	 (ent->flags & HFS_ISLOCKED) ? " (locked)" : "");
  printf("Volume was created on %s", ctime(&ent->crdate));
  printf("Volume was last modified on %s", ctime(&ent->mddate));
  printf("Volume has %lu bytes free\n", ent->freebytes);
}

/*
 * NAME:	hfs->getcwd()
 * DESCRIPTION:	return full path to current directory
 */
char *hfs_cwd(hfsvol *vol)
{
  char *path, name[33];
  long cwd;
  int pathlen;

  path    = malloc(1);
  path[0] = 0;
  pathlen = 0;
  cwd     = hfs_getcwd(vol);

  while (cwd != HFS_CNID_ROOTPAR)
    {
      char *new;
      int namelen, i;

      if (hfs_dirinfo(vol, &cwd, name) < 0)
	return 0;

      if (pathlen)
	strcat(name, ":");

      namelen = strlen(name);

      new = realloc(path, namelen + pathlen + 1);
      if (new == 0)
	{
	  free(path);
	  ERROR(ENOMEM, 0);
	  return 0;
	}

      if (pathlen == 0)
	new[0] = 0;

      path = new;

      /* push string down to make room for path prefix (memmove()-ish) */

      i = pathlen + 1;
      for (new = path + namelen + pathlen; i--; new--)
	*new = *(new - namelen);

      memcpy(path, name, namelen);

      pathlen += namelen;
    }

  return path;
}
