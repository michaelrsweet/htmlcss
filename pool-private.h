//
// Private memory pool header for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_POOL_PRIVATE_H
#  define HTMLCSS_POOL_PRIVATE_H
#  include "common-private.h"
#  include "dict.h"
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

  bool		fonts_loaded;		// Did we load the fonts?
  size_t	num_fonts;		// Number of fonts in pool
  size_t	alloc_fonts;		// Allocated size of fonts array
  _hc_font_info_t *fonts;		// Fonts array
  size_t	font_index[256];	// Index into fonts array

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

extern void	_hcPoolDeleteFonts(hc_pool_t *pool);
extern bool	_hcPoolError(hc_pool_t *pool, int linenum, const char *message, ...) _HC_FORMAT_ARGS(3, 4);
extern bool	_hcPoolErrorv(hc_pool_t *pool, int linenum, const char *message, va_list ap);


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !HTMLCSS_POOL_PRIVATE_H
