/*
 * HFVExplorer
 * Copyright (C) 1997-1998 by Anygraaf Oy
 * Author: Lauri Pesonen, email: lpesonen@clinet.fi or lauri.pesonen@anygraaf.fi
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

 /*
 * This file is derived from linux/fs/hfs/part_tbl.c
 * by Paul H. Hargrove
 * GNU GPL etc, see file "Copying"
 * 
 * Modified to suit my needs. Lauri Pesonen.
 */

// #include "afx.h"
#include "stdafx.h"


#include "mactypes.h"

extern "C" {
#include "floppy.h"
#include "part.h"

#define _FHOOK_INTERNAL_
#include "filehook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

} // extern "C"

#define HFS_SECTOR_SIZE		512

/* offsets to various blocks */
#define HFS_DD_BLK		0 /* Driver Descriptor block */
#define HFS_PMAP_BLK		1 /* First block of partition map */
#define HFS_MDB_BLK		2 /* Block (w/i partition) of MDB */

/* magic numbers for various disk blocks */
#define HFS_DRVR_DESC_MAGIC	0x4552 /* "ER": driver descriptor map */
#define HFS_OLD_PMAP_MAGIC	0x5453 /* "TS": old-type partition map */
#define HFS_NEW_PMAP_MAGIC	0x504D /* "PM": new-type partition map */
#define HFS_SUPER_MAGIC		0x4244 /* "BD": HFS MDB (super block) */
#define HFS_MFS_SUPER_MAGIC	0xD2D7 /* MFS MDB (super block) */

typedef short S16;
typedef long S32;
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

#pragma pack(1)
/*
 * The Macintosh Driver Descriptor Block
 *
 * On partitioned Macintosh media this is block 0.
 * We really only need the "magic number" to check for partitioned media.
 */
typedef struct {
	U16	ddSig;			/* The signature word */
	S16	ddBlkSize;	/* device block size (bytes) */
	/* a bunch more stuff we don't need */
} hfs_drvr_desc;

/* 
 * The new style Mac partition map
 *
 * For each partition on the media there is a physical block (512-byte
 * block) containing one of these structures.  These blocks are
 * contiguous starting at block 1.
 */
typedef struct {
	U16	pmSig;				/* Signature bytes to verify that this is a partition map block */
	U16	reSigPad;			/* padding */
	U32	pmMapBlkCnt;	/* (At least in block 1) this is the number of partition map blocks */
	S32	pmPyPartStart;	/* The physical block number
					   of the first block in this partition */
	S32	pmPartBlkCnt;	/* The number of physical blocks in this partition */
	U8	pmPartName[32];	/* (null terminated?) string
					   giving the name of this partition */
	U8	pmPartType[32];	/* (null terminated?) string
					   giving the type of this partition */
	/* a bunch more stuff we don't need */
} new_pmap;

/* 
 * The old style Mac partition map
 *
 * The partition map consists for a 2-byte signature followed by an
 * array of these structures.  The map is terminated with an all-zero
 * one of these.
 */
typedef struct {
	S32	pdStart;
	S32	pdSize;
	U32	pdFSID;
} old_pmap_entry;

typedef struct {
	U16	pdSig;	/* Signature bytes */
	old_pmap_entry pdEntry[42];
} old_pmap;
#pragma pack()

/*
 * parse_new_part_table()
 *
 * Parse a new style partition map looking for the
 * start and length of the 'part'th HFS partition.
 */
static int parse_new_part_table(
	HANDLE hcd, 
	int drive,
	char *buf,
	int part, 
	S32 *size, 
	S32 *start,
	BOOL is_cd
)
{
	new_pmap *pm;
	int pmap_entries;
	int hfs_part = 0;
	int entry;

	pm = (new_pmap *)buf;
	MACPC_D(pm->pmMapBlkCnt);
	pmap_entries = pm->pmMapBlkCnt;

	*start = 0;
	for (entry = 0; (entry < pmap_entries) && !(*start); ++entry) {
		if (entry) {
			/* read the next partition map entry */
			int read_bytes;
			read_bytes = cd_read( hcd, drive, HFS_SECTOR_SIZE*(HFS_PMAP_BLK + entry), HFS_SECTOR_SIZE, buf, is_cd );
			if(read_bytes != HFS_SECTOR_SIZE) {
				// hfs_warn("hfs_fs: unable to "
				//         "read partition map.\n");
				goto bail;
			}
			pm = (new_pmap *)buf;
			MACPC_W(pm->pmSig);
			if (pm->pmSig != HFS_NEW_PMAP_MAGIC) {
				// hfs_warn("hfs_fs: invalid "
				//         "entry in partition map\n");
				goto bail;
			}
		}

		/* look for an HFS partition */
		if (!memcmp(pm->pmPartType,"Apple_HFS",9) && 
		    (!part || ((++hfs_part) == part))) {
			/* Found it! */
			MACPC_D(pm->pmPyPartStart);
			MACPC_D(pm->pmPartBlkCnt);
			*start = pm->pmPyPartStart;
			*size = pm->pmPartBlkCnt;
		}
	}

	if (!(*start)) {
		if (part) {
			// hfs_warn("hfs_fs: unable to locate the "
			//         "HFS partition number %d.\n", part);
		} else {
			// hfs_warn("hfs_fs: unable to locate any "
			//	 "HFS partitions.\n");
		}
		goto bail;
	}
	return 0;

bail:
	return 1;
}

/*
 * parse_old_part_table()
 *
 * Parse a old style partition map looking for the
 * start and length of the 'part'th HFS partition.
 */
static int parse_old_part_table(
	char *buf,
	int part, 
	S32 *size, 
	S32 *start
)
{
	old_pmap *pm = (old_pmap *)buf;
	old_pmap_entry *p = &pm->pdEntry[0];
	int hfs_part = 0;

	while ((p->pdStart || p->pdSize || p->pdFSID) && !(*start)) {
		/* look for an HFS partition */
		MACPC_D(p->pdFSID);
		if ((p->pdFSID == 0x54465331) && /* == "TFS1" */
		    (!part || ((++hfs_part) == part))) {
			/* Found it! */
			MACPC_D(p->pdStart);
			MACPC_D(p->pdSize);
			*start = p->pdStart;
			*size = p->pdSize;
		}
		++p;
	}

	if (!(*start)) {
		if (part) {
			// hfs_warn("hfs_fs: unable to locate the "
			//          "HFS partition number %d.\n", part);
		} else {
			// hfs_warn("hfs_fs: unable to locate any "
			// 	 "HFS partitions.\n");
		}
		return 1;
	}
	return 0;
}

int get_mdb_block_address( HANDLE hcd, int drive, long *start, BOOL is_cd )
{
	char *buf;
	U16 sig, bs;
	int retval = 0;
	int part = 0;
	int read_bytes;
	long size;
	U16 magic;

	size = *start = 0;

	buf = (char *)VirtualAlloc( 
				NULL, 
				HFS_SECTOR_SIZE,
				MEM_RESERVE | MEM_COMMIT, 
				PAGE_READWRITE );
	if(!buf) return(0);

	/* Read block 0 to see if this media is partitioned */
	read_bytes = cd_read( hcd, drive, HFS_SECTOR_SIZE*HFS_DD_BLK, HFS_SECTOR_SIZE, buf, is_cd );
	if(read_bytes != HFS_SECTOR_SIZE) {
		// hfs_warn("hfs_fs: Unable to read block 0\n");
		goto bail;
	}

  sig = ((hfs_drvr_desc *)buf)->ddSig;
	bs = ((hfs_drvr_desc *)buf)->ddBlkSize;

	MACPC_W(sig);
	MACPC_W(bs);

#ifdef OLD
  if (sig != HFS_DRVR_DESC_MAGIC) {
		/* not on partitioned media */
		read_bytes = cd_read( hcd, drive, HFS_SECTOR_SIZE*HFS_MDB_BLK, HFS_SECTOR_SIZE, buf, is_cd );
		if(read_bytes != HFS_SECTOR_SIZE) {
			goto bail;
		}
		MACPC_W(sig);
	  if (sig == HFS_MFS_SUPER_MAGIC) {
			retval = 0;
		} else {
			retval = 1;
		}
		goto done;
	}
#else
  if (sig != HFS_DRVR_DESC_MAGIC) {
		/* not on partitioned media */
		read_bytes = cd_read( hcd, drive, HFS_SECTOR_SIZE*HFS_MDB_BLK, HFS_SECTOR_SIZE, buf, is_cd );
		if(read_bytes != HFS_SECTOR_SIZE) {
			goto bail;
		}
		MACPC_W(sig);
	  if (sig == HFS_MFS_SUPER_MAGIC) {
			retval = 0;
			goto done;
		} else {
			// retval = 1;
			// NEW: now we must not give up that easily. There may be
			// signature/block size header missing (Executor 2 CD).
			// We force it to search partition map, and surrender 
			// only it that fails.
			bs = HFS_SECTOR_SIZE;
		}
	}
#endif
	if (bs != HFS_SECTOR_SIZE) {
		// hfs_warn("hfs_fs: Block size %d != 512 on dev "
		//	"%s.\n", bs, hfs_mdb_name(sys_mdb));
		goto bail;
	}

	read_bytes = cd_read( hcd, drive, HFS_SECTOR_SIZE*HFS_PMAP_BLK, HFS_SECTOR_SIZE, buf, is_cd );
	if(read_bytes != HFS_SECTOR_SIZE) {
		// hfs_warn("hfs_fs: unable to read partition map.\n");
		goto bail;
	}

	magic = *((U16 *)buf);
	MACPC_W(magic);
	switch (magic) {
		case HFS_OLD_PMAP_MAGIC:
			retval = parse_old_part_table(buf, part, &size, start);
			break;
		case HFS_NEW_PMAP_MAGIC:
			retval = parse_new_part_table( hcd, drive, buf, part, &size, start, is_cd);
			break;
		default:
			// hfs_warn("hfs_fs: This disk has an unrecognized "
			//	 "partition map type.\n");
			goto bail;
	}
	if (size < 0) {
		// hfs_warn("hfs_fs: Partition size > 1 Terabyte.\n");
		goto bail;
	} else if (*start < 0) {
		// hfs_warn("hfs_fs: Partition begins beyond 1 Terabyte.\n");
		goto bail;
	}

done:
	VirtualFree( buf, 0, MEM_RELEASE  ); 
	*start = (*start) * HFS_SECTOR_SIZE;
	return !retval;

bail:
	VirtualFree( buf, 0, MEM_RELEASE  ); 
	return(0);
}
