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
#define STDIN_FILENO _fileno(stdin)
#else
# include <unistd.h>
#endif

#ifdef __EMX__
# include <sys/types.h>
#endif
# include <sys/stat.h>
# include <fcntl.h>
# include <string.h>

# include "libhfs\hfs.h"
# include "libhfs\data.h"
# include "copyin.h"
# include "binhex.h"
# include "crc.h"
#ifdef __EMX__
# include "io.h"
#endif

#include "..\adouble.h"

extern int crlf_translation;

char *cpi_error = "no error";

#define MACB_BLOCKSZ	128

#define TEXT_TYPE	"TEXT"
#define TEXT_CREA	"UNIX"

#define RAW_TYPE	"????"
#define RAW_CREA	"UNIX"

void pc_to_mac_charset( unsigned char *s );

static char raw_type[100];
static char raw_crea[100];
static int strip_extension = 0;

void copyin_set_raw_param( char *type, char *creator, int strip )
{
	if(type && creator) {
		strcpy( raw_type, type );
		strcpy( raw_crea, creator );
		strip_extension = strip;
	} else {
		strcpy( raw_type, RAW_TYPE );
		strcpy( raw_crea, RAW_CREA );
		strip_extension = 0;
	}
}

/* Copy routines =========================================================== */

/*
 * NAME:	do_macb()
 * DESCRIPTION:	perform copy using MacBinary II translation
 */
static
int do_macb(int ifile, hfsfile *ofile, long dsize, long rsize)
{
  char buf[HFS_BLOCKSZ * 4];
  long bytes, chunk;

  cpi_error = "error reading data";

  /* data fork */

  while (dsize)
    {
      if (dsize < sizeof(buf))
	chunk = (dsize + (MACB_BLOCKSZ - 1)) & ~(MACB_BLOCKSZ - 1);
      else
	chunk = sizeof(buf);

      bytes = read(ifile, buf, chunk);
      if (bytes < chunk)
	return -1;

      if (dsize > bytes)
	chunk = bytes;
      else
	chunk = dsize;

      if (hfs_write(ofile, buf, chunk) < 0)
	{
	  cpi_error = hfs_error;
	  return -1;
	}

      dsize -= chunk;
    }

  /* resource fork */

  if (hfs_setfork(ofile, 1) < 0)
    {
      cpi_error = hfs_error;
      return -1;
    }

  while (rsize)
    {
      if (rsize < sizeof(buf))
	chunk = (rsize + (MACB_BLOCKSZ - 1)) & ~(MACB_BLOCKSZ - 1);
      else
	chunk = sizeof(buf);

      bytes = read(ifile, buf, chunk);
      if (bytes < chunk)
	return -1;

      if (rsize > bytes)
	chunk = bytes;
      else
	chunk = rsize;

      if (hfs_write(ofile, buf, chunk) < 0)
	{
	  cpi_error = hfs_error;
	  return -1;
	}

      rsize -= chunk;
    }

  return 0;
}

/*
 * NAME:	do_binh()
 * DESCRIPTION:	perform copy using BinHex translation
 */
static
int do_binh(hfsfile *ofile, long dsize, long rsize)
{
  char buf[HFS_BLOCKSZ * 4];
  long bytes;

  /* data fork */

  while (dsize)
    {
      if (dsize > sizeof(buf))
	bytes = sizeof(buf);
      else
	bytes = dsize;

      if (bh_read(buf, bytes) < bytes)
	{
	  cpi_error = bh_error;
	  return -1;
	}

      if (hfs_write(ofile, buf, bytes) < 0)
	{
	  cpi_error = hfs_error;
	  return -1;
	}

      dsize -= bytes;
    }

  if (bh_readcrc() < 0)
    {
      cpi_error = bh_error;
      return -1;
    }

  /* resource fork */

  if (hfs_setfork(ofile, 1) < 0)
    {
      cpi_error = hfs_error;
      return -1;
    }

  while (rsize)
    {
      if (rsize > sizeof(buf))
	bytes = sizeof(buf);
      else
	bytes = rsize;

      if (bh_read(buf, bytes) < bytes)
	{
	  cpi_error = bh_error;
	  return -1;
	}

      if (hfs_write(ofile, buf, bytes) < 0)
	{
	  cpi_error = hfs_error;
	  return -1;
	}

      rsize -= bytes;
    }

  if (bh_readcrc() < 0)
    {
      cpi_error = bh_error;
      return -1;
    }

  return 0;
}

/*
 * NAME:	do_text()
 * DESCRIPTION:	perform copy using text translation
 */
static
int do_text(int ifile, hfsfile *ofile)
{
  char buf[HFS_BLOCKSZ * 4], *ptr;
  long bytes;

  while (1) {
    bytes = read(ifile, buf, sizeof(buf));
    if (bytes < 0)
		{
		  cpi_error = "error reading source file";
			return -1;
		}
    else if (bytes == 0)
			break;
    for (ptr = buf; ptr < buf + bytes; ++ptr) {
			if (*ptr == '\n')
			  *ptr = '\r';
		}
    if (hfs_write(ofile, buf, bytes) < 0)
		{
		  cpi_error = hfs_error;
			return -1;
		}
  }
  return 0;
}

#ifndef MAXLONG
#define MAXLONG     0x7fffffff  
#endif

/*
 * NAME:	do_raw()
 * DESCRIPTION:	perform copy using no translation
 */
static
int do_raw(int ifile, hfsfile *ofile,long copy_count)
{
  char buf[HFS_BLOCKSZ * 4];
  long bytes, howmucho;

  while (copy_count > 0) {
		howmucho = sizeof(buf);
		if(howmucho > copy_count) howmucho = copy_count;
    bytes = read(ifile, buf, howmucho);
    if (bytes < 0)
		{
			cpi_error = "error reading source file";
			return -1;
		}
		else if (bytes == 0)
			break;
		copy_count -= bytes;
    if (hfs_write(ofile, buf, bytes) < 0)
		{
			cpi_error = hfs_error;
			return -1;
		}
  }

  return 0;
}

/* Utility Routines ======================================================== */

/*
 * NAME:	opensrc()
 * DESCRIPTION:	open the source file; set hint for destination filename
 */
static
int opensrc(char *srcname, char **dsthint, char *ext)
{
  int fd, len;
  static char name[HFS_MAX_FLEN + 1], *ptr;

  if (strcmp(srcname, "-") == 0)
    {
      fd = dup(STDIN_FILENO);
#ifdef __EMX__
      /* We open the file in binary mode except when using text translation */
      if(ext==NULL || strcmp(ext, ".txt")) {
	if(setmode(fd, O_BINARY) == -1) {
	  cpi_error = "cannot change file mode";
	  return -1;
	}
      }
#endif
      srcname = "";
    }
  else
#ifdef __EMX__
    if(ext==NULL || strcmp(ext, ".txt"))
      fd = open(srcname, O_BINARY | O_RDONLY);
    else
      fd = open(srcname, O_RDONLY);
#else
    fd = open(srcname, O_RDONLY);
#endif
  if (fd < 0)
    {
      cpi_error = "error opening source file";
      return -1;
    }

#ifdef __EMX__
  /* Skip drive letter */
  if(srcname[1]==':')
    srcname += 2;

  /* Change '\' to '/' */
  {
    char *p = srcname;
    while (*p) {
      if (*p == '\\')
	*p = '/';
      p++;
    }
  }      
#endif
  ptr = strrchr(srcname, '/');
  if (ptr == 0)
    ptr = srcname;
  else
    ++ptr;

  if (ext == 0)
    len = strlen(ptr);
  else
    {
      ext = strstr(ptr, ext);
      if (ext == 0)
	len = strlen(ptr);
      else
	len = ext - ptr;
    }

  if (len > HFS_MAX_FLEN)
    len = HFS_MAX_FLEN;

  memcpy(name, ptr, len);
  name[len] = 0;

	if(strcmp(name,"Icon~") == 0) {
		name[4] = '\x00D';
	} else {
		pc_to_mac_charset( (unsigned char *)name );
	}

  for (ptr = name; *ptr; ++ptr)
    {
      switch (*ptr)
	{
	case ':':
	  *ptr = '-';
	  break;

	/*
	case '_':
	  *ptr = ' ';
	  break;
	*/
	}
    }

  *dsthint = name;

  return fd;
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
	  cpi_error = hfs_error;
	  return 0;
	}

      dstname = hint;
    }

  hfs_delete(vol, dstname);
  if (hfs_create(vol, dstname, type, creator) < 0)
    {
      cpi_error = hfs_error;

      if (dstname == hint)
	hfs_setcwd(vol, cwd);

      return 0;
    }

  file = hfs_open(vol, dstname);
  if (file == 0)
    {
      cpi_error = hfs_error;

      if (dstname == hint)
	hfs_setcwd(vol, cwd);

      return 0;
    }

  if (dstname == hint)
    {
      if (hfs_setcwd(vol, cwd) < 0)
	{
	  cpi_error = hfs_error;
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
void closefiles(int ifile, hfsfile *ofile, int *result)
{
  if (ofile && hfs_close(ofile) < 0 && *result == 0)
    {
      cpi_error = hfs_error;
      *result = -1;
    }

  if (close(ifile) < 0 && *result == 0)
    {
      cpi_error = "error closing source file";
      *result = -1;
    }
}

/* Interface Routines ====================================================== */

/*
 * NAME:	cpi->macb()
 * DESCRIPTION:	copy a UNIX file to an HFS file using MacBinary II translation
 */
int cpi_macb(char *srcname, hfsvol *vol, char *dstname)
{
  int ifile, result = 0;
  hfsfile *ofile;
  hfsdirent ent;
  char *dsthint, type[5], creator[5];
  unsigned char buf[MACB_BLOCKSZ];
  unsigned short crc;
  long dsize, rsize;

  ifile = opensrc(srcname, &dsthint, ".bin");
  if (ifile < 0)
    return -1;

  if (read(ifile, buf, MACB_BLOCKSZ) < MACB_BLOCKSZ)
    {
      cpi_error = "error reading MacBinary file header";
      close(ifile);
      return -1;
    }

  if (buf[0] != 0 || buf[74] != 0)
    {
      cpi_error = "invalid MacBinary file header";
      close(ifile);
      return -1;
    }

  crc = d_getw(&buf[124]);

  if (crc_macb(buf, 124, 0x0000) != crc)
    {
      /* (buf[82] == 0) => MacBinary I? */

      cpi_error = "unknown, unsupported, or corrupt MacBinary file";

      close(ifile);
      return -1;
    }

  if (buf[123] > 129)
    {
      cpi_error = "unsupported MacBinary file version";
      close(ifile);
      return -1;
    }

  if (buf[1] < 1 || buf[1] > 63 ||
      buf[2 + buf[1]] != 0)
    {
      cpi_error = "invalid MacBinary file header (bad file name)";
      close(ifile);
      return -1;
    }

  dsize = d_getl(&buf[83]);
  rsize = d_getl(&buf[87]);

  if (dsize < 0 || dsize > 0x7fffffff ||
      rsize < 0 || rsize > 0x7fffffff)
    {
      cpi_error = "invalid MacBinary file header (bad file length)";
      close(ifile);
      return -1;
    }

  dsthint = (char *) &buf[2];

  memcpy(type,    &buf[65], 4);
  memcpy(creator, &buf[69], 4);
  type[4] = creator[4] = 0;

  ofile = opendst(vol, dstname, dsthint, type, creator);
  if (ofile == 0)
    {
      close(ifile);
      return -1;
    }

  result = do_macb(ifile, ofile, dsize, rsize);

  if (result == 0 && hfs_fstat(ofile, &ent) < 0)
    {
      cpi_error = hfs_error;
      result = -1;
    }

  ent.fdflags = (buf[73] << 8 | buf[101]) &
    ~(HFS_FNDR_ISONDESK | HFS_FNDR_HASBEENINITED | HFS_FNDR_RESERVED);

  ent.crdate = d_toutime(d_getl(&buf[91]));
  ent.mddate = d_toutime(d_getl(&buf[95]));

  if (result == 0 && hfs_fsetattr(ofile, &ent) < 0)
    {
      cpi_error = hfs_error;
      result = -1;
    }

  closefiles(ifile, ofile, &result);

  return result;
}

/*
 * NAME:	binhx()
 * DESCRIPTION:	auxiliary BinHex routine
 */
static
int binhx(char *fname, char *type, char *creator, short *fdflags,
	  long *dsize, long *rsize)
{
  int len;
  unsigned char byte, word[2], lword[4];

  if (bh_read(&byte, 1) < 1)
    {
      cpi_error = bh_error;
      return -1;
    }

  len = (unsigned char) byte;

  if (len < 1 || len > HFS_MAX_FLEN)
    {
      cpi_error = "invalid BinHex file header (bad file name)";
      return -1;
    }

  if (bh_read(fname, len + 1) < len + 1)
    {
      cpi_error = bh_error;
      return -1;
    }

  if (fname[len] != 0)
    {
      cpi_error = "invalid BinHex file header (bad file name)";
      return -1;
    }

  if (bh_read(type, 4) < 4 ||
      bh_read(creator, 4) < 4 ||
      bh_read(word, 2) < 2)
    {
      cpi_error = bh_error;
      return -1;
    }
  *fdflags = d_getw(word);

  if (bh_read(lword, 4) < 4)
    {
      cpi_error = bh_error;
      return -1;
    }
  *dsize = d_getl(lword);

  if (bh_read(lword, 4) < 4)
    {
      cpi_error = bh_error;
      return -1;
    }
  *rsize = d_getl(lword);

  if (*dsize < 0 || *dsize > 0x7fffffff ||
      *rsize < 0 || *rsize > 0x7fffffff)
    {
      cpi_error = "invalid BinHex file header (bad file length)";
      return -1;
    }

  if (bh_readcrc() < 0)
    {
      cpi_error = bh_error;
      return -1;
    }

  return 0;
}

/*
 * NAME:	cpi->binh()
 * DESCRIPTION:	copy a UNIX file to an HFS file using BinHex translation
 */
int cpi_binh(char *srcname, hfsvol *vol, char *dstname)
{
  int ifile, result;
  hfsfile *ofile;
  hfsdirent ent;
  char *dsthint, fname[HFS_MAX_FLEN + 1], type[5], creator[5];
  short fdflags;
  long dsize, rsize;

  ifile = opensrc(srcname, &dsthint, ".hqx");
  if (ifile < 0)
    return -1;

  if (bh_open(ifile) < 0)
    {
      cpi_error = bh_error;
      close(ifile);
      return -1;
    }

  if (binhx(fname, type, creator, &fdflags, &dsize, &rsize) < 0)
    {
      bh_close();
      close(ifile);
      return -1;
    }

  dsthint = fname;

  ofile = opendst(vol, dstname, dsthint, type, creator);
  if (ofile == 0)
    {
      bh_close();
      close(ifile);
      return -1;
    }

  result = do_binh(ofile, dsize, rsize);

  if (bh_close() < 0 && result == 0)
    {
      cpi_error = bh_error;
      result = -1;
    }

  if (result == 0 && hfs_fstat(ofile, &ent) < 0)
    {
      cpi_error = hfs_error;
      result = -1;
    }

  ent.fdflags = fdflags &
    ~(HFS_FNDR_ISONDESK | HFS_FNDR_HASBEENINITED | HFS_FNDR_ISINVISIBLE);

  if (result == 0 && hfs_fsetattr(ofile, &ent) < 0)
    {
      cpi_error = hfs_error;
      result = -1;
    }

  closefiles(ifile, ofile, &result);

  return result;
}

/*
 * NAME:	cpi->text()
 * DESCRIPTION:	copy a UNIX file to an HFS file using text translation
 */
int cpi_text(char *srcname, hfsvol *vol, char *dstname)
{
  int ifile, result = 0;
  hfsfile *ofile;
  char *dsthint;

  ifile = opensrc(srcname, &dsthint, ".txt");
  if (ifile < 0)
    return -1;

  ofile = opendst(vol, dstname, dsthint, TEXT_TYPE, TEXT_CREA);
  if (ofile == 0)
    {
      close(ifile);
      return -1;
    }

	if(crlf_translation) {
		setmode(ifile, O_TEXT);
	  result = do_text(ifile, ofile);
	} else {
		setmode(ifile, O_BINARY);
	  result = do_raw(ifile, ofile, MAXLONG);
	}
  closefiles(ifile, ofile, &result);

  return result;
}

/*
 * NAME:	cpi->raw()
 * DESCRIPTION:	copy a UNIX file to the data fork of an HFS file
 */
int cpi_raw(char *srcname, hfsvol *vol, char *dstname)
{
  int ifile, result = 0;
  hfsfile *ofile;
  char *dsthint;
	char new_hint[256];

  ifile = opensrc(srcname, &dsthint, 0);
  if (ifile < 0)
    return -1;

	if(strip_extension && *dsthint) {
		char *p;
		strcpy( new_hint, dsthint );
		p = strrchr( new_hint, '.' );
		if(p) *p = 0;
		dsthint = new_hint;
		// duplicates? should check if the file exists and if so modify new_hint
	}

  ofile = opendst(vol, dstname, dsthint, raw_type, raw_crea);
  if (ofile == 0)
    {
      close(ifile);
      return -1;
    }

  result = do_raw(ifile, ofile, MAXLONG);
  closefiles(ifile, ofile, &result);

  return result;
}

// LAURI
int cpi_raw_resource(char *srcname, hfsvol *vol, char *dstname)
{
  int ifile, result = 0;
  hfsfile *ofile;
  char *dsthint;

  ifile = opensrc(srcname, &dsthint, 0);
  if (ifile < 0)
    return -1;

  ofile = opendst(vol, dstname, dsthint, raw_type, raw_crea);
  if (ofile == 0) {
      close(ifile);
      return -1;
  }

	if(hfs_setfork(ofile, 1) < 0) {
		result = -1;
	} else {
	  result = do_raw(ifile, ofile, MAXLONG);
	}

  closefiles(ifile, ofile, &result);

  return result;
}

int cpi_double( char *fdata, char *fres, hfsvol *vol, char *dstname )
{
  int ifile0, ifile1, result = 0;
  hfsfile *ofile;
  char *dsthint;
	char typestr[5], creatorstr[5];
	unsigned long rstart, rlength;
  hfsdirent ent;

	if(!get_apple_double_type_creator( 
		fres, typestr, creatorstr, &rstart, &rlength ))
	{
    return -1;
	}

	// Open %file first so we get hint from real name
	ifile1 = opensrc( fres, &dsthint, 0 );
  if (ifile1 == 0)
    return -1;

	ifile0 = opensrc( fdata, &dsthint, 0 );
  if (ifile0 == 0) {
		close(ifile1);
    return -1;
	}

	remove_ardi_special_chars( dsthint );
  ofile = opendst( vol, dstname, dsthint, typestr, creatorstr );
  if (ofile == 0) {
    close(ifile0);
    close(ifile1);
    return -1;
  }

  if(hfs_setfork(ofile, 0) < 0) {
    cpi_error = hfs_error;
		result = -1;
	} else {
	  result = do_raw(ifile0, ofile, MAXLONG);
		if(result == 0) {
		  if(hfs_setfork(ofile, 1) < 0) {
				cpi_error = hfs_error;
				result = -1;
			} else {
				lseek( ifile1, rstart, SEEK_SET );
				result = do_raw(ifile1, ofile, rlength);
			}
		}
	}

  if (result == 0 && hfs_fstat(ofile, &ent) < 0) {
    cpi_error = hfs_error;
    result = -1;
  }

  ent.fdflags &= 
    ~(HFS_FNDR_ISONDESK | HFS_FNDR_HASBEENINITED | HFS_FNDR_ISINVISIBLE);

  if (result == 0 && hfs_fsetattr(ofile, &ent) < 0) {
		cpi_error = hfs_error;
		result = -1;
  }

  closefiles(ifile0, ofile, &result);
  closefiles(ifile1, NULL, &result);

  return result;
}
