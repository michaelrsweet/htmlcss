HTMLCSS - Lightweight HTML/CSS Library
======================================

![Version](https://img.shields.io/github/v/release/michaelrsweet/htmlcss?include_prereleases)
![Apache 2.0](https://img.shields.io/github/license/michaelrsweet/htmlcss)
[![Build](https://github.com/michaelrsweet/htmlcss/workflows/Build/badge.svg)](https://github.com/michaelrsweet/htmlcss/actions/workflows/build.yml)
[![Coverity Scan Status](https://img.shields.io/coverity/scan/22390.svg)](https://scan.coverity.com/projects/michaelrsweet-htmlcss)

HTMLCSS is a lightweight HTML/CSS parser written in C that allows applications
to prepare a HTML document for rendering or conversion.  HTMLCSS is extremely
portable and only requires a C99 compiler like GCC, Clang, Visual C, etc.
HTMLCSS is also extremely memory efficient, utilizing a shared string pool and
smart CSS cache to minimize the size of a HTML document in memory.

I'm providing HTMLCSS as open source under the Apache License Version 2.0 which
allows you do pretty much do whatever you like with it.  Please do provide
feedback and report bugs to the
[Github project page](https://github.com/michaelrsweet/htmlcss) so that everyone
can benefit.


Features
--------

Features include:

- HTML 5 markup parser
- CSS 3 stylesheet parser
- OFC/OFF/TTC/TTF font file parser (metadata and metrics only)
- GIF/JPG/PNG image file parser (metadata only)
- Functions to calculate CSS properties for a given node in a HTML document
- Functions to extract HTML "runs" consisting of CSS properties, content
  strings, and image references that can be rendered directly, including the
  :before and :after content from a stylesheet

HTMLCSS does *not* support dynamic HTML content created using Javascript in a
HTML document, as such content is typically used for interactive web pages while
HTMLCSS is intended for use with static content.


Requirements
------------

You'll need a C99 compiler like GCC, Clang, or Visual C, along with a POSIX-
compliant `make` program.


Building
--------

Run `configure` and `make` to build the library and unit test program:

    ./configure
    make
    make test


Installing
----------

Run `make install` to install the library and header files under `/usr/local`.
To install in a different location, specify the "prefix" variable, e.g.:

    make install prefix=/some/other/directory


Using
-----

See the file [DOCUMENTATION.md]() for documentation on using the library.


Legal Stuff
-----------

HTMLCSS is Copyright © 2018-2025 by Michael R Sweet.

HTMLCSS is licensed under the Apache License Version 2.0 with an exception to
allow linking against GPL2/LGPL2-only software.  See the files "LICENSE" and
"NOTICE" for more information.
