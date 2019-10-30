/*
 * Memory pool header for HTMLCSS library.
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

typedef struct _hc_pool_s hc_pool_t;	/* Memory allocation pool */

typedef int (*hc_error_cb_t)(void *ctx, const char *message, int linenum);
typedef char *(*hc_url_cb_t)(void *ctx, const char *url, char *buffer, size_t bufsize);


/*
 * Functions...
 */

extern void		hcPoolDelete(hc_pool_t *pool);
extern const char	*hcPoolGetLastError(hc_pool_t *pool);
extern const char	*hcPoolGetString(hc_pool_t *pool, const char *s);
extern const char	*hcPoolGetURL(hc_pool_t *pool, const char *url, const char *baseurl);
extern hc_pool_t	*hcPoolNew(void);
extern void		hcPoolSetErrorCallback(hc_pool_t *pool, hc_error_cb_t cb, void *ctx);
extern void		hcPoolSetURLCallback(hc_pool_t *pool, hc_url_cb_t cb, void *ctx);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_POOL_H */
