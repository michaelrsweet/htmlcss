/*
 * Unit test program for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 *
 * Usage:
 *
 *   ./testhtmlcss [--all] [--css] [--font] [--html] [files]
 */

/*
 * Include necessary headers...
 */

#include "htmlcss.h"
#include "css-private.h"
#include "image.h"
#include "sha3.h"


/*
 * Local functions...
 */

static int	error_cb(void *ctx, const char *message, int linenum);
static int	test_pool_functions(hc_pool_t *pool);
static int	test_sha3_functions(void);


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
  hc_font_t	*font;			/* Font */
  hc_node_t	*node,			/* Current node */
		*next;			/* Next node */
  int		level;			/* Indentation level */
  _hc_rule_t	*rule;			/* CSS rule */
  _hc_css_sel_t	*sel;			/* CSS selector */
  size_t	pindex,			/* Property index */
		pcount;			/* Property count */
  const char	*pname,			/* Property name */
		*pvalue;		/* Property value */
  int		test_all = 0,		/* Test everything? */
		show_css = 0,		/* Show flattened CSS? */
		show_font_cache = 0,	/* Show cached font information? */
		show_html = 0;		/* Show HTML? */


 /*
  * Initialize memory pool, stylesheet, and HTML document objects...
  */

  pool = hcPoolNew();
  hcPoolSetErrorCallback(pool, error_cb, NULL);

  css = hcCSSNew(pool);
  hcCSSImportDefault(css);

  html = hcHTMLNew(pool, css);

 /*
  * Parse command-line...
  */

  for (i = 1; i < argc; i ++)
  {
    if (!strcmp(argv[i], "--all"))
    {
      test_all = 1;
      continue;
    }
    else if (!strcmp(argv[i], "--css"))
    {
      show_css = 1;
      continue;
    }
    else if (!strcmp(argv[i], "--font"))
    {
      show_font_cache = 1;
      continue;
    }
    else if (!strcmp(argv[i], "--html"))
    {
      show_html = 1;
      continue;
    }
    else if (argv[i][0] == '-')
    {
      puts("Usage: ./testhtmlcss [--all] [--css] [--font] [--html] [files]");
      return (1);
    }

    hc_file_t *file = hcFileNewURL(pool, argv[i], NULL);

    if ((ext = strrchr(argv[i], '.')) == NULL)
      ext = ".html";

    if (!strcmp(ext, ".otc") || !strcmp(ext, ".otf") || !strcmp(ext, ".ttc") || !strcmp(ext, ".ttf"))
    {
      show_font_cache = 0;

      if ((font = hcFontNew(pool, file, 0)) != NULL)
      {
        hc_rect_t	extents;	/* Extents of text */

        printf("%s:\n    numFonts=%u\n    copyright=\"%s\"\n    family=\"%s\"\n    postscript_name=\"%s\"\n    version=\"%s\"\n    style=%d\n    weight=%d\n", argv[i], (unsigned)hcFontGetNumFonts(font), hcFontGetCopyright(font), hcFontGetFamily(font), hcFontGetPostScriptName(font), hcFontGetVersion(font), (int)hcFontGetStyle(font), hcFontGetWeight(font));

        hcFontComputeExtents(font, 10.0f, "Hello, world!", &extents);
        printf("    extents of \"Hello, world!\"=[%.3f %.3f %.3f %.3f]\n", extents.left, extents.bottom, extents.right, extents.top);

        hcFontComputeExtents(font, 10.0f, "0123456789", &extents);
        printf("    extents of \"0123456789\"=[%.3f %.3f %.3f %.3f]\n", extents.left, extents.bottom, extents.right, extents.top);

        if (hcFontGetNumFonts(font) > 1)
        {
          size_t	i;		/* Looping var */
          hc_font_t	*fontn;		/* Nth font */

          for (i = 1; i < hcFontGetNumFonts(font); i ++)
          {
            hcFileSeek(file, 0);

            if ((fontn = hcFontNew(pool, file, i)) != NULL)
	      printf("    postscript_name%d=\"%s\"\n    style%d=%d\n    weight%d=%d\n", (int)i, hcFontGetPostScriptName(fontn), (int)i, (int)hcFontGetStyle(fontn), (int)i, hcFontGetWeight(fontn));
            else
              printf("    UNABLE TO LOAD FONT #%d\n", (int)i);

            hcFontDelete(fontn);
          }
        }

        hcFontDelete(font);
      }
      else
        puts(hcPoolGetLastError(pool));
    }
    else if (!strcmp(ext, ".gif") || !strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg") || !strcmp(ext, ".png") || !strcmp(ext, ".svg") || !strcmp(ext, ".svgz"))
    {
      hc_image_t *image = hcImageNew(pool, file);
					/* Image */

      if (image)
      {
        hc_size_t size = hcImageGetSize(image);
					/* Image size */

        printf("%s: %dx%d pixels, %.1fx%.1f points (%s)\n", argv[i], hcImageGetWidth(image), hcImageGetHeight(image), size.width, size.height, hcImageGetFormat(image));
        hcImageDelete(image);
      }
      else
        printf("%s: Unable to open.\n", argv[i]);
    }
    else if (!strcmp(ext, ".css"))
      hcCSSImport(css, file);
    else
      hcHTMLImport(html, file);

    hcFileDelete(file);
  }

  if (test_all || argc == 1)
  {
   /*
    * Test hash functions...
    */

    if (!test_sha3_functions())
      return (1);

   /*
    * Test string pool functions...
    */

    if (!test_pool_functions(pool))
      return (1);
  }

  if (show_html)
  {
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
	size_t idx, count = hcNodeAttrGetCount(node);
	const hc_dict_t *props = hcNodeComputeCSSProperties(node, HC_COMPUTE_BASE);

	printf("<%s", hcElements[element]);
	for (idx = 0; idx < count; idx ++)
	{
	  const char *name, *value = hcNodeAttrGetIndexNameValue(node, idx, &name);
	  printf(" %s=\"%s\"", name, value);
	}
	printf("> {%p", (void *)props);
	for (idx = 0, count = hcDictGetCount(props); idx < count; idx ++)
	{
	  const char *key, *value = hcDictGetIndexKeyValue(props, idx, &key);

	  printf(" %s: %s;", key, value);
	}
	hc_text_t textinfo;
	if (hcNodeComputeCSSText(node, HC_COMPUTE_BASE, &textinfo))
	{
	  fputs("} {", stdout);
	  printf(" font=%p(%s)", (void *)textinfo.font, hcFontGetPostScriptName(textinfo.font));
	  printf(" font-family: %s;", textinfo.font_family);
	  printf(" font-size: %f;", textinfo.font_size);
	  printf(" font-stretch: %d;", textinfo.font_stretch);
	  printf(" font-style: %d;", textinfo.font_style);
	  printf(" font-variant: %d;", textinfo.font_variant);
	  printf(" font-weight: %d;", textinfo.font_weight);
	  printf(" line-height: %f;", textinfo.line_height);
	}
	puts("}");
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
  }

  if (show_css)
  {
    puts("Flattened CSS:\n");

    for (i = 0; i < (int)css->all_rules.num_rules; i ++)
    {
      int			j;		/* Looping var */
      int			num_sels = 0;	/* Number of selectors */
      _hc_css_sel_t	*sels[100];	/* Selectors */

      rule = css->all_rules.rules[i];

      if (!rule->sel)
	continue;

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

	for (j = 0, stmt = sel->stmts; j < (int)sel->num_stmts; j ++, stmt ++)
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

      putchar('<');
      for (j = 0; j < HC_SHA3_256_SIZE; j ++)
	printf("%02X", rule->hash[j]);
      putchar('>');
      putchar(' ');
      puts("{");

      for (pindex = 0, pcount = hcDictGetCount(rule->props); pindex < pcount; pindex ++)
      {
	pvalue = hcDictGetIndexKeyValue(rule->props, pindex, &pname);
	printf("  %s: %s;\n", pname, pvalue);
      }

      puts("}");
    }
  }

  if (show_font_cache)
  {
    size_t	findex,			/* Font index */
		num_fonts;		/* Number of cached fonts */
    hc_font_t	*font;			/* Cached font */

    puts("\nCached Fonts:");

    for (findex = 0, num_fonts = hcFontGetCachedCount(pool); findex < num_fonts; findex ++)
    {
      if ((font = hcFontGetCached(pool, findex)) != NULL)
      {
        hc_font_style_t style = hcFontGetStyle(font);

        printf("  \"%s\" (%s) %s %d\n", hcFontGetFamily(font), hcFontGetPostScriptName(font), style == HC_FONT_STYLE_NORMAL ? "normal" : style == HC_FONT_STYLE_ITALIC ? "italic" : "oblique", hcFontGetWeight(font));
      }
    }
  }

  hcHTMLDelete(html);
  hcCSSDelete(css);
  hcPoolDelete(pool);

  return (0);
}


/*
 * 'error_cb()' - Error callback for HTMLCSS...
 */

static int				/* O - 1 to continue */
error_cb(void       *ctx,		/* I - User data (not used) */
         const char *message,		/* I - Message string (not used) */
         int        linenum)		/* I - Line number (not used) */
{
  (void)ctx;
  (void)message;
  (void)linenum;

  return (1);
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


/*
 * 'test_sha3_functions()' - Test SHA3 hash functions...
 */

static int				/* O - 1 on success, 0 on failure */
test_sha3_functions(void)
{
  int		i,			/* Looping var */
		status = 1;		/* Return status */
  hc_sha3_t	ctx;			/* Hash context */
  unsigned char	hash[HC_SHA3_512_SIZE];	/* Hash buffer */
  static const unsigned char testin[] =
  {					/* Test input for hashing */
    0x3A, 0x3A, 0x81, 0x9C, 0x48, 0xEF, 0xDE, 0x2A,
    0xD9, 0x14, 0xFB, 0xF0, 0x0E, 0x18, 0xAB, 0x6B,
    0xC4, 0xF1, 0x45, 0x13, 0xAB, 0x27, 0xD0, 0xC1,
    0x78, 0xA1, 0x88, 0xB6, 0x14, 0x31, 0xE7, 0xF5,
    0x62, 0x3C, 0xB6, 0x6B, 0x23, 0x34, 0x67, 0x75,
    0xD3, 0x86, 0xB5, 0x0E, 0x98, 0x2C, 0x49, 0x3A,
    0xDB, 0xBF, 0xC5, 0x4B, 0x9A, 0x3C, 0xD3, 0x83,
    0x38, 0x23, 0x36, 0xA1, 0xA0, 0xB2, 0x15, 0x0A,
    0x15, 0x35, 0x8F, 0x33, 0x6D, 0x03, 0xAE, 0x18,
    0xF6, 0x66, 0xC7, 0x57, 0x3D, 0x55, 0xC4, 0xFD,
    0x18, 0x1C, 0x29, 0xE6, 0xCC, 0xFD, 0xE6, 0x3E,
    0xA3, 0x5F, 0x0A, 0xDF, 0x58, 0x85, 0xCF, 0xC0,
    0xA3, 0xD8, 0x4A, 0x2B, 0x2E, 0x4D, 0xD2, 0x44,
    0x96, 0xDB, 0x78, 0x9E, 0x66, 0x31, 0x70, 0xCE,
    0xF7, 0x47, 0x98, 0xAA, 0x1B, 0xBC, 0xD4, 0x57,
    0x4E, 0xA0, 0xBB, 0xA4, 0x04, 0x89, 0xD7, 0x64,
    0xB2, 0xF8, 0x3A, 0xAD, 0xC6, 0x6B, 0x14, 0x8B,
    0x4A, 0x0C, 0xD9, 0x52, 0x46, 0xC1, 0x27, 0xD5,
    0x87, 0x1C, 0x4F, 0x11, 0x41, 0x86, 0x90, 0xA5,
    0xDD, 0xF0, 0x12, 0x46, 0xA0, 0xC8, 0x0A, 0x43,
    0xC7, 0x00, 0x88, 0xB6, 0x18, 0x36, 0x39, 0xDC,
    0xFD, 0xA4, 0x12, 0x5B, 0xD1, 0x13, 0xA8, 0xF4,
    0x9E, 0xE2, 0x3E, 0xD3, 0x06, 0xFA, 0xAC, 0x57,
    0x6C, 0x3F, 0xB0, 0xC1, 0xE2, 0x56, 0x67, 0x1D,
    0x81, 0x7F, 0xC2, 0x53, 0x4A, 0x52, 0xF5, 0xB4,
    0x39, 0xF7, 0x2E, 0x42, 0x4D, 0xE3, 0x76, 0xF4,
    0xC5, 0x65, 0xCC, 0xA8, 0x23, 0x07, 0xDD, 0x9E,
    0xF7, 0x6D, 0xA5, 0xB7, 0xC4, 0xEB, 0x7E, 0x08,
    0x51, 0x72, 0xE3, 0x28, 0x80, 0x7C, 0x02, 0xD0,
    0x11, 0xFF, 0xBF, 0x33, 0x78, 0x53, 0x78, 0xD7,
    0x9D, 0xC2, 0x66, 0xF6, 0xA5, 0xBE, 0x6B, 0xB0,
    0xE4, 0xA9, 0x2E, 0xCE, 0xEB, 0xAE, 0xB1
  };
  static const unsigned char testhash[] =
  {					/* Expected SHA3-512 hash */
    0x6E, 0x8B, 0x8B, 0xD1, 0x95, 0xBD, 0xD5, 0x60,
    0x68, 0x9A, 0xF2, 0x34, 0x8B, 0xDC, 0x74, 0xAB,
    0x7C, 0xD0, 0x5E, 0xD8, 0xB9, 0xA5, 0x77, 0x11,
    0xE9, 0xBE, 0x71, 0xE9, 0x72, 0x6F, 0xDA, 0x45,
    0x91, 0xFE, 0xE1, 0x22, 0x05, 0xED, 0xAC, 0xAF,
    0x82, 0xFF, 0xBB, 0xAF, 0x16, 0xDF, 0xF9, 0xE7,
    0x02, 0xA7, 0x08, 0x86, 0x20, 0x80, 0x16, 0x6C,
    0x2F, 0xF6, 0xBA, 0x37, 0x9B, 0xC7, 0xFF, 0xC2
  };


  puts("Testing SHA3 hash functions:\n");

  fputs("hcSHA3Init/Update/Final: ", stdout);

  hcSHA3Init(&ctx);
  hcSHA3Update(&ctx, testin, sizeof(testin));
  hcSHA3Final(&ctx, hash, sizeof(hash));

  if (memcmp(hash, testhash, sizeof(testhash)))
  {
    fputs("FAIL (", stdout);
    status = 0;
  }
  else
    fputs("PASS (", stdout);

  for (i = 0; i < HC_SHA3_512_SIZE; i ++)
    printf("%02X", hash[i]);
  puts(")\n");

  return (status);
}

