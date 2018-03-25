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

  }

  return (0);
}
