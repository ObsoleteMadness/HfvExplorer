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

#ifndef _MACTYPES_H_
#define _MACTYPES_H_

#include "charset.h"

#pragma pack(1)
typedef unsigned short Integer;
typedef unsigned long LongInt;
typedef signed char SignedByte;

struct hfs_point {
	Integer	v;
	Integer	h;
};

struct hfs_rect {
	Integer	top;
	Integer	left;
	Integer	bottom;
	Integer	right;
};

typedef unsigned char Str15[16];
typedef unsigned char String27[28];
typedef unsigned char Str31[32];

typedef struct {
	struct   hfs_rect	 frRect;
	Integer  frFlags;
	struct   hfs_point frLocation;
	Integer  frView;
} DInfo; // Finder information

typedef struct {
	struct hfs_point frScroll;
	LongInt		 frOpenChain;
	Integer		 frUnused;
	Integer		 frComment;
	LongInt		 frPutAway;
} DXInfo; // additional Finder information

// If the document is of file type 'TEXT' or 'PICT' and 
// (or 'ttro')
// if the TeachText application is available -> ask

// The document "x" could not be
// opened, because the application program
// that created it could not be found.

// The document "x" could not be
// opened, because the application
// "y" could not be found.

// custom message...

// The document "x" could not be
// opened, because the application program
// that created it could not be found. Do you
// want to openit using "TeachText"?

// 'STR '
#define kCustomMissingName (WORD)(-16396)
#define kCustomMissingMessage (WORD)(-16397)

// 'ICN#'
#define kCustomIconResource (WORD)(-16455)

#define fInvisible			16384
#define kIsOnDesk				0x1  // unused
#define kColorMask			0xE
#define kIsShared				0x40
#define kHasBeenInited	0x100
#define kHasCustomIcon	0x400
#define kIsStationery		0x800
#define kNameLocked			0x1000
#define kHasBundle			0x2000
#define kIsInvisible		0x4000
#define kIsAlias				0x8000

typedef struct {
	Integer			bbID; 					// boot blocks signature
	LongInt			bbEntry; 				// entry point to boot code
	Integer			bbVersion; 			// boot blocks version number
	Integer			bbPageFlags; 		// used internally
	Str15				bbSysName;   		// System filename
	Str15				bbShellName;   	// Finder filename
	Str15				bbDbg1Name;   	// debugger filename
	Str15				bbDbg2Name;   	// debugger filename
	Str15				bbScreenName;   // name of startup screen
	Str15				bbHelloName;  	// name of startup program
	Str15				bbScrapName;  	// name of system scrap file
	Integer			bbCntFCBs;			// number of FCBs to allocate
	Integer			bbCntEvts;			// number of event queue elements
	LongInt			bb128KSHeap;		// system heap size on 128K Mac
	LongInt			bb256KSHeap;		// used internally
	LongInt			bbSysHeapSize;	// system heap size on all machines
	Integer			filler;					// reserved
	LongInt			bbSysHeapExtra; // additional system heap space
	LongInt			bbSysHeapFract; // fraction of RAM for system heap
} BootBlkHdr; // boot block header

typedef struct {
	LongInt		 fdType;
	LongInt		 fdCreator;
	Integer		 fdFlags;
	struct hfs_point fdLocation;
	Integer		 fdFldr;
} FInfo; // Finder information

typedef struct {
	Integer	fdIconID;
	Integer	fdUnused[4];
	Integer	fdComment;
	LongInt	fdPutAway;
} FXInfo; // additional Finder information

typedef struct {
	Integer  xdrStABN;       // first allocation block
	Integer  xdrNumABlks;    // number of allocation blocks
} ExtDescriptor;

typedef struct { // master directory block
	Integer        drSigWord;				// volume signature
	LongInt        drCrDate;				// date and time of volume creation
	LongInt        drLsMod;					// date and time of last modification
	Integer        drAtrb;					// volume attributes
	Integer        drNmFls;					// number of files in root directory
	Integer        drVBMSt;					// first block of volume bitmap
	Integer        drAllocPtr;			// start of next allocation search
	Integer        drNmAlBlks;			// number of allocation blocks in volume
	LongInt        drAlBlkSiz;			// size (in bytes) of allocation blocks
	LongInt        drClpSiz;				// default clump size
	Integer        drAlBlSt;				// first allocation block in volume
	LongInt        drNxtCNID;				// next unused catalog node ID
	Integer        drFreeBks;				// number of unused allocation blocks
	String27       drVN;						// volume name
	LongInt        drVolBkUp;				// date and time of last backup
	Integer        drVSeqNum;				// volume backup sequence number
	LongInt        drWrCnt;					// volume write count
	LongInt        drXTClpSiz;			// clump size for extents overflow file
	LongInt        drCTClpSiz;			// clump size for catalog file
	Integer        drNmRtDirs;			// number of directories in root directory
	LongInt        drFilCnt;				// number of files in volume
	LongInt        drDirCnt;				// number of directories in volume
	LongInt        drFndrInfo[8];		// information used by the Finder
	Integer        drVCSize;				// size (in blocks) of volume cache
	Integer        drVBMCSize;			// size (in blocks) of volume bitmap cache
	Integer        drCtlCSize;			// size (in blocks) of common volume cache
	LongInt        drXTFlSize;			// size of extents overflow file
	ExtDescriptor  drXTExtRec[3];	  // extent record for extents overflow file
	LongInt        drCTFlSize;			// size of catalog file
	ExtDescriptor  drCTExtRec[3];		// extent record for catalog file
} MDB;

typedef struct {
	LongInt       ndFLink;       // forward link
	LongInt       ndBLink;       // backward link
	unsigned char	ndType;			   // node type
	SignedByte    ndNHeight;     // node level
	Integer       ndNRecs;       // number of records in node
	Integer       ndResv2;       // reserved
} NodeDescriptor;

#define NODESIZE 512

typedef struct {
	Integer        bthDepth;    // current depth of tree
	LongInt        bthRoot;     // number of root node
	LongInt        bthNRecs;    // number of leaf records in tree
	LongInt	       bthFNode;    // number of first leaf node
	LongInt        bthLNode;    // number of last leaf node
	Integer        bthNodeSize; // size of a node
	Integer        bthKeyLen;   // maximum length of a key
	LongInt        bthNNodes;   // total number of nodes in tree
	LongInt        bthFree;     // number of free nodes
	SignedByte     bthResv[76]; // reserved
} BTHdrRec;    // B*-tree header

typedef struct {
	SignedByte        ckrKeyLen;    // key length
	SignedByte        ckrResrv1;    // reserved
	LongInt						ckrParID;     // parent directory ID
	Str31							ckrCName;			// catalog node name
} CatKeyRec; // catalog key record

typedef enum {cdrDirRec=1, cdrFilRec, cdrThdRec, cdrFThdRec} CatDataType;

typedef struct {
	SignedByte        cdrType;				// record type
	SignedByte        cdrResrv2;			// reserved
	union {
		struct cdrType_cdrDirRec { // directory record
      Integer    dirFlags;     // directory flags
      Integer		 dirVal;       // directory valence
      LongInt		 dirDirID;     // directory ID
      LongInt		 dirCrDat;     // date and time of creation
      LongInt		 dirMdDat;     // date and time of last modification
      LongInt		 dirBkDat;     // date and time of last backup
      DInfo			 dirUsrInfo;   // Finder information
      DXInfo		 dirFndrInfo;  // additional Finder information
			LongInt		 dirResrv[4];  // reserved
		} d;
		struct cdrType_cdrFilRec { // file record
      SignedByte			filFlags;     // file flags
      SignedByte			filTyp;				// file type
      FInfo						filUsrWds;    // Finder information
      LongInt					filFlNum;     // file ID
      Integer					filStBlk;     // first alloc. blk. of data fork
      LongInt					filLgLen;     // logical EOF of data fork
      LongInt					filPyLen;     // physical EOF of data fork
      Integer					filRStBlk;    // first alloc. blk. of resource fork
      LongInt					filRLgLen;    // logical EOF of resource fork
      LongInt					filRPyLen;    // physical EOF of resource fork
      LongInt					filCrDat;     // date and time of creation
      LongInt					filMdDat;     // date and time of last modification
      LongInt					filBkDat;     // date and time of last backup
      FXInfo					filFndrInfo;  // additional Finder information
      Integer					filClpSize;   // file clump size
			ExtDescriptor   filExtRec[3]; // first data fork extent record
      ExtDescriptor		filRExtRec[3]; // first resource fork extent record
      LongInt					filResrv;     // reserved
		} f;
		struct cdrType_cdrThdRec { // directory thread record
			LongInt					thdResrv[2]; // reserved
      LongInt					thdParID;    // parent ID for this directory
      Str31						thdCName;    // name of this directory
		} dt;
		struct cdrType_cdrFThdRec { // file thread record
			LongInt					fthdResrv[2]; // reserved
      LongInt					fthdParID;    // parent ID for this file
      Str31						fthdCName;    // name of this file
		} ft;
	} u;
} CatDataRec; // catalog data records

typedef struct {
	SignedByte					xkrKeyLen;    // key length
	SignedByte					xkrFkType;    // fork type
	LongInt							xkrFNum;      // file number
	Integer							xkrFABN;      // starting file allocation block
} ExtKeyRec; // extent key record

// node types
#define ndIndxNode    (unsigned char)0x00     // index node
#define ndHdrNode     (unsigned char)0x01     // header node
#define ndMapNode     (unsigned char)0x02     // map node
#define ndLeafNode    (unsigned char)0xFF     // leaf node

#define CNID_ROOT_PARENT    1
#define CNID_ROOT_ID        2
#define CNID_EXTENTS_FILE   3
#define CNID_CATALOG_FILE   4
#define CNID_BAD_ALLOC_FILE 5

void macpc_w( char *s );
void macpc_d( char *s );
void macpc_str( unsigned char *s, int max_len );

void macpc_MDB( MDB *pMDB );
void macpc_ndescr( NodeDescriptor *pndescr );
void macpc_header_rec( BTHdrRec *pheader_rec );
void macpc_catkeyrec( CatKeyRec *pcatkeyrec );
void macpc_catdatarec( CatDataRec *pcatdatarec );

#define MACPC_W(s) macpc_w((char *)&(s))
#define MACPC_D(s) macpc_d((char *)&(s))

void pc_to_mac_charset( unsigned char *s );
void mac_to_pc_charset( unsigned char *s );
void mac_date_to_string( unsigned long date, char *buf );

typedef struct {
	unsigned long resource_data_offset;
	unsigned long resource_map_offset;
	unsigned long resource_data_length;
	unsigned long resource_map_length;
} resource_header;
void macpc_resource_header( resource_header *p );

typedef struct {
	char reserved_for_header_copy[16];
	char reserved_for_handle_next_map[4];
	char reserved_for_fref_number[2];
	Integer res_fork_attributes;
	Integer type_list_offset; // from the beginning of the map
	Integer name_list_offset; // from the beginning of the map
	// type list starts here
	// ref list follows
	// resource name list follows
} resource_map_header;
void macpc_resource_map_header( resource_map_header *p );

typedef struct {
	unsigned long res_type;
	Integer res_count1; // count-1
	Integer ref_list_offset; // from the beginning of res type list
} resource_type_item;
void macpc_resource_type_item( resource_type_item *p );

typedef struct {
	Integer res_id;
	Integer res_name_offset; // from the beginning of the name list
	unsigned char res_attrib;
	unsigned char res_data_offset_hi;
	Integer res_data_offset_lo; // from the beginning of the data
	LongInt reserved; // for handle to resource
} resource_ref_list_entry;
void macpc_resource_ref_list_entry( resource_ref_list_entry *p );


enum {
	resSysHeap					= 64,							/*System or application heap?*/
	resPurgeable				= 32,							/*Purgeable resource?*/
	resLocked						= 16,							/*Load it in locked?*/
	resProtected				= 8,							/*Protected?*/
	resPreload					= 4,							/*Load in on OpenResFile?*/
	resChanged					= 2, 							/*Resource changed?*/
	resCompressed       = 1 							/* lauri */
};


#define APPLE_SINGLE_MAGIC 0x00051600
#define APPLE_DOUBLE_MAGIC 0x00051607

#define APPLE_DOUBLE_REQUIRED_VERSION 0x00020000

typedef enum {
	Data_Fork = 1,
	Resource_Fork,
	Real_Name,
	Comment,
	Icon_BW,
	Icon_Color,
	File_Dates = 8,
	Finder_Info,
	Macintosh_File_Info,
	ProDOS_File_Info,
	MSDOS_File_Info,
	Short_Name,
	AFP_File_Info,
	Directory_ID
} apple_single_double_entry_type;

typedef struct {
	LongInt Magic;
	LongInt Version;
	unsigned char Filler[16];
	Integer entry_count;
} apple_single_double_header;
void mappc_apple_single_double_header(apple_single_double_header *p);

typedef struct {
	LongInt id;
	LongInt offset;
	LongInt length;
} apple_single_double_entry;
void mappc_apple_single_double_entry(apple_single_double_entry *p);

#ifdef __cplusplus
class CHFVVolume;

class CHFVFile
{
public:
	CHFVFile();
	~CHFVFile();

	int Open( CHFVVolume *pvol );
	void Close();
	LONG Seek( LONG lOff, UINT nFrom );
	UINT Read( void* lpBuf, UINT nCount );
	int intern_open();
	int open_special( CHFVVolume *pvol, int which );
	int open_by_CDR( CHFVVolume *pvol, CatDataRec *pCDR, int type );

	CString m_name;
	int m_opened;
	BTHdrRec m_header_rec;
	unsigned char m_map_record[256];

	enum SpecialOpenFile { OpenCatalog = 0x0, OpenExtents = 0x1 };
	enum OpenFileType { OpenResourceFork = 0x0, OpenDataFork = 0x1 };

protected:
	LongInt m_clump_size;
	LongInt m_file_size;
	ExtDescriptor m_first_extents[3];
	int m_isbtree;
	long m_start;
	long m_fast_limit;
	long m_offset;
	CHFVVolume *m_pvol;
};

#define IS_NO_FLOPPY (-1)

class CDSKFile : public CFile
{
public:
	CDSKFile();
	~CDSKFile();
	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	void Close();
	BOOL GetStatus( CFileStatus& rStatus );
	LONG Seek(LONG lOff, UINT nFrom);
	UINT Read(void* lpBuf, UINT nCount);
	DWORD GetLength(void);

	int m_drive;
	HANDLE m_hfloppy;
	HANDLE m_hcd;
	HANDLE m_hd;
	LONG m_seek;
};

class CHFVVolume
{
public:
	CHFVVolume();
	~CHFVVolume();
	BOOL find_volume_MDB();
	int update_if_needed(int ask);
	int open();
	void close();
	void update_time_stamp();

	// if volume is open these are guaranteed to be open too
	CHFVFile m_catalog_file;
	CHFVFile m_extents_file;
	unsigned long m_hfs_start;
	MDB m_MDB;
	int m_opened;
	CString m_file_name;
	CString m_volume_name;
	CFileStatus m_file_last_status;
	BOOL m_file_opened;
	CTime m_file_last_modified;
	unsigned long m_length;
	HTREEITEM m_rootitem;
	int m_free;
	int m_is_floppy;
	int m_is_cd;
	int m_is_hd;
	int m_is_removable;

	CDSKFile m_file;
	BOOL m_floppy_update_needed;

protected:
};

#define FINDER_MAGIC_OFFSET 20000
#define FINDER_MAGIC_LOW    (-24000)
#define FINDER_MAGIC_HIGH   (-16000)
int unmagic( int x );
int is_inside_magic( int x, int y );

enum {
	FINDER_VIEW_MODE_ICON = 0,
	FINDER_VIEW_MODE_ICON_SMALL,
	FINDER_VIEW_MODE_ICON_NAME,
	FINDER_VIEW_MODE_ICON_DATE,
	FINDER_VIEW_MODE_ICON_SIZE,
	FINDER_VIEW_MODE_ICON_KIND,
	FINDER_VIEW_MODE_ICON_LABEL,
	FINDER_VIEW_MODE_ICON_BUTTON
};
int get_finder_view_mode( int flags );

enum {
  FINDER_COLOR_NONE = 0,

  // Reverse order for some reason.
  FINDER_COLOR_ESSENTIAL = 7,
  FINDER_COLOR_HOT = 6,
  FINDER_COLOR_IN_PROGRESS = 5,
  FINDER_COLOR_COOL = 4,
  FINDER_COLOR_PERSONAL = 3,
  FINDER_COLOR_PROJECT_1 = 2,
  FINDER_COLOR_PROJECT_2 = 1
};

char *get_label_text( int finder_color );

/*
int finder_color = (pitem->catdatarec.u.f.filUsrWds.fdFlags & kColorMask) >> 1;

orange
red
magenta
cyan
blue
green
brown
*/

#endif // __cplusplus

#pragma pack()

#endif
