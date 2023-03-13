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
#ifdef __EMX__
# include <ctype.h>
# include <sys/types.h>
#endif
# include <sys/stat.h>

#ifdef WIN32
# include <io.h>
# include <stdio.h>
#define S_ISDIR(x) ( ((x) & _S_IFDIR) != 0 )
//  not needed
int getopt( int argc, char *argv[], char *str ) {return(0);}
#else
# include <unistd.h>
#endif

# include <fcntl.h>
# include <errno.h>

# include "libhfs\hfs.h"
# include "hcwd.h"
# include "hfsutil.h"
# include "glob.h"
# include "hcopy.h"
# include "copyin.h"
# include "copyout.h"
#ifdef __EMX__
# include "os2lib.h"
#endif

extern int optind;

typedef int (*cpifunc)(char *, hfsvol *, char *);
typedef int (*cpofunc)(hfsvol *, char *, char *);

/*
 * NAME:	ufs->automode()
 * DESCRIPTION:	automatically choose copyin transfer mode
 */

static
cpifunc ufs_automode(char *path)
{
  int i;
  struct {
    char *ext;
    cpifunc func;
  } exts[] = {
    { ".bin", cpi_macb },
    { ".hqx", cpi_binh },

    { ".txt", cpi_text },
    { ".c",   cpi_text },
    { ".h",   cpi_text },

    { 0,      0        }
  };

  path += strlen(path);

  for (i = 0; exts[i].ext; ++i)
    {
      if (strcasecmp(path - strlen(exts[i].ext), exts[i].ext) == 0)
	return exts[i].func;
    }

  return cpi_raw;
}

/*
 * NAME:	do_copyin()
 * DESCRIPTION:	copy files from UNIX to HFS
 */
#ifndef WIN32
static
#endif
int do_copyin(hfsvol *vol, int argc, char *argv[], char *dest, int mode)
{
  hfsdirent ent;
  cpifunc copyfile = 0;
  int i, result = 0;

  if (argc > 1 && (hfs_stat(vol, dest, &ent) < 0 ||
		   ! (ent.flags & HFS_ISDIR)))
    {
      ERROR(ENOTDIR, 0);
      hfs_perrorp(dest);

      return 1;
    }

  switch (mode)
    {
    case 'm':
      copyfile = cpi_macb;
      break;

    case 'b':
      copyfile = cpi_binh;
      break;

    case 't':
      copyfile = cpi_text;
      break;

    case 'r':
      copyfile = cpi_raw;
      break;

    case 'R':
      copyfile = cpi_raw_resource;
      break;
    }

  for (i = 0; i < argc; ++i)
    {
      if (mode == 'a')
	copyfile = ufs_automode(argv[i]);

      if (copyfile(argv[i], vol, dest) < 0)
	{
	  ERROR(0, cpi_error);
	  hfs_perrorp(argv[i]);

	  result = 1;
	}
    }

  return result;
}

/*
 * NAME:	hfs->automode()
 * DESCRIPTION:	automatically choose copyout transfer mode
 */
static
cpofunc hfs_automode(hfsvol *vol, char *path)
{
  hfsdirent ent;

  if (hfs_stat(vol, path, &ent) < 0)
    return cpo_macb;

  if (strcmp(ent.u.file.type, "TEXT") == 0 ||
      strcmp(ent.u.file.type, "ttro") == 0)
    return cpo_text;
  else if (ent.u.file.rsize == 0)
    return cpo_raw;

  return cpo_macb;
}

/*
 * NAME:	do_copyout()
 * DESCRIPTION:	copy files from HFS to UNIX
 */
#ifndef WIN32
static
#endif
int do_copyout(hfsvol *vol, int argc, char *argv[], char *dest, int mode)
{
  struct stat sbuf;
  cpofunc copyfile;
  int i, result = 0;

  if (argc > 1 && (stat(dest, &sbuf) < 0 ||
		   ! S_ISDIR(sbuf.st_mode)))
    {
      ERROR(ENOTDIR, 0);
      hfs_perrorp(dest);

      return 1;
    }

  switch (mode)
    {
    case 'm':
      copyfile = cpo_macb;
      break;

    case 'b':
      copyfile = cpo_binh;
      break;

    case 't':
      copyfile = cpo_text;
      break;

    case 'r':
      copyfile = cpo_raw;
      break;

    case 'R':
      copyfile = cpo_raw_resource;
      break;
		}

  for (i = 0; i < argc; ++i)
    {
      if (mode == 'a')
	copyfile = hfs_automode(vol, argv[i]);

      if (copyfile(vol, argv[i], dest) < 0)
	{
	  ERROR(0, cpo_error);
	  hfs_perrorp(argv[i]);

	  result = 1;
	}
    }

  return result;
}

/*
 * NAME:	usage()
 * DESCRIPTION:	display usage message
 */
static
int usage(void)
{
  fprintf(stderr, "Usage: %s [-m|-b|-t|-r|-a] source-path [...] target-path\n",
	  argv0);

  return 1;
}

#ifndef WIN32
/*
 * NAME:	hcopy->main()
 * DESCRIPTION:	implement hcopy command
 */
int hcopy_main(int argc, char *argv[])
{
  int nargs, mode = 'a', result;
  char *target;
  int fargc;
  char **fargv;
  hfsvol *vol;
  int (*copy)(hfsvol *, int, char *[], char *, int);

  while (1)
    {
      int opt;

      opt = getopt(argc, argv, "mbtra");
      if (opt == EOF)
	break;

      switch (opt)
	{
	case '?':
	  return usage();

	default:
	  mode = opt;
	}
    }

  nargs = argc - optind;

  if (nargs < 2)
    return usage();

  target = argv[argc - 1];

#ifdef __EMX__
  /* Allow destination filename to be d:path */
  if (strchr(target, ':') && target[0] != '.' && target[0] != '/' &&
      !(strlen(target)>=2 && target[1]==':' && 
	isalpha(target[0]) && !strchr(target+2, ':')))
#else
  if (strchr(target, ':') && target[0] != '.' && target[0] != '/')
#endif
    {
      vol = hfs_remount(hcwd_getvol(-1), O_RDWR);
      if (vol == 0)
	return 1;

      copy  = do_copyin;
      fargc = nargs - 1;
      fargv = &argv[optind];
    }
  else
    {
      vol = hfs_remount(hcwd_getvol(-1), O_RDONLY);
      if (vol == 0)
	return 1;

      copy  = do_copyout;
      fargv = hfs_glob(vol, nargs - 1, &argv[optind], &fargc);
    }

  if (fargv == 0)
    {
      fprintf(stderr, "%s: globbing error\n", argv0);
      result = 1;
    }
  else
    result = copy(vol, fargc, fargv, target, mode);

  if (hfs_umount(vol) < 0 && result == 0)
    {
      hfs_perror("Error closing HFS volume");
      result = 1;
    }

  if (fargv && fargv != &argv[optind])
    free(fargv);

  return result;
}
#endif
