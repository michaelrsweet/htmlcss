/*
 * CSS computation functions for HTMLCSS library.
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

#  include "html-private.h"


/*
 * Local functions...
 */

static hc_dict_t *hc_create_props(hc_node_t *node, hc_compute_t compute);


/*
 * 'hcNodeComputeCSSBox()' - Compute the box properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSBox(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_box_t     *box)			/* O - Box properties */
{
  (void)node;
  (void)compute;
  (void)box;

  return (0);
}


/*
 * 'hcNodeComputeCSSContent()' - Compute the content: value for the given HTML node.
 *
 * The returned string must be freed using `free()`.
 */

char *					/* O - Content string or `NULL` if none/error */
hcNodeComputeCSSContent(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  (void)node;
  (void)compute;

  return (NULL);
}


/*
 * 'hcNodeComputeCSSDisplay()' - Compute the display property for the given HTML node.
 */

hc_display_t				/* O - 1 on success, 0 on failure */
hcNodeComputeCSSDisplay(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  (void)node;
  (void)compute;

  return (HC_DISPLAY_NONE);
}


/*
 * 'hcNodeComputeCSSMedia()' - Compute the media properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSMedia(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_media_t   *media)		/* O - Media properties */
{
  (void)node;
  (void)compute;
  (void)media;

  return (0);
}


/*
 * 'hcNodeComputeCSSProperties()' - Compute the properties for the given node.
 */

hc_dict_t *				/* O - Properties or `NULL` on error */
hcNodeComputeCSSProperties(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  hc_dict_t	*props;			/* Properties */


  /* TODO: Need a better caching/memory management story here */
  if (!node || node->element < HC_ELEMENT_DOCTYPE)
    return (NULL);
  else if (compute == HC_COMPUTE_NORMAL && node->value.element.props)
    return (hcDictCopy(node->value.element.props));

  props = hc_create_props(node, compute);

  if (compute == HC_COMPUTE_NORMAL)
    node->value.element.props = props;

  return (props);
}


/*
 * 'hcNodeComputeCSSTable()' - Compute the table properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSTable(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_table_t   *table)		/* O - Table properties */
{
  (void)node;
  (void)compute;
  (void)table;

  return (0);
}


/*
 * 'hcNodeComputeCSSText()' - Compute the text properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSText(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_text_t    *text)			/* O - Text properties */
{
  (void)node;
  (void)compute;
  (void)text;

  return (0);
}


/*
 * 'hc_create_props()' - Create a property dictionary for a node.
 */

static hc_dict_t *			/* O - Dictionary */
hc_create_props(hc_node_t    *node,	/* I - HTML node */
                hc_compute_t compute)	/* I - Pseudo-class, if any */
{
  (void)node;
  (void)compute;

  return (NULL);
}
