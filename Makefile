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
CFLAGS	=	$(OPTIM) -Wall '-DVERSION="$(VERSION)"' $(OPTIONS)
LDFLAGS	=	$(OPTIM)
LIBS	=	-lm
#OPTIM	=	-Os -g
OPTIM	=	-g -fsanitize=address
OPTIONS	=
#OPTIONS	=	-DDEBUG

HEADERS	=	\
		common.h \
		css.h \
		dict.h \
		font.h \
		html.h \
		htmlcss.h \
		pool.h \
		run.h \
		sha3.h
PHEADERS =	\
		common-private.h \
		css-private.h \
		font-private.h \
		html-private.h
LIBOBJS	=	\
		common.o \
		css-compute.o \
		css-core.o \
		css-import.o \
		css-rule.o \
		dict.o \
		font-core.o \
		font-extents.o \
		html-attr.o \
		html-core.o \
		html-find.o \
		html-load.o \
		html-node.o \
		pool.o \
		sha3.o
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
	$(CC) $(LDFLAGS) -o testhtmlcss testhtmlcss.o libhtmlcss.a $(LIBS)
	./testhtmlcss testsuite/basic.html

libhtmlcss.a:	$(LIBOBJS)
	ar -rcv libhtmlcss.a $(LIBOBJS)
	ranlib libhtmlcss.a

css-import.o:	default-css.h

default-css.h:	testsuite/default.css Makefile
	echo "static const char *default_css =" >default-css.h
	sed -e '1,$$s/\\/\\\\/g' -e '1,$$s/"/\\"/g' <testsuite/default.css | awk '{print "\"" $$0 "\\n\""}' >>default-css.h
	echo ";" >>default-css.h

$(OBJS):	Makefile $(HEADERS) $(PHEADERS)
