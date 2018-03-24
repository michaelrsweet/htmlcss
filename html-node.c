/*
 * HTML node functions for HTMLCSS library.
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

static void		html_delete(html_node_t *node);
static html_node_t	*html_new(html_node_t *parent, html_element_t element);
static void		html_remove(html_node_t *node);


/*
 * 'htmlDeleteNode()' - Delete a HTML node from a document.
 */

void
htmlDeleteNode(html_t      *html,	/* I - HTML document */
               html_node_t *node)	/* I - HTML node */
{
  html_node_t	*current,		/* Current node */
		*next;			/* Next node */


  if (!html || !node)
    return;

  if (node == html->root)
    html->root = NULL;

  html_remove(node);

  for (current = node->first_child; current; current = next)
  {
   /*
    * Get the next node...
    */

    if ((next = current->first_child) != NULL)
    {
     /*
      * Free parent nodes after child nodes have been freed...
      */

      current->first_child = NULL;
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
 * 'htmlGetComment()' - Get a HTML node's comment value, if any.
 */

const char *				/* O - Comment value */
htmlGetComment(html_node_t *node)	/* I - HTML node */
{
  return (node && node->element == HTML_ELEMENT_COMMENT ? node->value.comment : NULL);
}


/*
 * 'htmlGetDOCTYPE()' - Get a HTML document's DOCTYPE value, if any.
 */

const char *				/* O - DOCTYPE value */
htmlGetDOCTYPE(html_t *html)		/* I - HTML document */
{
  return (html && html->root ? html->root->value.doctype : NULL);
}


/*
 * 'htmlGetElement()' - Get a HTML node's element/type.
 */

html_element_t				/* O - Node element/type */
htmlGetElement(html_node_t *node)	/* I - HTML node */
{
  return (node ? node->element : HTML_ELEMENT_WILDCARD);
}


/*
 * 'htmlGetFirstChildNode()' - Get a HTML node's first child node, if any.
 */

html_node_t *				/* O - First child node or `NULL` if none */
htmlGetFirstChildNode(html_node_t *node)/* I - HTML node */
{
  return (node ? node->first_child : NULL);
}


/*
 * 'htmlGetLastChildNode()' - Get a HTML node's last child node, if any.
 */

html_node_t *				/* O - Last child node or `NULL` if none */
htmlGetLastChildNode(html_node_t *node)	/* I - HTML node */
{
  return (node ? node->last_child : NULL);
}


/*
 * 'htmlGetNextSiblingNode()' - Get a HTML node's next sibling node, if any.
 */

html_node_t *				/* O - Next sibling node or `NULL` if none */
htmlGetNextSiblingNode(
    html_node_t *node)			/* I - HTML node */
{
  return (node ? node->next_sibling : NULL);
}


/*
 * 'htmlGetParentNode()' - Get a HTML node's parent node, if any.
 */

html_node_t *				/* O - Parent node or `NULL` if none */
htmlGetParentNode(html_node_t *node)	/* I - HTML node */
{
  return (node ? node->parent : NULL);
}


/*
 * 'htmlGetPrevSiblingNode()' - Get a HTML node's previous sibling node, if any.
 */

html_node_t *				/* O - Previous sibling node or `NULL` if none */
htmlGetPrevSiblingNode(
    html_node_t *node)			/* I - HTML node */
{
  return (node ? node->prev_sibling : NULL);
}


/*
 * 'htmlGetRootNode()' - Get the root node for a document.
 */

html_node_t *				/* O - Root node or `NULL` if none */
htmlGetRootNode(html_t *html)		/* I - HTML document */
{
  return (html ? html->root : NULL);
}


/*
 * 'htmlGetString()' - Get a HTML node's string value, if any.
 */

const char *				/* O - String value */
htmlGetString(html_node_t *node)	/* I - HTML node */
{
  return (node && node->element == HTML_ELEMENT_STRING ? node->value.string : NULL);
}


/*
 * 'htmlNewComment()' - Create a new HTML comment node.
 */

html_node_t *				/* O - New HTML comment node */
htmlNewComment(html_node_t *parent,	/* I - Parent node */
               const char  *c)		/* I - Comment value */
{
  html_node_t	*node;			/* New node */


  if (!parent || !c)
    return (NULL);

  if ((node = html_new(parent, HTML_ELEMENT_COMMENT)) != NULL)
    node->value.comment = strdup(c);

  return (node);
}


/*
 * 'htmlNewElement()' - Create a new HTML element node.
 */

html_node_t *				/* O - New HTML element node */
htmlNewElement(html_node_t    *parent,	/* I - Parent node */
               html_element_t element)	/* I - HTML element */
{
  if (!parent || element <= HTML_ELEMENT_DOCTYPE || element >= HTML_ELEMENT_MAX)
    return (NULL);

  return (html_new(parent, element));
}


/*
 * 'htmlNewRoot()' - Create a new root node.
 */

html_node_t *				/* O - New root node */
htmlNewRoot(html_t     *html,		/* I - HTML document */
            const char *doctype)	/* I - DOCTYPE value */
{
  html_node_t	*node;			/* New node */


  if (!html || html->root || !doctype)
    return (NULL);

  if ((node = html_new(NULL, HTML_ELEMENT_DOCTYPE)) != NULL)
  {
    html->root          = node;
    node->value.doctype = strdup(doctype);
  }

  return (node);
}


/*
 * 'htmlNewString()' - Create a new HTML string node.
 */

html_node_t *				/* O - New HTML string node */
htmlNewString(html_node_t *parent,	/* I - Parent node */
              const char  *s)		/* I - String value */
{
  html_node_t	*node;			/* New node */


  if (!parent || !s)
    return (NULL);

  if ((node = html_new(parent, HTML_ELEMENT_STRING)) != NULL)
    node->value.string = strdup(s);

  return (node);
}


/*
 * 'html_delete()' - Free a HTML node.
 */

static void
html_delete(html_node_t *node)		/* I - HTML node */
{
  if (node->element == HTML_ELEMENT_STRING)
  {
    free(node->value.string);
  }
  else if (node->element == HTML_ELEMENT_COMMENT)
  {
    free(node->value.comment);
  }
  else if (node->element == HTML_ELEMENT_DOCTYPE)
  {
    free(node->value.doctype);
  }
  else if (node->value.element.num_attrs > 0)
  {
    int			i;		/* Looping var */
    _html_attr_t	*attr;		/* Current attribute */

    for (i = node->value.element.num_attrs, attr = node->value.element.attrs; i > 0; i --, attr ++)
    {
      free(attr->name);
      free(attr->value);
    }

    if (node->value.element.attrs)
      free(node->value.element.attrs);
  }

  free(node);
}


/*
 * 'html_new()' - Create a new HTML node.
 */

static html_node_t *			/* O - New node or `NULL` on error */
html_new(html_node_t    *parent,	/* I - Parent node or `NULL` if root node */
         html_element_t element)	/* I - Element/node type */
{
  html_node_t	*node;			/* New node */


  if ((node = (html_node_t *)calloc(1, sizeof(html_node_t *))) != NULL)
  {
    node->element = element;
    node->parent  = parent;

    if (parent)
    {
      if (parent->last_child)
      {
        node->prev_sibling               = parent->last_child;
        parent->last_child->next_sibling = node;
        parent->last_child               = node;
      }
      else
      {
        parent->first_child = node;
        parent->last_child  = node;
      }
    }
  }

  return (node);
}


/*
 * 'html_remove()' - Remove a HTML node from its parent.
 */

static void
html_remove(html_node_t *node)		/* I - HTML node */
{
  if (node->parent)
  {
    if (node->prev_sibling)
      node->prev_sibling->next_sibling = node->next_sibling;
    else
      node->parent->first_child = node->next_sibling;

    if (node->next_sibling)
      node->next_sibling->prev_sibling = node->prev_sibling;
    else
      node->parent->last_child = node->prev_sibling;

    node->parent       = NULL;
    node->prev_sibling = NULL;
    node->next_sibling = NULL;
  }
}
