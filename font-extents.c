/*
 * Font extents function for HTMLCSS library.
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

#include "font-private.h"


/*
 * 'hcFontComputeExtents()' - Compute the extents of a string when rendered using the
 *                            given font object, size, style, etc.
 */

int					/* O - 1 on success, 0 on failure */
hcFontComputeExtents(
    const hc_font_t *font,		/* I - Font object */
    float           size,		/* I - Font size */
    const char      *s,			/* I - String */
    hc_rect_t       *extents)		/* O - Extents of the string */
{
  (void)font;
  (void)size;
  (void)s;
  (void)extents;

  return (0);
}
