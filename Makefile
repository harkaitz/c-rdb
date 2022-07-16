## Configuration
DESTDIR    =
PREFIX     =/usr/local
AR         =ar
CC         =gcc
CFLAGS     =-Wall -g
CPPFLAGS   =
LIBRARIES  =librdb.a
HEADERS    =rdb/rdb.h
SOURCES    =rdb/rdb.c
CFLAGS_ALL =$(LDFLAGS) $(CFLAGS) $(CPPFLAGS)

## STANDARD TARGETS
all: $(LIBRARIES)
install: all
	install -d                  $(DESTDIR)$(PREFIX)/include/rdb
	install -m644 $(HEADERS)    $(DESTDIR)$(PREFIX)/include/rdb
	install -d                  $(DESTDIR)$(PREFIX)/lib
	install -m644 $(LIBRARIES)  $(DESTDIR)$(PREFIX)/lib
clean:
	rm -f $(PROGRAMS) $(LIBRARIES)
librdb.a: $(SOURCES) $(HEADERS)
	mkdir -p .b
	cd .b && $(CC) -c $(SOURCES:rdb/%=../rdb/%) $(CFLAGS_ALL)
	$(AR) -crs $@ .b/*.o
	rm -f .b/*.o
## -- license --
ifneq ($(PREFIX),)
install: install-license
install-license: LICENSE
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/c-rdb
	cp LICENSE $(DESTDIR)$(PREFIX)/share/doc/c-rdb
endif
## -- license --
