/*
 * Unit test program for HTMLCSS library.
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

#include "htmlcss.h"
#include "css-private.h"


/*
 * Local functions...
 */

static int	test_pool_functions(hc_pool_t *pool);


/*
 * 'main()' - Main entry for unit tests.
 */

int					/* O - Exit status */
main(int  argc,				/* I - Number of command-line arguments */
     char *argv[])			/* I - Command-line arguments */
{
  int		i;			/* Looping var */
  const char	*ext;			/* Filename extension */
  hc_pool_t	*pool;			/* Memory pool */
  hc_css_t	*css;			/* Stylesheet */
  hc_html_t	*html;			/* HTML document */
  hc_node_t	*node,			/* Current node */
		*next;			/* Next node */
  int		level;			/* Indentation level */
  hc_element_t	element;		/* Current element */
  _hc_rule_t	*rule;			/* CSS rule */
  size_t	rcount;			/* Rule count */
  _hc_css_sel_t	*sel;			/* CSS selector */
  size_t	pindex,			/* Property index */
		pcount;			/* Property count */
  const char	*pname,			/* Property name */
		*pvalue;		/* Property value */


 /*
  * Test string pool functions...
  */

  pool = hcPoolNew();

  if (!test_pool_functions(pool))
    return (1);

 /*
  * Test CSS/HTML functions...
  */

  css = hcCSSNew(pool);

  hcCSSImportDefault(css);

  html = hcHTMLNew(pool, css);

  for (i = 1; i < argc; i ++)
  {
    if ((ext = strrchr(argv[i], '.')) == NULL || strcmp(ext, ".css"))
      hcHTMLLoad(html, argv[i], NULL);
    else
      hcCSSImport(css, argv[i], NULL, NULL);
  }

  puts("HTML document tree:\n");

  for (node = hcHTMLGetRootNode(html), level = 0; node; node = next)
  {
    hc_element_t element = hcNodeGetElement(node);

    printf("%*s", level * 2, "");

    if (element == HC_ELEMENT_STRING)
    {
      const char *s = hcNodeGetString(node);

      while (*s)
      {
	if (*s < ' ')
	  printf("\\%03o", *s);
	else
	  putchar(*s);

	s ++;
      }

      putchar('\n');
    }
    else if (element == HC_ELEMENT_COMMENT)
      printf("<!-- %s -->\n", hcNodeGetComment(node));
    else if (element == HC_ELEMENT_DOCTYPE)
      printf("<!DOCTYPE %s>\n", hcNodeAttrGetNameValue(node, ""));
    else
    {
      int idx, count = hcNodeAttrGetCount(node);

      printf("<%s", hcElements[element]);
      for (idx = 0; idx < count; idx ++)
      {
	const char *name, *value = hcNodeAttrGetIndexNameValue(node, idx, &name);
	printf(" %s=\"%s\"", name, value);
      }
      puts(">");
    }

    if ((next = hcNodeGetFirstChildNode(node)) != NULL)
      level += 2;
    else
    {
      if ((next = hcNodeGetNextSiblingNode(node)) == NULL)
      {
	next = hcNodeGetParentNode(node);
	level -= 2;

	while (next && !hcNodeGetNextSiblingNode(next))
	{
	  next = hcNodeGetParentNode(next);
	  level -= 2;
	}

	if (next)
	  next = hcNodeGetNextSiblingNode(next);
      }
    }
  }

  puts("Flattened CSS:\n");

  for (element = HC_ELEMENT_WILDCARD; element < HC_ELEMENT_MAX; element ++)
  {
    for (rule = css->rules[element], rcount = css->num_rules[element]; rcount > 0; rule ++, rcount --)
    {
      int		num_sels = 0;	/* Number of selectors */
      _hc_css_sel_t	*sels[100];	/* Selectors */

      for (sel = rule->sel; sel && num_sels < 100; sel = sel->prev)
        sels[num_sels++] = sel;

      while (num_sels > 0)
      {
        _hc_css_selstmt_t *stmt;	/* Matching statement */

        num_sels --;
        sel = sels[num_sels];

        switch (sel->relation)
        {
          case _HC_RELATION_CHILD :	/* Child (descendent) of previous (E F) */
              break;
          case _HC_RELATION_IMMED_CHILD: /* Immediate child of previous (E > F) */
              fputs("> ", stdout);
              break;
          case _HC_RELATION_SIBLING:	/* Sibling of previous (E ~ F) */
              fputs("~ ", stdout);
              break;
          case _HC_RELATION_IMMED_SIBLING: /* Immediate sibling of previous (E + F) */
              fputs("+ ", stdout);
              break;
        }

        if (sel->element == HC_ELEMENT_WILDCARD)
          fputs("*", stdout);
        else
          fputs(hcElements[sel->element], stdout);

        for (i = 0, stmt = sel->stmts; i < (int)sel->num_stmts; i ++, stmt ++)
        {
          switch (stmt->match)
          {
            case _HC_MATCH_ATTR_EXIST :
                printf("[%s]", stmt->name);
                break;
            case _HC_MATCH_ATTR_EQUALS :
                printf("[%s=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_ATTR_CONTAINS :
                printf("[%s*=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_ATTR_BEGINS :
                printf("[%s^=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_ATTR_ENDS :
                printf("[%s$=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_ATTR_LANG :
                printf("[%s|=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_ATTR_SPACE :
                printf("[%s~=\"%s\"]", stmt->name, stmt->value);
                break;
            case _HC_MATCH_CLASS :
                printf(".%s", stmt->name);
                break;
            case _HC_MATCH_ID :
                printf("#%s", stmt->name);
                break;
            case _HC_MATCH_PSEUDO_CLASS :
                if (stmt->value)
		  printf(":%s(%s)", stmt->name, stmt->value);
                else
		  printf(":%s", stmt->name);
                break;
          }
        }

        putchar(' ');
      }

      puts("{");

      for (pindex = 0, pcount = hcDictGetCount(rule->props); pindex < pcount; pindex ++)
      {
        pvalue = hcDictGetIndexKeyValue(rule->props, pindex, &pname);
        printf("  %s: %s;\n", pname, pvalue);
      }

      puts("}");
    }
  }

  hcHTMLDelete(html);
  hcCSSDelete(css);
  hcPoolDelete(pool);

  return (0);
}


/*
 * 'test_pool_functions()' - Test memory pool functions.
 */

static int				/* I - 1 on success, 0 on failure */
test_pool_functions(hc_pool_t *pool)	/* I - Memory pool */
{
  int		i, j;			/* Looping vars */
  const char	*strings[235],		/* Strings from memory pool */
		*temp;			/* Temporary string pointer */
  static const char * const words[235] =/* Test strings */
  {
    "accordant",
    "actinomere",
    "advisableness",
    "agitate",
    "alef",
    "Alsophila",
    "Amoy",
    "anchoritish",
    "annulation",
    "anticontagionist",
    "anxietude",
    "approximation",
    "areologically",
    "Ascella",
    "atangle",
    "autobasidiomycetous",
    "azoflavine",
    "ballast",
    "Basilidianism",
    "beefeater",
    "bepaper",
    "bicornate",
    "biternate",
    "blunge",
    "Bostrychidae",
    "breastfeeding",
    "Bryum",
    "button",
    "Callithrix",
    "capitulum",
    "cartboot",
    "cavate",
    "certify",
    "cheat",
    "chloroauric",
    "Chrysotis",
    "clammish",
    "cnemial",
    "cogue",
    "commentatorship",
    "Condylura",
    "contemporary",
    "cordmaker",
    "could",
    "craniological",
    "crownling",
    "curratow",
    "cyton",
    "deadwort",
    "deflagration",
    "denitrator",
    "despiteful",
    "diastrophy",
    "dioecious",
    "disenamour",
    "distortional",
    "dopebook",
    "dropsically",
    "earlet",
    "Eimak",
    "emblement",
    "endorsation",
    "entreat",
    "equanimousness",
    "estop",
    "Evodia",
    "exploratively",
    "faitour",
    "feminacy",
    "fin",
    "flintily",
    "forbiddable",
    "Fourierite",
    "fuchsinophilous",
    "gallows",
    "geanticline",
    "ghaist",
    "glossoptosis",
    "gorily",
    "grieved",
    "gurl",
    "Hamamelidaceae",
    "headmost",
    "hemianopia",
    "hesperidin",
    "Hippoglossus",
    "homoiothermic",
    "Hugo",
    "hydrotherapeutics",
    "hypogean",
    "idose",
    "impersuadable",
    "incomprehension",
    "Inermia",
    "inobservation",
    "intercombination",
    "interwrought",
    "iridoncus",
    "isotomous",
    "Jezebelian",
    "Kalandariyah",
    "kiln",
    "Kua",
    "lanciferous",
    "leadable",
    "leucocism",
    "linja",
    "logographical",
    "lupinaster",
    "magician",
    "manganeisen",
    "Maskoi",
    "Medize",
    "merchantableness",
    "metastoma",
    "middlebuster",
    "miscompute",
    "Mogollon",
    "monospherical",
    "mountainette",
    "muscatorium",
    "Myxobacteriaceae",
    "necrographer",
    "neuropath",
    "nominally",
    "nonelemental",
    "nonprojection",
    "norwester",
    "obituarist",
    "oenomel",
    "onychitis",
    "organizational",
    "osteosis",
    "outtrail",
    "overgrow",
    "overtart",
    "Paguridea",
    "pancyclopedic",
    "parallepipedous",
    "pashaship",
    "pedagogy",
    "penworker",
    "peritrich",
    "Phalangerinae",
    "phonographically",
    "physiophilosophical",
    "pinguid",
    "plastics",
    "plural",
    "polyaxial",
    "Popian",
    "potator",
    "precoloration",
    "preinform",
    "presphenoid",
    "probeer",
    "pronunciative",
    "protopoditic",
    "pseudospherical",
    "pumpkinify",
    "pyrophosphate",
    "quink",
    "ramhood",
    "reassociation",
    "recurve",
    "reheater",
    "repand",
    "respue",
    "revokement",
    "rimose",
    "rosoli",
    "Sabaist",
    "Salvia",
    "sauqui",
    "schoolkeeping",
    "scrod",
    "selaginellaceous",
    "semiserious",
    "serrate",
    "sheetwork",
    "sickeningly",
    "Siphoneae",
    "sleighty",
    "snobbism",
    "songstress",
    "spectator",
    "spitish",
    "squaremouth",
    "stearin",
    "stockproof",
    "stromboid",
    "subequality",
    "subtrapezoidal",
    "Sunday",
    "superstrong",
    "sweeten",
    "syntone",
    "tangence",
    "teachability",
    "tenontophyma",
    "Teutomania",
    "theurgic",
    "thyreoprotein",
    "tobaccoism",
    "totty",
    "transmarginal",
    "trichromat",
    "trochanteric",
    "tumefacient",
    "typographical",
    "unaldermanly",
    "unbowed",
    "unconditional",
    "underborne",
    "undexterously",
    "uneviscerated",
    "ungalling",
    "unimbowered",
    "unlikelihood",
    "unofficiousness",
    "unproficiently",
    "unride",
    "unsnaggled",
    "untenanted",
    "unwillingness",
    "urinology",
    "vapored",
    "vermiculite",
    "viremic",
    "waeg",
    "Wazir",
    "whistly",
    "woadman",
    "xenium",
    "zanella"
  };


  puts("Testing memory pool functions:\n");

  for (i = 0; i < (int)(sizeof(words) / sizeof(words[0])); i ++)
  {
    if ((strings[i] = hcPoolGetString(pool, words[i])) == NULL)
    {
      printf("FAILED getting word string #%d ('%s')\n", i + 1, words[i]);
      return (0);
    }
  }

  printf("PASSED adding %d word strings to pool.\n", i);

  for (i = 0; i < (int)(sizeof(words) / sizeof(words[0])); i ++)
  {
    if (strcmp(strings[i], words[i]))
    {
      printf("FAILED verifying word string #%d (got '%s', expected '%s')\n", i + 1, strings[i], words[i]);
      return (0);
    }

    for (j = i + 1; j < (int)(sizeof(words) / sizeof(words[0])); j ++)
    {
      if (strings[i] == strings[j])
      {
	printf("FAILED word string #%d ('%s') and #%d ('%s') have the same pointer.\n", i + 1, strings[i], j + 1, strings[j]);
	return (0);
      }
    }
  }

  printf("PASSED verifying %d word strings in pool.\n", i);

  for (i = 0; i < (int)(sizeof(words) / sizeof(words[0])); i ++)
  {
    if ((temp = hcPoolGetString(pool, words[i])) == NULL)
    {
      printf("FAILED getting word string #%d ('%s')\n", i + 1, words[i]);
      return (0);
    }

    if (temp != strings[i])
    {
      printf("FAILED word string #%d ('%s') did not reuse pointer.\n", i + 1, words[i]);
      return (0);
    }
  }

  printf("PASSED string reuse checks of %d word strings in pool.\n\n", i);

  return (1);
}
