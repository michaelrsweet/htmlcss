/*
 * Private font header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
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

#  include "common-private.h"
#  include "font.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _hc_font_metric_s	/**** Font metric information ****/
{
  short			width,		/* Advance width */
			left_bearing;	/* Left side bearing */
} _hc_font_metric_t;

struct _hc_font_s
{
  hc_pool_t		*pool;		/* Memory pool */
  const char		*copyright;	/* Copyright string */
  const char		*family;	/* Font family string */
  const char		*postscript_name;
					/* PostScript name string */
  const char		*version;	/* Font version string */
  _hc_font_metric_t	*widths[256];	/* Character metrics (sparse array) */
  float			units;		/* Width units */
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
