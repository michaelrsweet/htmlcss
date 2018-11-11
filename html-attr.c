/*
 * HTML attribute functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
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
 * 'hcNodeAttrRemove()' - Delete an element attribute.
 */

void
hcNodeAttrRemove(hc_node_t *node,	/* I - Element node */
               const char  *name)	/* I - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return;

  hcDictRemove(node->value.element.attrs, name);
}


/*
 * 'hcNodeAttrGet()' - Get the value of an element attribute.
 */

const char *				/* O - Value or `NULL` if not present */
hcNodeAttrGet(hc_node_t *node,		/* I - Element node */
            const char  *name)		/* I - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (hcDictGet(node->value.element.attrs, name));
}


/*
 * 'hcNodeAttrCount()' - Get the number of attributes for an element.
 */

size_t					/* O - Number of attributes */
hcNodeAttrCount(hc_node_t *node)	/* I - Element node */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE)
    return (0);
  else
    return (hcDictCount(node->value.element.attrs));
}


/*
 * 'hcNodeAttrIndex()' - Get the name and value of a specified attribute.
 */

const char *				/* O - Attribute value or `NULL` */
hcNodeAttrIndex(hc_node_t *node,	/* I - Element node */
                 size_t      idx,	/* I - Attribute index (0-based) */
                 const char  **name)	/* O - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (hcDictIndex(node->value.element.attrs, idx, name));
}


/*
 * 'hcNodeAttrSet()' - Add an element attribute.
 */

void
hcNodeAttrSet(hc_node_t *node,		/* I - Element node */
            const char  *name,		/* I - Attribute name */
            const char  *value)		/* I - Attribute value */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name || !value)
    return;

  hcDictSet(node->value.element.attrs, name, value);
}
