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
#include "icon.h"

// Phew. this was hard. iterative process with no docs...

#pragma pack(1)
typedef struct ICONDIR {
    WORD          idReserved;
    WORD          idType;
    WORD          idCount;
    // ICONDIRENTRY  idEntries[1];
} ICONHEADER;
  
typedef struct {
    BYTE  bWidth;
    BYTE  bHeight;
    BYTE  bColorCount;
    BYTE  bReserved;
    WORD  wPlanes;
    WORD  wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
} IconDirectoryEntry;
#pragma pack()

#define COLORICONSIZE 1024
#define BW_ICONSIZE	256

static void make_std_colors( RGBQUAD *c )
{
	int i;
	for(i=0; i<16; i++) {
		c[i].rgbReserved = 0;
	}
	c[0].rgbBlue		= 0;
	c[0].rgbGreen		= 0;
	c[0].rgbRed			= 0;
	c[1].rgbBlue		= 0;
	c[1].rgbGreen		= 0;
	c[1].rgbRed			= 0x80;
	c[2].rgbBlue		= 0;
	c[2].rgbGreen		= 0x80;
	c[2].rgbRed			= 0;
	c[3].rgbBlue		= 0;
	c[3].rgbGreen		= 0x80;
	c[3].rgbRed			= 0x80;
	c[4].rgbBlue		= 0x80;
	c[4].rgbGreen		= 0;
	c[4].rgbRed			= 0;
	c[5].rgbBlue		= 0x80;
	c[5].rgbGreen		= 0;
	c[5].rgbRed			= 0x80;
	c[6].rgbBlue		= 0x80;
	c[6].rgbGreen		= 0x80;
	c[6].rgbRed			= 0;
	c[7].rgbBlue		= 0xC0;
	c[7].rgbGreen		= 0xC0;
	c[7].rgbRed			= 0xC0;
	c[8].rgbBlue		= 0x80;
	c[8].rgbGreen		= 0x80;
	c[8].rgbRed			= 0x80;
	c[9].rgbBlue		= 0;
	c[9].rgbGreen		= 0;
	c[9].rgbRed			= 0xFF;
	c[10].rgbBlue		= 0;
	c[10].rgbGreen	= 0xFF;
	c[10].rgbRed		= 0;
	c[11].rgbBlue		= 0;
	c[11].rgbGreen	= 0xFF;
	c[11].rgbRed		= 0xFF;
	c[12].rgbBlue		= 0xFF;
	c[12].rgbGreen	= 0;
	c[12].rgbRed		= 0;
	c[13].rgbBlue		= 0xFF;
	c[13].rgbGreen	= 0;
	c[13].rgbRed		= 0xFF;
	c[14].rgbBlue		= 0xFF;
	c[14].rgbGreen	= 0xFF;
	c[14].rgbRed		= 0;
	c[15].rgbBlue		= 0xFF;
	c[15].rgbGreen	= 0xFF;
	c[15].rgbRed		= 0xFF;
}

int SaveIcon( HICON hicon16, HICON hicon32, char *lpszIconFile )
{
	ICONINFO iconinfo32;
	ICONHEADER ih;
	IconDirectoryEntry ide;
	BITMAPINFO *lpbmi = 0;
	CFile cf;
	int size32 = 0, ncolors;
	HDC hdc = 0;
	char *lpvBitsMask = 0, *lpvBitsColor;
	int ret = 0;
	int datasize1, datasize2, part1height;
	
	if(!GetIconInfo(hicon32,&iconinfo32)) return(0);
	part1height = iconinfo32.hbmColor ? 32 : 64;

	if(!cf.Open( lpszIconFile, CFile::modeReadWrite|CFile::modeCreate )) {
		return(0);
	}

  ih.idReserved = 0;
  ih.idType = 1;
  ih.idCount = 1;
	cf.Write( &ih, sizeof(ih) );

	cf.Write( &ide, sizeof(ide) );

	hdc = ::GetDC(0);
	lpvBitsMask = (char *)malloc( 5000 );
	lpvBitsColor = (char *)malloc( 5000 );

  lpbmi = (BITMAPINFO *)malloc( sizeof(BITMAPINFOHEADER) + 5000 );

  lpbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  lpbmi->bmiHeader.biWidth = 32;
  lpbmi->bmiHeader.biHeight = part1height;
  lpbmi->bmiHeader.biPlanes = 1;
  lpbmi->bmiHeader.biBitCount = 1;
  lpbmi->bmiHeader.biCompression = 0;
  lpbmi->bmiHeader.biSizeImage = COLORICONSIZE;
  lpbmi->bmiHeader.biXPelsPerMeter = 0;
  lpbmi->bmiHeader.biYPelsPerMeter = 0;
  lpbmi->bmiHeader.biClrUsed = 0;
  lpbmi->bmiHeader.biClrImportant = 0;
	make_std_colors( lpbmi->bmiColors );

	GetDIBits(
		hdc,
		iconinfo32.hbmMask,
		0, part1height,
		NULL,
		(LPBITMAPINFO)lpbmi,
		DIB_RGB_COLORS
	);
	GetDIBits(
		hdc,
		iconinfo32.hbmMask,
		0, part1height,
		lpvBitsMask,
		(LPBITMAPINFO)lpbmi,
		DIB_RGB_COLORS
	);
	datasize1 = lpbmi->bmiHeader.biSizeImage;
	datasize2 = 0;
	if(iconinfo32.hbmColor) {
		// make_std_colors( lpbmi->bmiColors );
	  lpbmi->bmiHeader.biHeight = 32;
	  lpbmi->bmiHeader.biSizeImage = COLORICONSIZE;

		// lp changed 4 -> 8 for version 1.2.7
	  lpbmi->bmiHeader.biBitCount = 8;

		GetDIBits(
			hdc,
			iconinfo32.hbmColor,
			0, lpbmi->bmiHeader.biHeight,
			NULL,
			(LPBITMAPINFO)lpbmi,
			DIB_RGB_COLORS
		);
		GetDIBits(
			hdc,
			iconinfo32.hbmColor,
			0, 32,
			lpvBitsColor,
			(LPBITMAPINFO)lpbmi,
			DIB_RGB_COLORS
		);
		datasize2 = lpbmi->bmiHeader.biSizeImage;
	}
	switch(lpbmi->bmiHeader.biBitCount) {
		case 1:
      ncolors = 2;
			break;
		case 4:
      ncolors = 16;
			break;
		case 8:
      ncolors = 256;
			break;
		default:
      ncolors = 0;
			break;
	}

	size32 = datasize1 + datasize2 +
					 sizeof(BITMAPINFOHEADER) + ncolors*sizeof(RGBQUAD);
	ret = 1;

  lpbmi->bmiHeader.biHeight = 64;
	cf.Write( lpbmi, sizeof(BITMAPINFOHEADER)+ncolors*sizeof(RGBQUAD) );

	if(iconinfo32.hbmColor) {
		cf.Write( lpvBitsColor, datasize2 );
	}
	cf.Write( lpvBitsMask, datasize1 );

	::ReleaseDC(0,hdc);
 	
	free(lpbmi);
	free(lpvBitsMask);
	free(lpvBitsColor);

	cf.Seek(sizeof(ih),CFile::begin);

	ide.bWidth = 32;
	ide.bHeight = 32;
	ide.bColorCount = ncolors;
	ide.bReserved = 0;
	ide.wPlanes = 0;
	ide.wBitCount = 0;
	ide.dwBytesInRes = size32;
	ide.dwImageOffset = sizeof(ih) + sizeof(ide);
	cf.Write( &ide, sizeof(ide) );

	cf.Close();

	return(ret);
}
