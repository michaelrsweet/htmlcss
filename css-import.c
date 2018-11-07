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

typedef enum _css_type_e
{
  _CSS_TYPE_ERROR,
  _CSS_TYPE_RESERVED,
  _CSS_TYPE_IDENT,
  _CSS_TYPE_STRING,
  _CSS_TYPE_NUMBER
} _css_type_t;


/*
 * Local functions...
 */

static char	*css_read(_css_file_t *f, _css_type_t *type, char *buffer, size_t bufsize);


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
  char		buffer[256];		/* Current value */
  _css_type_t	type;			/* Value type */
  static const char * const types[] =	/* Types */
  {
    "ERROR",
    "RESERVED",
    "IDENT",
    "STRING",
    "NUMBER"
  };


  printf("cssImport(css=%p, url=\"%s\", fp=%p, s=\"%s\")\n", css, url, fp, s);

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

    if (!strchr(url, ':'))
    {
      strncpy(filename, url, sizeof(filename) - 1);
      filename[sizeof(filename) - 1] = '\0';
    }
    else if (!(css->url_cb)(url, filename, sizeof(filename), css->url_ctx))
    {
      _htmlcssError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    if ((f.file.fp = fopen(filename, "rb")) == NULL)
    {
      _htmlcssError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    printf("Reading CSS from \"%s\"...\n", filename);
  }

  while (css_read(&f, &type, buffer, sizeof(buffer)))
  {
    printf("CSS: %s %s\n", types[type], buffer);
  }

  if (f.file.fp != fp)
    fclose(f.file.fp);

  return (ret);
}


/*
 * 'css_read_token()' - Read a token from the CSS file.
 */

static char *				/* O - Token or `NULL` on EOF */
css_read(_css_file_t *f,		/* I - CSS file */
	 _css_type_t *type,		/* O - Tokem type */
	 char        *buffer,		/* I - Buffer */
	 size_t      bufsize)		/* I - Size of buffer */
{
  int	ch;				/* Current character */
  char	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */
  static const char *reserved = ":;{}[])";
					/* Reserved characters */

  bufptr = buffer;
  bufend = buffer + bufsize - 1;
  *type  = _CSS_TYPE_ERROR;

  for (;;)
  {
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
      *type     = _CSS_TYPE_RESERVED;

      if (ch == ':')
      {
        if ((ch = _htmlcssFileGetc(&f->file)) == ':')
          *bufptr++ = (char)ch;
        else
	  _htmlcssFileUngetc(ch, &f->file);
      }
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

      *type = _CSS_TYPE_STRING;
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
	else if (ch == '*' && bufptr > buffer && bufptr[-1] == '/')
	{
	 /*
	  * Skip C-style comment...
	  */

	  int	asterisk = 0;		/* Did we see the closing asterisk? */

	  bufptr --;

	  while ((ch = _htmlcssFileGetc(&f->file)) != EOF)
	  {
	    if (ch == '/' && asterisk)
	      break;
	    else
	      asterisk = ch == '*';
	  }

	  if (bufptr == buffer)
	  {
	    if (ch == EOF)
	      return (NULL);
	    else
	      break;
	  }
	}
	else if (bufptr < bufend)
	  *bufptr++ = (char)ch;
	else
	  break;

	if (ch == '(')
	  break;
      }
      while ((ch = _htmlcssFileGetc(&f->file)) != EOF);

      if (bufptr == buffer)
        continue;
      else if (ch != EOF && !isspace(ch & 255) && ch != '(')
	_htmlcssFileUngetc(ch, &f->file);

      if (isdigit(*buffer & 255) || (*buffer == '.' && isdigit(buffer[1] & 255)))
        *type = _CSS_TYPE_NUMBER;
      else
        *type = _CSS_TYPE_IDENT;
    }

    *bufptr = '\0';

    return (buffer);
  }
}
