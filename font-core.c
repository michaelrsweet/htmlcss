/*
 * Core font object functions for HTMLCSS library.
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
 * 'hcFontDelete()' - Free all memory used for a font family object.
 */

void
hcFontDelete(hc_font_t *font)		/* I - Font object */
{
  (void)font;
}


/*
 * 'hcFontNew()' - Create a new font object for the named font family.
 */

hc_font_t *				/* O - New font object */
hcFontNew(hc_pool_t  *pool,		/* I - Memory pool */
          const char *family)		/* I - Font family name */
{
  (void)pool;
  (void)family;

  return (NULL);
}
