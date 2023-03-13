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

#include "stdafx.h"
#include "mactypes.h"
#include "floppy.h"
#include "hfs\libhfs\hfs.h"
#include "hfs\interface.h"
#define _FHOOK_INTERNAL_
#include "filehook.h"
#include "utils.h"

extern "C" {
#include "hfs\libhfs\data.h"
}

#include <time.h>

int unmagic( int x )
{
	if( x >= FINDER_MAGIC_LOW && x <= FINDER_MAGIC_HIGH ) {
		x += FINDER_MAGIC_OFFSET;
	}
	return(x);
}

int is_inside_magic( int x, int y )
{
	if( x >= FINDER_MAGIC_LOW && x <= FINDER_MAGIC_HIGH &&
		  y >= FINDER_MAGIC_LOW && y <= FINDER_MAGIC_HIGH )
	{
		return(1);
	} else {
		return(0);
	}
}

void macpc_w( char *s )
{
	*((unsigned short *)s) = 
		 ((unsigned short)((unsigned char *)s)[0] << 8) |
     ((unsigned short)((unsigned char *)s)[1]);
}

void macpc_d( char *s )
{
	*((unsigned long *)s) = 
		 ((unsigned long)((unsigned char *)s)[0] << 24) |
     ((unsigned long)((unsigned char *)s)[1] << 16) |
     ((unsigned long)((unsigned char *)s)[2] <<  8) |
     ((unsigned long)((unsigned char *)s)[3]);
}

/*
void macpc_w( char *s )
{
	char s0;
	s0 = s[0];
	s[0] = s[1];
	s[1] = s0;
}

void macpc_d( char *s )
{
	char s0, s1;
	s0 = s[0];
	s1 = s[1];
	s[0] = s[3];
	s[1] = s[2];
	s[2] = s1;
	s[3] = s0;
}
*/

// avoid memcpy: down copy is different from compiler to compiler
void macpc_str( unsigned char *s, int max_len )
{
	int i, len;
	len = (int) *s;
	if(len > max_len) len = max_len;
	for(i=0; i<len; i++) {
		s[i] = s[i+1];
	}
	s[i] = 0;
	mac_to_pc_charset(s);
}

void macpc_extrec( ExtDescriptor *ep )
{
	MACPC_W(ep->xdrStABN);
	MACPC_W(ep->xdrNumABlks);
}

void macpc_MDB( MDB *pMDB )
{
	MACPC_W(pMDB->drSigWord);
	MACPC_D(pMDB->drCrDate);
	MACPC_D(pMDB->drLsMod);
	MACPC_W(pMDB->drAtrb);
	MACPC_W(pMDB->drNmFls);
	MACPC_W(pMDB->drVBMSt);
	MACPC_W(pMDB->drAllocPtr);
	MACPC_W(pMDB->drNmAlBlks);
	MACPC_D(pMDB->drAlBlkSiz);
	MACPC_D(pMDB->drClpSiz);
	MACPC_W(pMDB->drAlBlSt);
	MACPC_D(pMDB->drNxtCNID);
	MACPC_W(pMDB->drFreeBks);
	macpc_str( pMDB->drVN, 27 );
	MACPC_D(pMDB->drVolBkUp);
	MACPC_W(pMDB->drVSeqNum);
	MACPC_D(pMDB->drWrCnt);
	MACPC_D(pMDB->drXTClpSiz);
	MACPC_D(pMDB->drCTClpSiz);
	MACPC_W(pMDB->drNmRtDirs);
	MACPC_D(pMDB->drFilCnt);
	MACPC_D(pMDB->drDirCnt);
	MACPC_D(pMDB->drFndrInfo[0]);
	MACPC_D(pMDB->drFndrInfo[1]);
	MACPC_D(pMDB->drFndrInfo[2]);
	MACPC_D(pMDB->drFndrInfo[3]);
	MACPC_D(pMDB->drFndrInfo[4]);
	MACPC_D(pMDB->drFndrInfo[5]);
	MACPC_D(pMDB->drFndrInfo[6]);
	MACPC_D(pMDB->drFndrInfo[7]);
	MACPC_W(pMDB->drVCSize);
	MACPC_W(pMDB->drVBMCSize);
	MACPC_W(pMDB->drCtlCSize);
	MACPC_D(pMDB->drXTFlSize);
	macpc_extrec( &pMDB->drXTExtRec[0] );
	macpc_extrec( &pMDB->drXTExtRec[1] );
	macpc_extrec( &pMDB->drXTExtRec[2] );
	MACPC_D(pMDB->drCTFlSize);
	macpc_extrec( &pMDB->drCTExtRec[0] );
	macpc_extrec( &pMDB->drCTExtRec[1] );
	macpc_extrec( &pMDB->drCTExtRec[2] );
}

void macpc_ndescr( NodeDescriptor *pndescr )
{
	MACPC_D(pndescr->ndFLink);
	MACPC_D(pndescr->ndBLink);
	MACPC_W(pndescr->ndNRecs);
	MACPC_W(pndescr->ndResv2);
}

void macpc_header_rec( BTHdrRec *pheader_rec )
{
	MACPC_W(pheader_rec->bthDepth);
	MACPC_D(pheader_rec->bthRoot);
	MACPC_D(pheader_rec->bthNRecs);
	MACPC_D(pheader_rec->bthFNode);
	MACPC_D(pheader_rec->bthLNode);
	MACPC_W(pheader_rec->bthNodeSize);
	MACPC_W(pheader_rec->bthKeyLen);
	MACPC_D(pheader_rec->bthNNodes);
	MACPC_D(pheader_rec->bthFree);
}

void macpc_catkeyrec( CatKeyRec *pcatkeyrec )
{
	// length 
	MACPC_D(pcatkeyrec->ckrParID);
	macpc_str( pcatkeyrec->ckrCName, 31 );
}

void mappc_apple_single_double_header( apple_single_double_header *p )
{
	MACPC_D(p->Magic);
	MACPC_D(p->Version);
	// unsigned char Filler[16]
	MACPC_W(p->entry_count);
}

void mappc_apple_single_double_entry( apple_single_double_entry *p )
{
	MACPC_D(p->id);
	MACPC_D(p->offset);
	MACPC_D(p->length);
}

void macpc_resource_header( resource_header *p )
{
	MACPC_D(p->resource_data_offset);
	MACPC_D(p->resource_map_offset);
	MACPC_D(p->resource_data_length);
	MACPC_D(p->resource_map_length);
}

void macpc_resource_map_header( resource_map_header *p )
{
	// char reserved_for_header_copy[16];
	// char reserved_for_handle_next_map[4];
	// char reserved_for_fref_number[2];
	MACPC_W(p->res_fork_attributes);
	MACPC_W(p->type_list_offset);
	MACPC_W(p->name_list_offset);
}

void macpc_resource_type_item( resource_type_item *p )
{
	MACPC_D(p->res_type);
	MACPC_W(p->res_count1);
	MACPC_W(p->ref_list_offset);
}

void macpc_resource_ref_list_entry( resource_ref_list_entry *p )
{
	MACPC_W(p->res_id);
	MACPC_W(p->res_name_offset);
	// p->res_data_offset_hi; IS OK
	MACPC_W(p->res_data_offset_lo);
	MACPC_D(p->reserved);
}

void macpc_catdatarec( CatDataRec *pcatdatarec )
{
	switch( pcatdatarec->cdrType ) {
		case cdrDirRec:
			MACPC_W(pcatdatarec->u.d.dirFlags);
			MACPC_W(pcatdatarec->u.d.dirVal);
			MACPC_D(pcatdatarec->u.d.dirDirID);
			MACPC_D(pcatdatarec->u.d.dirCrDat);
			MACPC_D(pcatdatarec->u.d.dirMdDat);
			MACPC_D(pcatdatarec->u.d.dirBkDat);
			// pcatdatarec->u.d.dirUsrInfo.frRect
			MACPC_W(pcatdatarec->u.d.dirUsrInfo.frFlags);
			// pcatdatarec->u.d.dirUsrInfo.frLocation
			MACPC_W(pcatdatarec->u.d.dirUsrInfo.frView);
			// pcatdatarec->u.d.dirFndrInfo.frScroll
			MACPC_D(pcatdatarec->u.d.dirFndrInfo.frOpenChain);
			MACPC_W(pcatdatarec->u.d.dirFndrInfo.frUnused);
			MACPC_W(pcatdatarec->u.d.dirFndrInfo.frComment);
			MACPC_D(pcatdatarec->u.d.dirFndrInfo.frPutAway);
			MACPC_D(pcatdatarec->u.d.dirResrv[0]);
			MACPC_D(pcatdatarec->u.d.dirResrv[1]);
			MACPC_D(pcatdatarec->u.d.dirResrv[2]);
			MACPC_D(pcatdatarec->u.d.dirResrv[3]);
			break;
		case cdrFilRec:
			MACPC_D(pcatdatarec->u.f.filUsrWds.fdType);
			MACPC_D(pcatdatarec->u.f.filUsrWds.fdCreator);
			MACPC_W(pcatdatarec->u.f.filUsrWds.fdFlags);
			// pcatdatarec->u.f.filUsrWds.fdLocation
			MACPC_W(pcatdatarec->u.f.filUsrWds.fdFldr);
			MACPC_D(pcatdatarec->u.f.filFlNum);
			MACPC_W(pcatdatarec->u.f.filStBlk);
			MACPC_D(pcatdatarec->u.f.filLgLen);
			MACPC_D(pcatdatarec->u.f.filPyLen);
			MACPC_W(pcatdatarec->u.f.filRStBlk);
			MACPC_D(pcatdatarec->u.f.filRLgLen);
			MACPC_D(pcatdatarec->u.f.filRPyLen);
			MACPC_D(pcatdatarec->u.f.filCrDat);
			MACPC_D(pcatdatarec->u.f.filMdDat);
			MACPC_D(pcatdatarec->u.f.filBkDat);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdIconID);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdUnused[0]);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdUnused[1]);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdUnused[2]);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdUnused[3]);
			MACPC_W(pcatdatarec->u.f.filFndrInfo.fdComment);
			MACPC_D(pcatdatarec->u.f.filFndrInfo.fdPutAway);
			MACPC_W(pcatdatarec->u.f.filClpSize);
			macpc_extrec( &pcatdatarec->u.f.filExtRec[0] );
			macpc_extrec( &pcatdatarec->u.f.filExtRec[1] );
			macpc_extrec( &pcatdatarec->u.f.filExtRec[2] );
			macpc_extrec( &pcatdatarec->u.f.filRExtRec[0] );
			macpc_extrec( &pcatdatarec->u.f.filRExtRec[1] );
			macpc_extrec( &pcatdatarec->u.f.filRExtRec[2] );
			MACPC_D(pcatdatarec->u.f.filResrv);
			break;
		case cdrThdRec:
			MACPC_D(pcatdatarec->u.dt.thdResrv[0]);
			MACPC_D(pcatdatarec->u.dt.thdResrv[1]);
			MACPC_D(pcatdatarec->u.dt.thdParID);
			macpc_str( pcatdatarec->u.dt.thdCName, 31 );
			break;
		case cdrFThdRec:
			MACPC_D(pcatdatarec->u.ft.fthdResrv[0]);
			MACPC_D(pcatdatarec->u.ft.fthdResrv[1]);
			MACPC_D(pcatdatarec->u.ft.fthdParID);
			macpc_str( pcatdatarec->u.ft.fthdCName, 31 );
			break;
	}
}

// incorrect
#if 0
void mac_date_to_string( unsigned long date, char *buf )
{
	unsigned long years, days, hours, mins, secs;
	unsigned long year, month, day;

	years = date / (60L * 60 * 24 * 365);
	date -= years * (60L * 60 * 24 * 365);
	days = date / (60L * 60 * 24);
	date -= days * (60L * 60 * 24);
	hours = date / (60L * 60);
	date -= hours * (60L * 60);
	mins = date / 60L;
	secs = date - mins * 60L;

	year = years + 1904;
	month = 1 + days / 30;
	day = 1 + days % 30;

	wsprintf( 
		buf, 
		"%04ld-%02ld-%02ld %02ld:%02ld:%02ld",
		year,
		month,
		day,
		hours,
		mins,
		secs
		);
}
#else
void mac_date_to_string( unsigned long date, char *buf )
{
	long crdate;
	char *p;

	crdate = d_toutime(date);
	p = ctime( (const long *)&crdate );

	if(p) {
		lstrcpy( buf, p );
		p = strchr( buf, '\r' );
		if(p) *p = 0;
		p = strchr( buf, '\n' );
		if(p) *p = 0;
	} else {
		*buf = 0;
	}
}
#endif

CHFVVolume::CHFVVolume()
{
	m_file_name = "";
	m_opened = 0;
	m_hfs_start = 0;
	m_floppy_update_needed = 0;
	m_free = 1;
}

CHFVVolume::~CHFVVolume()
{
}

BOOL CHFVVolume::find_volume_MDB()
{
	DWORD i = 0;
	int found = 0;

	char p[512];

	m_file.Seek( 0, CFile::begin );
	if(m_file.Read( p, 512 ) != 512) return(0);
	// while(!found && i < m_length - 512) {
	while(!found && i < 8192) {
		if( *(unsigned short *)p == 0x4442 ) { // these are still in Motorola
			found = 1;
		} else if( *(int *)p == 0xD7D2 ) { // still in Motorola
			found = 2;
		} else {
			i += 512;
			m_file.Read( p, 512 );
		}
	}

	switch(found) {
		case 0:
			// AfxMessageBox( "This is not a Mac volume.", MB_OK | MB_ICONHAND );
			break;
		case 1:
			m_file.Seek( -512, CFile::current );
			m_file.Read( &m_MDB, sizeof( MDB ) );
			macpc_MDB( &m_MDB );
			break;
		case 2:
			AfxMessageBox( "Old flat MFS volumes not supported.", MB_OK | MB_ICONHAND );
			break;
	}
	return(found == 1);
}

int CHFVVolume::update_if_needed(int ask)
{
	CFileStatus rStatus;

	if(!m_opened) {
		return(open());
	}
	if(m_file.m_drive != IS_NO_FLOPPY) {
		if(m_floppy_update_needed) {
			m_floppy_update_needed = 0;
			return(open());
		}
		return(0);
	}

	m_file.GetStatus( rStatus );
	if( rStatus.m_mtime != m_file_last_status.m_mtime ) {
		if(ask) {
			if(AfxMessageBox( 
				"Contents of volume \"" + m_volume_name+ "\" have changed. Reload now?",
				MB_YESNO ) == IDNO) 
			{
				if(AfxMessageBox( 
					"Not reloading will cause data integrity problems. Reload?",
					MB_YESNO ) == IDNO) return(0);
			}
		}
		return(open());
	}
	return(0);
}




//////////////////////////////////////////////////////
CDSKFile::CDSKFile()
{
	m_drive = IS_NO_FLOPPY;
	m_hfloppy = 0;
	m_hcd = 0;
	m_hd = 0;
	m_seek = 0;
}

CDSKFile::~CDSKFile()
{
}

BOOL CDSKFile::Open(
	LPCTSTR lpszFileName, 
	UINT nOpenFlags,
	CFileException* pError
)
{
	int i;
	char name[_MAX_PATH];

	for(i=0; i<MAX_FLOPPIES; i++) {
		if(is_floppy_by_index(i)) {
			get_floppy_volume_file_name( i, name );
			if(stricmp(name,lpszFileName) == 0) {
				m_drive = i;
				m_hfloppy = floppy_init( i, TRUE );
				m_seek = 0;
				return(m_hfloppy != 0);
			}
		}
	}
	for(i=2; i<MAX_DEVICES; i++) {
		get_cd_volume_file_name( i, name );
		if(stricmp(name,lpszFileName) == 0) {
			m_drive = i;
			m_hcd = cd_init( i, TRUE );
			m_seek = 0;
			if(!try_mount_cd( m_hcd, m_drive )) {
				cd_final( m_drive );
				m_hcd = 0;
				m_seek = 0;
			}
			return(m_hcd != 0);
		}
	}
	for(i=2; i<MAX_DEVICES; i++) {
		get_hd_volume_file_name( i, name );
		if(stricmp(name,lpszFileName) == 0) {
			m_drive = i;
			m_hd = hd_init( i, TRUE );
			m_seek = 0;
			if(!try_mount_hd( m_hd, m_drive )) {
				hd_final( m_drive );
				m_hd = 0;
				m_seek = 0;
			}
			return(m_hd != 0);
		}
	}
	m_drive = IS_NO_FLOPPY;
	return(CFile::Open(lpszFileName,nOpenFlags,pError));
}

void CDSKFile::Close()
{
	if(m_drive != IS_NO_FLOPPY) {
		writeback_flush_all();
		if(m_hfloppy) {
			floppy_final( m_drive );
			m_hfloppy = 0;
			m_seek = 0;
		} else if(m_hcd) {
			cd_final( m_drive );
			m_hcd = 0;
			m_seek = 0;
		} else if(m_hd) {
			hd_final( m_drive );
			m_hd = 0;
			m_seek = 0;
		}
		return;
	}
	CFile::Close();
}

BOOL CDSKFile::GetStatus(CFileStatus& rStatus)
{
	if(m_drive != IS_NO_FLOPPY) {
		if(m_hfloppy || m_hcd || m_hd) {
			// struct, not a class...
			memset( &rStatus, 0, sizeof(CFileStatus) );
			return(1);
		} else {
			return(0);
		}
	}
	return(CFile::GetStatus(rStatus));
}

LONG CDSKFile::Seek(LONG lOff, UINT nFrom)
{
	LONG sz;
	long new_seek;

	if(m_drive != IS_NO_FLOPPY) {
		if(m_hfloppy || m_hcd || m_hd) {
			if(m_hfloppy)
				sz = get_floppy_size(m_hfloppy,m_drive);
			else if(m_hd)
				sz = get_hd_size(m_hd,m_drive);
			else
				sz = get_cd_size(m_hcd,m_drive);
			new_seek = m_seek;
			switch( nFrom ) {
				case begin:
					new_seek = lOff;
					break;
				case current:
					new_seek += lOff;
					break;
				case end:
					// TESTTEST: WAS: - Off
					new_seek = sz + lOff;
					break;
			}
			if(new_seek < 0 || new_seek >= sz) {
				return(-1);
			} else {
				m_seek = new_seek;
				return(m_seek);
			}
		} else {
			return(-1);
		}
	}
	return(CFile::Seek(lOff, nFrom));
}

UINT CDSKFile::Read(void* lpBuf, UINT nCount)
{
	if(m_drive != IS_NO_FLOPPY) {
		int got = 0;
		if(m_hfloppy) {
			got = floppy_read( m_hfloppy, m_drive, m_seek, nCount, (char *)lpBuf );
			if(got > 0) m_seek += got;
		} else if(m_hd) {
			got = cd_read( m_hd, m_drive, m_seek, nCount, (char *)lpBuf, FALSE );
			if(got > 0) m_seek += got;
		} else if(m_hcd) {
			got = cd_read( m_hcd, m_drive, m_seek, nCount, (char *)lpBuf, TRUE );
			if(got > 0) m_seek += got;
		}
		return(got);
	}
	return(CFile::Read(lpBuf, nCount));
}

DWORD CDSKFile::GetLength()
{
	if(m_drive != IS_NO_FLOPPY) {
		if(m_hfloppy) {
			return(get_floppy_size(m_hfloppy,m_drive));
		} else if(m_hd) {
			return(get_hd_size(m_hd,m_drive));
		} else if(m_hcd) {
			return(get_cd_size(m_hcd,m_drive));
		} else {
			return(-1);
		}
	}
	return(CFile::GetLength());
}
//////////////////////////////////////////////////////



CHFVFile::CHFVFile()
{
	m_isbtree = 0;
	m_file_size = 0;
	m_start = 0;
}

CHFVFile::~CHFVFile()
{
}

int CHFVFile::Open( CHFVVolume *pvol )
{
	m_pvol = pvol;
	m_isbtree = 0;
	m_offset = 0;
	m_opened = 1;
	return(1);
}

void CHFVFile::Close()
{
	m_opened = 0;
}

LONG CHFVFile::Seek( LONG lOff, UINT nFrom )
{
	if(!m_opened) return(0);

	if(nFrom == CFile::begin) {
		m_offset = lOff;
	} else if(nFrom == CFile::current) {
		m_offset += lOff;
	} else { // end
		m_offset = m_file_size + lOff; // TEST: WAS: -lOff
	}
	return(m_offset);
}

UINT CHFVFile::Read( void* lpBuf, UINT nCount )
{
	UINT ret, ret_pp = 0;
	long pos, left, bytes_in_block, xtra;
	int i;

	if(!m_opened) return(0);

	pos = m_start + m_offset;

	if(m_offset >= m_fast_limit) {

		// __asm int 3

		left = m_offset;
		for(i=0; i<3; i++) {
			bytes_in_block = m_first_extents[i].xdrNumABlks * m_pvol->m_MDB.drAlBlkSiz;
			if(left < bytes_in_block) {
				pos = m_pvol->m_hfs_start + 
							m_first_extents[i].xdrStABN * m_pvol->m_MDB.drAlBlkSiz +
							left;
				if(left + (long)nCount > bytes_in_block) {
					// hits fragment borders
					xtra = bytes_in_block - left;
					ret_pp = Read( &((char *)lpBuf)[xtra], nCount - xtra );
					nCount = xtra;
				}
				break;
			} else {
				left -= bytes_in_block;
			}
		}
	}

	m_pvol->m_file.Seek( pos, CFile::begin );
	ret = m_pvol->m_file.Read( lpBuf, nCount );
	m_offset += ret;
	return(ret_pp + ret);
}

int CHFVFile::intern_open()
{
	NodeDescriptor ndescr;

	// file global start point in real file
	m_start = m_pvol->m_hfs_start + m_first_extents[0].xdrStABN * m_pvol->m_MDB.drAlBlkSiz;
	m_fast_limit = m_first_extents[0].xdrNumABlks * m_pvol->m_MDB.drAlBlkSiz;
	m_offset = 0;
	m_opened = 1;

	if(m_isbtree) {
		Seek( 0, CFile::begin );
		Read( &ndescr, sizeof(NodeDescriptor) );
		macpc_ndescr( &ndescr );
		Read( &m_header_rec, sizeof(BTHdrRec) );
		macpc_header_rec( &m_header_rec );
		Seek( 128, CFile::current );
		Read( &m_map_record, sizeof(m_map_record) );
	}
	return(1);
}

int CHFVFile::open_by_CDR( CHFVVolume *pvol, CatDataRec *pCDR, int type )
{
	m_pvol = pvol;
	if(!m_pvol->m_opened) return(0);
	m_isbtree = 0;
	m_name = "Catalog File";
	m_clump_size = pCDR->u.f.filClpSize;
	if(type == OpenResourceFork) {
		m_file_size = pCDR->u.f.filRLgLen;
		memcpy( m_first_extents, pCDR->u.f.filRExtRec, sizeof(ExtDescriptor) * 3 );
	} else {
		m_file_size = pCDR->u.f.filLgLen;
		memcpy( m_first_extents, pCDR->u.f.filExtRec, sizeof(ExtDescriptor) * 3 );
	}
	return(intern_open());
}

int CHFVFile::open_special( CHFVVolume *pvol, int which )
{
	m_pvol = pvol;
	m_isbtree = 1;

	if(which == OpenCatalog) {
		m_name = "Catalog File";
		m_clump_size = pvol->m_MDB.drCTClpSiz;
		m_file_size = pvol->m_MDB.drCTFlSize;
		memcpy( m_first_extents, pvol->m_MDB.drCTExtRec, sizeof(ExtDescriptor) * 3 );
	} else { // OpenExtents
		m_name = "Extents overflow File";
		m_clump_size = pvol->m_MDB.drXTClpSiz;
		m_file_size = pvol->m_MDB.drXTFlSize;
		memcpy( m_first_extents, pvol->m_MDB.drXTExtRec, sizeof(ExtDescriptor) * 3 );
	}
	return(intern_open());
}

void CHFVVolume::update_time_stamp()
{
	if(m_opened) {
		TRY
		{
			m_file.GetStatus( m_file_last_status );
		}
		CATCH( CFileException, e )
		{
		}
		END_CATCH
	}
}
	

int CHFVVolume::open()
{
	if(m_opened) close();
	
	if(m_file_name == "") return(0);

	TRY
	{
		if(m_file.Open( m_file_name, CFile::modeRead | CFile::shareDenyNone )) {
			m_file.GetStatus( m_file_last_status );
			m_length = m_file.GetLength();
			m_opened = 1;
			if(find_volume_MDB()) {

				// It seems that we must always use here
				// physical block size, not allocation block size
				m_hfs_start = m_MDB.drAlBlSt * 512;
				// m_hfs_start = m_MDB.drAlBlSt * m_MDB.drAlBlkSiz;

				m_catalog_file.open_special( this, CHFVFile::OpenCatalog );
				m_extents_file.open_special( this, CHFVFile::OpenExtents );
				m_volume_name = CString(m_MDB.drVN);
			} else {
				close();
			}
		} else {
			// msg
		}
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH

	/*
	if(!m_opened && m_file_name != "") {
		char rootdir[10];
		wsprintf( rootdir, "%c:\\", m_file_name.GetAt(0) );
		if(GetDriveType( rootdir ) != DRIVE_REMOVABLE) {
		// if(m_file.m_drive != IS_NO_FLOPPY) { //not valid!
			AfxMessageBox( "Could not open volume " + m_file_name + ".", MB_OK | MB_ICONHAND );
		}
	}
	*/
	return (m_opened);
}

void CHFVVolume::close()
{
	if(m_opened) {
		TRY
		{
			m_file.Close();
		}
		CATCH( CFileException, e )
		{
		}
		END_CATCH
		m_opened = 0;
	}
}

int get_finder_view_mode( int flags )
{
	int mode = FINDER_VIEW_MODE_ICON;

	if( (flags & 0x200) == 0x200 ) {
		mode = FINDER_VIEW_MODE_ICON_NAME;
	} else if( (flags & 0x300) == 0x300 ) {
		mode = FINDER_VIEW_MODE_ICON_DATE;
	} else if( (flags & 0x400) == 0x400 ) {
		mode = FINDER_VIEW_MODE_ICON_SIZE;
	} else if( (flags & 0x500) == 0x500 ) {
		mode = FINDER_VIEW_MODE_ICON_KIND;
	} else if( (flags & 0x700) == 0x700 ) {
		mode = FINDER_VIEW_MODE_ICON_LABEL;
	} else if( (flags & 0x140) == 0x140 ) {
		mode = FINDER_VIEW_MODE_ICON_SMALL;
	}
	// FINDER_VIEW_MODE_ICON_BUTTON ?
	return(mode);
}

char *get_label_text( int finder_color )
{
	char *s;

	switch( finder_color ) {
		case FINDER_COLOR_ESSENTIAL:
			s = "Essential";
			break;
		case FINDER_COLOR_HOT:
			s = "Hot";
			break;
		case FINDER_COLOR_IN_PROGRESS:
			s = "In Progress";
			break;
		case FINDER_COLOR_COOL:
			s = "Cool";
			break;
		case FINDER_COLOR_PERSONAL:
			s = "Personal";
			break;
		case FINDER_COLOR_PROJECT_1:
			s = "Project 1";
			break;
		case FINDER_COLOR_PROJECT_2:
			s = "Project 2";
			break;
		default:
			s = "None";
			break;
	}
	return(s);
}

