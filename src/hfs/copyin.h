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

extern char *cpi_error;

int cpi_macb(char *, hfsvol *, char *);
int cpi_binh(char *, hfsvol *, char *);
int cpi_text(char *, hfsvol *, char *);
int cpi_raw(char *, hfsvol *, char *);

// LAURI
int cpi_double( char *, char *, hfsvol *, char * );
int cpi_raw_resource( char *, hfsvol *, char * );

void copyin_set_raw_param( char *type, char *creator, int strip );
