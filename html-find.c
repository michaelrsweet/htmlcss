/*
 * HTML find functions for HTMLCSS library.
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
 * 'htmlFindNode()' - Find the first node in a document that matches the given
 *                    element and/or ID string.
 */

html_node_t *				/* O - First matching node or `NULL` */
htmlFindNode(html_t         *html,	/* I - HTML document */
             html_element_t element,	/* I - Element or `HTML_ELEMENT_WILDCARD` for any */
             const char     *id)	/* I - ID string or `NULL` for any */
{
}


/*
 * 'htmlFindNextNode()' - Find the next node in a document that matches the
 *                        given element and/or ID string.
 */

html_node_t *				/* O - First matching node or `NULL` */
htmlFindNextNode(
    html_t         *html,		/* I - HTML document */
    html_node_t    *current,		/* I - Current node */
    html_element_t element,		/* I - Element or `HTML_ELEMENT_WILDCARD` for any */
    const char     *id)			/* I - ID string or `NULL` for any */
{
}
