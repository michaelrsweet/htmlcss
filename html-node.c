/*
 * HTML node functions for HTMLCSS library.
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
 * Local functions...
 */

static void		html_delete(hc_node_t *node);
static hc_node_t	*html_new(hc_node_t *parent, hc_element_t element, const char *s);
static void		html_remove(hc_node_t *node);


/*
 * 'hcNodeDelete()' - Delete a HTML node from a document.
 */

void
hcNodeDelete(hc_html_t *html,		/* I - HTML document */
             hc_node_t *node)		/* I - HTML node */
{
  hc_node_t	*current,		/* Current node */
		*next;			/* Next node */


  if (!html || !node)
    return;

  if (node == html->root)
    html->root = NULL;

  html_remove(node);

  for (current = node->value.element.first_child; current; current = next)
  {
   /*
    * Get the next node...
    */

    if ((next = hcNodeGetFirstChildNode(current)) != NULL)
    {
     /*
      * Free parent nodes after child nodes have been freed...
      */

      current->value.element.first_child = NULL;
      continue;
    }

    if ((next = current->next_sibling) == NULL)
    {
     /*
      * Next node is the parent, which we'll free as needed...
      */

      if ((next = current->parent) == node)
        next = NULL;
    }

   /*
    * Free child...
    */

    html_delete(current);
  }

 /*
  * Then free the memory used by the parent node...
  */

  html_delete(node);
}


/*
 * 'hcNodeGetComment()' - Get a HTML node's comment value, if any.
 */

const char *				/* O - Comment value */
hcNodeGetComment(hc_node_t *node)	/* I - HTML node */
{
  return (node && node->element == HC_ELEMENT_COMMENT ? node->value.comment : NULL);
}


/*
 * 'hcHTMLGetDOCTYPE()' - Get a HTML document's DOCTYPE value, if any.
 */

const char *				/* O - DOCTYPE value */
hcHTMLGetDOCTYPE(hc_html_t *html)	/* I - HTML document */
{
  return (html && html->root ? hcNodeAttrGetNameValue(html->root, "") : NULL);
}


/*
 * 'hcNodeGetElement()' - Get a HTML node's element/type.
 */

hc_element_t				/* O - Node element/type */
hcNodeGetElement(hc_node_t *node)	/* I - HTML node */
{
  return (node ? node->element : HC_ELEMENT_WILDCARD);
}


/*
 * 'hcNodeGetFirstChildNode()' - Get a HTML node's first child node, if any.
 */

hc_node_t *				/* O - First child node or `NULL` if none */
hcNodeGetFirstChildNode(hc_node_t *node)/* I - HTML node */
{
  return (node && node->element >= HC_ELEMENT_DOCTYPE ? node->value.element.first_child : NULL);
}


/*
 * 'hcNodeGetLastChildNode()' - Get a HTML node's last child node, if any.
 */

hc_node_t *				/* O - Last child node or `NULL` if none */
hcNodeGetLastChildNode(hc_node_t *node)	/* I - HTML node */
{
  return (node && node->element >= HC_ELEMENT_DOCTYPE ? node->value.element.last_child : NULL);
}


/*
 * 'hcNodeGetNextSiblingNode()' - Get a HTML node's next sibling node, if any.
 */

hc_node_t *				/* O - Next sibling node or `NULL` if none */
hcNodeGetNextSiblingNode(
    hc_node_t *node)			/* I - HTML node */
{
  return (node ? node->next_sibling : NULL);
}


/*
 * 'hcNodeGetParentNode()' - Get a HTML node's parent node, if any.
 */

hc_node_t *				/* O - Parent node or `NULL` if none */
hcNodeGetParentNode(hc_node_t *node)	/* I - HTML node */
{
  return (node ? node->parent : NULL);
}


/*
 * 'hcNodeGetPrevSiblingNode()' - Get a HTML node's previous sibling node, if any.
 */

hc_node_t *				/* O - Previous sibling node or `NULL` if none */
hcNodeGetPrevSiblingNode(
    hc_node_t *node)			/* I - HTML node */
{
  return (node ? node->prev_sibling : NULL);
}


/*
 * 'hcHTMLGetRootNode()' - Get the root node for a document.
 */

hc_node_t *				/* O - Root node or `NULL` if none */
hcHTMLGetRootNode(hc_html_t *html)	/* I - HTML document */
{
  return (html ? html->root : NULL);
}


/*
 * 'hcNodeGetString()' - Get a HTML node's string value, if any.
 */

const char *				/* O - String value */
hcNodeGetString(hc_node_t *node)	/* I - HTML node */
{
  return (node && node->element == HC_ELEMENT_STRING ? node->value.string : NULL);
}


/*
 * 'hcNodeNewComment()' - Create a new HTML comment node.
 */

hc_node_t *				/* O - New HTML comment node */
hcNodeNewComment(hc_node_t *parent,	/* I - Parent node */
               const char  *c)		/* I - Comment value */
{
  if (!parent || !c)
    return (NULL);

  return (html_new(parent, HC_ELEMENT_COMMENT, c));
}


/*
 * 'hcNodeNewElement()' - Create a new HTML element node.
 */

hc_node_t *				/* O - New HTML element node */
hcNodeNewElement(hc_node_t    *parent,	/* I - Parent node */
               hc_element_t element)	/* I - HTML element */
{
  if (!parent || element <= HC_ELEMENT_DOCTYPE || element >= HC_ELEMENT_MAX)
    return (NULL);

  return (html_new(parent, element, NULL));
}


/*
 * 'hcHTMLNewRootNode()' - Create a new root node.
 */

hc_node_t *				/* O - New root node */
hcHTMLNewRootNode(hc_html_t  *html,	/* I - HTML document */
                  const char *doctype)	/* I - DOCTYPE value */
{
  hc_node_t	*node;			/* New node */


  if (!html || html->root || !doctype)
    return (NULL);

  if ((node = html_new(NULL, HC_ELEMENT_DOCTYPE, NULL)) != NULL)
  {
    html->root = node;
    hcNodeAttrSetNameValue(node, "", doctype);
  }

  return (node);
}


/*
 * 'hcNodeNewString()' - Create a new HTML string node.
 */

hc_node_t *				/* O - New HTML string node */
hcNodeNewString(hc_node_t  *parent,	/* I - Parent node */
                const char *s)		/* I - String value */
{
  if (!parent || !s)
    return (NULL);

  return (html_new(parent, HC_ELEMENT_STRING, s));
}


/*
 * '_hcNodeNewUnknown()' - Create a new unknown HTML element or processing
 *                       directive node.
 */

hc_node_t *				/* O - New HTML unknown node */
_hcNodeNewUnknown(hc_node_t  *parent,	/* I - Parent node */
                  const char *unk)	/* I - Unknown value (excluding "<>") */
{
  if (!parent || !unk)
    return (NULL);

  return (html_new(parent, HC_ELEMENT_UNKNOWN, unk));
}


/*
 * 'html_delete()' - Free a HTML node.
 */

static void
html_delete(hc_node_t *node)		/* I - HTML node */
{
  if (node->element >= HC_ELEMENT_DOCTYPE)
    hcDictDelete(node->value.element.attrs);

  free(node);
}


/*
 * 'html_new()' - Create a new HTML node.
 */

static hc_node_t *			/* O - New node or `NULL` on error */
html_new(hc_node_t    *parent,		/* I - Parent node or `NULL` if root node */
         hc_element_t element,		/* I - Element/node type */
         const char   *s)		/* I - String, if any */
{
  hc_node_t	*node;			/* New node */
  size_t	nodesize;		/* Node size */
  size_t	slen = s ? strlen(s) : 0;
					/* Length of string */


  if (parent && parent->element < HC_ELEMENT_DOCTYPE)
    return (NULL);

  if (element < HC_ELEMENT_DOCTYPE)
    nodesize = sizeof(hc_node_t) - sizeof(node->value) + strlen(s) + 1;
  else
    nodesize = sizeof(hc_node_t);

  if ((node = (hc_node_t *)calloc(1, nodesize)) != NULL)
  {
    node->element = element;
    node->parent  = parent;

    if (element == HC_ELEMENT_STRING)
      memcpy(node->value.string, s, slen);
    else if (element == HC_ELEMENT_COMMENT)
      memcpy(node->value.comment, s, slen);
    else if (element == HC_ELEMENT_UNKNOWN)
      memcpy(node->value.unknown, s, slen);

    if (parent)
    {
      if (parent->value.element.last_child)
      {
        node->prev_sibling                             = parent->value.element.last_child;
        parent->value.element.last_child->next_sibling = node;
        parent->value.element.last_child               = node;
      }
      else
      {
        parent->value.element.first_child = node;
        parent->value.element.last_child  = node;
      }
    }
  }

  return (node);
}


/*
 * 'html_remove()' - Remove a HTML node from its parent.
 */

static void
html_remove(hc_node_t *node)		/* I - HTML node */
{
  if (node->parent)
  {
    if (node->prev_sibling)
      node->prev_sibling->next_sibling = node->next_sibling;
    else
      node->parent->value.element.first_child = node->next_sibling;

    if (node->next_sibling)
      node->next_sibling->prev_sibling = node->prev_sibling;
    else
      node->parent->value.element.last_child = node->prev_sibling;

    node->parent       = NULL;
    node->prev_sibling = NULL;
    node->next_sibling = NULL;
  }
}
