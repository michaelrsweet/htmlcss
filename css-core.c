/*
 * CSS import functions for HTMLCSS library.
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
 * Local functions...
 */


/*
 * 'hcCSSDelete()' - Free memory associated with a stylesheet.
 */

void
hcCSSDelete(hc_css_t *css)			/* I - Stylesheet */
{
  if (!css)
    return;

  free(css);
}


/*
 * 'hcNewCSS()' - Allocate a new stylesheet.
 */

hc_css_t *					/* O - Stylesheet */
hcNewCSS(hc_pool_t *pool)		/* I - Memory pool */
{
  hc_css_t	*css = (hc_css_t *)calloc(1, sizeof(hc_css_t));
					/* Stylesheet */


  if (css)
  {
    css->pool     = pool;
    css->error_cb = _hcDefaultErrorCB;
    css->url_cb   = _hcDefaultURLCB;
  }

  return (css);
}


/*
 * 'hcCSSSetMedia()' - Set the base media settings.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSSetMedia(hc_css_t      *css,		/* I - Stylesheet */
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
