MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = _cave
EXTRA_DIST = $(noinst_DATA)

AUTOMAKE_OPTIONS = 1.11 parallel-tests

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

