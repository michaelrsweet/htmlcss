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
 * Local types...
 */

typedef struct _css_file_s
{
  css_t			*css;		/* Stylesheet */
  _htmlcss_file_t	file;		/* File information */
} _css_file_t;


/*
 * Local functions...
 */


/* TBD */


/*
 * 'cssImport()' - Import CSS definitions from a URL, file, or string.
 */

int					/* O - 1 on success, 0 on error */
cssImport(css_t      *css,		/* I - Stylesheet */
          const char *url,		/* I - URL or filename */
          FILE       *fp,		/* I - File pointer or `NULL` */
          const char *s)		/* I - String or `NULL` */
{
  int		ret = 1;		/* Return value */
  _css_file_t	f;			/* Local file info */


  if (!css || (!url && !fp && !s))
  {
    errno = EINVAL;
    return (0);
  }

  f.css          = css;
  f.file.url     = url;
  f.file.fp      = fp;
  f.file.s       = s;
  f.file.sptr    = s;
  f.file.linenum = 1;

  if (url && !fp && !s)
  {
    char	filename[1024];		/* Local filename buffer */

    if (!(css->url_cb)(url, filename, sizeof(filename), css->url_ctx))
    {
      _htmlcssError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    if ((f.file.fp = fopen(filename, "rb")) == NULL)
    {
      _htmlcssError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }
  }

//  ret = cssImportFile(css, fp, base);

  if (f.file.fp != fp)
    fclose(f.file.fp);

  return (ret);
}
