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

#include "html-private.h"


/*
 * 'htmlDeleteAttr()' - Delete an element attribute.
 */

void
htmlDeleteAttr(html_node_t *node,	/* I - Element node */
               const char  *name)	/* I - Attribute name */
{
  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name)
    return;

  htmlcssDictRemove(node->value.element.attrs, name);
}


/*
 * 'htmlGetAttr()' - Get the value of an element attribute.
 */

const char *				/* O - Value or `NULL` if not present */
htmlGetAttr(html_node_t *node,		/* I - Element node */
            const char  *name)		/* I - Attribute name */
{
  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (htmlcssDictGet(node->value.element.attrs, name));
}


/*
 * 'htmlGetAttrCount()' - Get the number of attributes for an element.
 */

size_t					/* O - Number of attributes */
htmlGetAttrCount(html_node_t *node)	/* I - Element node */
{
  if (!node || node->element < HTML_ELEMENT_DOCTYPE)
    return (0);
  else
    return (htmlcssDictCount(node->value.element.attrs));
}


/*
 * 'htmlGetAttrIndex()' - Get the name and value of a specified attribute.
 */

const char *				/* O - Attribute value or `NULL` */
htmlGetAttrIndex(html_node_t *node,	/* I - Element node */
                 size_t      idx,	/* I - Attribute index (0-based) */
                 const char  **name)	/* O - Attribute name */
{
  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (htmlcssDictIndex(node->value.element.attrs, idx, name));
}


/*
 * 'htmlNewAttr()' - Add an element attribute.
 */

void
htmlNewAttr(html_node_t *node,		/* I - Element node */
            const char  *name,		/* I - Attribute name */
            const char  *value)		/* I - Attribute value */
{
  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name || !value)
    return;

  htmlcssDictSet(node->value.element.attrs, name, value);
}
