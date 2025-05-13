//
// HTML find functions for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#include "html-private.h"


//
// Local functions...
//

static hc_node_t	*html_walk_next(hc_node_t *current);


//
// 'hcHTMLFindNode()' - Find the first node in a document that matches the given
//                      element and/or ID string.
//

hc_node_t *				// O - First matching node or `NULL`
hcHTMLFindNode(hc_html_t    *html,	// I - HTML document
	       hc_node_t    *current,	// I - Current node or `NULL`
	       hc_element_t element,	// I - Element or `HC_ELEMENT_WILDCARD` for any
	       const char   *id)	// I - ID string or `NULL` for any
{
  const char	*current_id;		// Current ID attribute


  if (!html)
    return (NULL);

  if (!current)
    current = html->root;
  else
    current = html_walk_next(current);

  while (current)
  {
    if (element == HC_ELEMENT_WILDCARD || current->element == element)
    {
      if (!id)
        break;

      if ((current_id = hcNodeAttrGetNameValue(current, "id")) != NULL && !strcmp(current_id, id))
        break;
    }

    current = html_walk_next(current);
  }

  return (current);
}


//
// 'html_walk_next()' - Walk the node tree.
//

static hc_node_t *			// O - Next logical node or `NULL`
html_walk_next(hc_node_t *current)	// I - Current node
{
  hc_node_t	*next;			// Next node


  if ((next = hcNodeGetFirstChildNode(current)) == NULL)
  {
    if ((next = hcNodeGetNextSiblingNode(current)) == NULL)
    {
      do
      {
        next = hcNodeGetParentNode(current);
      }
      while (next && hcNodeGetNextSiblingNode(next) == NULL);

      next = hcNodeGetNextSiblingNode(next);
    }
  }

  return (next);
}
