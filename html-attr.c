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
 * 'hcNodeAttrGetCount()' - Get the number of attributes for an element.
 */

size_t					/* O - Number of attributes */
hcNodeAttrGetCount(hc_node_t *node)	/* I - Element node */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE)
    return (0);
  else
    return (hcDictGetCount(node->value.element.attrs));
}


/*
 * 'hcNodeAttrGetIndexNameValue()' - Get the name and value of a specified attribute.
 */

const char *				/* O - Attribute value or `NULL` */
hcNodeAttrGetIndexNameValue(
    hc_node_t  *node,			/* I - Element node */
    size_t     idx,			/* I - Attribute index (0-based) */
    const char **name)			/* O - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (hcDictGetIndexKeyValue(node->value.element.attrs, idx, name));
}


/*
 * 'hcNodeAttrGetNameValue()' - Get the value of an element attribute.
 */

const char *				/* O - Value or `NULL` if not present */
hcNodeAttrGetNameValue(
    hc_node_t  *node,			/* I - Element node */
    const char *name)			/* I - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return (NULL);

  return (hcDictGetKeyValue(node->value.element.attrs, name));
}


/*
 * 'hcNodeAttrRemove()' - Delete an element attribute.
 */

void
hcNodeAttrRemove(hc_node_t  *node,	/* I - Element node */
                 const char *name)	/* I - Attribute name */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name)
    return;

  hcDictRemoveKey(node->value.element.attrs, name);
}


/*
 * 'hcNodeAttrSetNameValue()' - Add an element attribute.
 */

void
hcNodeAttrSetNameValue(
    hc_html_t  *html,			/* I - HTML document */
    hc_node_t  *node,			/* I - Element node */
    const char *name,			/* I - Attribute name */
    const char *value)			/* I - Attribute value */
{
  if (!node || node->element < HC_ELEMENT_DOCTYPE || !name || !value)
    return;

  if (!node->value.element.attrs)
    node->value.element.attrs = hcDictNew(html->pool);

  hcDictSetKeyValue(node->value.element.attrs, name, value);
}
