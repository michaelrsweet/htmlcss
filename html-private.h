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
#  include <stdlib.h>
#  include <errno.h>
#  include <string.h>

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

typedef struct _html_attr_s
{
  char			*name,		/* Attribute name */
			*value;		/* Attribute value */
} _html_attr_t;

struct _html_node_s
{
  html_element_t	element;	/* Element type */
  html_node_t		*parent,	/* Parent node */
			*child,		/* Child node */
			*prev,		/* Previous (sibling) node */
			*next;		/* Next (sibling) node */
  union
  {
    char		*string;	/*** String value */
    struct
    {
      int		num_attrs;	/***** Number of attributes */
      _html_attr_t	*attrs;		/***** Attributes */
    }			element;	/*** Element value */
  }			value;		/* Node value */
};

struct _html_s
{
  css_t			*css;		/* Stylesheet */
  html_node_t		*root;		/* Root node */
};

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_HTML_PRIVATE_H */
