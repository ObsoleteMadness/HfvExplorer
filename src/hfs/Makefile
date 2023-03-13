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

### PROJECT INFORMATION #######################################################

export NAME = hfsutils
export VERMAJOR =	0
export VERMINOR =	10

# Define WIN32 to build the Win32 version.
#export WIN32=YES

# Define DEBUG to get debug information.
#export DEBUG=1

### TRANSLATOR DEFINITIONS ####################################################

export CC		= gcc
export AR		= ar
export EMXDOC		= emxdoc
export IPFC		= /prog/ddk/tools/ipfc
export HARDLINK		= copy
export EMXBIND		= emxbind
export NTBIND		= ntbind

#
# Options
#

ifdef DEBUG
 export COPTS=-Wall -g
 export DEFINES+=-DDEBUG
else
 export COPTS=-Wall -O2
 export EMXBINDOPT=-s
 export NTBINDOPT=-strip
endif

INCLUDES=-Ilibhfs
DEFINES=	
LIBS=-llibhfs

export CFLAGS=$(COPTS)
EXTRA_CFLAGS = $(INCLUDES) $(DEFINES)
EXTRA_LDFLAGS+=-Llibhfs

ifdef WIN32
 export CFLAGS+=-Zrsx32
 export LDFLAGS+=-Zrsx32
 BINDIR=bin/win32
else
 BINDIR=bin/os2
endif

### DIRECTORIES ###############################################################

DOCDIR =	doc
LIBHFS_DIR =	libhfs
RSXDIR =	rsx

SUB_DIRS = $(LIBHFS_DIR) $(RSXDIR) $(DOCDIR)

###############################################################################

CLITARGETS_BASE = hattrib hcd hcopy hdel hdir hformat hls hmkdir \
		  hmount hpwd hrename hrmdir humount hvol
CLITARGETS =	$(addprefix $(BINDIR)/,$(addsuffix .exe,$(CLITARGETS_BASE)))

LIBHFS =	$(LIBHFS_DIR)/libhfs.a

RSX_EXE =	$(RSXDIR)/build/rsx.exe
EMXL =		$(RSXDIR)/stub/emxldpmi.exe
RSXHFS =	$(BINDIR)/rsxhfs.exe

HFSUTIL_AOUT =  $(BINDIR)/hfsutil
HFSUTIL =	$(BINDIR)/hfsutil.exe
SPAWN_AOUT =	$(BINDIR)/spawn
SPAWN =		$(BINDIR)/spawn.exe

CLIOBJS =	$(addprefix $(BINDIR)/, hattrib.o hcd.o hcopy.o hdel.o \
		hformat.o hls.o hmkdir.o  \
		hmount.o hpwd.o hrename.o hrmdir.o humount.o hvol.o)
UTILOBJS =	$(addprefix $(BINDIR)/, crc.o binhex.o copyin.o copyout.o  \
		darray.o dlist.o dstring.o glob.o suid.o)

###############################################################################

include Rules.make

all :: $(CLITARGETS) $(HFSUTIL)

clean ::
	-del /N $(subst /,\,$(BINDIR)\*) 2> NUL:

###############################################################################

$(HFSUTIL_AOUT): $(LIBHFS) $(BINDIR)/hfsutil.o $(BINDIR)/hcwd.o $(CLIOBJS) $(UTILOBJS)
	$(CC) $(LDFLAGS) $(EXTRA_LDFLAGS) $(BINDIR)/hfsutil.o $(BINDIR)/hcwd.o \
		$(CLIOBJS) $(UTILOBJS)  \
		$(LIBS) -o $@

$(HFSUTIL): $(HFSUTIL_AOUT) $(EMXL)
ifdef WIN32
	$(NTBIND) $(NTBINDOPT) -s $(EMXL) $<
else
	$(EMXBIND) -b $(EMXBINDOPT) $(EMXL) $<
endif

$(SPAWN_AOUT): spawn.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $^

$(SPAWN): $(SPAWN_AOUT) $(EMXL)
ifdef WIN32
	$(NTBIND) $(NTBINDOPT) -s $(EMXL) $<
else
	$(EMXBIND) -b $(EMXBINDOPT) $(EMXL) $<
endif

$(CLITARGETS): $(SPAWN)
	$(HARDLINK) $(subst /,\,$(SPAWN) $@)

$(BINDIR)/%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $<

$(RSXHFS): $(RSX_EXE)
	copy $(subst /,\,$(RSX_EXE) $(RSXHFS))

### FILE LISTS ################################################################

ARCHIVE_BASE 	= hfsu
ARCHIVE_EXT 	= zip
VER		= $(VERMAJOR)$(VERMINOR)
ifdef WIN32
DIST_ARCHIVE	= dist/$(ARCHIVE_BASE)w$(VER).$(ARCHIVE_EXT)
else
DIST_ARCHIVE	= dist/$(ARCHIVE_BASE)o$(VER).$(ARCHIVE_EXT)
endif
SRC_ARCHIVE	= dist/src.$(ARCHIVE_EXT)
ZIP		= /tools/bin/zip
CREATE_DIRS	= bin bin/os2 bin/win32

DIST_FILES = $(addprefix $(BINDIR)/, \
	hattrib.exe hcd.exe hcopy.exe hdel.exe \
	hdir.exe hformat.exe hfsutil.exe hls.exe hmkdir.exe \
	hmount.exe hpwd.exe hrename.exe hrmdir.exe humount.exe \
	hvol.exe) \
	bin/csxhfs.exe $(RSXHFS) \
	$(SRC_ARCHIVE) \
	$(addprefix $(DOCDIR)/, COPYING README hfsutils.txt)

ifndef WIN32
DIST_FILES += $(DOCDIR)/hfsutils.inf
endif

LIBHFS_SRC = $(addprefix $(LIBHFS_DIR)/, \
	block.c block.h btree.c btree.h data.c data.h \
	file.c file.h hfs.c hfs.h internal.h low.c low.h \
	node.c node.h record.c record.h volume.c volume.h \
	os2open.c os2open.h \
	Makefile)

DOC_SRC = $(addprefix $(DOCDIR)/, \
	COPYING hfsutils.src readme.os2.src readme.win.src Makefile)

RSX_SRC = $(addprefix $(RSXDIR)/, \
	adosx32.h cdosx32.c cdosx32.h copy32.h dasd.c dasdrm.c dasdrm.h \
	djio.c djio.h doserrno.c doserrno.h dpmi.h dpmi10.h dpmitype.h \
	excep32.h externa.h fs.c fs.h gnuaout.h intrm.c kdeb.c kdeb.h \
	libc.c loadprg.c loadprg.h Makefile ofiles.rsp printf.c printf.h \
	process.c process.h ptrace.c ptrace.h rmlib.c rmlib.h rsx.c rsx.h \
	signals.c signals.h start32.c start32.h statemx.c statemx.h \
	sysdep.h sysdep2.c sysdj.c sysemx.c termio.c termio.h timedos.c \
	timedos.h version.c version.h \
	$(addprefix asm32/, \
	adosx32.s callback.s copy32.s crt0.s dpmi.c dpmi10.s \
	excep32.s fpu.c regs386.inc) \
	build \
	$(addprefix loader/, \
	crt1.asm load2.asm loader.c loader.h Makefile) \
	$(addprefix stub/, \
	emx.inc emxl.asm headers.inc version.inc Makefile))

SRC_FILES = binhex.c copyin.c copyout.c crc.c darray.c dlist.c dstring.c \
	glob.c hattrib.c hcd.c hcopy.c hcwd.c hdel.c hformat.c hfsutil.c \
	hls.c hmkdir.c hmount.c hpwd.c hrename.c hrmdir.c humount.c \
	hvol.c spawn.c suid.c binhex.h copyin.h copyout.h crc.h darray.h \
	dlist.h dstring.h glob.h hattrib.h hcd.h hcopy.h hcwd.h hdel.h \
	hformat.h hfsutil.h hls.h hmkdir.h hmount.h hpwd.h hrename.h \
	hrmdir.h humount.h hvol.h os2lib.h suid.h version.h \
	Rules.make \
	$(LIBHFS_SRC) $(DOC_SRC) $(RSX_SRC) $(CREATE_DIRS)
# Makefile is included separately to prevent unneccessary remake of
# source archive when platform is changed
 
dist: $(DIST_ARCHIVE)

$(SRC_ARCHIVE): $(SRC_FILES)
	$(ZIP) $@ $^ Makefile

$(DIST_ARCHIVE): $(DIST_FILES)
	$(ZIP) -j $@ $^

### DEPENDENCIES FOLLOW #######################################################

$(BINDIR)/binhex.o: binhex.c binhex.h crc.h
$(BINDIR)/copyin.o: copyin.c libhfs/hfs.h libhfs/data.h copyin.h binhex.h crc.h
$(BINDIR)/copyout.o: copyout.c libhfs/hfs.h libhfs/data.h copyout.h binhex.h \
 crc.h
$(BINDIR)/crc.o: crc.c crc.h
$(BINDIR)/darray.o: darray.c darray.h
$(BINDIR)/dlist.o: dlist.c dlist.h
$(BINDIR)/dstring.o: dstring.c dstring.h
$(BINDIR)/glob.o: glob.c dlist.h dstring.h libhfs/hfs.h glob.h
$(BINDIR)/hattrib.o: hattrib.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hattrib.h
$(BINDIR)/hcd.o: hcd.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hcd.h
$(BINDIR)/hcopy.o: hcopy.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hcopy.h copyin.h \
 copyout.h
$(BINDIR)/hcwd.o: hcwd.c libhfs/hfs.h hcwd.h
$(BINDIR)/hdel.o: hdel.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hdel.h
$(BINDIR)/hformat.o: hformat.c libhfs/hfs.h hcwd.h hfsutil.h suid.h hformat.h
$(BINDIR)/hfsutil.o: hfsutil.c libhfs/hfs.h hcwd.h hfsutil.h suid.h version.h \
 hattrib.h hcd.h hcopy.h hdel.h hformat.h hls.h hmkdir.h hmount.h \
 hpwd.h hrename.h hrmdir.h humount.h hvol.h
$(BINDIR)/hls.o: hls.c libhfs/hfs.h hcwd.h hfsutil.h darray.h dlist.h dstring.h \
 glob.h hls.h
$(BINDIR)/hmkdir.o: hmkdir.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hmkdir.h
$(BINDIR)/hmount.o: hmount.c libhfs/hfs.h hcwd.h hfsutil.h suid.h hmount.h
$(BINDIR)/hpwd.o: hpwd.c libhfs/hfs.h hcwd.h hfsutil.h hpwd.h
$(BINDIR)/hrename.o: hrename.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hrename.h
$(BINDIR)/hrmdir.o: hrmdir.c libhfs/hfs.h hcwd.h hfsutil.h glob.h hrmdir.h
$(BINDIR)/humount.o: humount.c libhfs/hfs.h hcwd.h hfsutil.h humount.h
$(BINDIR)/hvol.o: hvol.c libhfs/hfs.h hcwd.h hfsutil.h hvol.h
$(BINDIR)/suid.o: suid.c suid.h
