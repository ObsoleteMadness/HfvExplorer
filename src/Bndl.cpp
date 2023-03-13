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
#include "HFVExplorer.h"

#include "HFVExplorerDoc.h"
#include "HFVExplorerListView.h"
#include "CFATVolume.h"
#include "special.h"
// #include "utils.h"
#include "hfs\libhfs\hfs.h"
#include "hfs\interface.h"
#include "AskNewVolume.h"
#include "MainFrm.h"
#include "floppy.h"
#include "adouble.h"
#include "aspecial.h"

#include <stdlib.h>
#include "utils.h"

// #define NO_SMALL_ICONS

enum { HASH_SIZE = ICONTABLESIZE };

static int icons_in_cache = 0;
static int hash_file_ok = 0;
static int hash_file_impossible = 0;
static CFile hash_file;

type_creator_link_t *hash_table[HASH_SIZE];

void dump_data( unsigned char *fname, unsigned char *p, int count )
{
	CFile f, *fp;

	unlink( (char*)fname);

	fp = &f;

	if(do_create_file( fp, CString(fname) )) {
		TRY
		{
		fp->Write( p, count );
		}
		CATCH( CFileException, e )
		{
		}
		END_CATCH
		silent_close( fp );
	}
}

void CHFVExplorerDoc::init_hash_table()
{
  CString ini_name;
	CFile *fp;

	fp = &hash_file;

	if(hash_file_ok) {
		hash_file_ok = 0;
		silent_close( fp );
	}

	get_icons_file_name( ini_name );
	if(ini_name != "") {
		if(do_open_readwrite( fp, ini_name )) {
			hash_file_ok = 1;
		} else if(do_create_file( fp, ini_name )) {
			hash_file_ok = 1;
		}
	}

	memset( hash_table, 0, sizeof(type_creator_link_t *) * HASH_SIZE );
	icons_in_cache = 0;

	load_hash_table();
}

int CHFVExplorerDoc::exists_icon_cache_file()
{
	CString name;
	char *cname;
	int ret;

	get_icons_file_name( name );
	cname = name.GetBuffer( _MAX_PATH );
	if(exists( cname )) {
		ret = 1;
	} else {
		ret = 0;
	}
	name.ReleaseBuffer();
	return(ret);
}

int CHFVExplorerDoc::delete_cache_OK()
{
	CString name;
	char *cname;
	int ret;

	get_icons_file_name( name );
	cname = name.GetBuffer( _MAX_PATH );
	if(exists( cname )) {
		silent_delete( name );
		ret = 1;
	} else {
		ret = 0;
	}
	name.ReleaseBuffer();
	return(ret);
}
 
int CHFVExplorerDoc::get_icon_cache_count()
{
	return(icons_in_cache);
}

void CHFVExplorerDoc::get_icons_file_name( CString &name )
{
	CHFVExplorerApp	*pApp;
	pApp = (CHFVExplorerApp *)AfxGetApp();

	name = "";

	char modulex[MAX_PATH];
	GetModuleFileName( pApp->m_hInstance, modulex, MAX_PATH-1 );
	
	CString module = CString( modulex );
	int i = module.ReverseFind( '\\' );
	if( i > 0 ) {
		name = module.Left( i ) + CString("\\HFVICONS.DAT");
	}
}

typedef struct {
	int i1, i2, i3;
} three_ints;

typedef struct {
	OSType type, creator;
} two_OSType;

void CHFVExplorerDoc::load_hash_table()
{
	CString ini_name;
	CFile *fp;
	two_OSType owner;
	char *buf;
	HANDLE h16, h32;
	// unsigned char *cim16, *cim32;

	local_icn_sharp_t ip16;
	local_icn_sharp_t ip32;

	unsigned char *mask16 = 0;
	unsigned char *mask32 = 0;

	three_ints ints3;

	if(!hash_file_ok) return;

	h16 = GlobalAlloc( GHND, 10000 );
	if(!h16) return;

	h32 = GlobalAlloc( GHND, 10000 );
	if(!h32) return;

	mask16 = (unsigned char *)malloc( 10000 );
	if(!mask16) return;

	mask32 = (unsigned char *)malloc( 10000 );
	if(!mask32) return;

	TRY
	{
	fp = &hash_file;
	while(1) {
		ip16.icon = 0;
		ip32.icon = 0;
		ip16.h = 0;
		ip32.h = 0;
		ip16.color_icon_mask = 0;
		ip32.color_icon_mask = 0;

		if(fp->Read( &owner, sizeof(owner) ) != sizeof(owner)) break;

		if(fp->Read( &ip16.bytes, sizeof(ip16.bytes) ) != sizeof(ip16.bytes)) break;
		if(ip16.bytes > 1024 || ip16.bytes < 0) {
			delete_cache();
			goto panic;
		}
		if(ip16.bytes) {
			ip16.h = h16;
			buf = (char *)GlobalLock(ip16.h);
			if((int)fp->Read( buf, ip16.bytes ) != ip16.bytes) break;
			GlobalUnlock(ip16.h);
			if((int)fp->Read( &ip16.color_icon_mask, sizeof(ip16.color_icon_mask) ) != sizeof(ip16.color_icon_mask)) break;
			if(ip16.color_icon_mask) {
				ip16.color_icon_mask = mask16;
				if((int)fp->Read( ip16.color_icon_mask, ip16.bytes ) != ip16.bytes) break;
			}
		}
		if(fp->Read( &ints3, sizeof(ints3) ) != sizeof(ints3)) break;
		ip16.small = ints3.i1;
		ip16.iscolor = ints3.i2;
		ip16.dim = ints3.i3;

		if(fp->Read( &ip32.bytes, sizeof(ip32.bytes) ) != sizeof(ip32.bytes)) break;
		if(ip32.bytes > 1024 || ip32.bytes < 0) {
			delete_cache();
			goto panic;
		}
		if(ip32.bytes) {
			ip32.h = h32;
			buf = (char *)GlobalLock(ip32.h);
			if((int)fp->Read( buf, ip32.bytes ) != ip32.bytes) break;
			GlobalUnlock(ip32.h);
			if((int)fp->Read( &ip32.color_icon_mask, sizeof(ip32.color_icon_mask) ) != sizeof(ip32.color_icon_mask)) break;
			if(ip32.color_icon_mask) {
				ip32.color_icon_mask = mask32;
				if((int)fp->Read( ip32.color_icon_mask, ip32.bytes ) != ip32.bytes) break;
			}
		}
		if(fp->Read( &ints3, sizeof(ints3) ) != sizeof(ints3)) break;
		ip32.small = ints3.i1;
		ip32.iscolor = ints3.i2;
		ip32.dim = ints3.i3;

#ifndef NO_SMALL_ICONS
		create_cache_icon( &ip16, 0 );
#endif
		create_cache_icon( &ip32, 0 );
		add_to_icon_cache( ip16.icon );
		add_to_icon_cache( ip32.icon );
		insert_into_hash_table ( owner.type, owner.creator, &ip16, &ip32 );
	}
	}
	CATCH( CFileException, e )
	{
	}
	END_CATCH

panic: // corrupted cache

	free(mask32);
	free(mask16);

	GlobalFree( h16 );
	GlobalFree( h32 );
}

void CHFVExplorerDoc::delete_cache()
{
	CString name;
	silent_close( &hash_file );
	hash_file_ok = 0;
	get_icons_file_name( name );
	silent_delete( name );
}

void CHFVExplorerDoc::save_hash_table(
	OSType type, 
	OSType creator, 
	local_icn_sharp_t *ip16,
	local_icn_sharp_t *ip32
)
{
	CFile *fp;
	int failure = 0;
	char *buf;
	long old_length;

	if(!hash_file_ok) return;

	fp = &hash_file;

	TRY
	{
		old_length = fp->Seek( 0, CFile::end );
	}
	CATCH( CFileException, e )
	{
		hash_file_ok = 0;
		silent_close( fp );
		return;
	}
	END_CATCH

	TRY
	{
	fp->Write( &type, sizeof(OSType) );
	fp->Write( &creator, sizeof(OSType) );
	if(!ip16->h) ip16->bytes = 0;
	fp->Write( &ip16->bytes, sizeof(ip16->bytes) );
	if(ip16->h) {
		buf = (char *)GlobalLock(ip16->h);
		fp->Write( buf, ip16->bytes );
		GlobalUnlock(ip16->h);
		fp->Write( &ip16->color_icon_mask, sizeof(ip16->color_icon_mask) );
		if(ip16->color_icon_mask) {
			fp->Write( ip16->color_icon_mask, ip16->bytes );
		}
	}
	fp->Write( &ip16->small, sizeof(ip16->small) );
	fp->Write( &ip16->iscolor, sizeof(ip16->iscolor) );
	fp->Write( &ip16->dim, sizeof(ip16->dim) );

	if(!ip32->h) ip32->bytes = 0;
	fp->Write( &ip32->bytes, sizeof(ip32->bytes) );
	if(ip32->h) {
		buf = (char *)GlobalLock(ip32->h);
		fp->Write( buf, ip32->bytes );
		GlobalUnlock(ip32->h);
		fp->Write( &ip32->color_icon_mask, sizeof(ip32->color_icon_mask) );
		if(ip32->color_icon_mask) {
			fp->Write( ip32->color_icon_mask, ip32->bytes );
		}
	}
	fp->Write( &ip32->small, sizeof(ip32->small) );
	fp->Write( &ip32->iscolor, sizeof(ip32->iscolor) );
	fp->Write( &ip32->dim, sizeof(ip32->dim) );
	}
	CATCH( CFileException, e )
	{
		failure = 1;
	}
	END_CATCH

	if(failure) {
		TRY {
			fp->SetLength( old_length );
		} 
		CATCH( CFileException, e ) 
		{
			delete_cache();
		}
		END_CATCH
	}
}

void CHFVExplorerDoc::clear_hash_table()
{
	int i;
  struct type_creator_link_str *next;
  struct type_creator_link_str *x;

	if(hash_file_ok) {
		silent_close( &hash_file );
		hash_file_ok = 0;
	}

	for(i=0; i<HASH_SIZE; i++) {
		next = hash_table[i];
		while( next ) {
			x = next->next;
			free( next );
			next = x;
		}
	}
	memset( hash_table, 0, sizeof(type_creator_link_t *) * HASH_SIZE );
	icons_in_cache = 0;
	delete_mask_bitmaps();
}

HANDLE CHFVExplorerDoc::
get_fref( int volinx, CatDataRec *pCDR, INTEGER id )
{
	HANDLE h = 0;

	h = mac_load_any_resource ( 
				volinx, pCDR,
				(unsigned long)'FREF',
				0, id );
	return(h);
}

HANDLE CHFVExplorerDoc::
get_fref( CFile *fp, unsigned long g_offset, INTEGER id )
{
	HANDLE h = 0;

	h = mac_load_any_resource2 ( 
				fp, g_offset,
				(unsigned long)'FREF',
				0, id );
	return(h);
}

unsigned long CHFVExplorerDoc::
hash_func (OSType type, OSType creator)
{
  unsigned long retval;

  retval = ((unsigned long) type * creator) % HASH_SIZE;
  return retval;
}

type_creator_link_t ** CHFVExplorerDoc::
hashpp (OSType type, OSType creator)
{
  type_creator_link_t **retval;

	/*
  for (retval = &hash_table[hash_func (type, creator)];
       *retval && ((*retval)->type != type || (*retval)->creator != creator);
       retval = &(*retval)->next)
    ;
	*/

	retval = &hash_table[(int)hash_func (type, creator)];

	while( *retval ) {
		if( (*retval)->type == type && (*retval)->creator == creator ) {
			break;
		}
	  retval = &(*retval)->next;
	}
  return( retval );
}

int CHFVExplorerDoc::insert_into_hash_table (
	OSType type, 
	OSType creator, 
	local_icn_sharp_t *ip16,
	local_icn_sharp_t *ip32
	)
{
	/*
	if( type == 'APPL' && creator == 'SITx' ) {
		int i;
		i = 0;
	}
	*/
  if (ip16->icon || ip32->icon) {
    type_creator_link_t **linkpp;
    
    linkpp = hashpp (type, creator);
    if (!*linkpp) {
      type_creator_link_t *newlink;
      newlink = (type_creator_link_t *)malloc (sizeof (*newlink));
      if (newlink) {
        newlink->type = type;
        newlink->creator = creator;
        newlink->icon16 = ip16->icon;
        newlink->icon32 = ip32->icon;
        newlink->next = 0;
        *linkpp = newlink;
				icons_in_cache++;
				return(1);
      }
    }
  }
	return(0);
}

void outss( char *s, int i )
{
	char a[100];
	sprintf( a, "%d %08x", i, i );
	OutputDebugString( s );
	OutputDebugString( " " );
	OutputDebugString( a );
	OutputDebugString( "\r\n" );
}

// #define outs(s,i) outss(s,(int)i)
#define outs(s,i) 

#pragma optimize("g",off)
bndl_section_t *CHFVExplorerDoc::
extract_bndl_section (bndl_t **bndl_h, ResType code)
{
  bndl_section_t *retval;
  int i, n_sects, n_mappings;
	Integer nix;
	unsigned long nixlong;

outs( "bndl_h", bndl_h );
outs( "*bndl_h", *bndl_h );
outs( "code", code );

// OutputDebugString( "extract_bndl_section @start\r\n" );
	nix = (*bndl_h)->n_sections_minus_1;
outs( "nix", nix );
	MACPC_W( nix );

	// avoid some buggy files.
	if(nix > 255) {
		// OutputDebugString( "extract_bndl_section: strange section count, probably corrupt file\r\n" );
		nix &= 255;
	}

  n_sects = nix + 1;
outs( "n_sects", n_sects );
	retval = (*bndl_h)->section;
outs( "retval", retval );
  for (i = 0; i < n_sects; i++ ) {
			nixlong = retval->code;
			MACPC_D( nixlong );
			if(nixlong == code) break;
      
			nix = retval->n_mappings_minus_1;
			MACPC_W( nix );
      n_mappings = nix + 1;
      retval = (bndl_section_t *)
        ((char *)retval + sizeof (bndl_section_t)
                        + (n_mappings-1) * sizeof (local_mapping_t));
                         /*^^^^^^^^^^^^*/
                         /* we subtract one ANSI doesn't allow zero sized
                            arrays */
    }
  if (i >= n_sects)
    retval = 0;
outs( "retval", retval );
// OutputDebugString( "extract_bndl_section @end\r\n" );
  return retval;
}
#pragma optimize("g",on)

int CHFVExplorerDoc::
find_local_icon(
	INTEGER local_id, 
	local_icn_sharp_t 
	local_icns[],
  INTEGER n_icn )
{
  int i, retval;
  
  for (i = 0; i < n_icn && local_icns[i].local_id != local_id; ++i)
    ;
  retval = i < n_icn ? i : -1;
  return retval;
}

void CHFVExplorerDoc::
process_bndl (
	int volinx, 
	CatDataRec *pCDR,
	bndl_t **bndl_h, 
	OSType creator
)
{
  if (bndl_h) {
	    bndl_section_t *sectp;
	    int n_icn, n_fref;
	    local_icn_sharp_t *local_icns16;
	    local_icn_sharp_t *local_icns32;
	    int i;
			Integer nix;
			LongInt nixlong;
	
	    sectp = extract_bndl_section (bndl_h, 'ICN#');
	    if (sectp) {
				nix = sectp->n_mappings_minus_1;
				MACPC_W( nix );
		    n_icn = nix + 1;
		    local_icns16 = (local_icn_sharp_t *)malloc (n_icn * sizeof (*local_icns16));
		    local_icns32 = (local_icn_sharp_t *)malloc (n_icn * sizeof (*local_icns32));
		    for (i = 0; i < n_icn ; ++i) {
						nix = sectp->mapping[i].local_id;
						MACPC_W( nix );
		        local_icns16[i].local_id = nix;
		        local_icns32[i].local_id = nix;
						nix = sectp->mapping[i].resource_id;
						MACPC_W( nix );

						// memset( &local_icns16[i], 0, sizeof(local_icn_sharp_t) );
						// memset( &local_icns32[i], 0, sizeof(local_icn_sharp_t) );

						mac_load_icon2( volinx, pCDR, nix, 1, &local_icns16[i] );
						mac_load_icon2( volinx, pCDR, nix, 0, &local_icns32[i] );
						add_to_icon_cache( local_icns16[i].icon );
						add_to_icon_cache( local_icns32[i].icon );


						// MISSING_FREE? Handled properly below?
		    }
		    sectp = extract_bndl_section (bndl_h, 'FREF');
		    if (sectp) {
					nix = sectp->n_mappings_minus_1;
					MACPC_W( nix );
			    n_fref = nix + 1;
			    for (i = 0; i < n_fref ; ++i) {
							HANDLE h;
			        fref_hand fref_h;
			
							nix = sectp->mapping[i].resource_id;
							MACPC_W( nix );
			        h = get_fref( volinx, pCDR, nix );
			        if (h) {
									unsigned char *lpstr = (unsigned char *)GlobalLock( h );
									int i16, i32;
									fref_h = (fref_hand)&lpstr;
									nix = (*fref_h)->local_id;
									MACPC_W( nix );
			            i16 = find_local_icon (nix, local_icns16, n_icn);
									i32 = find_local_icon (nix, local_icns32, n_icn);
									nixlong = (*fref_h)->type;
									MACPC_D( nixlong );

									// THIS VALIDATION WAS MISSING. lp 19980423
									if(i16 >= 0 && i32 >= 0) {
										if( insert_into_hash_table (
												nixlong, creator, 
												&local_icns16[i16], &local_icns32[i32] ) ) 
										{
											save_hash_table( 
													nixlong, creator, 
													&local_icns16[i16], &local_icns32[i32] );
										}
									}

									GlobalUnlock( h );
									GlobalFree( h );
			          }
			      }
			    }
			    for (i = 0; i < n_icn ; ++i) {
						if(local_icns16[i].h) GlobalFree(local_icns16[i].h);
						if(local_icns32[i].h) GlobalFree(local_icns32[i].h);
						if(local_icns16[i].color_icon_mask) free(local_icns16[i].color_icon_mask);
						if(local_icns32[i].color_icon_mask) free(local_icns32[i].color_icon_mask);
					}
		      free (local_icns16);
		      free (local_icns32);
		    }
	}
}

void CHFVExplorerDoc::
process_bndl2 (
	CFile *fp,
	unsigned long g_offset,
	bndl_t **bndl_h, 
	OSType creator
)
{
  if (bndl_h) {
	    bndl_section_t *sectp;
	    int n_icn, n_fref;
	    local_icn_sharp_t *local_icns16;
	    local_icn_sharp_t *local_icns32;
	    int i;
			Integer nix;
			LongInt nixlong;
	
	    sectp = extract_bndl_section (bndl_h, 'ICN#');
	    if (sectp) {
				nix = sectp->n_mappings_minus_1;
				MACPC_W( nix );
		    n_icn = nix + 1;
		    local_icns16 = (local_icn_sharp_t *)malloc (n_icn * sizeof (*local_icns16));
		    local_icns32 = (local_icn_sharp_t *)malloc (n_icn * sizeof (*local_icns32));
		    for (i = 0; i < n_icn ; ++i) {
						nix = sectp->mapping[i].local_id;
						MACPC_W( nix );
		        local_icns16[i].local_id = nix;
		        local_icns32[i].local_id = nix;
						nix = sectp->mapping[i].resource_id;
						MACPC_W( nix );
						mac_load_icon3( fp, g_offset, nix, 1, &local_icns16[i] );
						mac_load_icon3( fp, g_offset, nix, 0, &local_icns32[i] );
						add_to_icon_cache( local_icns16[i].icon );
						add_to_icon_cache( local_icns32[i].icon );
						// MISSING_FREE? Handled properly below?
		    }
		    sectp = extract_bndl_section (bndl_h, 'FREF');
		    if (sectp) {
					nix = sectp->n_mappings_minus_1;
					MACPC_W( nix );
			    n_fref = nix + 1;
			    for (i = 0; i < n_fref ; ++i) {
							HANDLE h;
			        fref_hand fref_h;
			
							nix = sectp->mapping[i].resource_id;
							MACPC_W( nix );
			        h = get_fref( fp, g_offset, nix );
			        if (h) {
									unsigned char *lpstr = (unsigned char *)GlobalLock( h );
									int i16, i32;
									fref_h = (fref_hand)&lpstr;
									nix = (*fref_h)->local_id;
									MACPC_W( nix );
			            i16 = find_local_icon (nix, local_icns16, n_icn);
			            i32 = find_local_icon (nix, local_icns32, n_icn);
									nixlong = (*fref_h)->type;
									MACPC_D( nixlong );

									// THIS VALIDATION WAS MISSING. lp 19980423
									if(i16 >= 0 && i32 >= 0) {
										if( insert_into_hash_table (
											nixlong, creator, 
											&local_icns16[i16], &local_icns32[i32] ) )
										{
											save_hash_table( 
													nixlong, creator, 
													&local_icns16[i16], &local_icns32[i32] );
										}
									}
									GlobalUnlock( h );
									GlobalFree( h );
			          }
			      }
			    }
			    for (i = 0; i < n_icn ; ++i) {
						if(local_icns16[i].h) GlobalFree(local_icns16[i].h);
						if(local_icns32[i].h) GlobalFree(local_icns32[i].h);
					}
		      free (local_icns16);
		      free (local_icns32);
		    }
	}
}

icn_sharp_hand CHFVExplorerDoc::
map_type_and_creator (
	OSType type, 
	OSType creator,
	int prefer_small )
{
  type_creator_link_t **linkpp;
  icn_sharp_hand retval = 0;

  linkpp = hashpp (type, creator);
	if( *linkpp ) {
		if( prefer_small ) {
			retval = (*linkpp)->icon16;
		}
		if( !retval ) 
			retval = (*linkpp)->icon32;
	}
  return retval;
}

void CHFVExplorerDoc::process_bundle(
	int volinx, 
	CatDataRec *pCDR,
	unsigned long creator )
{
	HANDLE h = 0;
	LPSTR lpstr;

	if( (pCDR->u.f.filUsrWds.fdFlags & kHasBundle) == 0 ) return;
	h = mac_load_any_resource ( volinx, pCDR, (unsigned long)'BNDL', 1, 0 );
	if(!h) return;
	lpstr = (LPSTR)GlobalLock( h );
	if(lpstr) {
		process_bndl( volinx, pCDR, (bndl_t **)&lpstr, creator );
		GlobalUnlock( h );
		GlobalFree( h );
	}
}

// here bundle is known to exist
void CHFVExplorerDoc::process_bundle2 (
	CFile *fp,
	unsigned long fork_start,
	unsigned long creator )
{
	HANDLE h = 0;
	LPSTR lpstr;
	h = mac_load_any_resource2( fp, fork_start, (unsigned long)'BNDL', 1, 0 );
	if(!h) return;
	lpstr = (LPSTR)GlobalLock( h );
	if(lpstr) {
		process_bndl2( fp, fork_start, (bndl_t **)&lpstr, creator );
		GlobalUnlock( h );
		GlobalFree( h );
	}
}

/*
	8,6		== used to be
	10,6	== alike but without dragon
	12,8  == old fashioned (no mask)
	12,12 == about same
	8,12  == good, but NOT with white window background -- damn!
	0,12  == old fashioned, white background
*/

/*
unsigned char funfun( unsigned char a, unsigned char b, int func )
{
	unsigned char f;
	switch(func) {
		case  0: f = 0        ; 	break;
		case  1: f = (!a)&(!b);		break;
		case  2: f = (!a)&  b ;		break;
		case  3: f =  !a      ;		break;
		case  4: f =   a &(!b);		break;
		case  5: f =       !b ;		break;
		case  6: f =   a ^  b ;		break;
		case  7: f = (!a)|(!b);		break;
		case  8: f =   a &  b ;		break;
		case  9: f = !(a ^  b);		break;
		case 10: f =        b ;		break;
		case 11: f = (!a)|  b ;		break;
		case 12: f =   a      ;		break;
		case 13: f =   a |(!b);		break;
		case 14: f =   a |  b ;		break;
		case 15: f = 255      ;		break;
	}
	return(f);
}
*/

void adjust_bw_buffer( unsigned char *p, int count )
{
	int i, half = (count>>1);
	unsigned char *ANDbitmask, *XORbitmask, and;

	negate_buffer( p, count );

	ANDbitmask = p;
	XORbitmask = p + half;

	for(i=0; i<half; i++) {
		and = *ANDbitmask;
		*ANDbitmask++ &= *XORbitmask;
		*XORbitmask++ ^= and;
	}

	/*
	unsigned char a, b, a1, b1;
	int fa = ::GetProfileInt("Test","fa",0);
	int fb = ::GetProfileInt("Test","fb",0);

	for(i=0; i<half; i++) {
		a = *ANDbitmask;
		b = *XORbitmask;
		a1 = funfun( a,b,fa );
		b1 = funfun( a,b,fb );
		*ANDbitmask = a1;
		*XORbitmask = b1;
		ANDbitmask++;
		XORbitmask++;
	}
	*/
}

void CHFVExplorerDoc::create_cache_icon( 
	local_icn_sharp_t *dat,
	int invert )
{
	unsigned char *p;
	int half;

	if(dat->h) {
		p = (unsigned char *)GlobalLock( dat->h );

		if(dat->iscolor) {
			dat->bytes = dat->small ? (COLORICONSIZE>>2) : COLORICONSIZE;
			dat->icon = map_colors( 
				(unsigned char *)p, 
				dat->small, 
				dat->bytes, 
				&dat->color_icon_mask );
		} else {
			dat->bytes = dat->small ? (BW_ICONSIZE>>2) : BW_ICONSIZE;
			half = dat->bytes >> 1;

			// dump_data( (unsigned char *)"c:\\koe2.txt", p, 1024 );

			// memset( &p[137], 0, 255 );

			if(invert) {
				adjust_bw_buffer( p, dat->bytes );
			}

			CHFVExplorerApp	*pApp = (CHFVExplorerApp *)AfxGetApp();
			dat->icon = CreateIcon( 
				pApp->m_hInstance, dat->dim, dat->dim, 1, 1, 
				(unsigned char *)p,					// AND
				(unsigned char *)p + half		// XOR
			);
		}
		GlobalUnlock( dat->h );
	}
}

void CHFVExplorerDoc::mac_load_icon2(
	int volinx, 
	CatDataRec *pCDR,
	Integer id, 
	int small,
	local_icn_sharp_t *dat )
{
	dat->h = 0;
	dat->icon = 0;
	dat->dim = small ? 16 : 32;
	dat->iscolor = 0;
	dat->small = small;
	dat->color_icon_mask = 0;

#ifdef NO_SMALL_ICONS
	if(small) return;
#endif

	if(m_bits_per_pixel >= 8) {
		dat->h = mac_load_any_resource ( 
					volinx, pCDR,
					dat->small ? (unsigned long)'ics8' :	(unsigned long)'icl8',
					0, id );
	}
	if(dat->h) {
		dat->iscolor = 1;
	} else {
		dat->h = mac_load_any_resource( 
					volinx,
					pCDR,
					small ? (unsigned long)'ics#' : (unsigned long)'ICN#',
					0, id );
	}
	create_cache_icon( dat, 1 );
}

void CHFVExplorerDoc::mac_load_icon3( 
	CFile *fp,
	unsigned long g_offset,
	Integer id, 
	int small,
	local_icn_sharp_t *dat )
{
	dat->h = 0;
	dat->icon = 0;
	dat->dim = small ? 16 : 32;
	dat->iscolor = 0;
	dat->small = small;
	dat->color_icon_mask = 0;

#ifdef NO_SMALL_ICONS
	if(small) return;
#endif

	if(m_bits_per_pixel >= 8) {
		dat->h = mac_load_any_resource2 ( 
					fp, g_offset,
					dat->small ? (unsigned long)'ics8' :	(unsigned long)'icl8',
					0, id );
	}
	if(dat->h) {
		dat->iscolor = 1;
	} else {
		dat->h = mac_load_any_resource2 ( 
					fp, g_offset,
					dat->small ? (unsigned long)'ics#' : (unsigned long)'ICN#',
					0, id );
	}
	create_cache_icon( dat, 1 );
}

// do not hash, do not save to disk, DO add to memory cache
void CHFVExplorerDoc::load_custom_icons( 
	int volinx, 
	CatDataRec *pCDR,
	HICON *phicon16, 
	HICON *phicon32 
)
{
	local_icn_sharp_t ic16, ic32;

	mac_load_icon2( volinx, pCDR, kCustomIconResource, 1, &ic16 );
	mac_load_icon2( volinx, pCDR, kCustomIconResource, 0, &ic32 );
	add_to_icon_cache( ic16.icon );
	add_to_icon_cache( ic32.icon );
	*phicon16 = ic16.icon;
	*phicon32 = ic32.icon;
	// MISSING_FREE - FIXED
	if(ic16.color_icon_mask) free( ic16.color_icon_mask );
	if(ic32.color_icon_mask) free( ic32.color_icon_mask );
	// if(ic16.icon) AfxMessageBox( "File has small 0xBFB9." );
	// if(ic32.icon) AfxMessageBox( "File has large 0xBFB9." );
}

void CHFVExplorerDoc::load_custom_icons( 
	CFile *fp,
	unsigned long g_offset,
	HICON *phicon16, 
	HICON *phicon32 
)
{
	local_icn_sharp_t ic16, ic32;
	mac_load_icon3( fp, g_offset, kCustomIconResource, 1, &ic16 );
	mac_load_icon3( fp, g_offset, kCustomIconResource, 0, &ic32 );
	add_to_icon_cache( ic16.icon );
	add_to_icon_cache( ic32.icon );
	*phicon16 = ic16.icon;
	*phicon32 = ic32.icon;
	// MISSING_FREE - FIXED
	if(ic16.color_icon_mask) free( ic16.color_icon_mask );
	if(ic32.color_icon_mask) free( ic32.color_icon_mask );
	// if(ic16.icon) AfxMessageBox( "File has small 0xBFB9." );
	// if(ic32.icon) AfxMessageBox( "File has large 0xBFB9." );
}
