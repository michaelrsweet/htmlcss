/*
 * Private HTML header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2021 by Michael R Sweet.
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

struct _hc_node_s
{
  hc_element_t	element;		/* Element type */
  hc_node_t	*parent;		/* Parent node */
  hc_node_t	*prev_sibling;		/* Previous (sibling) node */
  hc_node_t	*next_sibling;		/* Next (sibling) node */
  union
  {
    char	comment[1];		/*** Comment value */
    struct
    {
      hc_node_t	*first_child;		/***** First child node */
      hc_node_t	*last_child;		/***** Last child node */
      hc_dict_t	*attrs;			/***** Attributes dictionary */
      hc_html_t	*html;			/***** HTML document */
      const hc_dict_t *base_props;	/***** Base CSS properties dictionary */
    }		element;		/*** Element value */
    char	string[1];		/*** String value */
    char	unknown[1];		/*** Unknown element/directive value */
  }		value;			/* Node value */
};

struct _hc_html_s
{
  hc_pool_t	*pool;			/* Memory pool */
  hc_css_t	*css;			/* Stylesheet */
  hc_node_t	*root;			/* Root node */
  hc_error_cb_t	error_cb;		/* Error callback */
  void		*error_ctx;		/* Error callback context pointer */
  hc_url_cb_t	url_cb;			/* URL callback */
  void		*url_ctx;		/* URL callback context pointer */
};


/*
 * Private functions...
 */

extern bool		_hcNodeComputeCSSTextFont(hc_node_t *node, const hc_dict_t *props, hc_text_t *text);
extern hc_element_t	_hcElementLookup(const char *s);
extern hc_node_t	*_hcNodeNewUnknown(hc_node_t *parent, const char *unk);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_HTML_PRIVATE_H */
