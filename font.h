/*
 * Font header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
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

#  include "file.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef enum				/**** Font style ****/
{
  HC_FONT_STYLE_NORMAL,			/* Normal font */
  HC_FONT_STYLE_ITALIC,			/* Italic font */
  HC_FONT_STYLE_OBLIQUE			/* Oblique (angled) font */
} hc_font_style_t;

typedef enum				/**** Font variant ****/
{
  HC_FONT_VARIANT_NORMAL,		/* Normal font */
  HC_FONT_VARIANT_SMALL_CAPS		/* Font whose lowercase letters are small capitals */
} hc_font_variant_t;

typedef enum				/**** Font weight ****/
{
  HC_FONT_WEIGHT_NORMAL,		/* Normal weight, nominally 400 */
  HC_FONT_WEIGHT_BOLD,			/* Bold weight, nominally 700 */
  HC_FONT_WEIGHT_BOLDER,		/* Bolder */
  HC_FONT_WEIGHT_LIGHTER,		/* Lighter */
  HC_FONT_WEIGHT_100 = 100,		/* Weight 100 (Thin) */
  HC_FONT_WEIGHT_200 = 200,		/* Weight 200 (Extra/Ultra-Light) */
  HC_FONT_WEIGHT_300 = 300,		/* Weight 300 (Light) */
  HC_FONT_WEIGHT_400 = 400,		/* Weight 400 (Normal/Regular) */
  HC_FONT_WEIGHT_500 = 500,		/* Weight 500 (Medium) */
  HC_FONT_WEIGHT_600 = 600,		/* Weight 600 (Semi/Demi-Bold) */
  HC_FONT_WEIGHT_700 = 700,		/* Weight 700 (Bold) */
  HC_FONT_WEIGHT_800 = 800,		/* Weight 800 (Extra/Ultra-Bold) */
  HC_FONT_WEIGHT_900 = 900		/* Weight 900 (Black/Heavy) */
} hc_font_weight_t;

typedef struct _hc_font_s hc_font_t;	/**** Font object ****/

typedef struct hc_rect_s		/* Rectangle */
{
  float			left;		/* Left offset */
  float			top;		/* Top offset */
  float			right;		/* Right offset */
  float			bottom;		/* Bottom offset */
} hc_rect_t;


/*
 * Functions...
 */

extern int		hcFontComputeExtents(const hc_font_t *font, float size, const char *s, hc_rect_t *extents);
extern void		hcFontDelete(hc_font_t *font);
extern const char	*hcFontGetCopyright(hc_font_t *font);
extern const char	*hcFontGetFamily(hc_font_t *font);
extern const char	*hcFontGetPostScriptName(hc_font_t *font);
extern hc_font_style_t	hcFontGetStyle(hc_font_t *font);
extern const char	*hcFontGetVersion(hc_font_t *font);
extern hc_font_weight_t	hcFontGetWeight(hc_font_t *font);
extern hc_font_t	*hcFontNew(hc_pool_t *pool, hc_file_t *file);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FONT_H */
