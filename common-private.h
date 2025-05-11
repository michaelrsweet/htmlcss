//
// Common private header file for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_COMMON_PRIVATE_H
#  define HTMLCSS_COMMON_PRIVATE_H
#  include "common.h"
#  ifdef _WIN32
#    include <io.h>
#    include <direct.h>

//
// Microsoft renames the POSIX functions to _name, and introduces
// a broken compatibility layer using the original names.  As a result,
// random crashes can occur when, for example, strdup() allocates memory
// from a different heap than used by malloc() and free().
//
// To avoid moronic problems like this, we #define the POSIX function
// names to the corresponding non-standard Microsoft names.
//

#    define access	_access
#    define close	_close
#    define fileno	_fileno
#    define lseek	_lseek
#    define mkdir(d,p)	_mkdir(d)
#    define open	_open
#    define read	_read
#    define rmdir	_rmdir
#    define snprintf	_snprintf
#    define strdup	_strdup
#    define unlink	_unlink
#    define vsnprintf	_vsnprintf
#    define write	_write

//
// Map various parameters for POSIX...
//

#    define F_OK	00
#    define W_OK	02
#    define R_OK	04
#    define O_RDONLY	_O_RDONLY
#    define O_WRONLY	_O_WRONLY
#    define O_CREAT	_O_CREAT
#    define O_TRUNC	_O_TRUNC

#  else
#    include <unistd.h>
#  endif // _WIN32


//
// DEBUG is typically defined for debug builds.  _HC_DEBUG maps to fprintf when
// DEBUG is defined and is a no-op otherwise...
//

#  ifdef DEBUG
#    define _HC_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#  else
#    define _HC_DEBUG(...)
#  endif // DEBUG


//
// _HC_FORMAT_ARGS tells the compiler to validate printf-style format
// arguments, producing warnings when there are issues...
//

#  if defined(__has_extension) || defined(__GNUC__)
#    define _HC_FORMAT_ARGS(a,b) __attribute__ ((__format__(__printf__, a, b)))
#  else
#    define _HC_FORMAT_ARGS(a,b)
#  endif // __has_extension || __GNUC__


#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus


//
// Types...
//

typedef int (*_hc_compare_func_t)(const void *, const void *);
					// bsearch/qsort comparison function
typedef unsigned char _hc_uchar_t;	// Unsigned 8-bit byte


//
// Functions...
//

extern bool	_hcDefaultErrorCB(void *ctx, const char *message, int linenum);
extern char	*_hcDefaultURLCB(void *ctx, const char *url, char *buffer, size_t bufsize);


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !HTMLCSS_COMMON_PRIVATE_H
