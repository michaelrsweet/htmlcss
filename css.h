/*
 * CSS header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_CSS_H
#  define HTMLCSS_CSS_H

/*
 * Include necessary headers...
 */

#  include "html.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef enum
{
  CSS_BACKGROUND_ATTACHMENT_SCROLL,
  CSS_BACKGROUND_ATTACHMENT_FIXED
} css_background_attachment_t;

typedef enum
{
  CSS_BACKGROUND_REPEAT_NO_REPEAT,
  CSS_BACKGROUND_REPEAT_REPEAT,
  CSS_BACKGROUND_REPEAT_REPEAT_X,
  CSS_BACKGROUND_REPEAT_REPEAT_Y
} css_background_repeat_t;

typedef enum
{
  CSS_BORDER_COLLAPSE_COLLAPSE,
  CSS_BORDER_COLLAPSE_SEPARATE
} css_border_collapse_t;

typedef enum
{
  CSS_BORDER_STYLE_HIDDEN,
  CSS_BORDER_STYLE_NONE,
  CSS_BORDER_STYLE_DOTTED,
  CSS_BORDER_STYLE_DASHED,
  CSS_BORDER_STYLE_SOLID,
  CSS_BORDER_STYLE_DOUBLE,
  CSS_BORDER_STYLE_GROOVE,
  CSS_BORDER_STYLE_RIDGE,
  CSS_BORDER_STYLE_INSET,
  CSS_BORDER_STYLE_OUTSET
} css_border_style_t;

typedef enum
{
  CSS_BREAK_AVOID,			/* Avoid a break */
  CSS_BREAK_AUTO,			/* Break as needed */
  CSS_BREAK_ALWAYS,			/* Always break */
  CSS_BREAK_LEFT,			/* Break to the next left-hand page */
  CSS_BREAK_RIGHT			/* Break to the next right-hand page */
} css_break_t;

typedef enum
{
  CSS_CAPTION_SIDE_TOP,
  CSS_CAPTION_SIDE_BOTTOM
} css_caption_side_t;

typedef enum
{
  CSS_DIRECTION_LTR,			/* Left to right */
  CSS_DIRECTION_RTL			/* Right to left */
} css_direction_t;

typedef enum
{
  CSS_DISPLAY_NONE,
  CSS_DISPLAY_BLOCK,
  CSS_DISPLAY_INLINE,
  CSS_DISPLAY_INLINE_BLOCK,
  CSS_DISPLAY_INLINE_TABLE,
  CSS_DISPLAY_LIST_ITEM,
  CSS_DISPLAY_TABLE,
  CSS_DISPLAY_TABLE_CAPTION,
  CSS_DISPLAY_TABLE_HEADER_GROUP,
  CSS_DISPLAY_TABLE_FOOTER_GROUP,
  CSS_DISPLAY_TABLE_ROW_GROUP,
  CSS_DISPLAY_TABLE_ROW,
  CSS_DISPLAY_TABLE_COLUMN_GROUP,
  CSS_DISPLAY_TABLE_COLUMN,
  CSS_DISPLAY_TABLE_CELL
} css_display_t;

typedef enum
{
  CSS_EMPTY_CELLS_HIDE,
  CSS_EMPTY_CELLS_SHOW
} css_empty_cells_t;

typedef enum
{
  CSS_FLOAT_NONE,
  CSS_FLOAT_LEFT,
  CSS_FLOAT_RIGHT
} css_float_t;

typedef enum
{
  CSS_FONT_STYLE_NORMAL,
  CSS_FONT_STYLE_ITALIC,
  CSS_FONT_STYLE_OBLIQUE
} css_font_style_t;

typedef enum
{
  CSS_FONT_VARIANT_NORMAL,
  CSS_FONT_VARIANT_SMALL_CAPS
} css_font_variant_t;

typedef enum
{
  CSS_FONT_WEIGHT_NORMAL,		/* Nominally 400 */
  CSS_FONT_WEIGHT_BOLD,			/* Nominally 700 */
  CSS_FONT_WEIGHT_BOLDER,
  CSS_FONT_WEIGHT_LIGHTER,
  CSS_FONT_WEIGHT_100 = 100,
  CSS_FONT_WEIGHT_200 = 200,
  CSS_FONT_WEIGHT_300 = 300,
  CSS_FONT_WEIGHT_400 = 400,
  CSS_FONT_WEIGHT_500 = 500,
  CSS_FONT_WEIGHT_600 = 600,
  CSS_FONT_WEIGHT_700 = 700,
  CSS_FONT_WEIGHT_800 = 800,
  CSS_FONT_WEIGHT_900 = 900
} css_font_weight_t;

typedef enum
{
  CSS_LIST_STYLE_POSITION_INSIDE,
  CSS_LIST_STYLE_POSITION_OUTSIDE
} css_list_style_position_t;

typedef enum
{
  CSS_LIST_STYLE_TYPE_NONE,
  CSS_LIST_STYLE_TYPE_DISC,
  CSS_LIST_STYLE_TYPE_CIRCLE,
  CSS_LIST_STYLE_TYPE_SQUARE,
  CSS_LIST_STYLE_TYPE_DECIMAL,
  CSS_LIST_STYLE_TYPE_DECIMAL_LEADING_ZERO,
  CSS_LIST_STYLE_TYPE_LOWER_ROMAN,
  CSS_LIST_STYLE_TYPE_UPPER_ROMAN,
  CSS_LIST_STYLE_TYPE_LOWER_GREEK,
  CSS_LIST_STYLE_TYPE_LOWER_LATIN,
  CSS_LIST_STYLE_TYPE_UPPER_LATIN,
  CSS_LIST_STYLE_TYPE_ARMENIAN,
  CSS_LIST_STYLE_TYPE_GEORGIAN,
  CSS_LIST_STYLE_TYPE_LOWER_ALPHA,
  CSS_LIST_STYLE_TYPE_UPPER_ALPHA
} css_list_style_type_t;

typedef enum
{
  CSS_OVERFLOW_HIDDEN,
  CSS_OVERFLOW_VISIBLE,
  CSS_OVERFLOW_SCROLL,
  CSS_OVERFLOW_AUTO
} css_overflow_t;

typedef enum
{
  CSS_TABLE_LAYOUT_AUTO,
  CSS_TABLE_LAYOUT_FIXED
} css_table_layout_t;

typedef enum
{
  CSS_TEXT_ALIGN_LEFT,
  CSS_TEXT_ALIGN_RIGHT,
  CSS_TEXT_ALIGN_CENTER,
  CSS_TEXT_ALIGN_JUSTIFY
} css_text_align_t;

typedef enum
{
  CSS_TEXT_DECORATION_NONE,
  CSS_TEXT_DECORATION_UNDERLINE,
  CSS_TEXT_DECORATION_OVERLINE,
  CSS_TEXT_DECORATION_LINE_THROUGH
} css_text_decoration_t;

typedef enum
{
  CSS_TEXT_TRANSFORM_NONE,
  CSS_TEXT_TRANSFORM_CAPITALIZE,
  CSS_TEXT_TRANSFORM_LOWERCASE,
  CSS_TEXT_TRANSFORM_UPPERCASE
} css_text_transform_t;

typedef enum
{
  CSS_UNICODE_BIDI_NORMAL,
  CSS_UNICODE_BIDI_EMBED,
  CSS_UNICODE_BIDI_OVERRIDE
} css_unicode_bidi_t;

typedef enum
{
  CSS_WHITE_SPACE_NORMAL,
  CSS_WHITE_SPACE_NOWRAP,
  CSS_WHITE_SPACE_PRE,
  CSS_WHITE_SPACE_PRE_LINE,
  CSS_WHITE_SPACE_PRE_WRAP
} css_white_space_t;

typedef struct css_color_s		/* sRGBA color */
{
  float			red;		/* Red, 0.0 to 1.0 */
  float			green;		/* Green, 0.0 to 1.0 */
  float			blue;		/* Blue, 0.0 to 1.0 */
  float			alpha;		/* Alpha, 0.0 (transparent) to 1.0 (opaque) */
} css_color_t;

typedef struct css_border_props_s	/* CSS border properties */
{
  css_color_t		color;		/* Border color */
  css_border_style_t	style;		/* Border style */
  float			width;		/* Border width */
} css_border_props_t;

typedef struct css_border_s		/* All CSS border properties */
{
  css_border_props_t	left;
  css_border_props_t	top;
  css_border_props_t	right;
  css_border_props_t	bottom;
} css_border_t;

typedef struct css_rect_s		/* Rectangle */
{
  float			left;		/* Left offset */
  float			top;		/* Top offset */
  float			right;		/* Right offset */
  float			bottom;		/* Bottom offset */
} css_rect_t;

typedef struct css_point_s		/* Point/coordinate */
{
  float			left;		/* Horizontal position */
  float			top;		/* Vertical position */
} css_point_t;

typedef struct css_size_s		/* Point/coordinate */
{
  float			width;		/* Width */
  float			height;		/* Height */
} css_size_t;

typedef struct css_box_s		/* CSS box properties */
{
  css_rect_t		bounds;		/* Computed bounds */
  css_size_t		size;		/* Computed size */
  css_rect_t		clip;		/* Clip bounds */
  css_size_t		max_size;
  css_size_t		min_size;
  css_background_attachment_t
			background_attachment;
  css_color_t		background_color;
  char			*background_image;
  css_point_t		background_position;
  css_background_repeat_t
			background_repeat;
  css_border_collapse_t	border_collapse;
  css_color_t		border_color;
  css_size_t		border_spacing;
  css_border_t		border;
  css_break_t		break_after;
  css_break_t		break_before;
  css_break_t		break_inside;
  css_float_t		float_value;
  char			*list_style_image;
  css_list_style_position_t
			list_style_position;
  css_rect_t		margin;
  int			orphans;
  css_overflow_t	overflow;
  css_rect_t		padding;
  int			widows;
  int			z_index;
} css_box_t;

typedef struct css_media_s		/* CSS media properties */
{
  css_rect_t		margin;
  css_size_t		size;
} css_media_t;

typedef struct css_table_s		/* CSS table properties */
{
  css_caption_side_t	caption_side;
  css_empty_cells_t	empty_cells;
  css_table_layout_t	table_layout;

} css_table_t;

typedef struct css_text_s		/* CSS text properties */
{
  css_direction_t	direction;
  char			*font_family;
  float			font_size;
  css_font_style_t	font_style;
  css_font_variant_t	font_variant;
  css_font_weight_t	font_weight;
  float			letter_spacing;
  float			line_height;
  char			*quotes[4];
  css_text_align_t	text_align;
  css_text_decoration_t	text_decoration;
  float			text_indent;
  css_text_transform_t	text_transform;
  css_unicode_bidi_t	unicode_bidi;
  css_white_space_t	white_space;
  float			word_spacing;
} css_text_t;

typedef struct _css_s css_t;		/* CSS data */


/*
 * Functions...
 */

extern void	cssDelete(css_t *css);
extern css_t	*cssNew(void);
extern int	cssImport(css_t *css, const char *url, FILE *fp, const char *s);
extern void	cssSetErrorCallback(css_t *css, htmlcss_error_cb_t cb, void *ctx);
extern void	cssSetURLCallback(css_t *css, htmlcss_url_cb_t cb, void *ctx);
extern int	cssSetMedia(css_t *css, const char *media, float width, float height);

extern int	cssComputeBox(css_t *css, html_node_t *node, css_box_t *box);
extern int	cssComputeDisplay(css_t *css, html_node_t *node, css_display_t *display);
extern int	cssComputeMedia(css_t *css, html_node_t *node, css_media_t *media);
extern int	cssComputeTable(css_t *css, html_node_t *node, css_table_t *table);
extern int	cssComputeText(css_t *css, html_node_t *node, css_text_t *text);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_CSS_H */
