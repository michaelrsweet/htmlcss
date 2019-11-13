/*
 * CSS header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_CSS_H
#  define HTMLCSS_CSS_H

/*
 * Include necessary headers...
 */

#  include "dict.h"
#  include "file.h"
#  include "font.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Constants...
 */

#  define HC_LENGTH_AUTO	-999999999.0f
					/* Automatic length */
#  define HC_MAX_BOX_SHADOW	4	/* Maximum number of box shadow values */


/*
 * Types...
 */

typedef enum
{
  HC_BACKGROUND_ATTACHMENT_SCROLL,
  HC_BACKGROUND_ATTACHMENT_FIXED
} hc_background_attachment_t;

typedef enum
{
  HC_BACKGROUND_BOX_BORDER_BOX,
  HC_BACKGROUND_BOX_PADDING_BOX,
  HC_BACKGROUND_BOX_CONTENT_BOX
} hc_background_box_t;

typedef enum
{
  HC_BACKGROUND_REPEAT_NO_REPEAT,
  HC_BACKGROUND_REPEAT_REPEAT,
  HC_BACKGROUND_REPEAT_REPEAT_X,
  HC_BACKGROUND_REPEAT_REPEAT_Y
} hc_background_repeat_t;

typedef enum
{
  HC_BOOL_FALSE,
  HC_BOOL_TRUE
} hc_bool_t;

typedef enum
{
  HC_BORDER_COLLAPSE_SEPARATE,
  HC_BORDER_COLLAPSE_COLLAPSE
} hc_border_collapse_t;

typedef enum
{
  HC_BORDER_IMAGE_REPEAT_STRETCH,
  HC_BORDER_IMAGE_REPEAT_REPEAT,
  HC_BORDER_IMAGE_REPEAT_ROUND,
  HC_BORDER_IMAGE_REPEAT_SPACE
} hc_border_image_repeat_t;


typedef enum
{
  HC_BORDER_STYLE_HIDDEN,
  HC_BORDER_STYLE_NONE,
  HC_BORDER_STYLE_DOTTED,
  HC_BORDER_STYLE_DASHED,
  HC_BORDER_STYLE_SOLID,
  HC_BORDER_STYLE_DOUBLE,
  HC_BORDER_STYLE_GROOVE,
  HC_BORDER_STYLE_RIDGE,
  HC_BORDER_STYLE_INSET,
  HC_BORDER_STYLE_OUTSET
} hc_border_style_t;

typedef enum
{
  HC_BREAK_AUTO,			/* Break as needed */
  HC_BREAK_ALWAYS,			/* Always break */
  HC_BREAK_AVOID,			/* Avoid a break */
  HC_BREAK_LEFT,			/* Break to the next left-hand page */
  HC_BREAK_RIGHT			/* Break to the next right-hand page */
} hc_break_t;

typedef enum
{
  HC_CAPTION_SIDE_TOP,
  HC_CAPTION_SIDE_BOTTOM
} hc_caption_side_t;

typedef enum
{
  HC_DIRECTION_LTR,			/* Left to right */
  HC_DIRECTION_RTL			/* Right to left */
} hc_direction_t;

typedef enum
{
  HC_DISPLAY_NONE,
  HC_DISPLAY_BLOCK,
  HC_DISPLAY_INLINE,
  HC_DISPLAY_INLINE_BLOCK,
  HC_DISPLAY_INLINE_TABLE,
  HC_DISPLAY_LIST_ITEM,
  HC_DISPLAY_TABLE,
  HC_DISPLAY_TABLE_CAPTION,
  HC_DISPLAY_TABLE_HEADER_GROUP,
  HC_DISPLAY_TABLE_FOOTER_GROUP,
  HC_DISPLAY_TABLE_ROW_GROUP,
  HC_DISPLAY_TABLE_ROW,
  HC_DISPLAY_TABLE_COLUMN_GROUP,
  HC_DISPLAY_TABLE_COLUMN,
  HC_DISPLAY_TABLE_CELL
} hc_display_t;

typedef enum
{
  HC_EMPTY_CELLS_HIDE,
  HC_EMPTY_CELLS_SHOW
} hc_empty_cells_t;

typedef enum
{
  HC_FLOAT_NONE,
  HC_FLOAT_LEFT,
  HC_FLOAT_RIGHT
} hc_float_t;

typedef enum
{
  HC_LIST_STYLE_POSITION_INSIDE,
  HC_LIST_STYLE_POSITION_OUTSIDE
} hc_list_style_position_t;

typedef enum
{
  HC_LIST_STYLE_TYPE_NONE,
  HC_LIST_STYLE_TYPE_DISC,
  HC_LIST_STYLE_TYPE_CIRCLE,
  HC_LIST_STYLE_TYPE_SQUARE,
  HC_LIST_STYLE_TYPE_DECIMAL,
  HC_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO,
  HC_LIST_STYLE_TYPE_LOWER_ROMAN,
  HC_LIST_STYLE_TYPE_UPPER_ROMAN,
  HC_LIST_STYLE_TYPE_LOWER_GREEK,
  HC_LIST_STYLE_TYPE_LOWER_LATIN,
  HC_LIST_STYLE_TYPE_UPPER_LATIN,
  HC_LIST_STYLE_TYPE_ARMENIAN,
  HC_LIST_STYLE_TYPE_GEORGIAN,
  HC_LIST_STYLE_TYPE_LOWER_ALPHA,
  HC_LIST_STYLE_TYPE_UPPER_ALPHA
} hc_list_style_type_t;

typedef enum
{
  HC_OVERFLOW_HIDDEN,
  HC_OVERFLOW_VISIBLE,
  HC_OVERFLOW_SCROLL,
  HC_OVERFLOW_AUTO
} hc_overflow_t;

typedef enum
{
  HC_TABLE_LAYOUT_AUTO,
  HC_TABLE_LAYOUT_FIXED
} hc_table_layout_t;

typedef enum
{
  HC_TEXT_ALIGN_LEFT,
  HC_TEXT_ALIGN_RIGHT,
  HC_TEXT_ALIGN_CENTER,
  HC_TEXT_ALIGN_JUSTIFY
} hc_text_align_t;

typedef enum
{
  HC_TEXT_DECORATION_NONE,
  HC_TEXT_DECORATION_UNDERLINE,
  HC_TEXT_DECORATION_OVERLINE,
  HC_TEXT_DECORATION_LINE_THROUGH
} hc_text_decoration_t;

typedef enum
{
  HC_TEXT_TRANSFORM_NONE,
  HC_TEXT_TRANSFORM_CAPITALIZE,
  HC_TEXT_TRANSFORM_LOWERCASE,
  HC_TEXT_TRANSFORM_UPPERCASE
} hc_text_transform_t;

typedef enum
{
  HC_UNICODE_BIDI_NORMAL,
  HC_UNICODE_BIDI_EMBED,
  HC_UNICODE_BIDI_OVERRIDE
} hc_unicode_bidi_t;

typedef enum
{
  HC_WHITE_SPACE_NORMAL,
  HC_WHITE_SPACE_NOWRAP,
  HC_WHITE_SPACE_PRE,
  HC_WHITE_SPACE_PRE_LINE,
  HC_WHITE_SPACE_PRE_WRAP
} hc_white_space_t;

typedef struct hc_color_s		/* sRGBA color */
{
  float			red;		/* Red, 0.0 to 1.0 */
  float			green;		/* Green, 0.0 to 1.0 */
  float			blue;		/* Blue, 0.0 to 1.0 */
  float			alpha;		/* Alpha, 0.0 (transparent) to 1.0 (opaque) */
} hc_color_t;

typedef struct hc_point_s		/* Point/coordinate */
{
  float			left;		/* Horizontal position */
  float			top;		/* Vertical position */
} hc_point_t;

typedef struct hc_size_s		/* Point/coordinate */
{
  float			width;		/* Width */
  float			height;		/* Height */
} hc_size_t;

typedef struct hc_border_props_s	/* CSS border properties */
{
  hc_color_t		color;		/* Border color */
  hc_border_style_t	style;		/* Border style */
  float			width;		/* Border width */
} hc_border_props_t;

typedef struct hc_box_shadow_s		/* Box shadow values */
{
  float			horizontal_offset;
  float			vertical_offset;
  float			blur_radius;
  float			spread_distance;
  hc_color_t		color;
  int			inset;
} hc_box_shadow_t;

/* Higher-level types */


typedef enum				/* What to compute */
{
  HC_COMPUTE_BASE,			/* Base content */
  HC_COMPUTE_BEFORE,			/* Content before element */
  HC_COMPUTE_AFTER,			/* Content after element */
  HC_COMPUTE_FIRST_LINE,		/* First line of block */
  HC_COMPUTE_FIRST_LETTER		/* First letter of block */
} hc_compute_t;

typedef struct hc_border_s		/* All CSS border properties */
{
  hc_border_props_t	left;
  hc_border_props_t	top;
  hc_border_props_t	right;
  hc_border_props_t	bottom;
} hc_border_t;

typedef struct hc_border_radius_s	/* CSS border-xxx-radius properties */
{
  hc_size_t		bottom_left;	/* Bottom-left border radius */
  hc_size_t		bottom_right;	/* Bottom-right border radius */
  hc_size_t		top_left;	/* Top-left border radius */
  hc_size_t		top_right;	/* Top-right border radius */
} hc_border_radius_t;

typedef struct hc_box_s			/* CSS box properties */
{
  hc_rect_t		bounds;		/* Computed bounds */
  hc_size_t		size;		/* Computed size */
  hc_rect_t		clip;		/* Clip bounds */
  hc_size_t		max_size;
  hc_size_t		min_size;
  hc_background_attachment_t
			background_attachment;
  hc_background_box_t	background_clip;
  hc_color_t		background_color;
  const char		*background_image;
  hc_background_box_t	background_origin;
  hc_point_t		background_position;
  hc_background_repeat_t background_repeat;
  hc_size_t		background_size;
  hc_border_collapse_t	border_collapse;
  const char		*border_image;
  hc_bool_t		border_image_fill;
  hc_rect_t		border_image_outset;
  hc_border_image_repeat_t border_image_repeat[2];
  hc_rect_t		border_image_slice;
  hc_rect_t		border_image_width;
  hc_border_radius_t	border_radius;
  hc_size_t		border_spacing;
  hc_border_t		border;
  hc_box_shadow_t	box_shadow[HC_MAX_BOX_SHADOW];
  int			box_shadow_count;
  hc_break_t		break_after;
  hc_break_t		break_before;
  hc_break_t		break_inside;
  hc_float_t		float_value;
  const char		*list_style_image;
  hc_list_style_position_t
			list_style_position;
  hc_rect_t		margin;
  int			orphans;
  hc_overflow_t		overflow;
  hc_rect_t		padding;
  int			widows;
  int			z_index;
} hc_box_t;

typedef struct hc_media_s		/* CSS media properties */
{
  const char		*type;		/* "print", "screen", etc. */
  int			color_bits;	/* Color bits */
  int			monochrome_bits;/* Grayscale bits */
  hc_rect_t		margin;		/* Margins */
  hc_size_t		size;		/* Dimensions */
} hc_media_t;

typedef struct hc_table_s		/* CSS table properties */
{
  hc_caption_side_t	caption_side;
  hc_empty_cells_t	empty_cells;
  hc_table_layout_t	table_layout;
} hc_table_t;

typedef struct hc_text_s		/* CSS text properties */
{
  hc_direction_t	direction;
  hc_font_t		*font;		/* Loaded font */
  const char		*font_family;
  float			font_size;
  float			font_size_adjust;
  hc_font_stretch_t	font_stretch;
  hc_font_style_t	font_style;
  hc_font_variant_t	font_variant;
  hc_font_weight_t	font_weight;
  float			letter_spacing;
  float			line_height;
  const char		*quotes[4];
  hc_text_align_t	text_align;
  hc_text_decoration_t	text_decoration;
  float			text_indent;
  hc_text_transform_t	text_transform;
  hc_unicode_bidi_t	unicode_bidi;
  hc_white_space_t	white_space;
  float			word_spacing;
} hc_text_t;

typedef struct _hc_css_s hc_css_t;		/* CSS data */


/*
 * Functions...
 */

extern void	hcCSSDelete(hc_css_t *css);
extern hc_css_t	*hcCSSNew(hc_pool_t *pool);
extern int	hcCSSImport(hc_css_t *css, hc_file_t *file);
extern int	hcCSSImportDefault(hc_css_t *css);
extern void	hcCSSSetErrorCallback(hc_css_t *css, hc_error_cb_t cb, void *ctx);
extern void	hcCSSSetURLCallback(hc_css_t *css, hc_url_cb_t cb, void *ctx);
extern int	hcCSSSetMedia(hc_css_t *css, const char *type, int color_bits, int grayscale_bits, float width, float height);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_CSS_H */
