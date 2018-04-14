#
# Makefile for HTMLCSS library.
#
#     https://github.com/michaelrsweet/htmlcss
#
# Copyright © 2018 by Michael R Sweet.
#
# Licensed under Apache License v2.0.  See the file "LICENSE" for more
# information.
#

VERSION	=	0.1
prefix	=	$(DESTDIR)/usr/local
includedir =	$(prefix)/include
bindir	=	$(prefix)/bin
libdir	=	$(prefix)/lib
mandir	=	$(prefix)/share/man

CC	=	gcc
CFLAGS	=	-Os -g -Wall '-DVERSION="$(VERSION)"'
LIBS	=	-lm
HEADERS	=	\
		common.h \
		css.h \
		html.h \
		htmlcss.h \
		run.h
LIBOBJS	=	\
		common.o \
		css-compute.o \
		css-core.o \
		css-import.o \
		html-attr.o \
		html-core.o \
		html-find.o \
		html-load.o \
		html-node.o
OBJS	=	testhtmlcss.o $(LIBOBJS)
TARGETS	=	libhtmlcss.a testhtmlcss

.SUFFIXES:	.a .c .o
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

all:		$(TARGETS)

clean:
	rm -f $(TARGETS) $(OBJS)

install:	$(TARGETS)
	mkdir -p $(includedir)/htmlcss
	cp $(HEADERS) $(includedir)/htmlcss
	mkdir -p $(libdir)
	cp libhtmlcss.a $(libdir)
	ranlib $(libdir)/libhtmlcss.a

testhtmlcss:	testhtmlcss.o libhtmlcss.a
	$(CC) $(CFLAGS) -o testhtmlcss testhtmlcss.o libhtmlcss.a $(LIBS)
	./testhtmlcss

libhtmlcss.a:	$(LIBOBJS)
	ar -rcv libhtmlcss.a $(LIBOBJS)
	ranlib libhtmlcss.a

$(OBJS):	Makefile $(HEADERS) common-private.h css-private.h html-private.h
