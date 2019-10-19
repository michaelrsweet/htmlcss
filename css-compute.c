/*
 * CSS computation functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#  include "css-private.h"
#  include "font-private.h"
#  include "image.h"
#  include <ctype.h>


/*
 * Local types...
 */

typedef struct _hc_attrmap_s		/* HTML attribute to CSS mapping */
{
  hc_element_t	element;		/* HTML element */
  const char	*attr_name,		/* HTML attribute name */
		*prop_name,		/* CSS property name */
		*prop_value;		/* CSS property value (if any) */
} _hc_attrmap_t;


typedef struct _hc_css_match_s		/* Matching rule set */
{
  int		score;			/* Score */
  int		order;			/* Order in collection */
  _hc_rule_t	*rule;			/* Rule set */
} _hc_css_match_t;


/*
 * Local functions...
 */

static int		hc_compare_matches(_hc_css_match_t *a, _hc_css_match_t *b);
static const hc_dict_t	*hc_create_props(hc_node_t *node, hc_compute_t compute);
static int		hc_get_color(const char *value, hc_color_t *color);
static float		hc_get_length(const char *value, float max_value, hc_css_t *css, hc_text_t *text);
static int		hc_match_node(hc_node_t *node, _hc_css_sel_t *sel, const char *pseudo_class);
static int		hc_match_rule(hc_node_t *node, _hc_rule_t *rule, const char *pseudo_class);


/*
 * 'hcNodeComputeCSSBox()' - Compute the box properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSBox(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_box_t     *box)			/* O - Box properties */
{
  int			i;		/* Looping var */
  const char		*value;		/* Property value */
  hc_pool_t		*pool = node->value.element.html->pool;
					/* Memory pool */
  hc_css_t		*css = node->value.element.html->css;
					/* Stylesheet */
  const hc_dict_t	*props = hcNodeComputeCSSProperties(node, compute);
					/* Properties */
  hc_text_t		text;		/* Text font properties */
  const char		*bg_pos_size[4] =
  {					/* Background position/size values (late binding) */
    NULL, NULL,				/* X, Y */
    NULL, NULL				/* WIDTH, HEIGHT */
  };
  static const char * const boxes[] =
  {					/* background-clip/origin: values */
    "border-box",
    "padding-box",
    "content-box"
  };
  static const char * const repeats[] =
  {					/* background-repeat: values */
    "no-repeat",
    "repeat",
    "repeat-x",
    "repeat-y"
  };


  memset(box, 0, sizeof(hc_box_t));

  _hcNodeComputeCSSTextFont(node, props, &text);

 /*
  * Size constraint values...
  */

  if ((value = hcDictGetKeyValue(props, "min-width")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "min-height")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "max-width")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "max-height")) != NULL)
  {
  }

 /*
  * Background values (just a single background image is currently supported)
  */

  if ((value = hcDictGetKeyValue(props, "background")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		first_box = 1,		/* First box value? */
		pos_size = 0;		/* X/Y position/size */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (!strcmp(current, "scroll"))
      {
	box->background_attachment = HC_BACKGROUND_ATTACHMENT_SCROLL;
      }
      else if (!strcmp(current, "fixed"))
      {
	box->background_attachment = HC_BACKGROUND_ATTACHMENT_FIXED;
      }
      else if (!strncmp(current, "url(", 4))
      {
        char	url[1024];		/* URL string */

        if (sscanf(current, "url(%1023s)", url) == 1)
          box->background_image = hcPoolGetString(pool, url);
      }
      else if (!strcmp(current, "/"))
      {
        pos_size = 2;
      }
      else if (strchr("0123456789-.", *current))
      {
        if (pos_size < 4)
          bg_pos_size[pos_size] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if (!strcmp(current, "auto"))
      {
        if (pos_size < 2)
          pos_size = 2;

        if (pos_size == 2)
          bg_pos_size[2] = bg_pos_size[3] = hcPoolGetString(pool, current);
        else
          bg_pos_size[3] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if (!strcmp(current, "bottom") || !strcmp(current, "top"))
      {
        bg_pos_size[1] = hcPoolGetString(pool, current);
      }
      else if (!strcmp(current, "center"))
      {
        if (pos_size == 0)
          bg_pos_size[0] = bg_pos_size[1] = hcPoolGetString(pool, current);
        else if (pos_size == 1)
          bg_pos_size[1] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if ((!strcmp(current, "contain") || !strcmp(current, "cover")) && pos_size == 2)
      {
        bg_pos_size[2] = bg_pos_size[3] = hcPoolGetString(pool, current);
        pos_size       = 4;
      }
      else if (!strcmp(current, "left") || !strcmp(current, "right"))
      {
        bg_pos_size[0] = hcPoolGetString(pool, current);
      }
      else if (!hc_get_color(current, &box->background_color))
      {
	for (i = 0; i < (int)(sizeof(boxes) / sizeof(boxes[0])); i ++)
	{
	  if (!strcmp(current, boxes[i]))
	  {
	    if (first_box)
	    {
	      box->background_origin = (hc_background_box_t)i;
	      first_box              = 0;
	    }

	    box->background_clip = (hc_background_box_t)i;
	    break;
	  }
	}

	for (i = 0; i < (int)(sizeof(repeats) / sizeof(repeats[0])); i ++)
	{
	  if (!strcmp(current, repeats[i]))
	  {
	    box->background_repeat = (hc_background_repeat_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "background-attachment")) != NULL)
  {
    if (!strcmp(value, "scroll"))
      box->background_attachment = HC_BACKGROUND_ATTACHMENT_SCROLL;
    else if (!strcmp(value, "fixed"))
      box->background_attachment = HC_BACKGROUND_ATTACHMENT_FIXED;
  }

  if ((value = hcDictGetKeyValue(props, "background-clip")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(boxes) / sizeof(boxes[0])); i ++)
    {
      if (!strcmp(value, boxes[i]))
      {
        box->background_clip = (hc_background_box_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "background-color")) != NULL)
  {
    hc_get_color(value, &box->background_color);
  }

  if ((value = hcDictGetKeyValue(props, "background-image")) != NULL)
  {
    char	url[1024];		/* URL string */

    if (sscanf(value, "url(%1023s)", url) == 1)
      box->background_image = hcPoolGetString(node->value.element.html->pool, url);
  }

  if ((value = hcDictGetKeyValue(props, "background-origin")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(boxes) / sizeof(boxes[0])); i ++)
    {
      if (!strcmp(value, boxes[i]))
      {
        box->background_origin = (hc_background_box_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "background-position")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos_size = 0;		/* X/Y position/size */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789-.", *current))
      {
        if (pos_size < 2)
          bg_pos_size[pos_size] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if (!strcmp(current, "bottom") || !strcmp(current, "top"))
      {
        bg_pos_size[1] = hcPoolGetString(pool, current);
      }
      else if (!strcmp(current, "center"))
      {
        if (pos_size == 0)
          bg_pos_size[0] = bg_pos_size[1] = hcPoolGetString(pool, current);
        else if (pos_size == 1)
          bg_pos_size[1] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if (!strcmp(current, "left") || !strcmp(current, "right"))
      {
        bg_pos_size[0] = hcPoolGetString(pool, current);
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "background-repeat")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(repeats) / sizeof(repeats[0])); i ++)
    {
      if (!strcmp(value, repeats[i]))
      {
        box->background_repeat = (hc_background_repeat_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "background-size")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos_size = 2;		/* X/Y position/size */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789-.", *current))
      {
        if (pos_size < 4)
          bg_pos_size[pos_size] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if (!strcmp(current, "auto"))
      {
        if (pos_size == 2)
          bg_pos_size[2] = bg_pos_size[3] = hcPoolGetString(pool, current);
        else
          bg_pos_size[3] = hcPoolGetString(pool, current);

        pos_size ++;
      }
      else if ((!strcmp(current, "contain") || !strcmp(current, "cover")) && pos_size == 2)
      {
        bg_pos_size[2] = bg_pos_size[3] = hcPoolGetString(pool, current);
        pos_size       = 4;
      }
    }

    free(temp);
  }

  if (box->background_image)
  {
    hc_file_t *bg_file  = hcFileNewURL(pool, box->background_image, NULL /* TODO: Track dirname of parent document */);
    hc_image_t *bg_image = hcImageNew(pool, bg_file);
    hc_size_t bg_size = hcImageGetSize(bg_image);

    if (bg_pos_size[2] && bg_size.width > 0.0 && bg_size.height > 0.0)
    {
      if (!strcmp(bg_pos_size[2], "auto"))
      {
	box->background_size.width = bg_size.width;
      }
      if (!strcmp(bg_pos_size[2], "contain"))
      {
        float temp = css->media.size.width * bg_size.height / bg_size.width;

        if (temp < css->media.size.height)
          box->background_size.width = css->media.size.width;
        else
          box->background_size.width = css->media.size.height * bg_size.width / bg_size.height;
      }
      else if (!strcmp(bg_pos_size[2], "cover"))
      {
        float temp = css->media.size.width * bg_size.height / bg_size.width;

        if (temp >= css->media.size.height)
          box->background_size.width = css->media.size.width;
        else
          box->background_size.width = css->media.size.height * bg_size.width / bg_size.height;
      }
      else if (strchr("0123456789-.", bg_pos_size[2][0]))
      {
        box->background_size.width = hc_get_length(bg_pos_size[2], css->media.size.width, css, &text);

        if (!bg_pos_size[3])
          box->background_size.height = box->background_size.width * bg_size.height / bg_size.width;
      }
    }
    else
      box->background_size.width = bg_size.width;

    if (bg_pos_size[3] && bg_size.width > 0.0 && bg_size.height > 0.0)
    {
      if (!strcmp(bg_pos_size[3], "auto"))
      {
	box->background_size.height = bg_size.height;
      }
      if (!strcmp(bg_pos_size[3], "contain"))
      {
        float temp = css->media.size.height * bg_size.width / bg_size.height;

        if (temp < css->media.size.width)
          box->background_size.height = css->media.size.height;
        else
          box->background_size.height = css->media.size.width * bg_size.height / bg_size.width;
      }
      else if (!strcmp(bg_pos_size[3], "cover"))
      {
        float temp = css->media.size.height * bg_size.width / bg_size.height;

        if (temp > css->media.size.width)
          box->background_size.height = css->media.size.height;
        else
          box->background_size.height = css->media.size.width * bg_size.height / bg_size.width;
      }
      else if (strchr("0123456789-.", bg_pos_size[3][0]))
      {
        box->background_size.height = hc_get_length(bg_pos_size[3], css->media.size.height, css, &text);

        if (!bg_pos_size[2])
          box->background_size.width = box->background_size.height * bg_size.width / bg_size.height;
      }
    }
    else if (box->background_size.height == 0.0)
      box->background_size.height = bg_size.height;

    if (bg_pos_size[0])
    {
      if (!strcmp(bg_pos_size[0], "left"))
        box->background_position.left = 0.0f;
      else if (!strcmp(bg_pos_size[0], "center"))
        box->background_position.left = 0.5f * (css->media.size.width - bg_size.width);
      else if (!strcmp(bg_pos_size[0], "right"))
        box->background_position.left = css->media.size.width - bg_size.width;
      else if (strchr("0123456789-.", bg_pos_size[0][0]))
        box->background_position.left = hc_get_length(bg_pos_size[0], css->media.size.width - bg_size.width, css, &text);
    }

    if (bg_pos_size[1])
    {
      if (!strcmp(bg_pos_size[0], "top"))
        box->background_position.top = 0.0f;
      else if (!strcmp(bg_pos_size[0], "center"))
        box->background_position.top = 0.5f * (css->media.size.height - bg_size.height);
      else if (!strcmp(bg_pos_size[0], "bottom"))
        box->background_position.top = css->media.size.height - bg_size.height;
      else if (strchr("0123456789-.", bg_pos_size[0][0]))
        box->background_position.top = hc_get_length(bg_pos_size[0], css->media.size.height - bg_size.height, css, &text);
    }
  }

 /*
  * Border values...
  */

  if ((value = hcDictGetKeyValue(props, "border")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "border-collapse")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "border-radius")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "border-spacing")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "box-shadow")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "break-after")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "break-before")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "break-inside")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "float")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "list-style")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "list-style-image")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "list-style-position")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "margin")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "margin-bottom")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "margin-left")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "margin-right")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "margin-top")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "orphans")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "overflow")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "padding")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "padding-bottom")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "padding-left")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "padding-right")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "padding-top")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "widows")) != NULL)
  {
  }

  if ((value = hcDictGetKeyValue(props, "z-index")) != NULL)
  {
  }

  return (1);
}


/*
 * 'hcNodeComputeCSSContent()' - Compute the content: value for the given HTML node.
 *
 * The returned string must be freed using `free()`.
 */

char *					/* O - Content string or `NULL` if none/error */
hcNodeComputeCSSContent(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  (void)node;
  (void)compute;

  return (NULL);
}


/*
 * 'hcNodeComputeCSSDisplay()' - Compute the display property for the given HTML node.
 */

hc_display_t				/* O - 1 on success, 0 on failure */
hcNodeComputeCSSDisplay(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  int			i;		/* Looping var */
  const char		*value;		/* Property value */
  const hc_dict_t	*props = hcNodeComputeCSSProperties(node, compute);
					/* Properties */
  static const char	* const displays[] =
  {					/* display: values */
    "none",
    "block",
    "inline",
    "inline-block",
    "inline-table",
    "list-item",
    "table",
    "table-caption",
    "table-header-group",
    "table-footer-group",
    "table-row-group",
    "table-row",
    "table-column-group",
    "table-column",
    "table-cell"
  };


  if ((value = hcDictGetKeyValue(props, "display")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(displays) / sizeof(displays[0])); i ++)
      if (!strcmp(value, displays[i]))
        return ((hc_display_t)i);
  }

  return (HC_DISPLAY_NONE);
}


/*
 * 'hcNodeComputeCSSMedia()' - Compute the media properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSMedia(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_media_t   *media)		/* O - Media properties */
{
  (void)node;
  (void)compute;
  (void)media;

  return (0);
}


/*
 * 'hcNodeComputeCSSProperties()' - Compute the properties for the given node.
 *
 * The dictionary is stored in the stylesheet cache and must not be
 * deleted using the `hcDictDelete` function.
 */

const hc_dict_t *			/* O - Properties or `NULL` on error */
hcNodeComputeCSSProperties(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute)		/* I - Pseudo-class, if any */
{
  const hc_dict_t	*props;		/* Properties */


  if (!node || node->element < HC_ELEMENT_DOCTYPE)
    return (NULL);
  else if (compute == HC_COMPUTE_BASE && node->value.element.base_props)
    return (hcDictCopy(node->value.element.base_props));

  props = hc_create_props(node, compute);

  if (compute == HC_COMPUTE_BASE)
    node->value.element.base_props = props;

  return (props);
}


/*
 * 'hcNodeComputeCSSTable()' - Compute the table properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSTable(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_table_t   *table)		/* O - Table properties */
{
  (void)node;
  (void)compute;
  (void)table;

  return (0);
}


/*
 * 'hcNodeComputeCSSText()' - Compute the text properties for the given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
hcNodeComputeCSSText(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_text_t    *text)			/* O - Text properties */
{
  (void)node;
  (void)compute;
  (void)text;

  return (0);
}


/*
 * '_hcNodeComputeCSSTextFont()' - Compute the text font properties for the
 *                                 given HTML node.
 */

int					/* O - 1 on success, 0 on failure */
_hcNodeComputeCSSTextFont(
    hc_node_t       *node,		/* I - HTML node */
    const hc_dict_t *props,		/* I - Property dictionary */
    hc_text_t       *text)		/* O - Text properties */
{
  (void)node;
  (void)props;

  memset(text, 0, sizeof(hc_text_t));

  return (0);
}


/*
 * 'hc_compare_matches()' - Compare two matches...
 */

static int				/* O - Result of comparison */
hc_compare_matches(_hc_css_match_t *a,	/* I - First match */
                   _hc_css_match_t *b)	/* I - Second match */
{
  int	result = a->score - b->score;

  if (result == 0)
    result = b->order - a->order;

  return (result);
}


/*
 * 'hc_create_props()' - Create a property dictionary for a node.
 */

static const hc_dict_t *		/* O - Dictionary */
hc_create_props(hc_node_t    *node,	/* I - HTML node */
                hc_compute_t compute)	/* I - Pseudo-class, if any */
{
  hc_dict_t		*props;		/* Properties */
  size_t		i, j,		/* Looping vars */
			count,		/* Number of properties */
			num_matches = 0,/* Number of matches */
			alloc_matches = 0;
					/* Allocated matches */
  hc_css_t		*css;		/* Stylesheet */
  _hc_rulecol_t		*rulecol;	/* Current rule collection */
  _hc_rule_t		*rule;		/* Current rule */
  _hc_css_match_t	*matches = NULL,/* Matches */
			*match;		/* Current match */
  const char		*key,		/* Key from style */
			*value;		/* Value from style */
  hc_sha3_t		ctx;		/* SHA3 hashing context */
  hc_sha3_256_t		hash;		/* Hash for matches */
  static _hc_attrmap_t	attrs[] =	/* HTML attributes that map to CSS properties */
  {
    { HC_ELEMENT_WILDCARD, "align",       "text-align",       NULL },
    { HC_ELEMENT_BODY,     "background",  "background-image", NULL },
    { HC_ELEMENT_TABLE,    "background",  "background-image", NULL },
    { HC_ELEMENT_TD,       "background",  "background-image", NULL },
    { HC_ELEMENT_TH,       "background",  "background-image", NULL },
    { HC_ELEMENT_TR,       "background",  "background-image", NULL },
    { HC_ELEMENT_BODY,     "bgcolor",     "background-color", NULL },
    { HC_ELEMENT_TABLE,    "bgcolor",     "background-color", NULL },
    { HC_ELEMENT_TD,       "bgcolor",     "background-color", NULL },
    { HC_ELEMENT_TH,       "bgcolor",     "background-color", NULL },
    { HC_ELEMENT_TR,       "bgcolor",     "background-color", NULL },
    { HC_ELEMENT_TABLE,    "border",      "border",           NULL },
    { HC_ELEMENT_TABLE,    "cellpadding", "padding",          NULL },
    { HC_ELEMENT_TABLE,    "cellspacing", "margin",           NULL },
    { HC_ELEMENT_BR,       "clear",       "clear",            NULL },
    { HC_ELEMENT_FONT,     "color",       "color",            NULL },
    { HC_ELEMENT_WILDCARD, "dir",         "direction",        NULL },
    { HC_ELEMENT_FONT,     "face",        "font-family",      NULL },
    { HC_ELEMENT_IMG,      "height",      "height",           NULL },
    { HC_ELEMENT_TABLE,    "height",      "height",           NULL },
    { HC_ELEMENT_TD,       "height",      "height",           NULL },
    { HC_ELEMENT_TH,       "height",      "height",           NULL },
    { HC_ELEMENT_TR,       "height",      "height",           NULL },
    { HC_ELEMENT_IMG,      "hspace",      "padding-left",     NULL },
    { HC_ELEMENT_IMG,      "hspace",      "padding-right",    NULL },
    { HC_ELEMENT_HR,       "noshade",     "border-style",     "solid" },
    { HC_ELEMENT_TD,       "nowrap",      "white-space",      "no-wrap" },
    { HC_ELEMENT_TH,       "nowrap",      "white-space",      "no-wrap" },
    { HC_ELEMENT_BODY,     "text",        "color",            NULL },
    { HC_ELEMENT_LI,       "type",        "list-item-style",  NULL },
    { HC_ELEMENT_OL,       "type",        "list-item-style",  NULL },
    { HC_ELEMENT_UL,       "type",        "list-item-style",  NULL },
    { HC_ELEMENT_COL,      "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_COLGROUP, "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_IMG,      "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_TBODY,    "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_TD,       "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_TFOOT,    "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_TH,       "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_THEAD,    "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_TR,       "valign",      "vertical-align",   NULL },
    { HC_ELEMENT_IMG,      "vspace",      "padding-bottom",   NULL },
    { HC_ELEMENT_IMG,      "vspace",      "padding-top",      NULL },
    { HC_ELEMENT_COL,      "width",       "width",            NULL },
    { HC_ELEMENT_COLGROUP, "width",       "width",            NULL },
    { HC_ELEMENT_IMG,      "width",       "width",            NULL },
    { HC_ELEMENT_PRE,      "width",       "width",            NULL },
    { HC_ELEMENT_TABLE,    "width",       "width",            NULL },
    { HC_ELEMENT_TD,       "width",       "width",            NULL },
    { HC_ELEMENT_TH,       "width",       "width",            NULL }
  };
  static const char * const pseudo_classes[] =
  {					/* Pseudo-classes for each enum */
    NULL,
    "before"
    "after",
    "first-line",
    "first-letter"
  };


 /*
  * Collect all of the matching properties...
  */

  css = node->value.element.html->css;

  for (i = 0, rulecol = css->rules + HC_ELEMENT_WILDCARD; i < rulecol->num_rules; i ++)
  {
    int score = hc_match_rule(node, rulecol->rules[i], pseudo_classes[compute]);
					/* Score for current rule */

    if (score >= 0)
    {
      if (num_matches >= alloc_matches)
      {
        if ((match = realloc(matches, (alloc_matches + 16) * sizeof(_hc_css_match_t))) == NULL)
	{
	  free(matches);
	  return (NULL);
	}

        matches       = match;
        alloc_matches += 16;
      }

      match = matches + num_matches;
      num_matches ++;

      match->score = score;
      match->order = (int)num_matches;
      match->rule  = rulecol->rules[i];
    }
  }

  for (i = 0, rulecol = css->rules + node->element; i < rulecol->num_rules; i ++)
  {
    int score = hc_match_rule(node, rulecol->rules[i], pseudo_classes[compute]);
					/* Score for current rule */

    if (score >= 0)
    {
      if (num_matches >= alloc_matches)
      {
        if ((match = realloc(matches, (alloc_matches + 16) * sizeof(_hc_css_match_t))) == NULL)
	{
	  free(matches);
	  return (NULL);
	}

        matches       = match;
        alloc_matches += 16;
      }

      match = matches + num_matches;
      num_matches ++;

      match->score = score;
      match->order = (int)num_matches;
      match->rule  = rulecol->rules[i];
    }
  }

 /*
  * If we have no matches, return NULL...
  */

  if (num_matches == 0)
    return (NULL);

 /*
  * Sort matches...
  */

  qsort(matches, num_matches, sizeof(_hc_css_match_t), (_hc_compare_func_t)hc_compare_matches);

 /*
  * Build properties from attributes...
  */

  props = hcDictNew(css->pool);

  for (i = 0; i < (sizeof(attrs) / sizeof(attrs[0])); i ++)
  {
    if (attrs[i].element != HC_ELEMENT_WILDCARD && node->element != attrs[i].element)
      continue;

    if ((value = hcNodeAttrGetNameValue(node, attrs[i].attr_name)) != NULL)
    {
      if (attrs[i].prop_value)
        hcDictSetKeyValue(props, attrs[i].prop_name, attrs[i].prop_value);
      else
        hcDictSetKeyValue(props, attrs[i].prop_name, value);
    }
  }

  if ((value = hcNodeAttrGetNameValue(node, "style")) != NULL)
    _hcCSSImportString(css, props, value);

 /*
  * Hash the match to see if we have already calculated this set of
  * properties...
  */

  hcSHA3Init(&ctx);
  for (i = num_matches, match = matches; i > 0; i --, match ++)
    hcSHA3Update(&ctx, match->rule->hash, sizeof(match->rule->hash));
  for (i = 0, count = hcDictGetCount(props); i < count; i ++)
  {
    value = hcDictGetIndexKeyValue(props, i, &key);
    hcSHA3Update(&ctx, key, strlen(key));
    hcSHA3Update(&ctx, ":", 1);
    hcSHA3Update(&ctx, value, strlen(value));
    hcSHA3Update(&ctx, ";", 1);
  }
  hcSHA3Final(&ctx, hash, sizeof(hash));

  if ((rule = _hcRuleColFindHash(&css->all_rules, hash)) != NULL)
  {
    hcDictDelete(props);
    return (rule->props);
  }

 /*
  * No match, so synthesize the properties and add it...
  */

  for (i = num_matches, match = matches; i > 0; i --, match ++)
  {
    hc_dict_t *dict = match->rule->props;
					/* Dictionary for this match */

    for (j = 0, count = hcDictGetCount(dict); j < count; j ++)
    {
      value = hcDictGetIndexKeyValue(dict, j, &key);

      hcDictSetKeyValue(props, key, value);
    }
  }

 /*
  * Add a rule with this new hash...
  */

  rule = _hcRuleNew(css, hash, NULL, props);
  _hcRuleColAdd(css, &css->all_rules, rule);

  return (props);
}


/*
 * 'hc_get_color()' - Get the color for a named color.
 */

static int				/* O - 1 on success, 0 on failure */
hc_get_color(const char *value,		/* I - Color string */
             hc_color_t *color)		/* O - Color values */
{
  int		i;			/* Looping var */
  char		*ptr;			/* Pointer into value */
  hc_color_t	rgba;			/* Color values */
  static struct
  {
    const char	*name;			/* Color name */
    hc_color_t	rgba;			/* Color values */
  }		colors[] =
  {
    { "black",		{ 0.0f, 0.0f, 0.0f, 1.0f } },
    { "transparent",	{ 0.0f, 0.0f, 0.0f, 0.0f } },
    { "aqua",		{ 0.0f, 1.0f, 1.0f, 1.0f } },
    { "blue",		{ 0.0f, 0.0f, 1.0f, 1.0f } },
    { "fuschia",	{ 1.0f, 0.0f, 1.0f, 1.0f } },
    { "gray",		{ 0.5f, 0.5f, 0.5f, 1.0f } },
    { "green",		{ 0.0f, 0.5f, 0.0f, 1.0f } },
    { "lime",		{ 0.0f, 1.0f, 0.0f, 1.0f } },
    { "maroon",		{ 0.5f, 0.0f, 0.0f, 1.0f } },
    { "navy",		{ 0.0f, 0.0f, 0.5f, 1.0f } },
    { "olive",		{ 0.5f, 0.5f, 0.0f, 1.0f } },
    { "orange",		{ 1.0f, 0.65f, 0.0f, 1.0f } },
    { "purple",		{ 0.5f, 0.0f, 0.5f, 1.0f } },
    { "red",		{ 1.0f, 0.0f, 0.0f, 1.0f } },
    { "silver",		{ 0.75f, 0.75f, 0.75f, 1.0f } },
    { "teal",		{ 0.0f, 0.5f, 0.5f, 1.0f } },
    { "white",		{ 1.0f, 1.0f, 1.0f, 1.0f } },
    { "yellow",		{ 1.0f, 1.0f, 0.0f, 1.0f } }
  };


  if (!strncmp(value, "rgb(", 4))
  {
    rgba.red = (float)strtod(value + 4, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.red /= 100.0f;
    }
    else
    {
      rgba.red /= 255.0f;
    }
    if (*ptr != ',')
      return (0);

    rgba.green = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.green /= 100.0f;
    }
    else
    {
      rgba.green /= 255.0f;
    }
    if (*ptr != ',')
      return (0);

    rgba.blue = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.blue /= 100.0f;
    }
    else
    {
      rgba.blue /= 255.0f;
    }
    if (*ptr != ')')
      return (0);

    rgba.alpha = 1.0f;
    *color     = rgba;

    return (1);
  }
  else if (!strncmp(value, "rgba(", 5))
  {
    rgba.red = (float)strtod(value + 5, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.red /= 100.0f;
    }
    else
    {
      rgba.red /= 255.0f;
    }
    if (*ptr != ',')
      return (0);

    rgba.green = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.green /= 100.0f;
    }
    else
    {
      rgba.green /= 255.0f;
    }
    if (*ptr != ',')
      return (0);

    rgba.blue = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.blue /= 100.0f;
    }
    else
    {
      rgba.blue /= 255.0f;
    }
    if (*ptr != ',')
      return (0);
    rgba.alpha = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.alpha /= 100.0f;
    }
    if (*ptr != ')')
      return (0);

    *color = rgba;

    return (1);
  }
  else if (*value == '#')
  {
    size_t	len = strlen(value);

    if (len == 4 && (i = (int)strtol(value + 1, &ptr, 16)) >= 0 && !*ptr)
    {
      color->red   = ((i >> 8) & 15) / 15.0f;
      color->green = ((i >> 4) & 15) / 15.0f;
      color->blue  = (i & 15) / 15.0f;
      color->alpha = 1.0f;

      return (1);
    }
    else if (len == 7 && (i = (int)strtol(value + 1, &ptr, 16)) >= 0 && !*ptr)
    {
      color->red   = ((i >> 16) & 255) / 255.0f;
      color->green = ((i >> 8) & 255) / 255.0f;
      color->blue  = (i & 255) / 255.0f;
      color->alpha = 1.0f;

      return (1);
    }
    else
      return (0);
  }
  else
  {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
    {
      if (!strcmp(value, colors[i].name))
      {
        *color = colors[i].rgba;
        return (1);
      }
    }
  }

  return (0);
}


/*
 * 'hc_get_length()' - Get a length/measurement value.
 */

static float				/* O - Value in points or 0.0 on error */
hc_get_length(const char *value,	/* I - Value string */
              float      max_value,	/* I - Maximum value for percentages */
              hc_css_t   *css,		/* I - Stylesheet */
              hc_text_t  *text)		/* I - Text properties */
{
  char		*ptr;			/* Pointer to units after value */
  double	temp = strtod(value, &ptr);
					/* Interim value */

  if (ptr)
  {
    if (!strcmp(ptr, "%"))
      temp *= 0.01 * max_value;
    else if (!strcmp(ptr, "ch"))
    {
      hc_rect_t	extents;		/* Font extents */

      hcFontComputeExtents(text->font, text->font_size, "0", &extents);
      temp *= extents.right;
    }
    else if (!strcmp(ptr, "cm"))
      temp *= 72.0 / 2.54;
    else if (!strcmp(ptr, "em"))
      temp *= text->font_size;
    else if (!strcmp(ptr, "ex"))
      temp *= text->font_size * text->font->x_height / text->font->units;
    else if (!strcmp(ptr, "in"))
      temp *= 72.0;
    else if (!strcmp(ptr, "mm"))
      temp *= 72.0 / 25.4;
    else if (!strcmp(ptr, "pc"))
      temp *= 72.0 / 6.0;
    else if (!strcmp(ptr, "px"))
      temp *= 72.0 / 96.0;
    else if (!strcmp(ptr, "Q"))	/* Quarter-millimeters */
      temp *= 72.0 / 25.4 / 4.0;
    else if (!strcmp(ptr, "vh"))
      temp *= 0.01 * css->media.size.height;
    else if (!strcmp(ptr, "vmax"))
    {
      if (css->media.size.width > css->media.size.height)
        temp *= 0.01 * css->media.size.width;
      else
        temp *= 0.01 * css->media.size.height;
    }
    else if (!strcmp(ptr, "vmin"))
    {
      if (css->media.size.width < css->media.size.height)
        temp *= 0.01 * css->media.size.width;
      else
        temp *= 0.01 * css->media.size.height;
    }
    else if (!strcmp(ptr, "vw"))
      temp *= 0.01 * css->media.size.width;
    else if (strcmp(ptr, "pt"))
    {
      /* TODO: Show error */
      temp = 0.0;
    }
  }

  return ((float)temp);
}


/*
 * 'hc_match_node()' - Match a node to a selector...
 */

static int				/* O - Score */
hc_match_node(
    hc_node_t     *node,		/* I - HTML node */
    _hc_css_sel_t *sel,			/* I - CSS selector */
    const char    *pseudo_class)	/* I - Pseudo class, if any */
{
  int		score = 0;		/* Match score */
  size_t	i;			/* Looping var */
  _hc_css_selstmt_t *stmt;		/* Current statement */
  const char	*value;			/* Value */


  if (node->element != sel->element && sel->element != HC_ELEMENT_WILDCARD)
    return (-1);
  else if (sel->element != HC_ELEMENT_WILDCARD)
    score ++;

  for (i = sel->num_stmts, stmt = sel->stmts; i > 0; i--, stmt ++)
  {
    switch (stmt->match)
    {
      case _HC_MATCH_ATTR_EXIST :
          if (!hcNodeAttrGetNameValue(node, stmt->name))
            return (-1);

	  score += 100;
          break;
      case _HC_MATCH_ATTR_EQUALS :
          if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || strcmp(value, stmt->value))
            return (-1);

	  score += 100;
          break;
      case _HC_MATCH_ATTR_CONTAINS :
          if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || !strstr(value, stmt->value))
            return (-1);

	  score += 100;
          break;
      case _HC_MATCH_ATTR_BEGINS :
          if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || strncmp(value, stmt->value, strlen(stmt->value)))
            return (-1);

	  score += 100;
          break;
      case _HC_MATCH_ATTR_ENDS :
          {
            size_t	svaluelen,	/* Length of statement value */
			valuelen;	/* Length of attribute value */

	    if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || (svaluelen = strlen(stmt->value)) > (valuelen = strlen(value)) || strncmp(value + valuelen - svaluelen, stmt->value, svaluelen))
	      return (-1);

	    score += 100;
	  }
          break;
      case _HC_MATCH_ATTR_LANG :
          {
            size_t	svaluelen;	/* Length of statement value */

	    if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || strncmp(value, stmt->value, svaluelen = strlen(stmt->value)) || (svaluelen <= strlen(value) && value[svaluelen] != '-' && value[svaluelen]))
	      return (-1);

	    score += 100;
	  }
          break;
      case _HC_MATCH_ATTR_SPACE :
          {
            const char	*ptr;		/* Pointer to match */

	    if ((value = hcNodeAttrGetNameValue(node, stmt->name)) == NULL || (ptr = strstr(value, stmt->value)) == NULL)
	      return (-1);

            if ((ptr > value && !isspace(ptr[-1] & 255)) || !isspace(ptr[strlen(stmt->value)] & 255))
              return (-1);

	    score += 100;
          }
          break;
      case _HC_MATCH_CLASS :
          if ((value = hcNodeAttrGetNameValue(node, "class")) == NULL || strcmp(value, stmt->name))
            return (-1);

	  score += 100;
          break;
      case _HC_MATCH_ID :
          if ((value = hcNodeAttrGetNameValue(node, "id")) == NULL || strcmp(value, stmt->name))
            return (-1);

	  score += 10000;
          break;
      case _HC_MATCH_PSEUDO_CLASS :
          if (pseudo_class)
	  {
	    if (!strcmp(pseudo_class, stmt->name))
	      score += 100;
	    else
	      return (-1);
	  }
	  else
	  {
	   /*
	    * Test various pseudo-classes that match based on context.
	    *
	    * TODO: Add support for :first-of-type pseudo-class.
	    * TODO: Add support for :lang(l) pseudo-class.
	    * TODO: Add support for :last-of-type pseudo-class.
	    * TODO: Add support for :nth-last-child(n) pseudo-class.
	    * TODO: Add support for :nth-of-type(n) pseudo-class.
	    * TODO: Add support for :nth-last-of-type(n) pseudo-class.
	    * TODO: Add support for :only-of-type pseudo-class.
	    */

	    int node_matches = 0;	/* Does this node match? */

	    if (node->element == HC_ELEMENT_A && hcNodeAttrGetNameValue(node, "href") && !strcmp(stmt->name, "link"))
	    {
	      node_matches = 1;
	    }
	    else if (node->element == HC_ELEMENT_A && hcNodeAttrGetNameValue(node, "name") && !strcmp(stmt->name, "target"))
	    {
	      node_matches = 1;
	    }
	    else if (hcNodeAttrGetNameValue(node, "id") && !strcmp(stmt->name, "target"))
	    {
	      node_matches = 1;
	    }
	    else if (!node->prev_sibling && !strcmp(stmt->name, "first-child"))
	    {
	      node_matches = 1;
	    }
	    else if (!node->next_sibling && !strcmp(stmt->name, "last-child"))
	    {
	      node_matches = 1;
	    }
	    else if (!node->prev_sibling && !node->next_sibling && !strcmp(stmt->name, "only-child"))
	    {
	      node_matches = 1;
	    }
	    else if (node->element >= HC_ELEMENT_DOCTYPE && !node->value.element.first_child && !strcmp(stmt->name, "empty"))
	    {
	      node_matches = 1;
	    }
	    else if (node->element == HC_ELEMENT_HTML && !strcmp(stmt->name, "root"))
	    {
	      node_matches = 1;
	    }
	    else if (!strcmp(stmt->name, "nth-child"))
	    {
	     /*
	      * :nth-child(even)
	      * :nth-child(odd)
	      * :nth-child(NUMBER)
	      * :nth-child(MULTn)
	      * :nth-child(MULTn+OFFSET)
	      * :nth-child(MULTn-OFFSET)
	      */

              long	n = 1;		/* Child number */
              hc_node_t	*curnode;	/* Current node */

	     /*
	      * Calculate the child number...
	      */

              for (curnode = node; curnode->prev_sibling; curnode = curnode->prev_sibling)
                n ++;

             /*
              * Compare child number to value...
              */

              if (!strcmp(stmt->value, "even"))
              {
                node_matches = ((n & 1) == 0);
	      }
              else if (!strcmp(stmt->value, "odd"))
              {
                node_matches = ((n & 1) == 1);
	      }
	      else if (strchr("0123456789-+", stmt->value[0]))
	      {
                long	mult,		/* Multiplier */
			offset;		/* Offset */
	        char	*valptr;	/* Pointer into value */

                if ((mult = strtol(stmt->value, &valptr, 10)) == 0 && stmt->value[0] == '-' && stmt->value[1] == 'n')
                {
                  mult   = -1;
                  valptr = (char *)stmt->value + 1;
		}
		else if (mult == 0 && stmt->value[0] == '+' && stmt->value[1] == 'n')
		{
                  mult   = 1;
                  valptr = (char *)stmt->value + 1;
		}

                if (!valptr || !*valptr)
                {
                  node_matches = (mult == n);
                }
                else if (*valptr == 'n')
                {
                  offset = strtol(valptr, NULL, 10);
                  if (mult > 0)
                    node_matches = ((n - offset) % mult) == 0;
		  else
		    node_matches = (n == offset);
                }
	      }
	    }

            if (node_matches)
	      score += 100;
	    else
	      return (-1);
          }
	  break;
    }
  }

  return (score);
}


/*
 * 'hc_match_rule()' - Match a rule against the specified node...
 */

static int				/* O  - Number of matches */
hc_match_rule(hc_node_t  *node,		/* I  - HTML node */
              _hc_rule_t *rule,		/* I  - Rule */
              const char *pseudo_class)	/* I - Pseudo-class, if any */
{
  int		score,			/* Overall score */
		curscore;		/* Current score */
  hc_node_t	*curnode = node;	/* Current node */
  _hc_css_sel_t	*cursel = rule->sel;	/* Current selector */


  if ((score = hc_match_node(curnode, cursel, pseudo_class)) < 0)
    return (score);

  while (cursel->prev)
  {
    _hc_relation_t relation = cursel->relation;
					/* Relation to previous selector */

    cursel = cursel->prev;

    switch (relation)
    {
      case _HC_RELATION_CHILD :
          for (curnode = curnode->parent, curscore = 0; curnode; curnode = curnode->parent)
          {
            if ((curscore = hc_match_node(curnode, cursel, NULL)) < 0)
              return (-1);
	  }

          if (!curnode)
            return (-1);

          score += curscore;
          break;

      case _HC_RELATION_IMMED_CHILD :
          curnode = curnode->parent;

          if (!curnode || (curscore = hc_match_node(curnode, cursel, NULL)) < 0)
            return (-1);

          score += curscore;
          break;

      case _HC_RELATION_SIBLING :
          for (curnode = curnode->prev_sibling, curscore = 0; curnode; curnode = curnode->prev_sibling)
          {
            if ((curscore = hc_match_node(curnode, cursel, NULL)) < 0)
              return (-1);
	  }

          if (!curnode)
            return (-1);

          score += curscore;
          break;

      case _HC_RELATION_IMMED_SIBLING :
          curnode = curnode->prev_sibling;

          if (!curnode || (curscore = hc_match_node(curnode, cursel, NULL)) < 0)
            return (-1);

          score += curscore;
          break;
    }
  }

  return (score);
}

