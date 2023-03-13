/*
 * hfsutils - tools for reading and writing Macintosh HFS volumes
 * Copyright (C) 1997 Marcus Better
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

#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#include <os2.h>

/* Open a file. If path is a drive letter and we are under OS/2, 
   open in DASD mode and use _imphandle() to convert the handle to emx.
*/
int os2_open(const char *path, int oflag)
{
  if(_emx_env & 0x0200 
     && isalpha(path[0]) && path[1]==':' && path[2]==0) {
    HFILE hf;
    ULONG action, open_mode;
    APIRET rc;
    int handle;

    open_mode = OPEN_FLAGS_DASD | OPEN_SHARE_DENYREADWRITE;
    if (oflag & O_WRONLY || oflag & O_RDWR)
      open_mode |= OPEN_ACCESS_READWRITE;
    else
      open_mode |= OPEN_ACCESS_READONLY;
    rc = DosOpen(path, &hf, &action, 0, 0, 
		 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
		 open_mode, NULL);
    if (rc != NO_ERROR) {
      *_errno() = EACCES;
      return -1;
    }
    
    /* If opening for write, lock the drive. */
    if((open_mode & OPEN_ACCESS_WRITEONLY) |
       (open_mode & OPEN_ACCESS_READWRITE)) {
      char lock_param[1] = {0}, lock_data[1] = {0};
      ULONG paramlen=1, datalen=1;
      
      rc=DosDevIOCtl(hf, IOCTL_DISK, DSK_LOCKDRIVE,
		     lock_param, 1, &paramlen,
		     lock_data, 1, &datalen);
      if(rc != NO_ERROR) {
	DosClose(hf);
	*_errno() = EACCES;
	return -1;
      }
    }

    handle = _imphandle(hf);

    if (oflag & O_BINARY)
      if (setmode(handle, O_BINARY) < 0)
	return -1;

    return handle;
  }
  else
    return open(path, oflag);
}

/* If we are under OS/2, check if it is a DASD handle. If yes, then unlock
   the drive first.
   */
int os2_close(int handle) {
  if (_emx_env & 0x0200) {
    ULONG rc, open_mode;

    rc = DosQueryFHState(handle, &open_mode);
    if(rc == NO_ERROR) {
      if(open_mode & OPEN_FLAGS_DASD &&
	 ((open_mode & OPEN_ACCESS_WRITEONLY) |
	  (open_mode & OPEN_ACCESS_READWRITE))) {
	char lock_param[1] = {0}, lock_data[1] = {0};
	ULONG paramlen=1, datalen=1;
	
	DosDevIOCtl(handle, IOCTL_DISK, DSK_UNLOCKDRIVE,
		    lock_param, 1, &paramlen,
		    lock_data, 1, &datalen);
      }
    }
  }

  return close(handle);
}
