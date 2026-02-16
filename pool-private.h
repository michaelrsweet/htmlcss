//
// Private memory pool header for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2026 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_POOL_PRIVATE_H
#  define HTMLCSS_POOL_PRIVATE_H
#  include "common-private.h"
#  include "dict.h"
#  include <ttf.h>
#  include <stdarg.h>
#  include <locale.h>
#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus


//
// Types...
//

typedef struct _hc_font_info_s _hc_font_info_t;
					// Font cache information

struct _hc_pool_s			// Memory pool
{
  struct lconv	*loc;			// Locale information
  size_t	loc_declen;		// Length of decimal point

  ttf_cache_t	*fonts;			// Fonts array

  size_t	num_strings;		// Number of strings in pool
  size_t	alloc_strings;		// Allocated size of strings array
  char		**strings;		// Strings array

  hc_dict_t	*urls;			// URLs mapped to local files

  hc_error_cb_t	error_cb;		// Error callback
  void		*error_ctx;		// Error callback context pointer
  char		*last_error;		// Last error message

  hc_url_cb_t	url_cb;			// URL callback
  void		*url_ctx;		// URL callback context pointer
};


//
// Functions...
//

extern bool	_hcPoolError(hc_pool_t *pool, int linenum, const char *message, ...) _HC_FORMAT_ARGS(3, 4);
extern bool	_hcPoolErrorv(hc_pool_t *pool, int linenum, const char *message, va_list ap);


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !HTMLCSS_POOL_PRIVATE_H
