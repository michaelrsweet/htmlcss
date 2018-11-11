/*
 * CSS import functions for HTMLCSS library.
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
 * Local functions...
 */


/*
 * 'cssDelete()' - Free memory associated with a stylesheet.
 */

void
cssDelete(css_t *css)			/* I - Stylesheet */
{
  if (!css)
    return;

  free(css);
}


/*
 * 'cssNew()' - Allocate a new stylesheet.
 */

css_t *					/* O - Stylesheet */
cssNew(htmlcss_pool_t *pool)		/* I - Memory pool */
{
  css_t	*css = (css_t *)calloc(1, sizeof(css_t));
					/* Stylesheet */


  if (css)
  {
    css->pool     = pool;
    css->error_cb = _htmlcssDefaultErrorCB;
    css->url_cb   = _htmlcssDefaultURLCB;
  }

  return (css);
}


/*
 * 'cssSetMedia()' - Set the base media settings.
 */

int					/* O - 1 on success, 0 on failure */
cssSetMedia(css_t      *css,		/* I - Stylesheet */
            const char *media,		/* I - Media name ("print', etc.) */
            float      width,		/* I - Device width */
            float      height)		/* I - Device height */
{
  (void)css;
  (void)media;
  (void)width;
  (void)height;

  return (0);
}
