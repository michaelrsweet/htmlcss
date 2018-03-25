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
  int		i;			/* Looping var */
  _html_attr_t	*attr;			/* Current attribute */


  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name)
    return;

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs; i > 0; i --, attr ++)
  {
    if (!strcmp(name, attr->name))
    {
      free(attr->name);
      free(attr->value);

      if (i > 1)
        memmove(attr, attr + 1, (i - 1) * sizeof(_html_attr_t));

      node->value.element.num_attrs --;

      if (node->value.element.num_attrs == 0)
      {
        free(node->value.element.attrs);
        node->value.element.attrs = NULL;
      }
      return;
    }
  }
}


/*
 * 'htmlGetAttr()' - Get the value of an element attribute.
 */

const char *				/* O - Value or `NULL` if not present */
htmlGetAttr(html_node_t *node,		/* I - Element node */
            const char  *name)		/* I - Attribute name */
{
  int		i;			/* Looping var */
  _html_attr_t	*attr;			/* Current attribute */


  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name)
    return (NULL);

  for (i = node->value.element.num_attrs, attr = node->value.element.attrs; i > 0; i --, attr ++)
  {
    if (!strcmp(name, attr->name))
      return (attr->value);
  }

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
  _html_attr_t	*attr;			/* Current attribute */
  int		current_attrs,		/* Currently allocated attributes */
		alloc_attrs;		/* New allocated attributes */


  if (!node || node->element < HTML_ELEMENT_DOCTYPE || !name || !value)
    return;

 /*
  * Compute the number of attributes that are allocated - 0, 1, 4, 8, ...
  */

  if ((current_attrs = node->value.element.num_attrs) > 1)
    current_attrs = (current_attrs + 3) & ~3;
  if ((alloc_attrs = node->value.element.num_attrs + 1) > 1)
    alloc_attrs = (alloc_attrs + 3) & ~3;

  if (current_attrs != alloc_attrs)
  {
    if ((attr = realloc(node->value.element.attrs, alloc_attrs * sizeof(_html_attr_t))) == NULL)
      return;

    node->value.element.attrs = attr;
  }

  attr = node->value.element.attrs + node->value.element.num_attrs;
  node->value.element.num_attrs ++;

  attr->name  = strdup(name);
  attr->value = strdup(value);
}
