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
 * Local functions...
 */

static html_node_t	*html_walk_next(html_node_t *current);


/*
 * 'htmlFindNode()' - Find the first node in a document that matches the given
 *                    element and/or ID string.
 */

html_node_t *				/* O - First matching node or `NULL` */
htmlFindNode(html_t         *html,	/* I - HTML document */
             html_node_t    *current,	/* I - Current node or `NULL` */
             html_element_t element,	/* I - Element or `HTML_ELEMENT_WILDCARD` for any */
             const char     *id)	/* I - ID string or `NULL` for any */
{
  const char	*current_id;		/* Current ID attribute */


  if (!html)
    return (NULL);

  if (!current)
    current = html->root;
  else
    current = html_walk_next(current);

  while (current)
  {
    if (element == HTML_ELEMENT_WILDCARD || current->element == element)
    {
      if (!id)
        break;

      if ((current_id = htmlGetAttr(current, "id")) != NULL && !strcmp(current_id, id))
        break;
    }

    current = html_walk_next(current);
  }

  return (current);
}


/*
 * 'html_walk_next()' - Walk the node tree.
 */

static html_node_t *			/* O - Next logical node or `NULL` */
html_walk_next(html_node_t *current)	/* I - Current node */
{
  html_node_t	*next;			/* Next node */


  if ((next = htmlGetFirstChildNode(current)) == NULL)
  {
    if ((next = htmlGetNextSiblingNode(current)) == NULL)
    {
      do
      {
        next = htmlGetParentNode(current);
      }
      while (next && htmlGetNextSiblingNode(next) == NULL);

      next = htmlGetNextSiblingNode(next);
    }
  }

  return (next);
}
