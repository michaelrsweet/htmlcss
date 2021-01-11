#
# Makefile for HTMLCSS library.
#
#     https://github.com/michaelrsweet/htmlcss
#
# Copyright © 2018-2021 by Michael R Sweet.
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
CFLAGS	=	$(OPTIM) $(OPTIONS)
LDFLAGS	=	$(OPTIM)
LIBS	=	-lz -lm
#OPTIM	=	-Os -g
OPTIM	=	-g -fsanitize=address
OPTIONS	=	-Wall '-DVERSION="$(VERSION)"'
#OPTIONS	=	-Wall '-DVERSION="$(VERSION)"' -DDEBUG

HEADERS	=	\
		common.h \
		css.h \
		dict.h \
		file.h \
		font.h \
		html.h \
		htmlcss.h \
		image.h \
		pool.h \
		run.h \
		sha3.h
PHEADERS =	\
		common-private.h \
		css-private.h \
		file-private.h \
		font-private.h \
		html-private.h \
		pool-private.h
LIBOBJS	=	\
		common.o \
		css-compute.o \
		css-core.o \
		css-import.o \
		css-rule.o \
		dict.o \
		file.o \
		font-core.o \
		font-extents.o \
		font-find.o \
		html-attr.o \
		html-core.o \
		html-find.o \
		html-load.o \
		html-node.o \
		image.o \
		pool.o \
		sha3.o
OBJS	=	testhtmlcss.o $(LIBOBJS)
TARGETS	=	libhtmlcss.a testhtmlcss

DOCFILES =	\
		$(HEADERS) \
		$(LIBOBJS:.o=.c)
DOCFLAGS =	\
		--author "Michael R Sweet" \
		--copyright "Copyright (c) 2018-2020 by Michael R Sweet" \
		--docversion $(VERSION) \
		--title "HTMLCSS Programming Manual"

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

doc:
	codedoc $(DOCFLAGS) $(DOCFILES) >htmlcss.html

test:		testhtmlcss
	./testhtmlcss --all --css --font --html testhtmlcss.html

test-fonts:	testhtmlcss
	./testhtmlcss --font testsuite/*.ttf testsuite/*.otf

testhtmlcss:	testhtmlcss.o libhtmlcss.a
	$(CC) $(LDFLAGS) -o testhtmlcss testhtmlcss.o libhtmlcss.a $(LIBS)

libhtmlcss.a:	$(LIBOBJS)
	ar -rcv libhtmlcss.a $(LIBOBJS)
	ranlib libhtmlcss.a

css-import.o:	default-css.h

default-css.h:	default.css Makefile
	echo "static const char *default_css =" >default-css.h
	sed -e '1,$$s/\\/\\\\/g' -e '1,$$s/"/\\"/g' <default.css | awk '{print "\"" $$0 "\\n\""}' >>default-css.h
	echo ";" >>default-css.h

$(OBJS):	Makefile $(HEADERS) $(PHEADERS)

# Scan code with the Clang static analyzer <https://clang-analyzer.llvm.org>
clang:
	clang $(OPTIONS) -Werror --analyze $(OBJS:.o=.c)
	rm -rf $(OBJS:.o=.plist)

# Scan the code using Cppcheck <http://cppcheck.sourceforge.net>
cppcheck:
	cppcheck $(OPTIONS) --template=gcc --addon=cert.py --suppress=cert-MSC24-C --suppress=cert-EXP05-C --suppress=cert-API01-C $(OBJS:.o=.c) 2>cppcheck.log
	@test -s cppcheck.log && (echo ""; echo "Errors detected:"; echo ""; cat cppcheck.log; exit 1) || exit 0
