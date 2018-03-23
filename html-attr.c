/*
 * HTML attribute functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#  include "html-private.h"


/*
 * 'htmlDeleteAttr()' - Delete an element attribute.
 */

void
htmlDeleteAttr(html_node_t *node,	/* I - Element node */
               const char  *name)	/* I - Attribute name */
{
}


/*
 * 'htmlGetAttr()' - Get the value of an element attribute.
 */

const char *				/* O - Value or `NULL` if not present */
htmlGetAttr(html_node_t *node,		/* I - Element node */
            const char  *name)		/* I - Attribute name */
{
  return (NULL);
}


/*
 * 'htmlNewAttr()' - Add an element attribute.
 */

void
htmlNewAttr(html_node_t *node,		/* I - Element node */
            const char  *name,		/* I - Attribute name */
            const char  *value)		/* I - Attribute value */
{
}
