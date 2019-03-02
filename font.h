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

extern int		hcFontComputeExtents(const hc_font_t *font, float size, const char *s, hc_rect_t *extents);
extern void		hcFontDelete(hc_font_t *font);
extern const char	*hcFontGetFamily(hc_font_t *font);
extern hc_font_t	*hcFontNew(hc_pool_t *pool, hc_file_t *file);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FONT_H */
