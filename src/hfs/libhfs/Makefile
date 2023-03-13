###############################################################################
#
# hfsutils - tools for reading and writing Macintosh HFS volumes
# Copyright (C) 1997 Marcus Better
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
###############################################################################

EXTRA_CFLAGS = -DNODEVLOCKS

ARFLAGS =	rcs

###############################################################################

NAME =		libhfs
TARGET =	$(NAME).a
OBJS =		data.o block.o low.o file.o btree.o node.o record.o volume.o  \
		hfs.o os2open.o

###############################################################################

include ../Rules.make

all :: $(TARGET)

clean ::
	-del *.o 2> NUL:
	-del $(TARGET) 2> NUL:
	-del *~ 2> NUL:

###############################################################################

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

### DEPENDENCIES FOLLOW #######################################################

block.o: block.c internal.h hfs.h data.h block.h low.h
btree.o: btree.c internal.h hfs.h data.h block.h file.h btree.h node.h
data.o: data.c internal.h hfs.h data.h btree.h
file.o: file.c internal.h hfs.h data.h block.h file.h btree.h record.h \
 volume.h
hfs.o: hfs.c internal.h hfs.h data.h block.h low.h file.h btree.h \
 node.h record.h volume.h os2open.h
low.o: low.c internal.h hfs.h data.h block.h low.h file.h
node.o: node.c internal.h hfs.h data.h btree.h node.h
record.o: record.c internal.h hfs.h data.h record.h
volume.o: volume.c internal.h hfs.h data.h low.h btree.h record.h \
 volume.h
os2open.c: os2open.h
