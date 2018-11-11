/*
 * Memory pool header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLHC_POOL_H
#  define HTMLHC_POOL_H

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

typedef struct _hc_pool_s hc_pool_t;
					/* Memory allocation pool */


/*
 * Functions...
 */

extern hc_pool_t	*hcNewPool(void);
extern const char	*hcPoolAllocString(hc_pool_t *pool, const char *s);
extern void		hcPoolDelete(hc_pool_t *pool);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLHC_HTML_H */
