//
// Common header file for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_COMMON_H
#  define HTMLCSS_COMMON_H
#  include <stdio.h>
#  include <stdlib.h>
#  include <stdbool.h>
#  include <string.h>
#  include <ctype.h>
#  include <errno.h>


//
// Visibility and other annotations...
//

#  if _WIN32
#    define _HC_DEPRECATED(m)
#    define _HC_INTERNAL
#    define _HC_PRIVATE
#    define _HC_PUBLIC
#    define _HC_FORMAT(a,b)
#    define _HC_NONNULL(...)
#    define _HC_NORETURN
#  elif defined(__has_extension) || defined(__GNUC__)
#    define _HC_DEPRECATED(m)	__attribute__ ((deprecated(m))) __attribute__ ((visibility("default")))
#    define _HC_INTERNAL	__attribute__ ((visibility("hidden")))
#    define _HC_PRIVATE		__attribute__ ((visibility("default")))
#    define _HC_PUBLIC		__attribute__ ((visibility("default")))
#    define _HC_FORMAT(a,b)	__attribute__ ((__format__(__printf__, a,b)))
#    define _HC_NONNULL(...)	__attribute__ ((nonnull(__VA_ARGS__)))
#    define _HC_NORETURN	__attribute__ ((noreturn))
#  else
#    define _HC_DEPRECATED(m)
#    define _HC_INTERNAL
#    define _HC_PRIVATE
#    define _HC_PUBLIC
#    define _HC_FORMAT(a,b)
#    define _HC_NONNULL(...)
#    define _HC_NORETURN
#  endif // __has_extension || __GNUC__
#endif // !HTMLCSS_COMMON_H
