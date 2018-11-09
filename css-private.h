/*
 * Private CSS header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_CSS_PRIVATE_H
#  define HTMLCSS_CSS_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "css.h"
#  include "html-private.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef enum _css_match_e
{
  _CSS_MATCH_ACTIVE,			/* :active pseudo-class */
  _CSS_MATCH_AFTER,			/* :after pseudo-element */
  _CSS_MATCH_ATTR_EXIST,		/* [NAME] */
  _CSS_MATCH_ATTR_EQUALS,		/* [NAME=VALUE] */
  _CSS_MATCH_ATTR_CONTAINS,		/* [NAME*=VALUE] */
  _CSS_MATCH_ATTR_BEGINS,		/* [NAME^=VALUE] */
  _CSS_MATCH_ATTR_ENDS,			/* [NAME$=VALUE] */
  _CSS_MATCH_ATTR_LANG,			/* [NAME|=VALUE] (language/prefix match) */
  _CSS_MATCH_ATTR_SPACE,		/* [NAME~=VALUE] (space-delimited value match) */
  _CSS_MATCH_BEFORE,			/* :before pseudo-element */
  _CSS_MATCH_CHECKED,			/* :checked pseudo-class */
  _CSS_MATCH_CLASS,			/* .NAME */
  _CSS_MATCH_DISABLED,			/* :disabled pseudo-class */
  _CSS_MATCH_EMPTY,			/* :empty pseudo-class */
  _CSS_MATCH_ENABLED,			/* :enabled pseudo-class */
  _CSS_MATCH_FIRST,			/* :first pseudo-class */
  _CSS_MATCH_FIRST_CHILD,		/* :first-child pseudo-class */
  _CSS_MATCH_FIRST_LINE,		/* :first-line pseudo-element */
  _CSS_MATCH_FIRST_LETTER,		/* :first-letter pseudo-element */
  _CSS_MATCH_FIRST_OF_TYPE,		/* :first-of-type pseudo-element */
  _CSS_MATCH_FOCUS,			/* :focus pseudo-class */
  _CSS_MATCH_HOVER,			/* :hover pseudo-class */
  _CSS_MATCH_LAST_CHILD,		/* :last-child pseudo-class */
  _CSS_MATCH_LAST_OF_TYPE,		/* :last-of-type pseudo-class */
  _CSS_MATCH_LEFT,			/* :left pseudo-class */
  _CSS_MATCH_ID,			/* #NAME */
  _CSS_MATCH_LANG,			/* :lang(NAME) pseudo-class */
  _CSS_MATCH_LINK,			/* :link pseudo-class */
/*  _CSS_MATCH_NOT,*/			/* :not(SEL) pseudo-class (NOT CURRENTLY SUPPORTED) */
  _CSS_MATCH_NTH_CHILD,			/* :nth-child(NAME) pseudo-class */
  _CSS_MATCH_NTH_LAST_CHILD,		/* :nth-last-child(NAME) pseudo-class */
  _CSS_MATCH_NTH_LAST_OF_TYPE,		/* :nth-last-of-type(NAME) pseudo-class */
  _CSS_MATCH_NTH_OF_TYPE,		/* :nth-of-type(NAME) pseudo-class */
  _CSS_MATCH_ONLY_CHILD,		/* :only-child pseudo-class */
  _CSS_MATCH_ONLY_OF_TYPE,		/* :only-of-type pseudo-class */
  _CSS_MATCH_RIGHT,			/* :right pseudo-class */
  _CSS_MATCH_ROOT,			/* :root pseudo-class */
  _CSS_MATCH_TARGET,			/* :target pseudo-class */
  _CSS_MATCH_VISITED,			/* :visited pseudo-class */
} _css_match_t;

typedef struct _css_prop_s		/* CSS property */
{
  char			*name,		/* Property name */
			*value;		/* Property value */
} _css_prop_t;

typedef struct _css_sel_s		/* CSS selector */
{
  _css_match_t		match;		/* Matching rule */
  char			*name,		/* Name, if needed */
			*value;		/* Value, if needed */
} _css_sel_t;

/* CSS rules are linked lists starting at the leaf node (to speed lookups) */
typedef struct _css_rule_s		/* CSS selector rule */
{
  struct _css_rule_s	*prev;		/* Previous selector */
  html_element_t	element;	/* Element */
  char			child_of_prev,	/* Is immediate child of previous */
			follows_prev,	/* Is immediate sibling of previous */
			sibling_prev;	/* Is sibling of previous */
  int			num_sels;	/* Number of selectors */
  _css_sel_t		*sels;		/* Selectors */
  int			num_props;	/* Number of properties */
  _css_prop_t		*props;		/* Properties */
} _css_rule_t;

struct _css_s
{
  css_media_t		media;		/* Base media definition */
  htmlcss_error_cb_t	error_cb;	/* Error callback */
  void			*error_ctx;	/* Error callback context pointer */
  htmlcss_url_cb_t	url_cb;		/* URL callback */
  void			*url_ctx;	/* URL callback context pointer */
  int			num_rules[HTML_ELEMENT_MAX];
					/* Number of rules for each element */
  _css_rule_t		*rules[HTML_ELEMENT_MAX];
					/* Rules for each element */
};


/*
 * Functions...
 */


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_CSS_PRIVATE_H */
