/*
 * Font header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_FONT_H
#  define HTMLCSS_FONT_H

/*
 * Include necessary headers...
 */

#  include "css.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _hc_font_s hc_font_t;


/*
 * Functions...
 */

extern int		hcFontComputeExtents(hc_font_t *font, float size, hc_font_style_t style, hc_font_variant_t variant, hc_font_weight_t weight, const char *s, hc_rect_t *extents);
extern void		hcFontDelete(hc_font_t *font);
extern hc_font_t	*hcFontNew(hc_pool_t *pool, const char *family);



#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FONT_H */
