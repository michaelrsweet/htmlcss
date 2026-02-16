//
// CSS import functions for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2026 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#  include "css-private.h"
#  include "pool.h"


//
// 'hcCSSDelete()' - Free memory associated with a stylesheet.
//

void
hcCSSDelete(hc_css_t *css)		// I - Stylesheet
{
  hc_element_t	element;		// Current element


  if (!css)
    return;

  for (element = HC_ELEMENT_WILDCARD; element < HC_ELEMENT_MAX; element ++)
    _hcRuleColClear(css->rules + element, 0);
  _hcRuleColClear(&css->all_rules, 1);

  free(css);
}


//
// 'hcCSSNew()' - Allocate a new stylesheet.
//

hc_css_t *				// O - Stylesheet
hcCSSNew(hc_pool_t *pool)		// I - Memory pool
{
  hc_css_t	*css = (hc_css_t *)calloc(1, sizeof(hc_css_t));
					// Stylesheet


  if (css)
  {
    // Set defaults based on "universal" size (intersection of US Letter and
    // ISO A4) at 128ppi.
    css->pool = pool;

    hcCSSSetMedia(css, "print", 24, 8, 1058.27f, 1408.0f);
  }

  return (css);
}


//
// 'hcCSSSetMedia()' - Set the base media settings.
//

int					// O - 1 on success, 0 on failure
hcCSSSetMedia(
    hc_css_t   *css,			// I - Stylesheet
    const char *type,			// I - Media type ("print', etc.)
    int        color_bits,		// I - Bits of color supported
    int        monochrome_bits,		// I - Bits of grayscale supported
    float      width,			// I - Device width
    float      height)			// I - Device height
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
