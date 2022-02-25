## Configuration
DESTDIR    =
PREFIX     =/usr/local
AR         =ar
CC         =gcc
CFLAGS     =-Wall -g
CPPFLAGS   =

## Sources and targets
LIBRARIES  =librdb.a
HEADERS    =rdb/rdb.h
SOURCES    =rdb/rdb.c
## AUXILIARY
CFLAGS_ALL =$(LDFLAGS) $(CFLAGS) $(CPPFLAGS)

## STANDARD TARGETS
all: $(LIBRARIES)
help:
	@echo "all     : Build everything."
	@echo "clean   : Clean files."
	@echo "install : Install all produced files."
install: all
	install -d                  $(DESTDIR)$(PREFIX)/include/rdb
	install -m644 $(HEADERS)    $(DESTDIR)$(PREFIX)/include/rdb
	install -d                  $(DESTDIR)$(PREFIX)/lib
	install -m644 $(LIBRARIES)  $(DESTDIR)$(PREFIX)/lib
clean:
	rm -f $(PROGRAMS) $(LIBRARIES)

## LIBRARY
librdb.a: $(SOURCES) $(HEADERS)
	mkdir -p .b
	cd .b && $(CC) -c $(SOURCES:rdb/%=../rdb/%) $(CFLAGS_ALL)
	$(AR) -crs $@ .b/*.o
	rm -f .b/*.o
