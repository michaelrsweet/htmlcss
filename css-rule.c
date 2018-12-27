/*
 * CSS rule set support functions for HTMLCSS library.
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
#  include "pool-private.h"


/*
 * Local functions...
 */

static int	hc_compare_rules(_hc_rule_t **a, _hc_rule_t **b);


/*
 * '_hcCSSSelAddStmt()' - Add a matching statement to a selector.
 */

void
_hcCSSSelAddStmt(hc_css_t      *css,	/* I - Stylesheet */
                 _hc_css_sel_t *sel,	/* I - Selector */
                 _hc_match_t   match,	/* I - Match type */
                 const char    *name,	/* I - Name, if any */
                 const char    *value)	/* I - Value, if any */
{
  _hc_css_selstmt_t	*temp;		/* Current statement */


  if ((temp = realloc(sel->stmts, (sel->num_stmts + 1) * sizeof(_hc_css_selstmt_t))) != NULL)
  {
    sel->stmts = temp;
    temp += sel->num_stmts;
    sel->num_stmts ++;

    temp->match = match;
    temp->name  = hcPoolGetString(css->pool, name);
    temp->value = hcPoolGetString(css->pool, value);
  }
  else
    _hcPoolError(css->pool, 0, "Unable to allocate memory for selector statement.");
}


/*
 * '_hcCSSSelDelete()' - Free the memory used by a list of selectors.
 */

void
_hcCSSSelDelete(_hc_css_sel_t *sel)	/* I - Selectors */
{
  _hc_css_sel_t *prev;			/* Previous selector */


  while (sel)
  {
    prev = sel->prev;

    if (sel->stmts)
      free(sel->stmts);

    free(sel);

    sel = prev;
  }
}


/*
 * '_hcCSSSelHash()' - Create a SHA3-256 hash of a list of selectors.
 */

void
_hcCSSSelHash(_hc_css_sel_t *sel,	/* I - Selectors */
              hc_sha3_256_t hash)	/* O - Hash of selectors */
{
  hc_sha3_t	ctx;			/* SHA3 hashing context */


  hcSHA3Init(&ctx);

  while (sel)
  {
    hcSHA3Update(&ctx, &sel->element, sizeof(sel->element));
    hcSHA3Update(&ctx, &sel->relation, sizeof(sel->relation));
    if (sel->stmts)
      hcSHA3Update(&ctx, sel->stmts, sel->num_stmts * sizeof(_hc_css_selstmt_t));

    sel = sel->prev;
  }

  hcSHA3Final(&ctx, hash, HC_SHA3_256_SIZE);
}


/*
 * '_hcCSSSelNew()' - Create a new CSS selector.
 */

_hc_css_sel_t *				/* O - New selector */
_hcCSSSelNew(hc_css_t       *css,	/* I - Stylesheet */
             _hc_css_sel_t  *prev,	/* I - Previous selector, if any */
             hc_element_t   element,	/* I - Element or `HD_ELEMENT_WILDCARD` */
             _hc_relation_t rel)	/* I - Relation to previous selector */
{
  _hc_css_sel_t	*sel;			/* New selector */


  if ((sel = (_hc_css_sel_t *)calloc(1, sizeof(_hc_css_sel_t))) != NULL)
  {
    sel->prev     = prev;
    sel->element  = element;
    sel->relation = rel;
  }
  else
    _hcPoolError(css->pool, 0, "Unable to allocate memory for selector.");

  return (sel);
}


/*
 * '_hcRuleColAdd()' - Add a rule set to a collection.
 */

void
_hcRuleColAdd(hc_css_t      *css,	/* I - Stylesheet */
              _hc_rulecol_t *col,	/* I - Rule set collection */
              _hc_rule_t    *rule)	/* I - Rule set to add */
{
  _hc_rule_t	**ptr;			/* New rule array pointer */


  if (col->num_rules >= col->alloc_rules)
  {
    size_t	alloc_rules;		/* New allocation */

    if (col->alloc_rules == 0)
      alloc_rules = 1;
    else if (col->alloc_rules < 32)
      alloc_rules = col->alloc_rules * 2;
    else
      alloc_rules = col->alloc_rules + 32;

    if ((ptr = realloc(col->rules, alloc_rules * sizeof(_hc_rule_t *))) == NULL)
    {
      _hcPoolError(css->pool, 0, "Unable to allocate memory for selector rules.");
      return;
    }

    col->rules       = ptr;
    col->alloc_rules = alloc_rules;
  }

  col->rules[col->num_rules] = rule;
  col->num_rules ++;

  if (col->num_rules > 1)
    col->needs_sort = 1;
}


/*
 * '_hcRuleColClear()' - Empty a collection, optionally freeing the rule sets in it.
 */

void
_hcRuleColClear(
    _hc_rulecol_t *col,			/* I - Rule set collection */
    int           delete_rules)		/* I - 1 to delete rules, 0 to just clear collection */
{
  if (delete_rules && col->num_rules)
  {
    size_t	i;			/* Looping var */
    _hc_rule_t	**ptr;			/* Pointer into array */

    for (i = col->num_rules, ptr = col->rules; i > 0; i --, ptr ++)
      _hcRuleDelete(*ptr);
  }

  col->num_rules  = 0;
  col->needs_sort = 0;

  if (col->alloc_rules > 0)
  {
    free(col->rules);
    col->alloc_rules = 0;
    col->rules       = NULL;
  }
}


/*
 * '_hcRuleColFindHash()' - Find a rule set using its hash.
 */

_hc_rule_t *				/* O - Matching rule or `NULL` */
_hcRuleColFindHash(
    _hc_rulecol_t       *col,		/* I - Rule set collection */
    const hc_sha3_256_t hash)		/* I - SHA3-256 hash */
{
  _hc_rule_t	key,			/* Search key */
		*ptr,			/* Pointer to key */
		**match;		/* Matching rule */


  if (col->needs_sort)
  {
    qsort(col->rules, col->num_rules, sizeof(_hc_rule_t *), (_hc_compare_func_t)hc_compare_rules);
    col->needs_sort = 0;
  }

  memcpy(key.hash, hash, sizeof(key.hash));
  ptr = &key;

  if ((match = bsearch(&ptr, col->rules, col->num_rules, sizeof(_hc_rule_t *), (int (*)(const void *, const void *))hc_compare_rules)) != NULL)
    return (*match);
  else
    return (NULL);
}


/*
 * '_hcRuleDelete()' - Free memory used by a rule set.
 */

void
_hcRuleDelete(_hc_rule_t *rule)		/* I - Rule set */
{
  _hcCSSSelDelete(rule->sel);
  hcDictDelete(rule->props);
  free(rule);
}


/*
 * '_hcRuleNew()' - Create a new rule set.
 */

_hc_rule_t *				/* I - Rule set */
_hcRuleNew(
    hc_css_t            *css,		/* I - Stylesheet */
    const hc_sha3_256_t hash,		/* I - SHA3-256 hash */
    _hc_css_sel_t       *sel,		/* I - Selectors */
    hc_dict_t           *props)		/* I - Properties dictionary */
{
  _hc_rule_t	*rule;			/* New rule */


  if ((rule = calloc(1, sizeof(_hc_rule_t))) != NULL)
  {
    memcpy(rule->hash, hash, sizeof(rule->hash));
    rule->sel   = sel;
    rule->props = hcDictCopy(props);
  }
  else
    _hcPoolError(css->pool, 0, "Unable to allocate memory for selector rules.");

  return (rule);
}


/*
 * 'hc_compare_rules()' - Compare two rules in a collection.
 */

static int				/* O - Result of comparison */
hc_compare_rules(_hc_rule_t **a,	/* I - First rule */
                 _hc_rule_t **b)	/* I - Second rule */
{
  return (memcmp((*a)->hash, (*b)->hash, sizeof((*a)->hash)));
}
