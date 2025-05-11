//
// Private CSS header file for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_CSS_PRIVATE_H
#  define HTMLCSS_CSS_PRIVATE_H
#  include "css.h"
#  include "html-private.h"
#  include "sha3.h"
#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus


//
// Types...
//

//
// CSS selectors are linked lists starting at the leaf node to speed lookups.
// Each selector is a sequence of matching statements starting with an
// associated element (* wildcard, P, etc.) followed by zero or more additional
// matching statements (".classname", "#identifier", ":link", etc.)
//
// A list of selectors is associated with an array of properties, each of which
// is a simple key/value pair.  This association is called a rule set.
//
// For convenience and lookup efficiency, rule sets with compound selectors,
// e.g.:
//
//     h1, h2, h3 { font-weight: bold; }
//
// are split into three separate rule sets using each of the selector lists.
//

typedef enum _hc_match_e
{
  _HC_MATCH_ATTR_EXIST,			// [NAME]
  _HC_MATCH_ATTR_EQUALS,		// [NAME=VALUE]
  _HC_MATCH_ATTR_CONTAINS,		// [NAME*=VALUE]
  _HC_MATCH_ATTR_BEGINS,		// [NAME^=VALUE]
  _HC_MATCH_ATTR_ENDS,			// [NAME$=VALUE]
  _HC_MATCH_ATTR_LANG,			// [NAME|=VALUE] (language/prefix match)
  _HC_MATCH_ATTR_SPACE,			// [NAME~=VALUE] (space-delimited value match)
  _HC_MATCH_CLASS,			// .NAME
  _HC_MATCH_ID,				// #NAME
  _HC_MATCH_PSEUDO_CLASS		// :NAME or :NANE(VALUE) pseudo-class
} _hc_match_t;

typedef enum _hc_relation_e		// Relationship to previous selector
{
  _HC_RELATION_CHILD,			// Child (descendent) of previous (E F)
  _HC_RELATION_IMMED_CHILD,		// Immediate child of previous (E > F)
  _HC_RELATION_SIBLING,			// Sibling of previous (E ~ F)
  _HC_RELATION_IMMED_SIBLING		// Immediate sibling of previous (E + F)
} _hc_relation_t;

typedef struct _hc_css_selstmt_s	// CSS selector matching statements
{
  _hc_match_t		match;		// Matching rule
  const char		*name,		// Name, if needed
			*value;		// Value, if needed
} _hc_css_selstmt_t;

typedef struct _hc_css_sel_s		// CSS selector
{
  struct _hc_css_sel_s	*prev;		// Previous selector
  hc_element_t		element;	// Element
  _hc_relation_t	relation;	// Relation to previous
  size_t		num_stmts;	// Number of selector matching statements
  _hc_css_selstmt_t	*stmts;		// Matching statements
} _hc_css_sel_t;

typedef struct _hc_rule_s		// CSS rule set
{
  hc_sha3_256_t		hash;		// Hash of selector
  _hc_css_sel_t		*sel;		// Leaf selector
  hc_dict_t		*props;		// Properties
} _hc_rule_t;

typedef struct _hc_rulecol_s		// Collection of rules
{
  int			needs_sort;	// Needs sorting?
  size_t		alloc_rules;	// Allocated rules
  size_t		num_rules;	// Number of rules
  _hc_rule_t		**rules;	// Rules
} _hc_rulecol_t;

struct _hc_css_s
{
  hc_pool_t		*pool;		// Memory pool
  hc_media_t		media;		// Base media definition
  _hc_rulecol_t		all_rules;	// All rule sets in the stylesheet and document
  _hc_rulecol_t		rules[HC_ELEMENT_MAX];
					// Rule sets organized by element
};


//
// Functions...
//

extern void	_hcCSSImportString(hc_css_t *css, hc_dict_t *props, const char *s);
extern void	_hcCSSSelAddStmt(hc_css_t *css, _hc_css_sel_t *sel, _hc_match_t match, const char *name, const char *value);
extern void	_hcCSSSelDelete(_hc_css_sel_t *sel);
extern void	_hcCSSSelHash(_hc_css_sel_t *sel, hc_sha3_256_t hash);
extern _hc_css_sel_t *_hcCSSSelNew(hc_css_t *css, _hc_css_sel_t *prev, hc_element_t element, _hc_relation_t rel);

extern void	_hcRuleColAdd(hc_css_t *css, _hc_rulecol_t *col, _hc_rule_t *rule);
extern void	_hcRuleColClear(_hc_rulecol_t *col, int delete_rules);
extern _hc_rule_t *_hcRuleColFindHash(_hc_rulecol_t *col, const hc_sha3_256_t hash);
extern void	_hcRuleDelete(_hc_rule_t *rule);
extern _hc_rule_t *_hcRuleNew(hc_css_t *css, const hc_sha3_256_t hash, _hc_css_sel_t *sel, hc_dict_t *props);


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !HTMLCSS_CSS_PRIVATE_H
