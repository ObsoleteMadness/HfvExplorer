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

# include <stdio.h>
# include <string.h>

# include "binhex.h"
# include "crc.h"

char *bh_error = "no error";

static FILE *file;			/* input/output file */
static char line[67];			/* ASCII line buffer */
static int lptr;			/* next char in line buffer */
static int state86;			/* 8->6 encoding state */
static unsigned char lastch;		/* last encoded data byte */
static int runlen;			/* runlength of last data byte */
static unsigned short crc;		/* incremental CRC word */

static
unsigned char zero[2] = { 0, 0 };

static
char hqxheader[] = "(This file must be converted with BinHex 4.0)\n";

static
char enmap[] = "!\"#$%&'()*+,-012345689@ABCDEFGHI"
	       "JKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

static
signed char demap[256] = {
   0,  0,  0,  0,  0,  0,  0,  0,
   0, -1, -1,  0,  0, -1,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,
  -1,  1,  2,  3,  4,  5,  6,  7,
   8,  9, 10, 11, 12, 13,  0,  0,
  14, 15, 16, 17, 18, 19, 20,  0,
  21, 22,  0,  0,  0,  0,  0,  0,
  23, 24, 25, 26, 27, 28, 29, 30,
  31, 32, 33, 34, 35, 36, 37,  0,
  38, 39, 40, 41, 42, 43, 44,  0,
  45, 46, 47, 48,  0,  0,  0,  0,
  49, 50, 51, 52, 53, 54, 55,  0,
  56, 57, 58, 59, 60, 61,  0,  0,
  62, 63, 64,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0
};

# define HEADERMATCH	40
# define MAXLINELEN	64
# define ISRETURN(c)	(demap[(unsigned char) (c)] == -1)

/* BinHex Encoding ========================================================= */

/*
 * NAME:	bh->start()
 * DESCRIPTION:	begin BinHex encoding
 */
int bh_start(int fd)
{
  int dupfd;

  dupfd = dup(fd);
  if (dupfd < 0)
    {
      bh_error = "error duplicating output stream";
      return -1;
    }

#ifdef __EMX__
  file = fdopen(dupfd, "ab");
#else
  file = fdopen(dupfd, "a");
#endif
  if (file == 0)
    {
      close(dupfd);
      bh_error = "error creating output buffer";
      return -1;
    }

  line[0] = ':';
  lptr = 1;

  state86 = 0;
  runlen  = 0;

  crc = 0x0000;

  if (fputs(hqxheader, file) < 0)
    {
      fclose(file);
      bh_error = "error writing hqx header";
      return -1;
    }

  return 0;
}

/*
 * NAME:	flushline()
 * DESCRIPTION:	flush a line to the output file
 */
static
int flushline(void)
{
  line[lptr++] = '\n';
  line[lptr++] = 0;

  if (fputs(line, file) < 0)
    {
      bh_error = "error writing output data";
      return -1;
    }

  lptr = 0;

  return 0;
}

/*
 * NAME:	addchars()
 * DESCRIPTION:	insert bytes of data to the output stream
 */
static
int addchars(unsigned char *data, register int len)
{
  register unsigned char c;

  while (len--)
    {
      c = *data++;

      if (lptr == MAXLINELEN &&
	  flushline() < 0)
	return -1;

      switch (state86 & 0xff00)
	{
	case 0x0000:
	  line[lptr++] = enmap[c >> 2];
	  state86 = 0x0100 | (c & 0x03);
	  break;

	case 0x0100:
	  line[lptr++] = enmap[((state86 & 0x03) << 4) | (c >> 4)];
	  state86 = 0x0200 | (c & 0x0f);
	  break;

	case 0x0200:
	  line[lptr++] = enmap[((state86 & 0x0f) << 2) | (c >> 6)];

	  if (lptr == MAXLINELEN &&
	      flushline() < 0)
	    return -1;

	  line[lptr++] = enmap[c & 0x3f];
	  state86 = 0;
	  break;
	}
    }

  return 0;
}

/*
 * NAME:	rleflush()
 * DESCRIPTION:	run-length encode data
 */
static
int rleflush(void)
{
  unsigned char rle[] = { 0x90, 0x00, 0x90, 0x00 };

  if ((lastch != 0x90 && runlen < 4) ||
      (lastch == 0x90 && runlen < 3))
    {
      /* self representation */

      if (lastch == 0x90)
	{
	  while (runlen--)
	    if (addchars(rle, 2) < 0)
	      return -1;
	}
      else
	{
	  while (runlen--)
	    if (addchars(&lastch, 1) < 0)
	      return -1;
	}
    }
  else
    {
      /* run-length encoded */

      if (lastch == 0x90)
	{
	  rle[3] = runlen;

	  if (addchars(rle, 4) < 0)
	    return -1;
	}
      else
	{
	  rle[1] = lastch;
	  rle[3] = runlen;

	  if (addchars(&rle[1], 3) < 0)
	    return -1;
	}
    }

  runlen = 0;

  return 0;
}

/*
 * NAME:	bh->insert()
 * DESCRIPTION:	encode bytes of data, buffering lines and flushing
 */
int bh_insert(void *buf, register int len)
{
  register unsigned char *data = buf;

  crc = crc_binh(data, len, crc);

  for ( ; len--; ++data)
    {
      if (runlen)
	{
	  if (runlen == 0xff || lastch != *data)
	    {
	      if (rleflush() < 0)
		return -1;
	    }

	  if (lastch == *data)
	    {
	      ++runlen;
	      continue;
	    }
	}

      lastch = *data;
      runlen = 1;
    }

  return 0;
}

/*
 * NAME:	bh->insertcrc()
 * DESCRIPTION:	insert a two-byte CRC checksum
 */
int bh_insertcrc(void)
{
  unsigned char word[2];

  crc = crc_binh(zero, 2, crc);

  word[0] = (crc & 0xff00) >> 8;
  word[1] = (crc & 0x00ff) >> 0;

  if (bh_insert(word, 2) < 0)
    return -1;

  crc = 0x0000;

  return 0;
}

/*
 * NAME:	bh->end()
 * DESCRIPTION:	finish BinHex encoding
 */
int bh_end(void)
{
  int result = 0;

  if (runlen &&
      rleflush() < 0)
    result = -1;

  if (state86 && result == 0 &&
      addchars(zero, 1) < 0)
    result = -1;

  line[lptr++] = ':';

  if (result == 0 &&
      flushline() < 0)
    result = -1;

  if (fclose(file) < 0 && result == 0)
    {
      bh_error = "error flushing output data";
      result = -1;
    }

  return result;
}

/* BinHex Decoding ========================================================= */

/*
 * NAME:	bh->open()
 * DESCRIPTION:	begin BinHex decoding
 */
int bh_open(int fd)
{
  int dupfd, c;
  char *ptr;

  dupfd = dup(fd);
  if (dupfd < 0)
    {
      bh_error = "error duplicating input stream";
      return -1;
    }

#ifdef __EMX__
  file = fdopen(dupfd, "rb");
#else
  file = fdopen(dupfd, "r");
#endif
  if (file == 0)
    {
      close(dupfd);
      bh_error = "error creating input buffer";
      return -1;
    }

  state86 = 0;
  runlen  = 0;

  crc = 0x0000;

  /* find hqx header */

  for (ptr = hqxheader; ptr - hqxheader < HEADERMATCH; )
    {
      c = getc(file);
      if (c < 0)
	{
	  fclose(file);
	  bh_error = "hqx file header not found";
	  return -1;
	}

      if (c != *ptr++)
	ptr = hqxheader;
    }

  /* skip to CR/LF */

  do
    {
      c = getc(file);
      if (c < 0)
	{
	  fclose(file);
	  bh_error = "corrupt hqx file";
	  return -1;
	}
    }
  while (c != '\n' && c != '\r');

  /* skip whitespace */

  do
    {
      c = getc(file);
      if (c < 0)
	{
	  fclose(file);
	  bh_error = "corrupt hqx file";
	  return -1;
	}
    }
  while (ISRETURN(c));

  if (c != ':')
    {
      fclose(file);
      bh_error = "corrupt hqx file";
      return -1;
    }

  return 0;
}

/*
 * NAME:	hqxchar()
 * DESCRIPTION:	return the next hqx character from the input stream
 */
static
int hqxchar(void)
{
  int c;

  do
    c = getc(file);
  while (ISRETURN(c));

  if (c < 0)
    {
      if (feof(file))
	bh_error = "unexpected end of file";
      else
	bh_error = "error reading input file";

      return -1;
    }

  c = demap[(unsigned char) c];
  if (c == 0)
    {
      bh_error = "illegal character in hqx file";
      return -1;
    }

  return c - 1;
}

/*
 * NAME:	nextchar()
 * DESCRIPTION:	decode one character from the hqx stream
 */
static
int nextchar(void)
{
  int c, c2, ch;

  c = hqxchar();
  if (c < 0)
    return -1;

  switch (state86 & 0xff00)
    {
    case 0x0000:
      c2 = hqxchar();
      if (c2 < 0)
	return -1;

      ch = (c << 2) | (c2 >> 4);
      state86 = 0x0100 | (c2 & 0x0f);
      break;

    case 0x0100:
      ch = ((state86 & 0x0f) << 4) | (c >> 2);
      state86 = 0x0200 | (c & 0x03);
      break;

    case 0x0200:
      ch = ((state86 & 0x03) << 6) | c;
      state86 = 0;
      break;
    }

  return ch;
}

/*
 * NAME:	bh->read()
 * DESCRIPTION:	decode and return bytes from the hqx stream
 */
int bh_read(void *buf, register int len)
{
  register unsigned char *data = buf;
  unsigned char *ptr = data;
  int c, rl, count = len;

  while (len--)
    {
      if (runlen)
	{
	  *data++ = lastch;
	  --runlen;
	  continue;
	}

      c = nextchar();
      if (c < 0)
	return -1;

      if (c == 0x90)
	{
	  rl = nextchar();
	  if (rl < 0)
	    return -1;

	  if (rl > 0)
	    {
	      runlen = rl - 1;
	      ++len;
	      continue;
	    }
	}

      *data++ = lastch = c;
    }

  crc = crc_binh(ptr, count, crc);

  return count;
}

/*
 * NAME:	bh->readcrc()
 * DESCRIPTION:	read and compare CRC bytes
 */
int bh_readcrc(void)
{
  unsigned short check;
  unsigned char word[2];

  check = crc_binh(zero, 2, crc);

  if (bh_read(word, 2) < 2)
    return -1;

  crc = (word[0] << 8) |
        (word[1] << 0);

  if (crc != check)
    {
      bh_error = "CRC checksum error";
      return -1;
    }

  crc = 0x0000;

  return 0;
}

/*
 * NAME:	bh->close()
 * DESCRIPTION:	finish BinHex decoding
 */
int bh_close(void)
{
  int c, result = 0;

  /* skip whitespace */

  do
    c = getc(file);
  while (ISRETURN(c));

  /* skip optional exclamation */

  if (c == '!')
    {
      do
	c = getc(file);
      while (ISRETURN(c));
    }

  /* verify trailing colon */

  if (c != ':')
    {
      bh_error = "corrupt end of hqx file";
      result = -1;
    }

  if (fclose(file) < 0 && result == 0)
    {
      bh_error = "error closing input file";
      result = -1;
    }

  return result;
}
