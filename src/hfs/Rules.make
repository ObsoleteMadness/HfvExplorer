#
# Rules.make
# This file contains rules which are shared between multiple Makefiles.
#

#
# False targets.
#
.PHONY: dummy sub_dirs

#
# Make subdirectories
#
all:: sub_dirs

#
# Common rules
#
%.s: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -S $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) $(EXTRA_ASFLAGS) -o $@ $<

%.ipf: %.src
	$(EMXDOC) -i -c -o $@ $<

%.txt: %.src
	$(EMXDOC) -t -o $@ $<

%.inf: %.ipf
	$(IPFC) /inf $<

#
# A rule to make subdirectories
#
sub_dirs: dummy
ifdef SUB_DIRS
	for %i in ($(SUB_DIRS)) do $(MAKE) -C %i
endif

#
# Clean up subdirectories also
#
clean ::
ifdef SUB_DIRS
	for %i in ($(SUB_DIRS)) do $(MAKE) -C %i clean
endif


#
# A rule to do nothing
#
dummy:

