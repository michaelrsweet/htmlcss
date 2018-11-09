/*
 * CSS computation functions for HTMLCSS library.
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

#  include "css-private.h"


/*
 * 'cssComputeBox()' - Compute the box properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
cssComputeBox(css_t         *css,	/* I - Stylesheet */
              html_node_t   *node,	/* I - HTML node */
	      css_compute_t compute,	/* I - Pseudo-class, if any */
              css_box_t     *box)	/* O - Box properties */
{
  return (0);
}


/*
 * 'cssComputeContent()' - Compute the content: value for the given HTML node.
 *
 * The returned string must be freed using `free()`.
 */

char *					/* O - Content string or `NULL` if none/error */
cssComputeContent(css_t         *css,	/* I - Stylesheet */
                  html_node_t   *node,	/* I - HTML node */
                  css_compute_t compute)/* I - Pseudo-class, if any */
{
  return (NULL);
}


/*
 * 'cssComputeDisplay()' - Compute the display property for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
cssComputeDisplay(
    css_t         *css,			/* I - Stylesheet */
    html_node_t   *node,		/* I - HTML node */
    css_compute_t compute,		/* I - Pseudo-class, if any */
    css_display_t *display)		/* O - Display property */
{
  return (0);
}


/*
 * 'cssComputeMedia()' - Compute the media properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
cssComputeMedia(css_t         *css,	/* I - Stylesheet */
                html_node_t   *node,	/* I - HTML node */
		css_compute_t compute,	/* I - Pseudo-class, if any */
                css_media_t   *media)	/* O - Media properties */
{
  return (0);
}


/*
 * 'cssComputeTable()' - Compute the table properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
cssComputeTable(css_t         *css,	/* I - Stylesheet */
                html_node_t   *node,	/* I - HTML node */
		css_compute_t compute,	/* I - Pseudo-class, if any */
                css_table_t   *table)	/* O - Table properties */
{
  return (0);
}


/*
 * 'cssComputeText()' - Compute the text properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
cssComputeText(css_t         *css,	/* I - Stylesheet */
               html_node_t   *node,	/* I - HTML node */
	       css_compute_t compute,	/* I - Pseudo-class, if any */
               css_text_t    *text)	/* O - Text properties */
{
  return (0);
}
