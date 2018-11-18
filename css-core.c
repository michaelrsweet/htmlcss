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
#  include "pool.h"


/*
 * Local functions...
 */


/*
 * 'hcCSSDelete()' - Free memory associated with a stylesheet.
 */

void
hcCSSDelete(hc_css_t *css)		/* I - Stylesheet */
{
  if (!css)
    return;

  free(css);
}


/*
 * 'hcCSSNew()' - Allocate a new stylesheet.
 */

hc_css_t *				/* O - Stylesheet */
hcCSSNew(hc_pool_t *pool)		/* I - Memory pool */
{
  hc_css_t	*css = (hc_css_t *)calloc(1, sizeof(hc_css_t));
					/* Stylesheet */


  if (css)
  {
   /*
    * Set defaults based on "universal" size (intersection of US Letter and ISO A4) at
    * 128ppi.
    */

    css->pool     = pool;
    css->error_cb = _hcDefaultErrorCB;
    css->url_cb   = _hcDefaultURLCB;

    hcCSSSetMedia(css, "print", 24, 8, 1058.27f, 1408.0f);
  }

  return (css);
}


/*
 * '_hcCSSSelDelete()' - Delete a CSS selector.
 */

void
_hcCSSSelDelete(_hc_css_sel_t *sel)	/* I - Selector to delete */
{
  _hc_css_sel_t *prev;			/* Previous selector */


  while (sel)
  {
    prev = sel->prev;

    if (sel->stmts)
      free(sel->stmts);

    free(sel);

    sel = prev;
  }
}


/*
 * 'hcCSSSetMedia()' - Set the base media settings.
 */

int					/* O - 1 on success, 0 on failure */
hcCSSSetMedia(
    hc_css_t   *css,			/* I - Stylesheet */
    const char *type,			/* I - Media type ("print', etc.) */
    int        color_bits,		/* I - Bits of color supported */
    int        monochrome_bits,		/* I - Bits of grayscale supported */
    float      width,			/* I - Device width */
    float      height)			/* I - Device height */
{
  if (!css || !type || color_bits < 0 || monochrome_bits < 0 || width <= 0.0f || height <= 0.0f)
    return (0);

  css->media.type            = hcPoolGetString(css->pool, type);
  css->media.color_bits      = color_bits;
  css->media.monochrome_bits = monochrome_bits;
  css->media.size.width      = width;
  css->media.size.height     = height;

  return (0);
}
