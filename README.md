HTMLCSS - Lightweight HTML/CSS Library
======================================

![Version](https://img.shields.io/github/v/release/michaelrsweet/htmlcss?include_prereleases)
![Apache 2.0](https://img.shields.io/github/license/michaelrsweet/htmlcss)
[![Build](https://github.com/michaelrsweet/htmlcss/workflows/Build/badge.svg)](https://github.com/michaelrsweet/htmlcss/actions/workflows/build.yml)
[![Coverity Scan Status](https://img.shields.io/coverity/scan/22390.svg)](https://scan.coverity.com/projects/michaelrsweet-htmlcss)

HTMLCSS is a lightweight HTML/CSS parser written in C that allows applications
to prepare a HTML document for rendering or conversion.  Features include:

- HTML 5 markup parser
- CSS 3 stylesheet parser
- GIF/JPEG/PNG image file parser (metadata only)
- Functions to calculate CSS properties for a given node in a HTML document
- Functions to extract HTML "runs" consisting of CSS properties, content
  strings, and image references that can be rendered directly, including the
  :before and :after content from a stylesheet
- A device API for reporting capabilities and handling fonts
- A memory pool for efficiently storing and caching strings and CSS properties
- A URL API to support fetching remote resources

HTMLCSS does *not* support dynamic HTML content created using Javascript in a
HTML document, as such content is typically used for interactive/dynamic web
pages while HTMLCSS is intended for use with static content.


Requirements
------------

HTMLCSS requires a C99 compiler like GCC, Clang, or Visual C, along with
POSIX-compliant `make` and `sh` programs (Linux/Unix), Visual Studio (Windows),
and/or Xcode (macOS) for doing the build.  The only required library is ZLIB
(1.1 or later) for compression support.


Dcumentation and Examples
-------------------------

Documentation can be found in the "htmlcss.html" file.


Contributing Code
-----------------

Code contributions should be submitted as pull requests on the Github site:

    http://github.com/michaelrsweet/htmlcss/pulls

See the file "CONTRIBUTING.md" for more details.


Legal Stuff
-----------

HTMLCSS is Copyright © 2018-2025 by Michael R Sweet.

HTMLCSS is licensed under the Apache License Version 2.0.  See the files
"LICENSE" and "NOTICE" for more information.
