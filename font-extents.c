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
  int		ch,			/* Current character */
		width = 0;		/* Width */
  const short	*widths;		/* Widths */


  if (extents)
    memset(extents, 0, sizeof(hc_rect_t));

  if (!font || size <= 0.0f || !s || !extents)
    return (0);

  while (*s)
  {
    if ((*s & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
    {
      ch = ((*s & 0x1f) << 6) | (s[1] & 0x3f);
      s += 2;
    }
    else if ((*s & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80)
    {
      ch = ((*s & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
      s += 3;
    }
    else if ((*s & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
    {
      ch = ((*s & 0x07) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
      s += 4;
    }
    else if (*s & 0x80)
      return (0);
    else
      ch = *s++;

    if ((widths = font->widths[ch / 256]) != NULL)
      width += widths[ch & 255];
  }

  extents->right = size * width / _HC_FONT_UNITS;
  extents->top   = size;

  return (1);
}
