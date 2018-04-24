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

static char	*css_read_token(_css_file_t *f, char *buffer, size_t bufsize);
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


/*
 * 'css_read_token()' - Read a token from the CSS file.
 */

static char *				/* O - Token or `NULL` on EOF */
css_read_token(_css_file_t *f,		/* I - CSS file */
               char        *buffer,	/* I - Buffer */
               size_t      bufsize)	/* I - Size of buffer */
{
  int	ch;				/* Current character */
  char	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */
  static const char *reserved = "{}[])";
					/* Reserved characters */

  bufptr = buffer;
  bufend = buffer + bufsize - 1;

  while ((ch = _htmlcssFileGetc(&f->file)) != EOF)
  {
    if (!isspace(ch & 255))
      break;
  }

  if (ch == EOF)
    return (NULL);

  if (strchr(reserved, ch))
  {
   /*
    * Single character token...
    */

    *bufptr++ = (char)ch;
  }
  else if (ch == '\'' || ch == '\"')
  {
   /*
    * Quoted string...
    */

    int quote = ch;			/* Quote character */

    while ((ch = _htmlcssFileGetc(&f->file)) != EOF)
    {
      if (ch == quote)
	break;

      if (bufptr < bufend)
        *bufptr++ = (char)ch;
      else
        break;
    }
  }
  else
  {
   /*
    * Identifier or number...
    */

    do
    {
      if (isspace(ch & 255) || strchr(reserved, ch))
	break;

      if (bufptr < bufend)
	*bufptr++ = (char)ch;
      else
        break;

      if (ch == '(')
        break;
    }
    while ((ch = _htmlcssFileGetc(&f->file)) != EOF);

    if (ch != EOF && !isspace(ch & 255) && ch != '(')
      _htmlcssFileUngetc(ch, &f->file);
  }

  *bufptr = '\0';

  return (buffer);
}
