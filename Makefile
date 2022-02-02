## Configuration
DESTDIR    =
PREFIX     =/usr/local
AR         =ar
CC         =gcc
CFLAGS     =-Wall -g
CPPFLAGS   =
LIBS       ="-l:libhiredis.a" "-l:libuuid.a"
LIBS_PAM   ="-l:libpam_weblogin.o" "-l:libpam.a" -ldl
## Sources and targets
PROGRAMS   =rdbc rdbs
LIBRARIES  =librdb.a
HEADERS    =$(shell find rdb -iregex '.*\.h')
MARKDOWNS  =README.md
MANPAGES_3 =
SOURCES    =$(shell find rdb -iregex '.*\.c')
## AUXILIARY
CFLAGS_ALL =$(LDFLAGS) $(CFLAGS) $(CPPFLAGS)

## STANDARD TARGETS
all: $(PROGRAMS) $(LIBRARIES)
help:
	@echo "all     : Build everything."
	@echo "clean   : Clean files."
	@echo "install : Install all produced files."
install: all
	install -d                  $(DESTDIR)$(PREFIX)/bin
	install -m755 $(PROGRAMS)   $(DESTDIR)$(PREFIX)/bin
	install -d                  $(DESTDIR)$(PREFIX)/share/man/man3
	install -m644 $(MANPAGES_3) $(DESTDIR)$(PREFIX)/share/man/man3
clean:
	rm -f $(PROGRAMS) $(LIBRARIES)
ssnip:
	ssnip LICENSE $(MARKDOWNS) $(HEADERS) $(SOURCES) $(MANPAGES_3)

## LIBRARY
librdb.a: $(SOURCES) $(HEADERS)
	mkdir -p .b
	cd .b && $(CC) -c $(SOURCES:rdb/%=../rdb/%) $(CFLAGS_ALL)
	$(AR) -crs $@ .b/*.o
	rm -f .b/*.o
rdbc: ./tools/rdbc.c librdb.a
	$(CC) -o $@ ./tools/rdbc.c librdb.a $(CFLAGS_ALL) $(LIBS)
rdbs: ./tools/rdbs.c librdb.a
	$(CC) -o $@ ./tools/rdbs.c librdb.a $(CFLAGS_ALL) $(LIBS) $(LIBS_PAM)
