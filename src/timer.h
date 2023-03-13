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

#ifdef __cplusplus
extern "C" {
#endif

extern DWORD __Ttim;
extern HCURSOR __TScurs;
#define START_TIME_CONSUMING(ticks) __Ttim = GetTickCount()+ticks; __TScurs = 0;
#define END_TIME_CONSUMING if(__TScurs) _end_time_consuming(__TScurs)
#define UPDATE_TIME_CONSUMING(update) if(!__TScurs && (GetTickCount() > __Ttim)) { __TScurs = _start_time_consuming(update); __Ttim = 0x7FFFFFFF; }
void _end_time_consuming( HCURSOR hcursor );
HCURSOR _start_time_consuming(int update);

#ifdef __cplusplus
}
#endif
