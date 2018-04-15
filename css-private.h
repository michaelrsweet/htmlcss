/*
 * Private CSS header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_CSS_PRIVATE_H
#  define HTMLCSS_CSS_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "css.h"
#  include "html-private.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _css_prop_s
{
} _css_prop_t;

typedef struct _css_rule_s
{
} _css_rule_t;

struct _css_s
{
  css_media_t		media;		/* Base media definition */
  htmlcss_error_cb_t	error_cb;	/* Error callback */
  void			*error_ctx;	/* Error callback context pointer */
  htmlcss_url_cb_t	url_cb;		/* URL callback */
  void			*url_ctx;	/* URL callback context pointer */
};


/*
 * Functions...
 */


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_CSS_PRIVATE_H */
