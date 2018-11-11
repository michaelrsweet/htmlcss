/*
 * CSS import functions for HTMLCSS library.
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


/*
 * Local types...
 */

typedef struct _hc_css_file_s
{
  hc_css_t	*css;			/* Stylesheet */
  _hc_file_t	file;			/* File information */
} _hc_css_file_t;

typedef enum _hc_type_e
{
  _HC_TYPE_ERROR,			/* Error */
  _HC_TYPE_RESERVED,			/* Reserved character(s) */
  _HC_TYPE_STRING,			/* Unquoted string */
  _HC_TYPE_QSTRING,			/* Quoted string */
  _HC_TYPE_NUMBER			/* Number */
} _hc_type_t;


/*
 * Local functions...
 */

static void		hc_add_rule(hc_css_t *css, _hc_css_sel_t *sel, hc_dict_t *props);
static void		hc_add_selstmt(_hc_css_sel_t *sel, _hc_match_t match, const char *name, const char *value);
static _hc_css_sel_t	*hc_new_sel(_hc_css_sel_t *prev, hc_element_t element);
static char		*hc_read(_hc_css_file_t *f, _hc_type_t *type, char *buffer, size_t bufsize);


/*
 * 'hcCSSImport()' - Import CSS definitions from a URL, file, or string.
 */

int					/* O - 1 on success, 0 on error */
hcCSSImport(hc_css_t   *css,		/* I - Stylesheet */
            const char *url,		/* I - URL or filename */
            FILE       *fp,		/* I - File pointer or `NULL` */
            const char *s)		/* I - String or `NULL` */
{
  int		ret = 1;		/* Return value */
  _hc_css_file_t f;			/* Local file info */
  char		buffer[256];		/* Current value */
  _hc_type_t	type;			/* Value type */
  static const char * const types[] =	/* Types */
  {
    "ERROR",
    "RESERVED",
    "STRING",
    "QSTRING",
    "NUMBER"
  };


  printf("hcCSSImport(css=%p, url=\"%s\", fp=%p, s=\"%s\")\n", css, url, fp, s);

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
      _hcError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    if ((f.file.fp = fopen(filename, "rb")) == NULL)
    {
      _hcError(css->error_cb, css->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    printf("Reading CSS from \"%s\"...\n", filename);
  }

/*

Strategy for reading CSS:

1. Read selector or @rule up to the opening brace.
2a. For @rule: read selectors up to the opening brace, then read property:value; pairs until the closing brace.
2b. For selector, read property:value; pairs up to the closing brace.
3. Back to 1.

*/


  while (hc_read(&f, &type, buffer, sizeof(buffer)))
  {
    printf("CSS: %s %s\n", types[type], buffer);
  }

  if (f.file.fp != fp)
    fclose(f.file.fp);

  return (ret);
}


/*
 * 'hc_add_rule()' - Add a rule set to a stylesheet.
 */

static void
hc_add_rule(hc_css_t       *css,	/* I - Stylesheet */
	    _hc_css_sel_t *sel,		/* I - Selectors */
	    hc_dict_t     *props)	/* I - Properties */
{
}


/*
 * 'hc_add_selstmt()' - Add a matching statement to a selector.
 */

static void
hc_add_selstmt(_hc_css_sel_t   *sel,	/* I - Selector */
	       _hc_match_t match,	/* I - Matching statement type */
	       const char   *name,	/* I - Name, if any */
	       const char   *value)	/* I - Value, if any */
{
}


/*
 * 'hc_new_sel()' - Create a new selector.
 */

static _hc_css_sel_t *			/* O - New selector */
hc_new_sel(_hc_css_sel_t *prev,		/* I - Previous selector in list */
	   hc_element_t element)	/* I - Element */
{
  _hc_css_sel_t	*sel;			/* New selector */


  if ((sel = (_hc_css_sel_t *)calloc(1, sizeof(_hc_css_sel_t))) != NULL)
  {
    sel->prev    = prev;
    sel->element = element;
  }

  return (sel);
}



/*
 * 'hc_read_token()' - Read a token from the CSS file.
 */

static char *				/* O - Token or `NULL` on EOF */
hc_read(_hc_css_file_t *f,		/* I - CSS file */
	_hc_type_t     *type,		/* O - Tokem type */
	char           *buffer,		/* I - Buffer */
	size_t         bufsize)		/* I - Size of buffer */
{
  int	ch;				/* Current character */
  char	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */
  static const char *reserved = ",:;{}[])";
					/* Reserved characters */

  bufptr = buffer;
  bufend = buffer + bufsize - 1;
  *type  = _HC_TYPE_ERROR;

  for (;;)
  {
    while ((ch = _hcFileGetc(&f->file)) != EOF)
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
      *type     = _HC_TYPE_RESERVED;

      if (ch == ':')
      {
        if ((ch = _hcFileGetc(&f->file)) == ':')
          *bufptr++ = (char)ch;
        else
	  _hcFileUngetc(ch, &f->file);
      }
    }
    else if (ch == '\'' || ch == '\"')
    {
     /*
      * Quoted string...
      */

      int quote = ch;			/* Quote character */

      while ((ch = _hcFileGetc(&f->file)) != EOF)
      {
	if (ch == quote)
	  break;

	if (bufptr < bufend)
	  *bufptr++ = (char)ch;
	else
	  break;
      }

      *type = _HC_TYPE_QSTRING;
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

	  while ((ch = _hcFileGetc(&f->file)) != EOF)
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

	if (ch == '(' || (ch == '-' && (bufptr - buffer) == 4 && !memcmp(buffer, "<!--", 4)) || (ch == '>' && (bufptr - buffer) == 3 && !memcmp(buffer, "-->", 3)))
	  break;
      }
      while ((ch = _hcFileGetc(&f->file)) != EOF);

      if (bufptr == buffer)
        continue;
      else if (ch != EOF && !isspace(ch & 255) && ch != '(')
	_hcFileUngetc(ch, &f->file);

      if (isdigit(*buffer & 255) || (*buffer == '.' && isdigit(buffer[1] & 255)))
        *type = _HC_TYPE_NUMBER;
      else
        *type = _HC_TYPE_STRING;
    }

    *bufptr = '\0';

    return (buffer);
  }
}
