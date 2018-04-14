/*
 * CSS import functions for HTMLCSS library.
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

#  include "css-private.h"


/*
 * 'cssImport()' - Import CSS definitions from a named file.
 */

int					/* O - 1 on success, 0 on error */
cssImport(css_t      *css,		/* I - Stylesheet */
          const char *filename,		/* I - Filename */
          const char *base)		/* I - Base directory */
{
  FILE	*fp;				/* File pointer */
  int	ret;				/* Return value */


  if (!css || !filename)
  {
    errno = EINVAL;
    return (0);
  }

  if ((fp = fopen(filename, "r")) == NULL)
    return (0);

  ret = cssImportFile(css, fp, base);

  fclose(fp);

  return (ret);
}


/*
 * 'cssImportFile()' - Import CSS definitions from a file.
 */

int					/* O - 1 on success, 0 on error */
cssImportFile(css_t      *css,		/* I - Stylesheet */
              FILE       *fp,		/* I - File pointer */
              const char *base)		/* I - Base directory */
{
  if (!css || !fp)
  {
    errno = EINVAL;
    return (0);
  }


  return (0);
}


/*
 * 'cssImportString()' - Import CSS definitions from a string.
 */

int					/* O - 1 on success, 0 on error */
cssImportString(css_t      *css,	/* I - Stylesheet */
                const char *s,		/* I - String */
                const char *base)	/* I - Base directory */
{
  if (!css || !s)
  {
    errno = EINVAL;
    return (0);
  }

  return (0);
}
