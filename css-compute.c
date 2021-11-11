/*
 * CSS computation functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2021 by Michael R Sweet.
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
#  include <math.h>


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
static bool		hc_get_color(const char *value, hc_color_t *color);
static float		hc_get_length(const char *value, float max_value, float multiplier, hc_css_t *css, hc_text_t *text);
static int		hc_match_node(hc_node_t *node, _hc_css_sel_t *sel, const char *pseudo_class);
static int		hc_match_rule(hc_node_t *node, _hc_rule_t *rule, const char *pseudo_class);


/*
 * 'hcNodeComputeCSSBox()' - Compute the box properties for the given HTML node.
 */

bool					/* O - `true` on success, `false` on failure */
hcNodeComputeCSSBox(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_box_t     *box)			/* O - Box properties */
{
  int			i;		/* Looping var */
  const char		*value;		/* Property value */
  hc_css_t		*css;		/* Stylesheet */
  hc_pool_t		*pool;		/* Memory pool */
  const hc_dict_t	*props = hcNodeComputeCSSProperties(node, compute);
					/* Properties */
  hc_text_t		text;		/* Text font properties */
  hc_color_t		color;		/* Color value */
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
  static const char * const breaks[] =	/* break-xxx: and page-break-xxx: values */
  {
    "auto",
    "always",
    "avoid",
    "left",
    "right"
  };
  static const char * const repeats[] =
  {					/* background-repeat: values */
    "no-repeat",
    "repeat",
    "repeat-x",
    "repeat-y"
  };
  static const char * const image_repeats[] =
  {					/* border-image-repeat: values */
    "stretch",
    "repeat",
    "round",
    "space"
  };
  static const char * const styles[] =	/* border-style: values */
  {
    "hidden",
    "none",
    "dotted",
    "dashed",
    "solid",
    "double",
    "groove",
    "ridge",
    "inset",
    "outset"
  };
  static const char * const types[] =	/* list-style-type: values */
  {
    "disc",
    "circle",
    "square",
    "decimal",
    "decimal-leading-zero",
    "lower-roman",
    "upper-roman",
    "lower-greek",
    "upper-greek",
    "lower-latin",
    "upper-latin",
    "armenian",
    "georgian",
    "lower-alpha",
    "upper-alpha",
    "none"
  };


  if (!box)
    return (false);

  memset(box, 0, sizeof(hc_box_t));

  box->border.bottom.width = 1.0f;
  box->border.left.width   = 1.0f;
  box->border.right.width  = 1.0f;
  box->border.top.width    = 1.0f;

  box->orphans = 2;
  box->widows  = 2;

  if (!node)
    return (false);

  css  = node->value.element.html->css;
  pool = node->value.element.html->pool;

  _hcNodeComputeCSSTextFont(node, props, &text);

 /*
  * Size constraint values...
  */

#if 0
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
#endif /* 0 */

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
        box->background_size.width = hc_get_length(bg_pos_size[2], css->media.size.width, 72.0f / 96.0f, css, &text);

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
        box->background_size.height = hc_get_length(bg_pos_size[3], css->media.size.height, 72.0f / 96.0f, css, &text);

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
        box->background_position.left = hc_get_length(bg_pos_size[0], css->media.size.width - bg_size.width, 72.0f / 96.0f, css, &text);
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
        box->background_position.top = hc_get_length(bg_pos_size[0], css->media.size.height - bg_size.height, 72.0f / 96.0f, css, &text);
    }
  }

 /*
  * Border values...
  */

  if ((value = hcDictGetKeyValue(props, "border")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (hc_get_color(current, &color))
      {
	box->border.bottom.color = box->border.left.color = box->border.right.color = box->border.top.color = color;
      }
      else if (!strcmp(current, "thin"))
      {
	box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 0.5f;
      }
      else if (!strcmp(current, "medium"))
      {
	box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 1.0f;
      }
      else if (!strcmp(current, "thick"))
      {
	box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 2.0f;
      }
      else if (strchr("0123456789.", *current))
      {
	box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
      }
      else
      {
	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    box->border.bottom.style = box->border.left.style = box->border.right.style = box->border.top.style = (hc_border_style_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-bottom")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (hc_get_color(current, &color))
      {
	box->border.bottom.color = color;
      }
      else if (!strcmp(current, "thin"))
      {
	box->border.bottom.width = 0.5f;
      }
      else if (!strcmp(current, "medium"))
      {
	box->border.bottom.width = 1.0f;
      }
      else if (!strcmp(current, "thick"))
      {
	box->border.bottom.width = 2.0f;
      }
      else if (strchr("0123456789.", *current))
      {
	box->border.bottom.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
      }
      else
      {
	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    box->border.bottom.style = (hc_border_style_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-left")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (hc_get_color(current, &color))
      {
	box->border.left.color = color;
      }
      else if (!strcmp(current, "thin"))
      {
	box->border.left.width = 0.5f;
      }
      else if (!strcmp(current, "medium"))
      {
	box->border.left.width = 1.0f;
      }
      else if (!strcmp(current, "thick"))
      {
	box->border.left.width = 2.0f;
      }
      else if (strchr("0123456789.", *current))
      {
	box->border.left.width = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);
      }
      else
      {
	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    box->border.left.style = (hc_border_style_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-right")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (hc_get_color(current, &color))
      {
	box->border.right.color = color;
      }
      else if (!strcmp(current, "thin"))
      {
	box->border.right.width = 0.5f;
      }
      else if (!strcmp(current, "medium"))
      {
	box->border.right.width = 1.0f;
      }
      else if (!strcmp(current, "thick"))
      {
	box->border.right.width = 2.0f;
      }
      else if (strchr("0123456789.", *current))
      {
	box->border.right.width = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);
      }
      else
      {
	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    box->border.right.style = (hc_border_style_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-top")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (hc_get_color(current, &color))
      {
	box->border.top.color = color;
      }
      else if (!strcmp(current, "thin"))
      {
	box->border.top.width = 0.5f;
      }
      else if (!strcmp(current, "medium"))
      {
	box->border.top.width = 1.0f;
      }
      else if (!strcmp(current, "thick"))
      {
	box->border.top.width = 2.0f;
      }
      else if (strchr("0123456789.", *current))
      {
	box->border.top.width = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);
      }
      else
      {
	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    box->border.top.style = (hc_border_style_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-color")) != NULL)
  {
    if (hc_get_color(value, &box->border.bottom.color))
      box->border.left.color = box->border.right.color = box->border.top.color = box->border.bottom.color;
  }

  if ((value = hcDictGetKeyValue(props, "border-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        box->border.bottom.style = box->border.left.style = box->border.right.style = box->border.top.style = (hc_border_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-width")) != NULL)
  {
    if (!strcmp(value, "thin"))
      box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 0.5f;
    else if (!strcmp(value, "medium"))
      box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 1.0f;
    else if (!strcmp(value, "thick"))
      box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = 2.0f;
    else if (strchr("0123456789.", *value))
      box->border.bottom.width = box->border.left.width = box->border.right.width = box->border.top.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "border-bottom-color")) != NULL)
    hc_get_color(value, &box->border.bottom.color);

  if ((value = hcDictGetKeyValue(props, "border-left-color")) != NULL)
    hc_get_color(value, &box->border.left.color);

  if ((value = hcDictGetKeyValue(props, "border-right-color")) != NULL)
    hc_get_color(value, &box->border.right.color);

  if ((value = hcDictGetKeyValue(props, "border-top-color")) != NULL)
    hc_get_color(value, &box->border.top.color);

  if ((value = hcDictGetKeyValue(props, "border-bottom-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        box->border.bottom.style = (hc_border_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-left-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        box->border.left.style = (hc_border_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-right-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        box->border.right.style = (hc_border_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-top-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        box->border.top.style = (hc_border_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-bottom-width")) != NULL)
  {
    if (!strcmp(value, "thin"))
      box->border.bottom.width = 0.5f;
    else if (!strcmp(value, "medium"))
      box->border.bottom.width = 1.0f;
    else if (!strcmp(value, "thick"))
      box->border.bottom.width = 2.0f;
    else if (strchr("0123456789.", *value))
      box->border.bottom.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "border-left-width")) != NULL)
  {
    if (!strcmp(value, "thin"))
      box->border.left.width = 0.5f;
    else if (!strcmp(value, "medium"))
      box->border.left.width = 1.0f;
    else if (!strcmp(value, "thick"))
      box->border.left.width = 2.0f;
    else if (strchr("0123456789.", *value))
      box->border.left.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "border-right-width")) != NULL)
  {
    if (!strcmp(value, "thin"))
      box->border.right.width = 0.5f;
    else if (!strcmp(value, "medium"))
      box->border.right.width = 1.0f;
    else if (!strcmp(value, "thick"))
      box->border.right.width = 2.0f;
    else if (strchr("0123456789.", *value))
      box->border.right.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "border-top-width")) != NULL)
  {
    if (!strcmp(value, "thin"))
      box->border.top.width = 0.5f;
    else if (!strcmp(value, "medium"))
      box->border.top.width = 1.0f;
    else if (!strcmp(value, "thick"))
      box->border.top.width = 2.0f;
    else if (strchr("0123456789.", *value))
      box->border.top.width = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "border-image")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Current position */
    float	length;			/* Width/outset/slice value */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 14; current = strsep(&next, " \t"))
    {
      if (!strncmp(current, "url(", 4))
      {
	char	url[1024];		/* URL string */

	if (sscanf(current, "url(%1023s)", url) == 1)
	  box->border_image = hcPoolGetString(node->value.element.html->pool, url);
      }
      else if (strchr("0123456789.", *current))
      {
	length = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
	  case 0 :
	      box->border_image_slice.top    = length;
	      box->border_image_slice.right  = length;
	      box->border_image_slice.bottom = length;
	      box->border_image_slice.left   = length;
	      break;
	  case 1 :
	      box->border_image_slice.right = length;
	      box->border_image_slice.left  = length;
	      break;
	  case 2 :
	      box->border_image_slice.bottom = length;
	      break;
	  case 3 :
	      box->border_image_slice.left = length;
	      break;

	  case 4 :
	      box->border_image_width.top    = length;
	      box->border_image_width.right  = length;
	      box->border_image_width.bottom = length;
	      box->border_image_width.left   = length;
	      break;
	  case 5 :
	      box->border_image_width.right = length;
	      box->border_image_width.left  = length;
	      break;
	  case 6 :
	      box->border_image_width.bottom = length;
	      break;
	  case 7 :
	      box->border_image_width.left = length;
	      break;

	  case 8 :
	      box->border_image_outset.top    = length;
	      box->border_image_outset.right  = length;
	      box->border_image_outset.bottom = length;
	      box->border_image_outset.left   = length;
	      break;
	  case 9 :
	      box->border_image_outset.right = length;
	      box->border_image_outset.left  = length;
	      break;
	  case 10 :
	      box->border_image_outset.bottom = length;
	      break;
	  case 11 :
	      box->border_image_outset.left = length;
	      break;
        }

        pos ++;
      }
      else
      {
	for (i = 0; i < (int)(sizeof(image_repeats) / sizeof(image_repeats[0])); i ++)
	{
	  if (!strcmp(current, image_repeats[i]))
	  {
	    if (pos <= 12)
	    {
	      box->border_image_repeat[0] = (hc_border_image_repeat_t)i;
	      pos = 12;
	    }

            box->border_image_repeat[1] = (hc_border_image_repeat_t)i;
            pos ++;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-image-outset")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Current position */
    float	length;			/* Outset value */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 4; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
	length = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
	  case 0 :
	      box->border_image_outset.top    = length;
	      box->border_image_outset.right  = length;
	      box->border_image_outset.bottom = length;
	      box->border_image_outset.left   = length;
	      break;
	  case 1 :
	      box->border_image_outset.right = length;
	      box->border_image_outset.left  = length;
	      break;
	  case 2 :
	      box->border_image_outset.bottom = length;
	      break;
	  case 3 :
	      box->border_image_outset.left = length;
	      break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-image-repeat")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Current position */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      for (i = 0; i < (int)(sizeof(image_repeats) / sizeof(image_repeats[0])); i ++)
      {
	if (!strcmp(current, image_repeats[i]))
	{
	  if (pos == 0)
	    box->border_image_repeat[0] = (hc_border_image_repeat_t)i;

	  box->border_image_repeat[1] = (hc_border_image_repeat_t)i;
	  pos ++;
	  break;
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-image-slice")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Current position */
    float	length;			/* Slice value */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 4; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
	length = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
	  case 0 :
	      box->border_image_slice.top    = length;
	      box->border_image_slice.right  = length;
	      box->border_image_slice.bottom = length;
	      box->border_image_slice.left   = length;
	      break;
	  case 1 :
	      box->border_image_slice.right = length;
	      box->border_image_slice.left  = length;
	      break;
	  case 2 :
	      box->border_image_slice.bottom = length;
	      break;
	  case 3 :
	      box->border_image_slice.left = length;
	      break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-image-source")) != NULL)
  {
    if (!strncmp(value, "url(", 4))
    {
      char	url[1024];		/* URL string */

      if (sscanf(value, "url(%1023s)", url) == 1)
	box->border_image = hcPoolGetString(node->value.element.html->pool, url);
    }
  }

  if ((value = hcDictGetKeyValue(props, "border-image-width")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Current position */
    float	length;			/* Width value */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 4; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
	length = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
	  case 0 :
	      box->border_image_width.top    = length;
	      box->border_image_width.right  = length;
	      box->border_image_width.bottom = length;
	      box->border_image_width.left   = length;
	      break;
	  case 1 :
	      box->border_image_width.right = length;
	      box->border_image_width.left  = length;
	      break;
	  case 2 :
	      box->border_image_width.bottom = length;
	      break;
	  case 3 :
	      box->border_image_width.left = length;
	      break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-radius")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	radius;			/* Radius length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 8; current = strsep(&next, " \t"))
    {
      if (!strcmp(current, "/"))
      {
        if (pos <= 4)
          pos = 4;
	else
	  break;
      }
      else if (strchr("0123456789.", *current))
      {
        radius = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_radius.top_left.width      = radius;
              box->border_radius.top_left.height     = radius;
              box->border_radius.top_right.width     = radius;
              box->border_radius.top_right.height    = radius;
              box->border_radius.bottom_right.width  = radius;
              box->border_radius.bottom_right.height = radius;
              box->border_radius.bottom_left.width   = radius;
              box->border_radius.bottom_left.height  = radius;
              break;
	  case 1 :
              box->border_radius.top_right.width     = radius;
              box->border_radius.top_right.height    = radius;
              box->border_radius.bottom_right.width  = radius;
              box->border_radius.bottom_right.height = radius;
	      break;
	  case 2 :
              box->border_radius.bottom_right.width  = radius;
              box->border_radius.bottom_right.height = radius;
	      break;
	  case 3 :
              box->border_radius.bottom_left.width  = radius;
              box->border_radius.bottom_left.height = radius;
	      break;

          case 4 :
              box->border_radius.top_left.height     = radius;
              box->border_radius.top_right.height    = radius;
              box->border_radius.bottom_right.height = radius;
              box->border_radius.bottom_left.height  = radius;
              break;
	  case 5 :
              box->border_radius.top_right.height    = radius;
              box->border_radius.bottom_right.height = radius;
	      break;
	  case 6 :
              box->border_radius.bottom_right.height = radius;
	      break;
	  case 7 :
              box->border_radius.bottom_left.height = radius;
	      break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-bottom-left-radius")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	radius;			/* Radius length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
        radius = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_radius.bottom_left.width  = radius;
              box->border_radius.bottom_left.height = radius;
              break;
          case 1 :
              box->border_radius.bottom_left.height = radius;
              break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-bottom-right-radius")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	radius;			/* Radius length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
        radius = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_radius.bottom_right.width  = radius;
              box->border_radius.bottom_right.height = radius;
              break;
          case 1 :
              box->border_radius.bottom_right.height = radius;
              break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-top-left-radius")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	radius;			/* Radius length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
        radius = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_radius.top_left.width  = radius;
              box->border_radius.top_left.height = radius;
              break;
          case 1 :
              box->border_radius.top_left.height = radius;
              break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-top-right-radius")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	radius;			/* Radius length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
        radius = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_radius.top_right.width  = radius;
              box->border_radius.top_right.height = radius;
              break;
          case 1 :
              box->border_radius.top_right.height = radius;
              break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "border-spacing")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	spacing;		/* Spacing length */

    for (next = temp, current = strsep(&next, " \t"); current && pos < 2; current = strsep(&next, " \t"))
    {
      if (strchr("0123456789.", *current))
      {
        spacing = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
          case 0 :
              box->border_spacing.width  = spacing;
              box->border_spacing.height = spacing;
              break;
          case 1 :
              box->border_spacing.height = spacing;
              break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "box-shadow")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		pos = 0;		/* Position */
    float	length;			/* Shadow offset/blur/spread length */
    hc_color_t	color;			/* Shadow color */

    box->box_shadow.color = text.color;

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (!strcmp(current, "inset"))
        box->box_shadow.inset = true;
      else if (hc_get_color(current, &color))
        box->box_shadow.color = color;
      else if (strchr("0123456789.", *current))
      {
        length = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);

        switch (pos)
        {
	  case 0 :
	      box->box_shadow.horizontal_offset = length;
	      box->box_shadow.vertical_offset   = length;
	      break;
	  case 1 :
	      box->box_shadow.vertical_offset = length;
	      break;
	  case 2 :
	      box->box_shadow.blur_radius = length;
	      break;
	  case 3 :
	      box->box_shadow.spread_distance = length;
	      break;
        }

        pos ++;
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "break-after")) == NULL)
    value = hcDictGetKeyValue(props, "page-break-after");
  if (value)
  {
    for (i = 0; i < (int)(sizeof(breaks) / sizeof(breaks[0])); i ++)
    {
      if (!strcmp(value, breaks[i]))
      {
        box->break_after = (hc_break_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "break-before")) == NULL)
    value = hcDictGetKeyValue(props, "page-break-before");
  if (value)
  {
    for (i = 0; i < (int)(sizeof(breaks) / sizeof(breaks[0])); i ++)
    {
      if (!strcmp(value, breaks[i]))
      {
        box->break_before = (hc_break_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "break-inside")) == NULL)
    value = hcDictGetKeyValue(props, "page-break-inside");
  if (value)
  {
    for (i = 0; i < (int)(sizeof(breaks) / sizeof(breaks[0])); i ++)
    {
      if (!strcmp(value, breaks[i]))
      {
        box->break_inside = (hc_break_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "float")) != NULL)
  {
    if (!strcmp(value, "left"))
      box->float_value = HC_FLOAT_LEFT;
    else if (!strcmp(value, "none"))
      box->float_value = HC_FLOAT_NONE;
    else if (!strcmp(value, "right"))
      box->float_value = HC_FLOAT_RIGHT;
  }

  if ((value = hcDictGetKeyValue(props, "list-style")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (!strncmp(current, "url(", 4))
      {
	char	url[1024];		/* URL value */

	if (sscanf(current, "url(%1023s)", url) == 1)
	  box->list_style_image = hcPoolGetString(pool, url);
      }
      else if (!strcmp(current, "inside"))
	box->list_style_position = HC_LIST_STYLE_POSITION_INSIDE;
      else if (!strcmp(current, "outside"))
	box->list_style_position = HC_LIST_STYLE_POSITION_OUTSIDE;
      else
      {
	for (i = 0; i < (int)(sizeof(types) / sizeof(types[0])); i ++)
	{
	  if (!strcmp(current, types[i]))
	  {
	    box->list_style_type = (hc_list_style_type_t)i;
	    break;
	  }
	}
      }
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "list-style-image")) != NULL)
  {
    if (!strncmp(value, "url(", 4))
    {
      char	url[1024];		/* URL value */

      if (sscanf(value, "url(%1023s)", url) == 1)
        box->list_style_image = hcPoolGetString(pool, url);
    }
  }

  if ((value = hcDictGetKeyValue(props, "list-style-position")) != NULL)
  {
    if (!strcmp(value, "inside"))
      box->list_style_position = HC_LIST_STYLE_POSITION_INSIDE;
    else if (!strcmp(value, "outside"))
      box->list_style_position = HC_LIST_STYLE_POSITION_OUTSIDE;
  }

  if ((value = hcDictGetKeyValue(props, "list-style-type")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(types) / sizeof(types[0])); i ++)
    {
      if (!strcmp(value, types[i]))
      {
        box->list_style_type = (hc_list_style_type_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "orphans")) != NULL)
  {
    if (!strcmp(value, "inherit"))
      value = hcDictGetKeyValue(node->parent->value.element.base_props, "widows");

    if (value && isdigit(*value & 255))
      box->orphans = atoi(value);
  }

  if ((value = hcDictGetKeyValue(props, "overflow")) != NULL)
  {
    static const char * const overflows[] =
    {
      "hidden",
      "visible",
      "scroll",
      "auto"
    };

    for (i = 0; i < (int)(sizeof(overflows) / sizeof(overflows[0])); i ++)
    {
      if (!strcmp(value, overflows[i]))
      {
        box->overflow = (hc_overflow_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "widows")) != NULL)
  {
    if (!strcmp(value, "inherit"))
      value = hcDictGetKeyValue(node->parent->value.element.base_props, "widows");

    if (value && isdigit(*value & 255))
      box->widows = atoi(value);
  }

  if ((value = hcDictGetKeyValue(props, "z-index")) != NULL)
  {
    if (!strcmp(value, "auto"))
      box->z_index = 0;
    else if (isdigit(*value & 255))
      box->z_index = atoi(value);
  }

  if ((value = hcDictGetKeyValue(props, "margin")) != NULL)
  {
    char	*temp = strdup(value),	/* Copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		num_values = 0;		/* Number of values */
    float	values[4];		/* Values */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (num_values < 4)
      {
	if (!strcmp(current, "auto"))
	  values[num_values] = HC_LENGTH_AUTO;
	else if (strchr("0123456789+-.", *current))
	  values[num_values] = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);
	else
	  values[num_values] = 0.0f;
      }

      num_values ++;
    }

    switch (num_values)
    {
      default :
          break;
      case 1 :
          box->margin.top    = values[0];
	  box->margin.right  = values[0];
          box->margin.bottom = values[0];
	  box->margin.left   = values[0];
	  break;
      case 2 :
          box->margin.top    = values[0];
	  box->margin.right  = values[1];
          box->margin.bottom = values[0];
	  box->margin.left   = values[1];
	  break;
      case 3 :
          box->margin.top    = values[0];
	  box->margin.right  = values[1];
          box->margin.bottom = values[2];
	  box->margin.left   = values[1];
	  break;
      case 4 :
          box->margin.top    = values[0];
	  box->margin.right  = values[1];
          box->margin.bottom = values[2];
	  box->margin.left   = values[3];
	  break;
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "margin-bottom")) != NULL)
  {
    if (!strcmp(value, "auto"))
      box->margin.bottom = HC_LENGTH_AUTO;
    else if (strchr("0123456789+-.", *value))
      box->margin.bottom = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "margin-left")) != NULL)
  {
    if (!strcmp(value, "auto"))
      box->margin.left = HC_LENGTH_AUTO;
    else if (strchr("0123456789+-.", *value))
      box->margin.left = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "margin-right")) != NULL)
  {
    if (!strcmp(value, "auto"))
      box->margin.right = HC_LENGTH_AUTO;
    else if (strchr("0123456789+-.", *value))
      box->margin.right = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "margin-top")) != NULL)
  {
    if (!strcmp(value, "auto"))
      box->margin.top = HC_LENGTH_AUTO;
    else if (strchr("0123456789+-.", *value))
      box->margin.top = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "padding")) != NULL)
  {
    char	*temp = strdup(value),	/* Copy of value */
		*current,		/* Current value */
		*next;			/* Next value */
    int		num_values = 0;		/* Number of values */
    float	values[4];		/* Values */

    for (next = temp, current = strsep(&next, " \t"); current; current = strsep(&next, " \t"))
    {
      if (num_values < 4)
      {
	if (strchr("0123456789+-.", *current))
	  values[num_values] = hc_get_length(current, box->size.width, 72.0f / 96.0f, css, &text);
	else
	  values[num_values] = 0.0f;
      }

      num_values ++;
    }

    switch (num_values)
    {
      default :
          break;
      case 1 :
          box->padding.top    = values[0];
	  box->padding.right  = values[0];
          box->padding.bottom = values[0];
	  box->padding.left   = values[0];
	  break;
      case 2 :
          box->padding.top    = values[0];
	  box->padding.right  = values[1];
          box->padding.bottom = values[0];
	  box->padding.left   = values[1];
	  break;
      case 3 :
          box->padding.top    = values[0];
	  box->padding.right  = values[1];
          box->padding.bottom = values[2];
	  box->padding.left   = values[1];
	  break;
      case 4 :
          box->padding.top    = values[0];
	  box->padding.right  = values[1];
          box->padding.bottom = values[2];
	  box->padding.left   = values[3];
	  break;
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "padding-bottom")) != NULL)
  {
    if (strchr("0123456789+-.", *value))
      box->padding.bottom = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "padding-left")) != NULL)
  {
    if (strchr("0123456789+-.", *value))
      box->padding.left = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "padding-right")) != NULL)
  {
    if (strchr("0123456789+-.", *value))
      box->padding.right = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  if ((value = hcDictGetKeyValue(props, "padding-top")) != NULL)
  {
    if (strchr("0123456789+-.", *value))
      box->padding.top = hc_get_length(value, box->size.width, 72.0f / 96.0f, css, &text);
  }

  return (true);
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

bool					/* O - `true` on success, `false` on failure */
hcNodeComputeCSSMedia(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_media_t   *media)		/* O - Media properties */
{
  (void)node;
  (void)compute;
  (void)media;

  return (false);
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

bool					/* O - `true` on success, `false` on failure */
hcNodeComputeCSSTable(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_table_t   *table)		/* O - Table properties */
{
  const char		*value;		/* Property value */
//  hc_pool_t		*pool = node->value.element.html->pool;
					/* Memory pool */
//  hc_css_t		*css = node->value.element.html->css;
					/* Stylesheet */
  const hc_dict_t	*props = hcNodeComputeCSSProperties(node, compute);
					/* Properties */


  if (!table)
    return (false);

  memset(table, 0, sizeof(hc_table_t));

  if (!node)
    return (false);

  if ((value = hcDictGetKeyValue(props, "border-collapse")) != NULL)
  {
    if (!strcmp(value, "collapse"))
      table->border_collapse = HC_BORDER_COLLAPSE_COLLAPSE;
    else if (!strcmp(value, "separate"))
      table->border_collapse = HC_BORDER_COLLAPSE_SEPARATE;
  }

  return (true);
}


/*
 * 'hcNodeComputeCSSText()' - Compute the text properties for the given HTML node.
 */

bool					/* O - `true` on success, `false` on failure */
hcNodeComputeCSSText(
    hc_node_t    *node,			/* I - HTML node */
    hc_compute_t compute,		/* I - Pseudo-class, if any */
    hc_text_t    *text)			/* O - Text properties */
{
  int			i;		/* Looping var */
  const char		*value;		/* Property value */
  hc_css_t		*css;		/* Stylesheet */
  hc_pool_t		*pool;		/* Memory pool */
  const hc_dict_t	*props = hcNodeComputeCSSProperties(node, compute);
					/* Properties */
  static const char * const aligns[] =	/* text-align: values */
  {
    "left",
    "right",
    "center",
    "justify"
  };
  static const char * const decorations[] =
  {					/* text-decoration: values */
    "none",
    "underline",
    "overline",
    "line-through"
  };
  static const char * const transforms[] =
  {					/* text-transform: values */
    "none",
    "capitalize",
    "lowercase",
    "uppercase"
  };
  static const char * const unicode_bidis[] =
  {					/* unicode-bidi: values */
    "normal",
    "embed",
    "override"
  };
  static const char * const white_spaces[] =
  {					/* white-space: values */
    "normal",
    "nowrap",
    "pre",
    "pre-line",
    "pre-wrap"
  };


  if (!text)
    return (false);

  if (!_hcNodeComputeCSSTextFont(node, props, text))
    return (false);

  if (!node)
    return (false);

  css  = node->value.element.html->css;
  pool = node->value.element.html->pool;

  if ((value = hcDictGetKeyValue(props, "direction")) != NULL)
  {
    if (!strcmp(value, "ltr"))
      text->direction = HC_DIRECTION_LTR;
    else if (!strcmp(value, "rtl"))
      text->direction = HC_DIRECTION_RTL;
  }

  if ((value = hcDictGetKeyValue(props, "letter-spacing")) != NULL)
  {
    if (!strcmp(value, "normal"))
      text->letter_spacing = 0.0f;
    else
      text->letter_spacing = hc_get_length(value, css->media.size.width, 72.0f / 96.0f, css, text);
  }

  if ((value = hcDictGetKeyValue(props, "quotes")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next,			/* Next value */
		sep;			/* Separator character */
    int		quotes_pos = 0;		/* quotes: position */

    for (current = temp; *current && quotes_pos < 4; current = next)
    {
     /*
      * Extract one value...
      */

      while (isspace(*current & 255) && *current)
        current ++;

      if (!*current)
        break;

      for (next = current; *next; next ++)
      {
        if (isspace(*next & 255))
          break;
	else if (*next == '\'' || *next == '\"')
	{
	  int quote = *next++;

          while (*next)
          {
            if (*next == quote)
	      break;
	    else
	      next ++;
	  }
	}
      }

      sep   = *next;
      *next = '\0';

      if (*current == '\'' || *current == '\"')
        current ++;

      text->quotes[quotes_pos ++] = hcPoolGetString(pool, current);

      *next = sep;

      if (sep == '\'' || sep == '\"')
        next ++;
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "text-align")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(aligns) / sizeof(aligns[0])); i ++)
    {
      if (!strcmp(value, aligns[i]))
      {
        text->text_align = (hc_text_align_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "text-decoration")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(decorations) / sizeof(decorations[0])); i ++)
    {
      if (!strcmp(value, decorations[i]))
      {
        text->text_decoration = (hc_text_decoration_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "text-indent")) != NULL)
    text->text_indent = hc_get_length(value, css->media.size.width, 72.0f / 96.0f, css, text);

  if ((value = hcDictGetKeyValue(props, "text-transform")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(transforms) / sizeof(transforms[0])); i ++)
    {
      if (!strcmp(value, transforms[i]))
      {
        text->text_transform = (hc_text_transform_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "unicode-bidi")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(unicode_bidis) / sizeof(unicode_bidis[0])); i ++)
    {
      if (!strcmp(value, unicode_bidis[i]))
      {
        text->unicode_bidi = (hc_unicode_bidi_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "white-space")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(white_spaces) / sizeof(white_spaces[0])); i ++)
    {
      if (!strcmp(value, white_spaces[i]))
      {
        text->white_space = (hc_white_space_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "word-spacing")) != NULL)
  {
    if (!strcmp(value, "normal"))
      text->word_spacing = 0.0f;
    else
      text->word_spacing = hc_get_length(value, css->media.size.width, 72.0f / 96.0f, css, text);
  }

  return (true);
}


/*
 * '_hcNodeComputeCSSTextFont()' - Compute the text font properties for the
 *                                 given HTML node.
 */

bool					/* O - `true` on success, `false` on failure */
_hcNodeComputeCSSTextFont(
    hc_node_t       *node,		/* I - HTML node */
    const hc_dict_t *props,		/* I - Property dictionary */
    hc_text_t       *text)		/* O - Text properties */
{
  int			i;		/* Looping var */
  const char		*value;		/* Property value */
  hc_pool_t		*pool;		/* Memory pool */
  hc_css_t		*css;		/* Stylesheet */
  hc_text_t		parent_text;	/* Text properties for parent node */
  static const char * const stretches[] =
  {					/* font-stretch: values */
    "normal",
    "ultra-condensed",
    "extra-condensed",
    "condensed",
    "semi-condensed",
    "semi-expanded",
    "expanded",
    "extra-expanded",
    "ultra-expanded"
  };
  static const char * const styles[] =
  {					/* font-style: values */
    "normal",
    "italic",
    "oblique"
  };


 /*
  * Initialize default text values...
  */

  memset(text, 0, sizeof(hc_text_t));

  text->color.alpha = 1.0f;
  text->font_size   = 12.0f;
  text->font_weight = HC_FONT_WEIGHT_400;

 /*
  * Return if there is no node...
  */

  if (!node)
    return (false);

  pool = node->value.element.html->pool;
  css  = node->value.element.html->css;

  if ((value = hcDictGetKeyValue(props, "color")) != NULL)
    hc_get_color(value, &text->color);

  if ((value = hcDictGetKeyValue(props, "font")) != NULL)
  {
    char	*temp = strdup(value),	/* Temporary copy of value */
		*current,		/* Current value */
		*next,			/* Next value */
		sep;			/* Separator character */
    bool	saw_slash = false;	/* Did we see a slash? */
    int		font_pos = 0;		/* Position within the font value */

    for (current = temp; *current; current = next)
    {
     /*
      * Extract one value...
      */

      bool saw_comma = false;		/* Saw a comma? */

      while (isspace(*current & 255) && *current)
        current ++;

      if (!*current)
        break;

      for (next = current; *next; next ++)
      {
        if (isspace(*next & 255) && !saw_comma)
          break;
	else if (*next == ',')
	  saw_comma = true;
	else if (*next == '/')
	{
	  if (next == current)
	    next ++;
	  break;
	}
	else if (*next == '\'' || *next == '\"')
	{
	  int quote = *next++;

	  saw_comma = false;

          while (*next)
          {
            if (*next == quote)
	    {
	      next ++;
	      break;
	    }
	    else
	      next ++;
	  }
	}
	else
	  saw_comma = false;
      }

      sep   = *next;
      *next = '\0';

     /*
      * Decode...
      */

      if (*current == '\"' || *current == '\'' || !strcmp(current, "cursive") || !strcmp(current, "fantasy") || !strcmp(current, "monospace") || !strcmp(current, "sans-serif") || !strcmp(current, "serif"))
        text->font_family = hcPoolGetString(pool, current);
      else if (!strcmp(current, "normal"))
      {
        if (font_pos == 0)
          text->font_style = HC_FONT_STYLE_NORMAL;
	else if (font_pos == 1)
          text->font_variant = HC_FONT_VARIANT_NORMAL;
	else if (font_pos == 2)
          text->font_weight = HC_FONT_WEIGHT_400;
	else if (font_pos == 3)
          text->font_stretch = HC_FONT_STRETCH_NORMAL;
	else
          text->line_height = text->font_size * 1.2f;
      }
      else if (!strcmp(current, "small-caps"))
        text->font_variant = HC_FONT_VARIANT_SMALL_CAPS;
      else if (!strcmp(current, "bold"))
	text->font_weight = HC_FONT_WEIGHT_700;
      else if (!strcmp(current, "bolder"))
      {
        if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text) && (parent_text.font_weight + 300) < HC_FONT_WEIGHT_900)
          text->font_weight = (hc_font_weight_t)(parent_text.font_weight + 300);
	else
	  text->font_weight = HC_FONT_WEIGHT_900;
      }
      else if (!strcmp(current, "lighter"))
      {
        if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text) && (parent_text.font_weight - 300) > HC_FONT_WEIGHT_100)
          text->font_weight = (hc_font_weight_t)(parent_text.font_weight - 300);
	else
	  text->font_weight = HC_FONT_WEIGHT_100;
      }
      else if (*current >= '1' && *current <= '9' && current[1] == '0' && current[2] == '0' && !current[3])
	text->font_weight = (hc_font_weight_t)atoi(current);
      else if (!strcmp(current, "xx-small"))
	text->font_size = 7.0f;
      else if (!strcmp(current, "x-small"))
	text->font_size = 9.0f;
      else if (!strcmp(current, "small"))
	text->font_size = 10.0f;
      else if (!strcmp(current, "smaller"))
      {
        if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
          text->font_size = (float)round(parent_text.font_size / 1.2f);
	else
	  text->font_size = 10.0f;
      }
      else if (!strcmp(current, "medium"))
	text->font_size = 12.0f;
      else if (!strcmp(current, "large"))
	text->font_size = 14.0f;
      else if (!strcmp(current, "larger"))
      {
        if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
          text->font_size = (float)round(parent_text.font_size * 1.2f);
	else
	  text->font_size = 14.0f;
      }
      else if (!strcmp(current, "x-large"))
	text->font_size = 18.0f;
      else if (!strcmp(current, "xx-large"))
	text->font_size = 24.0f;
      else if (!strcmp(current, "/"))
        saw_slash = true;
      else if (strchr("0123456789.", *current))
      {
        if (saw_slash)
        {
          text->line_height = hc_get_length(current, text->font_size, text->font_size, css, text);
        }
        else
        {
	  if (!node->parent || !_hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
	    parent_text.font_size = 12.0f;

          text->font_size = hc_get_length(current, parent_text.font_size, 72.0f / 96.0f, css, &parent_text);
	}
      }
      else
      {
	for (i = 0; i < (int)(sizeof(stretches) / sizeof(stretches[0])); i ++)
	{
	  if (!strcmp(current, stretches[i]))
	  {
	    text->font_stretch = (hc_font_stretch_t)i;
	    break;
	  }
	}

	for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
	{
	  if (!strcmp(current, styles[i]))
	  {
	    text->font_style = (hc_font_style_t)i;
	    break;
	  }
	}
      }

     /*
      * Restore the separator character...
      */

      *next = sep;
      font_pos ++;
    }

    free(temp);
  }

  if ((value = hcDictGetKeyValue(props, "font-family")) != NULL)
    text->font_family = value;
  else if (!text->font_family)
    text->font_family = "sans-serif";

  if ((value = hcDictGetKeyValue(props, "font-size")) != NULL)
  {
    if (!strcmp(value, "xx-small"))
      text->font_size = 7.0f;
    else if (!strcmp(value, "x-small"))
      text->font_size = 9.0f;
    else if (!strcmp(value, "small"))
      text->font_size = 10.0f;
    else if (!strcmp(value, "smaller"))
    {
      if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
	text->font_size = (float)round(parent_text.font_size / 1.2f);
      else
	text->font_size = 10.0f;
    }
    else if (!strcmp(value, "medium"))
      text->font_size = 12.0f;
    else if (!strcmp(value, "large"))
      text->font_size = 14.0f;
    else if (!strcmp(value, "larger"))
    {
      if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
	text->font_size = (float)round(parent_text.font_size * 1.2f);
      else
	text->font_size = 14.0f;
    }
    else if (!strcmp(value, "x-large"))
      text->font_size = 18.0f;
    else if (!strcmp(value, "xx-large"))
      text->font_size = 24.0f;
    else if (strchr("0123456789.", *value))
    {
      if (!node->parent || !_hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text))
	parent_text.font_size = 12.0f;

      text->font_size = hc_get_length(value, parent_text.font_size, 72.0f / 96.0f, css, &parent_text);
    }
  }
  else if (text->font_size <= 0.0f)
    text->font_size = 12.0f;

  if ((value = hcDictGetKeyValue(props, "font-size-adjust")) != NULL)
  {
    if (!strcmp(value, "none"))
      text->font_size_adjust = 0.0f;
    else if (strchr("0123456789.", *value))
      text->font_size_adjust = hc_get_length(value, text->font_size, 72.0f / 96.0f, css, text);
  }

  if ((value = hcDictGetKeyValue(props, "font-stretch")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(stretches) / sizeof(stretches[0])); i ++)
    {
      if (!strcmp(value, stretches[i]))
      {
        text->font_stretch = (hc_font_stretch_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "font-style")) != NULL)
  {
    for (i = 0; i < (int)(sizeof(styles) / sizeof(styles[0])); i ++)
    {
      if (!strcmp(value, styles[i]))
      {
        text->font_style = (hc_font_style_t)i;
        break;
      }
    }
  }

  if ((value = hcDictGetKeyValue(props, "font-variant")) != NULL)
  {
    if (!strcmp(value, "normal"))
      text->font_variant = HC_FONT_VARIANT_NORMAL;
    else if (!strcmp(value, "small-caps"))
      text->font_variant = HC_FONT_VARIANT_SMALL_CAPS;
  }

  if ((value = hcDictGetKeyValue(props, "font-weight")) != NULL)
  {
    if (!strcmp(value, "normal"))
      text->font_weight = HC_FONT_WEIGHT_400;
    else if (!strcmp(value, "bold"))
      text->font_weight = HC_FONT_WEIGHT_700;
    else if (!strcmp(value, "bolder"))
    {
      if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text) && (parent_text.font_weight + 300) < HC_FONT_WEIGHT_900)
	text->font_weight = (hc_font_weight_t)(parent_text.font_weight + 300);
      else
	text->font_weight = HC_FONT_WEIGHT_900;
    }
    else if (!strcmp(value, "lighter"))
    {
      if (node->parent && _hcNodeComputeCSSTextFont(node->parent, node->parent->value.element.base_props, &parent_text) && (parent_text.font_weight - 300) > HC_FONT_WEIGHT_100)
	text->font_weight = (hc_font_weight_t)(parent_text.font_weight - 300);
      else
	text->font_weight = HC_FONT_WEIGHT_100;
    }
    else if (*value >= '1' && *value <= '9' && value[1] == '0' && value[2] == '0' && !value[3])
      text->font_weight = (hc_font_weight_t)atoi(value);
  }

  if ((value = hcDictGetKeyValue(props, "line-height")) != NULL)
  {
    if (!strcmp(value, "normal"))
      text->line_height = text->font_size * 1.2f;
    else if (strchr("0123456789.", *value))
      text->line_height = hc_get_length(value, text->font_size, text->font_size, css, text);
  }
  else if (text->line_height <= 0.0f)
    text->line_height = text->font_size * 1.2f;

 /*
  * Lookup font...
  */

  if (text->font_family)
  {
    char	*temp = strdup(text->font_family),
					/* Temporary copy of value */
		*current,		/* Current value */
		*next;			/* Next value */

    for (current = temp; *current && !text->font; current = next)
    {
     /*
      * Skip leading whitespace and commas...
      */

      while (*current && (isspace(*current & 255) || *current == ','))
        current ++;

      if (!*current)
        break;

     /*
      * Extract a family name...
      */

      if (*current == '\'' || *current == '\"')
      {
        char	quote = *current++;	/* Quote character */

        for (next = current; *next; next ++)
          if (*next == quote)
            break;
      }
      else
      {
        for (next = current; *next; next ++)
          if (isspace(*next & 255) || *next == ',')
            break;
      }

      if (*next)
	*next++ = '\0';

      text->font = hcFontFindCached(pool, current, text->font_stretch, text->font_style, text->font_variant, text->font_weight);
    }

    free(temp);
  }

  return (true);
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
    "before",
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

static bool				/* O - `true` on success, `false` on failure */
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
      return (false);

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
      return (false);

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

    return (true);
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
      return (false);

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
      return (false);

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
      return (false);
    rgba.alpha = (float)strtod(ptr + 1, &ptr);
    if (*ptr == '%')
    {
      ptr ++;
      rgba.alpha /= 100.0f;
    }
    if (*ptr != ')')
      return (false);

    *color = rgba;

    return (true);
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

      return (true);
    }
    else if (len == 7 && (i = (int)strtol(value + 1, &ptr, 16)) >= 0 && !*ptr)
    {
      color->red   = ((i >> 16) & 255) / 255.0f;
      color->green = ((i >> 8) & 255) / 255.0f;
      color->blue  = (i & 255) / 255.0f;
      color->alpha = 1.0f;

      return (true);
    }
    else
      return (false);
  }
  else
  {
    for (i = 0; i < (int)(sizeof(colors) / sizeof(colors[0])); i ++)
    {
      if (!strcmp(value, colors[i].name))
      {
        *color = colors[i].rgba;
        return (true);
      }
    }
  }

  return (false);
}


/*
 * 'hc_get_length()' - Get a length/measurement value.
 */

static float				/* O - Value in points or 0.0 on error */
hc_get_length(const char *value,	/* I - Value string */
              float      max_value,	/* I - Maximum value for percentages */
              float      multiplier,	/* I - Multiplier for plain number values */
              hc_css_t   *css,		/* I - Stylesheet */
              hc_text_t  *text)		/* I - Text properties */
{
  char		*ptr;			/* Pointer to units after value */
  double	temp = strtod(value, &ptr);
					/* Interim value */

  if (ptr)
  {
    if (!*ptr)
      temp *= multiplier;
    else if (!strcmp(ptr, "%"))
      temp *= 0.01 * max_value;
    else if (!strcmp(ptr, "ch") && text)
    {
      hc_rect_t	extents;		/* Font extents */

      hcFontComputeExtents(text->font, text->font_size, "0", &extents);
      temp *= extents.right;
    }
    else if (!strcmp(ptr, "cm"))
      temp *= 72.0 / 2.54;
    else if (!strcmp(ptr, "em") && text)
      temp *= text->font_size;
    else if (!strcmp(ptr, "ex") && text)
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

    if (!curnode)
      return (-1);

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

