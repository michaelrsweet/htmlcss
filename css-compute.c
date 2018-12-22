/*
 * CSS computation functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#  include "css-private.h"
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
  (void)node;
  (void)compute;
  (void)box;

  return (0);
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
  (void)node;
  (void)compute;

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

