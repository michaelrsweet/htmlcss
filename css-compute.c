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

#  include "css-private.h"


/*
 * 'hcCSSComputeBox()' - Compute the box properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSComputeBox(hc_css_t         *css,	/* I - Stylesheet */
              hc_node_t   *node,	/* I - HTML node */
	      hc_compute_t compute,	/* I - Pseudo-class, if any */
              hc_box_t     *box)	/* O - Box properties */
{
  return (0);
}


/*
 * 'hcCSSComputeContent()' - Compute the content: value for the given HTML node.
 *
 * The returned string must be freed using `free()`.
 */

char *					/* O - Content string or `NULL` if none/error */
hcCSSComputeContent(hc_css_t         *css,	/* I - Stylesheet */
                  hc_node_t   *node,	/* I - HTML node */
                  hc_compute_t compute)/* I - Pseudo-class, if any */
{
  return (NULL);
}


/*
 * 'hcCSSComputeDisplay()' - Compute the display property for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSComputeDisplay(
    hc_css_t         *css,			/* I - Stylesheet */
    hc_node_t   *node,		/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_display_t *display)		/* O - Display property */
{
  return (0);
}


/*
 * 'hcCSSComputeMedia()' - Compute the media properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSComputeMedia(hc_css_t         *css,	/* I - Stylesheet */
                hc_node_t   *node,	/* I - HTML node */
		hc_compute_t compute,	/* I - Pseudo-class, if any */
                hc_media_t   *media)	/* O - Media properties */
{
  return (0);
}


/*
 * 'hcCSSComputeTable()' - Compute the table properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSComputeTable(hc_css_t         *css,	/* I - Stylesheet */
                hc_node_t   *node,	/* I - HTML node */
		hc_compute_t compute,	/* I - Pseudo-class, if any */
                hc_table_t   *table)	/* O - Table properties */
{
  return (0);
}


/*
 * 'hcCSSComputeText()' - Compute the text properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSComputeText(hc_css_t         *css,	/* I - Stylesheet */
               hc_node_t   *node,	/* I - HTML node */
	       hc_compute_t compute,	/* I - Pseudo-class, if any */
               hc_text_t    *text)	/* O - Text properties */
{
  return (0);
}
