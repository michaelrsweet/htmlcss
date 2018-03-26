/*
 * Unit test program for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
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
  html_t	*html;			/* HTML document */
  html_node_t	*node,			/* Current node */
		*next;			/* Next node */
  int		level;			/* Indentation level */


  for (i = 1; i < argc; i ++)
  {
    html = htmlNew(NULL);

    if (htmlLoad(html, argv[i], NULL))
    {
      for (node = htmlGetRootNode(html), level = 0; node; node = next)
      {
        html_element_t element = htmlGetElement(node);

        printf("%*s", level * 2, "");

        if (element == HTML_ELEMENT_STRING)
        {
          const char *s = htmlGetString(node);

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
        else if (element == HTML_ELEMENT_COMMENT)
          printf("<!-- %s -->\n", htmlGetComment(node));
        else if (element == HTML_ELEMENT_DOCTYPE)
          printf("<!DOCTYPE %s>\n", htmlGetAttr(node, ""));
        else
        {
          int idx, count = htmlGetAttrCount(node);

          printf("<%s", htmlElements[element]);
          for (idx = 0; idx < count; idx ++)
          {
            const char *name, *value = htmlGetAttrIndex(node, idx, &name);
            printf(" %s=\"%s\"", name, value);
          }
          puts(">");
        }

        if ((next = htmlGetFirstChildNode(node)) != NULL)
          level += 2;
        else
        {
          if ((next = htmlGetNextSiblingNode(node)) == NULL)
          {
            next = htmlGetParentNode(node);
            level -= 2;

            while (next && !htmlGetNextSiblingNode(next))
            {
              next = htmlGetParentNode(next);
              level -= 2;
            }

            if (next)
              next = htmlGetNextSiblingNode(next);
          }
        }
      }
    }

    htmlDelete(html);
  }

  return (0);
}
