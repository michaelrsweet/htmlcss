/*
 * Private font header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_FONT_PRIVATE_H
#  define HTMLCSS_FONT_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "font.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Constants...
 */

#  define _HC_FONT_UNITS	16384	/* Unit size for font dimensions */


/*
 * Types...
 */

struct _hc_font_s
{
  hc_pool_t		*pool;		/* Memory pool */
  const char		*copyright;	/* Copyright string */
  const char		*family;	/* Font family string */
  const char		*postscript_name;
					/* PostScript name string */
  const char		*version;	/* Font version string */
  short			*widths[256];	/* Character widths (sparse array) */
  short			ascent,		/* Maximum ascent above baseline */
			descent,	/* Maximum descent below baseline */
			cap_height,	/* "A" height */
			x_height,	/* "x" height */
			x_max,		/* Bounding box */
			x_min,
			y_max,
			y_min,
			weight;		/* Font weight */
  float			italic_angle;	/* Angle of italic text */
  hc_font_style_t	style;		/* Font style */
};


/*
 * Functions...
 */



#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FONT_PRIVATE_H */
