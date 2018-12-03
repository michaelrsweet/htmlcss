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
#  include "default-css.h"


/*
 * Local types...
 */

typedef struct _hc_css_file_s
{
  hc_css_t	*css;			/* Stylesheet */
  _hc_file_t	file;			/* File information */
} _hc_css_file_t;

typedef enum _hc_logop_e		/* Logical operation */
{
  _HC_LOGOP_NONE,
  _HC_LOGOP_OR,
  _HC_LOGOP_AND
} _hc_logop_t;

typedef enum _hc_type_e			/* Token type */
{
  _HC_TYPE_ERROR,			/* Error */
  _HC_TYPE_RESERVED,			/* Reserved character(s) */
  _HC_TYPE_STRING,			/* Unquoted string */
  _HC_TYPE_QSTRING,			/* Quoted string */
  _HC_TYPE_NUMBER			/* Number */
} _hc_type_t;


/*
 * Local globals...
 */

static const char * const types[] =	/* Types */
{
  "ERROR",
  "RESERVED",
  "STRING",
  "QSTRING",
  "NUMBER"
};


/*
 * Local functions...
 */

static void		hc_add_rule(hc_css_t *css, _hc_css_sel_t *sel, hc_dict_t *props);
static void		hc_add_selstmt(hc_css_t *css, _hc_css_sel_t *sel, _hc_match_t match, const char *name, const char *value);
static int		hc_eval_media(_hc_css_file_t *f, _hc_type_t *type, char *buffer, size_t bufsize);
static _hc_css_sel_t	*hc_new_sel(_hc_css_sel_t *prev, hc_element_t element, _hc_relation_t rel);
static char		*hc_read(_hc_css_file_t *f, _hc_type_t *type, char *buffer, size_t bufsize);
static hc_dict_t	*hc_read_props(_hc_css_file_t *f, hc_dict_t *props);
static _hc_css_sel_t	*hc_read_sel(_hc_css_file_t *f, _hc_type_t *type, char *buffer, size_t bufsize);
static char		*hc_read_value(_hc_css_file_t *f, char *buffer, size_t bufsize);


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
  int		skip = 0;		/* Skip current definitions */
  int		in_media = 0;		/* In a media grouping? */
  int		num_sels = 0;		/* Number of selectors */
  _hc_css_sel_t	*sels[1000];		/* Selectors */


  _HC_DEBUG("hcCSSImport(css=%p, url=\"%s\", fp=%p, s=\"%s\")\n", css, url, fp, s);

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

    _HC_DEBUG("Reading CSS from \"%s\"...\n", filename);
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
    _HC_DEBUG("%s:%d: %s %s\n", f.file.url, f.file.linenum, types[type], buffer);

    if (!strcmp(buffer, "@import"))
    {
      int	in_url = 0;		/* In a URL? */
      char	path[256] = "";		/* Path to import */

      while (hc_read(&f, &type, buffer, sizeof(buffer)))
      {
        if (type == _HC_TYPE_QSTRING)
        {
          strncpy(path, buffer, sizeof(path) - 1);
          path[sizeof(path) - 1] = '\0';
          if (!in_url)
            break;
        }
        else if (type == _HC_TYPE_STRING && !strcmp(buffer, "url("))
          in_url = 1;
        else if (type == _HC_TYPE_RESERVED && !strcmp(buffer, ")") && in_url)
          break;
        else
        {
	  _hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unexpected %s token seen.", buffer);
	  ret = 0;
	  break;
        }
      }

      if (!path[0])
        break;

      if (hc_eval_media(&f, &type, buffer, sizeof(buffer)))
      {
        if (!hcCSSImport(css, path, NULL, NULL))
        {
          ret = 0;
          break;
        }
      }

      if (strcmp(buffer, ";"))
      {
	_hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unexpected %s token seen.", buffer);
	ret = 0;
	break;
      }
    }
    else if (!strcmp(buffer, "@media"))
    {
      if (in_media)
      {
	_hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unexpected nested @media.");
        break;
      }

      skip     = !hc_eval_media(&f, &type, buffer, sizeof(buffer));
      in_media = !strcmp(buffer, "{");
    }
    else if (buffer[0] == '@')
    {
      _hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unknown %s seen.", buffer);
      ret = 0;
      break;
    }
    else if (!strcmp(buffer, "}"))
    {
      if (in_media)
      {
        in_media = 0;
        skip     = 0;
      }
      else
      {
	_hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unexpected %s seen.", buffer);
	ret = 0;
	break;
      }
    }
    else if (num_sels < (int)(sizeof(sels) / sizeof(sels[0])))
    {
      if ((sels[num_sels] = hc_read_sel(&f, &type, buffer, sizeof(buffer))) == NULL)
      {
	ret = 0;
	break;
      }

      num_sels ++;

      if (!strcmp(buffer, "{"))
      {
	int		i;		/* Looping var */
        hc_dict_t	*props;		/* Properties */

        if ((props = hc_read_props(&f, NULL)) != NULL)
        {
	  if (skip)
	  {
	    _HC_DEBUG("%s:%d: Skipping %d properties for %d selectors.\n", f.file.url, f.file.linenum, (int)hcDictGetCount(props), num_sels);

	    for (i = 0; i < num_sels; i ++)
	      _hcCSSSelDelete(sels[i]);
	  }
	  else
	  {
	    _HC_DEBUG("%s:%d: Adding %d properties for %d selectors.\n", f.file.url, f.file.linenum, (int)hcDictGetCount(props), num_sels);

	    for (i = 0; i < num_sels; i ++)
	      hc_add_rule(css, sels[i], props);
	  }

	  hcDictDelete(props);
	  num_sels = 0;
        }
      }
      else if (strcmp(buffer, ","))
      {
	_hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Unexpected %s seen.", buffer);
	ret = 0;
	break;
      }
    }
    else
    {
      _hcError(css->error_cb, css->error_ctx, f.file.url, f.file.linenum, "Too many selectors seen.", buffer);
      ret = 0;
      break;
    }
  }

  if (f.file.fp != fp)
    fclose(f.file.fp);

  return (ret);
}


/*
 * 'hcCSSImportDefault()' - Import the default HTML stylesheet.
 */

int					/* O - 1 on success, 0 on error */
hcCSSImportDefault(hc_css_t *css)	/* I - Stylesheet */
{
  return (hcCSSImport(css, NULL, NULL, default_css));
}


/*
 * 'hc_add_rule()' - Add a rule set to a stylesheet.
 */

static void
hc_add_rule(hc_css_t      *css,		/* I - Stylesheet */
	    _hc_css_sel_t *sel,		/* I - Selectors */
	    hc_dict_t     *props)	/* I - Properties */
{
  _hc_rule_t	*rule;			/* New rule */


  if ((rule = realloc(css->rules[sel->element], (css->num_rules[sel->element] + 1) * sizeof(_hc_rule_t))) != NULL)
  {
    css->rules[sel->element] = rule;
    rule += css->num_rules[sel->element];
    css->num_rules[sel->element] ++;

    rule->sel   = sel;
    rule->props = hcDictCopy(props);
  }
  else
    _hcError(css->error_cb, css->error_ctx, NULL, 0, "Unable to allocate memory for selector rules.");
}


/*
 * 'hc_add_selstmt()' - Add a matching statement to a selector.
 */

static void
hc_add_selstmt(hc_css_t      *css,	/* I - Stylesheet */
               _hc_css_sel_t *sel,	/* I - Selector */
	       _hc_match_t   match,	/* I - Matching statement type */
	       const char    *name,	/* I - Name, if any */
	       const char    *value)	/* I - Value, if any */
{
  _hc_css_selstmt_t	*temp;		/* Current statement */


  if ((temp = realloc(sel->stmts, (sel->num_stmts + 1) * sizeof(_hc_css_selstmt_t))) != NULL)
  {
    sel->stmts = temp;
    temp += sel->num_stmts;
    sel->num_stmts ++;

    temp->match = match;
    temp->name  = hcPoolGetString(css->pool, name);
    temp->value = hcPoolGetString(css->pool, value);
  }
}


/*
 * 'hc_eval_media()' - Read and evaluate a media rule.
 */

static int				/* O - 1 if media rule matches, 0 otherwise */
hc_eval_media(_hc_css_file_t *f,	/* I - File to read from */
              _hc_type_t     *type,	/* O - Token type */
              char           *buffer,	/* I - Buffer */
              size_t         bufsize)	/* I - Size of buffer */
{
  int		media_result = -1;	/* Result of evaluation */
  int		media_current = -1;	/* Result of current expression */
  _hc_logop_t	logop = _HC_LOGOP_NONE;	/* Logical operation seen, if any */
  int		invert = 0;		/* Was "not" seen? */


  while (hc_read(f, type, buffer, bufsize))
  {
    if (*type == _HC_TYPE_RESERVED)
    {
      if (!strcmp(buffer, "{") || !strcmp(buffer, ";"))
        break;
      else if (!strcmp(buffer, "("))
      {
       /*
        * Skip subexpression...
        */

	while (hc_read(f, type, buffer, bufsize))
	{
	  if (*type == _HC_TYPE_RESERVED && !strcmp(buffer, ")"))
	    break;
	}

        if (*type != _HC_TYPE_RESERVED)
        {
	  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unexpected end-of-file.", buffer);
	  return (0);
        }

	media_current = 0;
	logop         = _HC_LOGOP_NONE;
	invert        = 0;
      }
      else if (!strcmp(buffer, ","))
      {
       /*
        * Separate expression...
        */

        if (media_current > 0 || media_result < 0)
          media_result = media_current;

        media_current = -1;
	logop         = _HC_LOGOP_NONE;
	invert        = 0;
      }
      else
        goto unexpected;
    }
    else if (*type == _HC_TYPE_STRING)
    {
      if (!strcmp(buffer, "and"))
      {
        if (media_current < 0 || logop != _HC_LOGOP_NONE)
        {
          goto unexpected;
        }
        else
	{
	  logop = _HC_LOGOP_AND;
	  continue;
	}
      }
      else if (!strcmp(buffer, "or"))
      {
        if (media_current < 0 || logop != _HC_LOGOP_NONE)
        {
          goto unexpected;
        }
        else
	{
	  logop = _HC_LOGOP_OR;
	  continue;
	}
      }
      else if ((!strcmp(buffer, f->css->media.type) || !strcmp(buffer, "all")) != invert)
      {
        if (media_current < 0 || logop != _HC_LOGOP_OR)
	  media_current = 0;

	logop  = _HC_LOGOP_NONE;
	invert = 0;
      }
      else if (logop != _HC_LOGOP_AND || media_current)
      {
        media_current = 1;
	logop         = _HC_LOGOP_NONE;
	invert        = 0;
      }
      else
      {
	logop  = _HC_LOGOP_NONE;
	invert = 0;
      }
    }
    else
      goto unexpected;
  }

  if (media_result < 0)
  {
   /*
    * Assign the overall result to the current result.  If the expression is empty,
    * evaluate to true...
    */

    if (media_current < 0)
      media_result = 1;			/* Empty expression */
    else
      media_result = media_current;	/* Single expression */
  }

  return (media_result);

 /*
  * If we get here we got something unexpected in the media matching expression...
  */

  unexpected:

  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unexpected token \"%s\" seen.", buffer);
  return (0);
}


/*
 * 'hc_new_sel()' - Create a new selector.
 */

static _hc_css_sel_t *			/* O - New selector */
hc_new_sel(_hc_css_sel_t  *prev,	/* I - Previous selector in list */
	   hc_element_t   element,	/* I - Element */
	   _hc_relation_t rel)		/* I - Relationship to previous */
{
  _hc_css_sel_t	*sel;			/* New selector */


  if ((sel = (_hc_css_sel_t *)calloc(1, sizeof(_hc_css_sel_t))) != NULL)
  {
    sel->prev     = prev;
    sel->element  = element;
    sel->relation = rel;
  }

  return (sel);
}



/*
 * 'hc_read()' - Read a string from the CSS file.
 */

static char *				/* O - String or `NULL` on EOF */
hc_read(_hc_css_file_t *f,		/* I - CSS file */
	_hc_type_t     *type,		/* O - String type */
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

        if (ch == '=')
        {
         /*
          * Comparison operator or FOO=...
          */

          if ((bufptr - buffer) == 1 || ((bufptr - buffer) == 2 && strchr("<>*^$|-", buffer[0])))
          {
            *type = _HC_TYPE_RESERVED;
            break;
          }

         /*
          * Return FOO and save "=" for later...
          */

          _hcFileUngetc('=', &f->file);
          bufptr --;
          break;
        }
      }
      while ((ch = _hcFileGetc(&f->file)) != EOF);

      if (bufptr == buffer)
        continue;
      else if (ch != EOF && !isspace(ch & 255) && ch != '(' && ch != '=')
	_hcFileUngetc(ch, &f->file);

      if (isdigit(*buffer & 255) || (*buffer == '.' && isdigit(buffer[1] & 255)))
        *type = _HC_TYPE_NUMBER;
      else if (!strcmp(buffer, "("))
        *type = _HC_TYPE_RESERVED;
      else if (*type == _HC_TYPE_ERROR)
        *type = _HC_TYPE_STRING;
    }

    *bufptr = '\0';

    return (buffer);
  }
}


/*
 * 'hc_read_props()' - Read properties in a group or string.
 */

static hc_dict_t *			/* O - Properties or `NULL` on error */
hc_read_props(_hc_css_file_t *f,	/* I - file to read from */
              hc_dict_t      *props)	/* I - Existing properties */
{
  _hc_type_t	type;			/* String type */
  char		buffer[256],		/* String buffer */
		name[256],		/* Name string */
		value[2048];		/* Value string */
  int		skip_remainder = 0;	/* Skip the remainder? */


  while (hc_read(f, &type, buffer, sizeof(buffer)))
  {
    _HC_DEBUG("%s:%d: (PROPS) %s %s\n", f->file.url, f->file.linenum, types[type], buffer);

    if (type == _HC_TYPE_RESERVED && !strcmp(buffer, "}"))
      break;
    else if (skip_remainder)
      continue;
    else if (type != _HC_TYPE_STRING)
    {
      _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unexpected %s seen.", buffer);
      skip_remainder = 1;
      continue;
    }

    strncpy(name, buffer, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    if (!hc_read(f, &type, buffer, sizeof(buffer)) || type != _HC_TYPE_RESERVED || strcmp(buffer, ":"))
    {
      _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing colon, saw %s instead.", buffer);
      skip_remainder = 1;
      continue;
    }

    if (!hc_read_value(f, value, sizeof(value)))
    {
      _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing property value.");
      break;
    }

    if (!hc_read(f, &type, buffer, sizeof(buffer)) || type != _HC_TYPE_RESERVED || strcmp(buffer, ";"))
    {
      _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing semi-colon, saw %s instead.", buffer);
      skip_remainder = 1;
      continue;
    }

    if (!props)
      props = hcDictNew(f->css->pool);

    _HC_DEBUG("%s:%d: (PROPS) Adding '%s: %s;'.\n", f->file.url, f->file.linenum, name, value);
    hcDictSetKeyValue(props, name, value);
  }

  _HC_DEBUG("%s:%d: (PROPS) Returning %d properties.\n", f->file.url, f->file.linenum, (int)hcDictGetCount(props));

  return (props);
}


/*
 * 'hc_read_sel()' - Read a CSS selector.
 *
 * On entry, "type" and "buffer" contain the initial selector string.  On exit
 * they contain the terminating token (typically "," or "{").
 */

static _hc_css_sel_t *			/* O  - Selector or `NULL` on error */
hc_read_sel(_hc_css_file_t *f,		/* I  - File to read from */
            _hc_type_t     *type,	/* IO - String type */
            char           *buffer,	/* I  - String buffer */
            size_t         bufsize)	/* I  - Size of string buffer */
{
  _hc_css_sel_t *sel = NULL;		/* Current selector */
  _hc_relation_t rel = _HC_RELATION_CHILD;
					/* Relationship with next selector */
  char		*ptr,			/* Pointer into buffer/value */
		name[256],		/* Attribute/pseudo-class name */
		value[256];		/* Value in selector */


  do
  {
    _HC_DEBUG("%s:%d: (SELECTOR) %s %s\n", f->file.url, f->file.linenum, types[*type], buffer);

    if (!strcmp(buffer, ":"))
    {
     /*
      * Match pseudo-class...
      */

      if (!hc_read(f, type, name, sizeof(name)) || *type != _HC_TYPE_STRING)
      {
        _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad pseudo-class.");
        goto error;
      }

      ptr = name + strlen(name) - 1;
      if (ptr > name && *ptr == '(')
      {
       /*
        * :NAME(VALUE) syntax...
        */

        *ptr = '\0';
        if (!hc_read(f, type, value, sizeof(value)) || *type != _HC_TYPE_STRING)
        {
	  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad value for ':%s'.", name);
	  goto error;
        }

        if (!hc_read(f, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED || strcmp(buffer, ")"))
        {
	  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad parenthesis after ':%s(%s'.", name, value);
	  goto error;
        }
      }
      else
        value[0] = '\0';

      if (!sel)
        sel = hc_new_sel(NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      hc_add_selstmt(f->css, sel, _HC_MATCH_PSEUDO_CLASS, name, value[0] ? value : NULL);
    }
    else if (buffer[0] == '.')
    {
     /*
      * Match class name...
      */

      if (!sel)
        sel = hc_new_sel(NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      hc_add_selstmt(f->css, sel, _HC_MATCH_CLASS, buffer + 1, NULL);
    }
    else if (buffer[0] == '#')
    {
     /*
      * Match ID string...
      */

      if (!sel)
        sel = hc_new_sel(NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      hc_add_selstmt(f->css, sel, _HC_MATCH_ID, buffer + 1, NULL);
    }
    else if (*type == _HC_TYPE_STRING)
    {
     /*
      * Match element name...
      */

      hc_element_t	element;	/* Element for selector */

      if (!strcmp(buffer, "*"))
        element = HC_ELEMENT_WILDCARD;
      else if ((element = _hcElementLookup(buffer)) == HC_ELEMENT_UNKNOWN)
      {
	_hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unknown selector '%s'.", buffer);
	goto error;
      }

      sel = hc_new_sel(sel, element, rel);
      rel = _HC_RELATION_CHILD;
    }
    else if (!strcmp(buffer, "["))
    {
     /*
      * Match attribute...
      */

      _hc_match_t	mtype;		/* Matching type */

      if (!hc_read(f, type, name, sizeof(name)) || *type != _HC_TYPE_STRING)
      {
        _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad attribute name.", name);
        goto error;
      }

      _HC_DEBUG("%s:%d: (SELECTOR) Attribute name '%s'.\n", f->file.url, f->file.linenum, name);

      if (!hc_read(f, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED)
      {
        _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad operator/terminator (%s '%s') after attribute name.", types[*type], buffer);
        goto error;
      }

      if (!strcmp(buffer, "]"))
        mtype = _HC_MATCH_ATTR_EXIST;
      else if (!strcmp(buffer, "="))
        mtype = _HC_MATCH_ATTR_EQUALS;
      else if (!strcmp(buffer, "*="))
        mtype = _HC_MATCH_ATTR_CONTAINS;
      else if (!strcmp(buffer, "^="))
        mtype = _HC_MATCH_ATTR_BEGINS;
      else if (!strcmp(buffer, "$="))
        mtype = _HC_MATCH_ATTR_ENDS;
      else if (!strcmp(buffer, "|="))
        mtype = _HC_MATCH_ATTR_LANG;
      else if (!strcmp(buffer, "~="))
        mtype = _HC_MATCH_ATTR_SPACE;
      else
      {
        _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unknown operator '%s' after attribute name.", buffer);
        goto error;
      }

      if (mtype == _HC_MATCH_ATTR_EXIST)
      {
       /*
        * No value to match...
        */

	if (!sel)
	  sel = hc_new_sel(NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

	hc_add_selstmt(f->css, sel, mtype, name, NULL);
      }
      else
      {
       /*
        * Get value...
        */

	_HC_DEBUG("%s:%d: (SELECTOR) Operator '%s'.\n", f->file.url, f->file.linenum, buffer);

	if (!hc_read(f, type, value, sizeof(value)) || *type != _HC_TYPE_QSTRING)
	{
	  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad attribute value.");
	  goto error;
	}

        _HC_DEBUG("%s:%d: (SELECTOR) Attribute value '%s'.\n", f->file.url, f->file.linenum, value);

	if (!hc_read(f, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED || strcmp(buffer, "]"))
	{
	  _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Missing/bad terminator after attribute value (Ss '%s').", types[*type], buffer);
	  goto error;
	}

	if (!sel)
	  sel = hc_new_sel(NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

	hc_add_selstmt(f->css, sel, mtype, name, value);
      }
    }
    else if (!strcmp(buffer, ">") && sel)
    {
     /*
      * Match immediate child...
      */

      rel = _HC_RELATION_IMMED_CHILD;
    }
    else if (!strcmp(buffer, "+") && sel)
    {
     /*
      * Match immediate sibling...
      */

      rel = _HC_RELATION_IMMED_SIBLING;
    }
    else if (!strcmp(buffer, "~") && sel)
    {
     /*
      * Match (subsequent) sibling...
      */

      rel = _HC_RELATION_SIBLING;
    }
    else if (!strcmp(buffer, "{") || !strcmp(buffer, ","))
      break;
    else
    {
      _hcError(f->css->error_cb, f->css->error_ctx, f->file.url, f->file.linenum, "Unknown selector '%s'.", buffer);
      goto error;
    }

  }
  while (hc_read(f, type, buffer, bufsize));

  _HC_DEBUG("%s:%d: (SELECTOR) %s (%d matching statements)\n", f->file.url, f->file.linenum, hcElements[sel->element], (int)sel->num_stmts);

  return (sel);

 /*
  * If we get here there was a hard error...
  */

  error:

  _hcCSSSelDelete(sel);

  return (NULL);
}


/*
 * 'hc_read_value()' - Read a value string.
 */

static char *				/* O - String or `NULL` on error */
hc_read_value(_hc_css_file_t *f,	/* I - File to read from */
              char           *buffer,	/* I - String buffer */
              size_t         bufsize)	/* I - Size of string buffer */
{
  int	ch,				/* Character from file */
	paren = 0;			/* Parenthesis */
  char	*bufptr = buffer,		/* Pointer into string buffer */
	*bufend = buffer + bufsize - 1,	/* End of string buffer */
	quote = '\0';			/* Quote character (if any) */


 /*
  * Skip leading whitespace...
  */

  while ((ch = _hcFileGetc(&f->file)) != EOF)
    if (!isspace(ch & 255) || ch == ';' || ch == '}')
      break;

  do
  {
    if (!paren && (ch == ';' || ch == '}'))
    {
      _hcFileUngetc(ch, &f->file);
      break;
    }

    if (bufptr < bufend)
      *bufptr++ = ch;

    if (ch == '(')
      paren ++;
    else if (ch == ')')
      paren --;
    else if (ch == '\"' || ch == '\'')
    {
      quote = ch;
    }
  }
  while ((ch = _hcFileGetc(&f->file)) != EOF);

 /*
  * Remove trailing whitespace...
  */

  while (bufptr > buffer && isspace(bufptr[-1] & 255))
    bufptr --;

  *bufptr = '\0';

  return (bufptr == buffer ? NULL : buffer);
}
