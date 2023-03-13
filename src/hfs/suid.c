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

# include <unistd.h>

# include "suid.h"

#ifndef __EMX__
static uid_t uid, euid;
static gid_t gid, egid;
#endif

/*
 * NAME:	suid->init()
 * DESCRIPTION:	initialize application which may be running setuid/setgid
 */
void suid_init(int enable)
{
#ifndef __EMX__
  uid  = getuid();
  gid  = getgid();

  euid = geteuid();
  egid = getegid();

  if (! enable)
    suid_disable();
#endif
}

/*
 * NAME:	suid->enable()
 * DESCRIPTION:	engage any setuid privileges
 */
void suid_enable(void)
{
#ifndef __EMX__
  setreuid(-1, euid);
  setregid(-1, egid);
#endif
}

/*
 * NAME:	suid->disable()
 * DESCRIPTION:	revoke all setuid privileges
 */
void suid_disable(void)
{
#ifndef __EMX__
  setreuid(-1, uid);
  setregid(-1, gid);
#endif
}
