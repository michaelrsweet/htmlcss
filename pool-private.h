/*
 * Private memory pool header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_POOL_PRIVATE_H
#  define HTMLCSS_POOL_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "common-private.h"
#  include "dict.h"
#  include <stdarg.h>


#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

struct _hc_pool_s			/* Memory pool */
{
  size_t	num_strings;		/* Number of strings in pool */
  size_t	alloc_strings;		/* Allocated size of strings array */
  char		**strings;		/* Strings array */

  hc_dict_t	*urls;			/* URLs mapped to local files */

  hc_error_cb_t	error_cb;		/* Error callback */
  void		*error_ctx;		/* Error callback context pointer */
  hc_url_cb_t	url_cb;			/* URL callback */
  void		*url_ctx;		/* URL callback context pointer */
};


/*
 * Functions...
 */

extern int	_hcPoolError(hc_pool_t *pool, int linenum, const char *message, ...) _HC_FORMAT_ARGS(3, 4);
extern int	_hcPoolErrorv(hc_pool_t *pool, int linenum, const char *message, va_list ap);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_POOL_PRIVATE_H */
