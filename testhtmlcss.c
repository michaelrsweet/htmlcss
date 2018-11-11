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


  pool = hcPoolNew();
  css  = hcCSSNew(pool);
  html = hcHTMLNew(pool, css);

  for (i = 1; i < argc; i ++)
  {
    if ((ext = strrchr(argv[i], '.')) == NULL || strcmp(ext, ".css"))
      hcHTMLLoad(html, argv[i], NULL);
    else
      hcCSSImport(css, argv[i], NULL, NULL);
  }

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

  hcHTMLDelete(html);
  hcCSSDelete(css);
  hcPoolDelete(pool);

  return (0);
}
