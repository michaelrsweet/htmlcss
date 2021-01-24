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
#  include "file-private.h"
#  include "default-css.h"


/*
 * Local types...
 */

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
static int		hc_eval_media(hc_css_t *css, hc_file_t *file, _hc_type_t *type, char *buffer, size_t bufsize);
static char		*hc_read(hc_file_t *file, _hc_type_t *type, char *buffer, size_t bufsize);
static hc_dict_t	*hc_read_props(hc_css_t *css, hc_file_t *file, hc_dict_t *props);
static _hc_css_sel_t	*hc_read_sel(hc_css_t *css, hc_file_t *file, _hc_type_t *type, char *buffer, size_t bufsize);
static char		*hc_read_value(hc_file_t *file, char *buffer, size_t bufsize);


/*
 * 'hcCSSImport()' - Import CSS definitions from a URL, file, or string.
 */

bool					/* O - `true` on success, `false` on error */
hcCSSImport(hc_css_t  *css,		/* I - Stylesheet */
            hc_file_t *file)		/* I - File */
{
  bool		ret = true;		/* Return value */
  char		buffer[256];		/* Current value */
  _hc_type_t	type;			/* Value type */
  int		skip = 0;		/* Skip current definitions */
  int		in_media = 0;		/* In a media grouping? */
  int		num_sels = 0;		/* Number of selectors */
  _hc_css_sel_t	*sels[1000];		/* Selectors */


  _HC_DEBUG("hcCSSImport(css=%p, file=%p)\n", (void *)css, (void *)file);

 /*
  * Range check input...
  */

  if (!css || !file)
  {
    errno = EINVAL;
    return (false);
  }

 /*
  * Read CSS...
  */

  while (hc_read(file, &type, buffer, sizeof(buffer)))
  {
    _HC_DEBUG("%s:%d: %s %s\n", file->url, file->linenum, types[type], buffer);

    if (!strcmp(buffer, "@import"))
    {
      int	in_url = 0;		/* In a URL? */
      char	path[256] = "";		/* Path to import */

      while (hc_read(file, &type, buffer, sizeof(buffer)))
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
	  _hcFileError(file, "Unexpected %s token seen.", buffer);
	  ret = false;
	  break;
        }
      }

      if (!path[0])
        break;

      if (hc_eval_media(css, file, &type, buffer, sizeof(buffer)))
      {
        hc_file_t *impfile = hcFileNewURL(file->pool, path, file->url);
					/* Import file */

        ret = hcCSSImport(css, impfile);

        hcFileDelete(impfile);

        if (!ret)
          break;
      }

      if (strcmp(buffer, ";"))
      {
	_hcFileError(file, "Unexpected %s token seen.", buffer);
	ret = false;
	break;
      }
    }
    else if (!strcmp(buffer, "@media"))
    {
      if (in_media)
      {
	_hcFileError(file, "Unexpected nested @media.");
        break;
      }

      skip     = !hc_eval_media(css, file, &type, buffer, sizeof(buffer));
      in_media = !strcmp(buffer, "{");
    }
    else if (buffer[0] == '@')
    {
      _hcFileError(file, "Unknown %s seen.", buffer);
      ret = false;
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
	_hcFileError(file, "Unexpected %s seen.", buffer);
	ret = false;
	break;
      }
    }
    else if (num_sels < (int)(sizeof(sels) / sizeof(sels[0])))
    {
      if ((sels[num_sels] = hc_read_sel(css, file, &type, buffer, sizeof(buffer))) == NULL)
      {
	ret = false;
	break;
      }

      num_sels ++;

      if (!strcmp(buffer, "{"))
      {
	int		i;		/* Looping var */
        hc_dict_t	*props;		/* Properties */

        if ((props = hc_read_props(css, file, NULL)) != NULL)
        {
	  if (skip)
	  {
	    _HC_DEBUG("%s:%d: Skipping %d properties for %d selectors.\n", file->url, file->linenum, (int)hcDictGetCount(props), num_sels);

	    for (i = 0; i < num_sels; i ++)
	      _hcCSSSelDelete(sels[i]);
	  }
	  else
	  {
	    _HC_DEBUG("%s:%d: Adding %d properties for %d selectors.\n", file->url, file->linenum, (int)hcDictGetCount(props), num_sels);

	    for (i = 0; i < num_sels; i ++)
	      hc_add_rule(css, sels[i], props);
	  }

	  hcDictDelete(props);
	  num_sels = 0;
        }
      }
      else if (strcmp(buffer, ","))
      {
	_hcFileError(file, "Unexpected %s seen.", buffer);
	ret = false;
	break;
      }
    }
    else
    {
      _hcFileError(file, "Too many selectors seen.");
      ret = false;
      break;
    }
  }

  return (ret);
}


/*
 * 'hcCSSImportDefault()' - Import the default HTML stylesheet.
 */

bool					/* O - `true` on success, `false` on error */
hcCSSImportDefault(hc_css_t *css)	/* I - Stylesheet */
{
  hc_file_t	*file = hcFileNewString(css->pool, default_css);
					/* String file */
  bool		ret = hcCSSImport(css, file);
					/* Return value */

  hcFileDelete(file);

  return (ret);
}


/*
 * '_hcCSSImportString()' - Import a style attribute string.
 */

void
_hcCSSImportString(hc_css_t   *css,	/* I - Stylesheet */
                   hc_dict_t  *props,	/* I - Property dictionary */
                   const char *s)	/* I - Style attribute string */
{
  hc_file_t	*file = hcFileNewString(css->pool, s);
					/* String file */

  hc_read_props(css, file, props);
  hcFileDelete(file);
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
  hc_sha3_256_t	hash;			/* Hash */


  _hcCSSSelHash(sel, hash);

  if ((rule = _hcRuleNew(css, hash, sel, props)) != NULL)
  {
    _hcRuleColAdd(css, &css->all_rules, rule);
    _hcRuleColAdd(css, css->rules + sel->element, rule);
  }
}


/*
 * 'hc_eval_media()' - Read and evaluate a media rule.
 */

static int				/* O - 1 if media rule matches, 0 otherwise */
hc_eval_media(hc_css_t   *css,		/* I - Stylesheet */
              hc_file_t  *file,		/* I - File to read from */
              _hc_type_t *type,		/* O - Token type */
              char       *buffer,	/* I - Buffer */
              size_t     bufsize)	/* I - Size of buffer */
{
  int		media_result = -1;	/* Result of evaluation */
  int		media_current = -1;	/* Result of current expression */
  _hc_logop_t	logop = _HC_LOGOP_NONE;	/* Logical operation seen, if any */
  int		invert = 0;		/* Was "not" seen? */


  while (hc_read(file, type, buffer, bufsize))
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

	while (hc_read(file, type, buffer, bufsize))
	{
	  if (*type == _HC_TYPE_RESERVED && !strcmp(buffer, ")"))
	    break;
	}

        if (*type != _HC_TYPE_RESERVED)
        {
	  _hcFileError(file, "Unexpected end-of-file.");
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
      else if ((!strcmp(buffer, css->media.type) || !strcmp(buffer, "all")) != invert)
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

  _hcFileError(file, "Unexpected token \"%s\" seen.", buffer);

  return (0);
}


/*
 * 'hc_read()' - Read a string from the CSS file.
 */

static char *				/* O - String or `NULL` on EOF */
hc_read(hc_file_t  *file,		/* I - CSS file */
	_hc_type_t *type,		/* O - String type */
	char       *buffer,		/* I - Buffer */
	size_t     bufsize)		/* I - Size of buffer */
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
    while ((ch = hcFileGetc(file)) != EOF)
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
        if ((ch = hcFileGetc(file)) == ':')
          *bufptr++ = (char)ch;
        else
	  hcFileUngetc(file, ch);
      }
    }
    else if (ch == '\'' || ch == '\"')
    {
     /*
      * Quoted string...
      */

      int quote = ch;			/* Quote character */

      while ((ch = hcFileGetc(file)) != EOF)
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

	  while ((ch = hcFileGetc(file)) != EOF)
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

          hcFileUngetc(file, '=');
          bufptr --;
          break;
        }
      }
      while ((ch = hcFileGetc(file)) != EOF);

      if (bufptr == buffer)
        continue;
      else if (ch != EOF && !isspace(ch & 255) && ch != '(' && ch != '=')
	hcFileUngetc(file, ch);

      if (isdigit(*buffer & 255) || (*buffer == '.' && bufptr > (buffer + 1) && isdigit(buffer[1] & 255)))
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
hc_read_props(hc_css_t  *css,		/* I - Stylesheet */
              hc_file_t *file,		/* I - file to read from */
              hc_dict_t *props)		/* I - Existing properties */
{
  _hc_type_t	type;			/* String type */
  char		buffer[256],		/* String buffer */
		name[256],		/* Name string */
		value[2048];		/* Value string */
  int		skip_remainder = 0;	/* Skip the remainder? */


  while (hc_read(file, &type, buffer, sizeof(buffer)))
  {
    _HC_DEBUG("%s:%d: (PROPS) %s %s\n", file->url, file->linenum, types[type], buffer);

    if (type == _HC_TYPE_RESERVED && !strcmp(buffer, "}"))
      break;
    else if (skip_remainder)
      continue;
    else if (type != _HC_TYPE_STRING)
    {
      _hcFileError(file, "Unexpected %s seen.", buffer);
      skip_remainder = 1;
      continue;
    }

    strncpy(name, buffer, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    if (!hc_read(file, &type, buffer, sizeof(buffer)) || type != _HC_TYPE_RESERVED || strcmp(buffer, ":"))
    {
      _hcFileError(file, "Missing colon, saw %s instead.", buffer);
      skip_remainder = 1;
      continue;
    }

    if (!hc_read_value(file, value, sizeof(value)))
    {
      _hcFileError(file, "Missing property value.");
      break;
    }

    if (!hc_read(file, &type, buffer, sizeof(buffer)) || type != _HC_TYPE_RESERVED || strcmp(buffer, ";"))
    {
      _hcFileError(file, "Missing semi-colon, saw %s instead.", buffer);
      skip_remainder = 1;
      continue;
    }

    if (!props)
      props = hcDictNew(css->pool);

    _HC_DEBUG("%s:%d: (PROPS) Adding '%s: %s;'.\n", file->url, file->linenum, name, value);
    hcDictSetKeyValue(props, name, value);
  }

  _HC_DEBUG("%s:%d: (PROPS) Returning %d properties.\n", file->url, file->linenum, (int)hcDictGetCount(props));

  return (props);
}


/*
 * 'hc_read_sel()' - Read a CSS selector.
 *
 * On entry, "type" and "buffer" contain the initial selector string.  On exit
 * they contain the terminating token (typically "," or "{").
 */

static _hc_css_sel_t *			/* O  - Selector or `NULL` on error */
hc_read_sel(hc_css_t   *css,		/* I  - Stylesheet */
	    hc_file_t  *file,		/* I  - File to read from */
            _hc_type_t *type,		/* IO - String type */
            char       *buffer,		/* I  - String buffer */
            size_t     bufsize)		/* I  - Size of string buffer */
{
  _hc_css_sel_t *sel = NULL;		/* Current selector */
  _hc_relation_t rel = _HC_RELATION_CHILD;
					/* Relationship with next selector */
  char		*ptr,			/* Pointer into buffer/value */
		name[256],		/* Attribute/pseudo-class name */
		value[256];		/* Value in selector */


  do
  {
    _HC_DEBUG("%s:%d: (SELECTOR) %s %s\n", file->url, file->linenum, types[*type], buffer);

    if (!strcmp(buffer, ":"))
    {
     /*
      * Match pseudo-class...
      */

      if (!hc_read(file, type, name, sizeof(name)) || *type != _HC_TYPE_STRING)
      {
        _hcFileError(file, "Missing/bad pseudo-class.");
        goto error;
      }

      ptr = name + strlen(name) - 1;
      if (ptr > name && *ptr == '(')
      {
       /*
        * :NAME(VALUE) syntax...
        */

        *ptr = '\0';
        if (!hc_read(file, type, value, sizeof(value)) || *type != _HC_TYPE_STRING)
        {
	  _hcFileError(file, "Missing/bad value for ':%s'.", name);
	  goto error;
        }

        if (!hc_read(file, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED || strcmp(buffer, ")"))
        {
	  _hcFileError(file, "Missing/bad parenthesis after ':%s(%s'.", name, value);
	  goto error;
        }
      }
      else
        value[0] = '\0';

      if (!sel)
        sel = _hcCSSSelNew(css, NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      _hcCSSSelAddStmt(css, sel, _HC_MATCH_PSEUDO_CLASS, name, value[0] ? value : NULL);
    }
    else if (buffer[0] == '.')
    {
     /*
      * Match class name...
      */

      if (!sel)
        sel = _hcCSSSelNew(css, NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      _hcCSSSelAddStmt(css, sel, _HC_MATCH_CLASS, buffer + 1, NULL);
    }
    else if (buffer[0] == '#')
    {
     /*
      * Match ID string...
      */

      if (!sel)
        sel = _hcCSSSelNew(css, NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

      _hcCSSSelAddStmt(css, sel, _HC_MATCH_ID, buffer + 1, NULL);
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
	_hcFileError(file, "Unknown selector '%s'.", buffer);
	goto error;
      }

      sel = _hcCSSSelNew(css, sel, element, rel);
      rel = _HC_RELATION_CHILD;
    }
    else if (!strcmp(buffer, "["))
    {
     /*
      * Match attribute...
      */

      _hc_match_t	mtype;		/* Matching type */

      if (!hc_read(file, type, name, sizeof(name)) || *type != _HC_TYPE_STRING)
      {
        _hcFileError(file, "Missing/bad attribute name.");
        goto error;
      }

      _HC_DEBUG("%s:%d: (SELECTOR) Attribute name '%s'.\n", file->url, file->linenum, name);

      if (!hc_read(file, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED)
      {
        _hcFileError(file, "Missing/bad operator/terminator (%s '%s') after attribute name.", types[*type], buffer);
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
        _hcFileError(file, "Unknown operator '%s' after attribute name.", buffer);
        goto error;
      }

      if (mtype == _HC_MATCH_ATTR_EXIST)
      {
       /*
        * No value to match...
        */

	if (!sel)
	  sel = _hcCSSSelNew(css, NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

	_hcCSSSelAddStmt(css, sel, mtype, name, NULL);
      }
      else
      {
       /*
        * Get value...
        */

	_HC_DEBUG("%s:%d: (SELECTOR) Operator '%s'.\n", file->url, file->linenum, buffer);

	if (!hc_read(file, type, value, sizeof(value)) || *type != _HC_TYPE_QSTRING)
	{
	  _hcFileError(file, "Missing/bad attribute value.");
	  goto error;
	}

        _HC_DEBUG("%s:%d: (SELECTOR) Attribute value '%s'.\n", file->url, file->linenum, value);

	if (!hc_read(file, type, buffer, bufsize) || *type != _HC_TYPE_RESERVED || strcmp(buffer, "]"))
	{
	  _hcFileError(file, "Missing/bad terminator after attribute value (%s '%s').", types[*type], buffer);
	  goto error;
	}

	if (!sel)
	  sel = _hcCSSSelNew(css, NULL, HC_ELEMENT_WILDCARD, _HC_RELATION_CHILD);

	_hcCSSSelAddStmt(css, sel, mtype, name, value);
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
      _hcFileError(file, "Unknown selector '%s'.", buffer);
      goto error;
    }

  }
  while (hc_read(file, type, buffer, bufsize));

#ifdef DEBUG
  if (sel)
    _HC_DEBUG("%s:%d: (SELECTOR) %s (%d matching statements)\n", file->url, file->linenum, hcElements[sel->element], (int)sel->num_stmts);
  else
    _HC_DEBUG("%s:%d: (SELECTOR) NULL\n", file->url, file->linenum);
#endif /* DEBUG */

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
hc_read_value(hc_file_t *file,		/* I - File to read from */
              char      *buffer,	/* I - String buffer */
              size_t    bufsize)	/* I - Size of string buffer */
{
  int	ch,				/* Character from file */
	paren = 0,			/* Parenthesis */
	quote = '\0';			/* Quote character (if any) */
  char	*bufptr = buffer,		/* Pointer into string buffer */
	*bufend = buffer + bufsize - 1;	/* End of string buffer */


 /*
  * Skip leading whitespace...
  */

  while ((ch = hcFileGetc(file)) != EOF)
    if (!isspace(ch & 255) || ch == ';' || ch == '}')
      break;

  do
  {
    if (!paren && !quote && (ch == ';' || ch == '}'))
    {
      hcFileUngetc(file, ch);
      break;
    }

    if (bufptr < bufend)
      *bufptr++ = (char)ch;

    if (ch == '(')
      paren ++;
    else if (ch == ')')
      paren --;
    else if (ch == '\\')
    {
      if ((ch = hcFileGetc(file)) != EOF)
      {
        if (bufptr < bufend)
          *bufptr++ = (char)ch;
      }
    }
    else if (ch == quote)
      quote = '\0';
    else if (ch == '\"' || ch == '\'')
      quote = ch;
  }
  while ((ch = hcFileGetc(file)) != EOF);

 /*
  * Remove trailing whitespace...
  */

  while (bufptr > buffer && isspace(bufptr[-1] & 255))
    bufptr --;

  *bufptr = '\0';

  return (bufptr == buffer ? NULL : buffer);
}
