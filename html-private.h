/*
 * Private HTML header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_HTML_PRIVATE_H
#  define HTMLCSS_HTML_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "html.h"
#  include "common-private.h"
#  include "dict.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Private types...
 */

struct _html_node_s
{
  html_element_t	element;	/* Element type */
  html_node_t		*parent;	/* Parent node */
  html_node_t		*prev_sibling;	/* Previous (sibling) node */
  html_node_t		*next_sibling;	/* Next (sibling) node */
  union
  {
    char		comment[1];	/*** Comment value */
    struct
    {
      html_node_t	*first_child;	/***** First child node */
      html_node_t	*last_child;	/***** Last child node */
      htmlcss_dict_t	*attrs;		/***** Attributes dictionary */
    }			element;	/*** Element value */
    char		string[1];	/*** String value */
    char		unknown[1];	/*** Unknown element/directive value */
  }			value;		/* Node value */
};

struct _html_s
{
  htmlcss_pool_t	*pool;		/* Memory pool */
  css_t			*css;		/* Stylesheet */
  html_node_t		*root;		/* Root node */
  htmlcss_error_cb_t	error_cb;	/* Error callback */
  void			*error_ctx;	/* Error callback context pointer */
  htmlcss_url_cb_t	url_cb;		/* URL callback */
  void			*url_ctx;	/* URL callback context pointer */
};


/*
 * Private functions...
 */

extern html_node_t	*_htmlNewUnknown(html_node_t *parent, const char *unk);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_HTML_PRIVATE_H */
