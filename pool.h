/*
 * Memory pool header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_POOL_H
#  define HTMLCSS_POOL_H

/*
 * Include necessary headers...
 */

#  include "common.h"


#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _htmlcss_pool_s htmlcss_pool_t;
					/* Memory allocation pool */


/*
 * Functions...
 */

extern htmlcss_pool_t	*htmlcssNewPool(void);
extern const char	*htmlcssPoolAllocString(htmlcss_pool_t *pool, const char *s);
extern void		htmlcssPoolDelete(htmlcss_pool_t *pool);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_HTML_H */
