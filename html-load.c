/*
 * HTML load functions for HTMLCSS library.
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

#include "html-private.h"
#include <ctype.h>


/*
 * Local macros...
 */

#define html_isblock(x)	((x) == HC_ELEMENT_ADDRESS || (x) == HC_ELEMENT_P || (x) == HC_ELEMENT_PRE || ((x) >= HC_ELEMENT_H1 && (x) <= HC_ELEMENT_H6) || (x) == HC_ELEMENT_HR || (x) == HC_ELEMENT_TABLE)
#define html_isleaf(x)	((x) == HC_ELEMENT_AREA || (x) == HC_ELEMENT_BASE || (x) == HC_ELEMENT_BR || (x) == HC_ELEMENT_COL || (x) == HC_ELEMENT_EMBED || (x) == HC_ELEMENT_HR ||(x) == HC_ELEMENT_IMG || (x) == HC_ELEMENT_INPUT || (x) == HC_ELEMENT_ISINDEX || (x) == HC_ELEMENT_LINK || (x) == HC_ELEMENT_META || (x) == HC_ELEMENT_PARAM || (x) == HC_ELEMENT_SOURCE || (x) == HC_ELEMENT_SPACER || (x) == HC_ELEMENT_TRACK || (x) == HC_ELEMENT_WBR)
#define html_islist(x)	((x) == HC_ELEMENT_DL || (x) == HC_ELEMENT_OL || (x) == HC_ELEMENT_UL || (x) == HC_ELEMENT_DIR || (x) == HC_ELEMENT_MENU)
#define html_islentry(x)	((x) == HC_ELEMENT_LI || (x) == HC_ELEMENT_DD || (x) == HC_ELEMENT_DT)
#define html_issuper(x)	((x) == HC_ELEMENT_CENTER || (x) == HC_ELEMENT_DIV || (x) == HC_ELEMENT_BLOCKQUOTE)
#define html_istable(x)	((x) == HC_ELEMENT_TBODY || (x) == HC_ELEMENT_THEAD || (x) == HC_ELEMENT_TFOOT || (x) == HC_ELEMENT_TR)
#define html_istentry(x)	((x) == HC_ELEMENT_TD || (x) == HC_ELEMENT_TH)


/*
 * Local types...
 */

typedef struct _html_file_s
{
  hc_html_t  *html;			/* HTML document */
  _hc_file_t file;			/* File info */
  hc_node_t  *parent;			/* Parent node */
} _html_file_t;


/*
 * Local functions...
 */

static int	html_compare_elements(const char **a, const char **b);
static int	html_parse_attr(_html_file_t *f, hc_node_t *node, int ch);
static int	html_parse_comment(_html_file_t *f);
static int	html_parse_doctype(_html_file_t *f);
static int	html_parse_element(_html_file_t *f, int ch);
static int	html_parse_unknown(_html_file_t *f, const char *unk);


/*
 * '_hcElementLookup() - Lookup an element enum from a string.
 */

hc_element_t				/* O - Element */
_hcElementLookup(const char *s)		/* I - String */
{
  const char	**match;		/* Matching element */


  match  = bsearch(&s, hcElements + 1, sizeof(hcElements) / sizeof(hcElements[0]) - 1, sizeof(hcElements[0]), (int (*)(const void *, const void *))html_compare_elements);

  if (match)
    return ((hc_element_t)(match - hcElements));
  else
    return (HC_ELEMENT_UNKNOWN);
}


/*
 * 'hcHTMLLoad()' - Load a HTML file into a document.
 */

int					/* O - 1 on success, 0 on failure */
hcHTMLLoad(hc_html_t  *html,		/* I - HTML document */
	   const char *url,		/* I - URL or `NULL` */
	   FILE       *fp)		/* I - File pointer or `NULL` */
{
  _html_file_t	f;			/* File data */
  int		status = 1,		/* Load status */
		ch;			/* Current character */
  char		buffer[8192],		/* Temporary buffer */
		*bufptr,		/* Pointer into buffer */
		*bufend;		/* End of buffer */


 /*
  * Range check input...
  */

  if (!html || html->root || (!url && !fp))
    return (0);

 /*
  * Open file as needed...
  */

  f.html         = html;
  f.file.url     = url;
  f.file.fp      = fp;
  f.file.s       = NULL;
  f.file.sptr    = NULL;
  f.file.linenum = 1;
  f.parent       = NULL;

  if (!fp)
  {
    if (!(html->url_cb)(url, buffer, sizeof(buffer), html->url_ctx))
    {
      _hcError(html->error_cb, html->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    if ((f.file.fp = fopen(buffer, "rb")) == NULL)
    {
      _hcError(html->error_cb, html->error_ctx, url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }
  }
  else if (!url)
  {
    if (fp == stdin)
      f.file.url = "<stdin>";
    else
      f.file.url = "<unknown>";
  }

 /*
  * Parse file...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = _hcFileGetc(&f.file)) != EOF)
  {
    if (ch == '<')
    {
     /*
      * Read a HTML element...
      */

      ch = _hcFileGetc(&f.file);

      if (isspace(ch) || ch == '=' || ch == '<')
      {
       /*
        * Sigh...  "<" followed by anything but an element name is invalid HTML,
        * but many pages are still broken.  Log it and abort if the error
        * callback says to...
	*/

        if (!_hcError(f.html->error_cb, f.html->error_ctx, f.file.url, f.file.linenum, "Unquoted '<'."))
        {
          status = 0;
          break;
        }

        if (bufptr >= (bufend - 1))
        {
	 /*
	  * Add text string...
	  */

	  if (f.parent)
	  {
	    *bufptr = '\0';
	    hcNodeNewString(f.parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = 0;
	    _hcError(f.html->error_cb, f.html->error_ctx, f.file.url, f.file.linenum, "Text without leading element or directive.");
	    break;
	  }
        }

        *bufptr++ = '<';

	if (ch == '<')
	  _hcFileUngetc(ch, &f.file);
        else
          *bufptr++ = ch;
      }
      else
      {
       /*
        * Got the first character of an element name, add any pending text and
        * then parse the element...
        */

	if (bufptr > buffer)
	{
	 /*
	  * Add text string...
	  */

	  if (f.parent)
	  {
	    *bufptr = '\0';
	    hcNodeNewString(f.parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = 0;
	    _hcError(f.html->error_cb, f.html->error_ctx, f.file.url, f.file.linenum, "Text without leading element or directive.");
	    break;
	  }
	}

	if (!(status = html_parse_element(&f, ch)))
	  break;
      }
    }
    else
    {
      if (bufptr < bufend)
        *bufptr++ = ch;

      if (ch == '\n' || bufptr >= bufend)
      {
	if (f.parent)
	{
	  *bufptr = '\0';
	  hcNodeNewString(f.parent, buffer);
	  bufptr = buffer;
	}
	else
	{
	  status = 0;
	  _hcError(f.html->error_cb, f.html->error_ctx, f.file.url, f.file.linenum, "Text without leading element or directive.");
	}
      }
    }

  }

  if (bufptr > buffer)
  {
   /*
    * Add trailing text string...
    */

    if (f.parent)
    {
      *bufptr = '\0';
      hcNodeNewString(f.parent, buffer);
    }
    else
    {
      status = 0;
      _hcError(f.html->error_cb, f.html->error_ctx, f.file.url, f.file.linenum, "Text without leading element or directive.");
    }
  }

 /*
  * Close file as needed and return...
  */

  if (f.file.fp != fp)
    fclose(f.file.fp);

  return (status);
}


/*
 * 'html_compare_elements()' - Compare two elements...
 */

static int				/* O - Result of comparison */
html_compare_elements(const char **a,	/* I - First string */
                      const char **b)	/* I - Second string */
{
#ifdef WIN32
  return (_stricmp(*a, *b));
#else
  return (strcasecmp(*a, *b));
#endif /* WIN32 */
}


/*
 * 'html_parse_attr()' - Parse an attribute.
 */

static int				/* O - Character or `EOF` to stop */
html_parse_attr(_html_file_t *f,	/* I - HTML file info */
                hc_node_t    *node,	/* I - HTML element node */
                int          ch)	/* I - Initial character */
{
  char	name[256],			/* Name string */
	value[2048],			/* Value string */
	*ptr,				/* Pointer into string */
	*end;				/* End of string */


 /*
  * Read name...
  */

  ptr = name;
  end = name + sizeof(name) - 1;

  do
  {
    if (ptr < end)
      *ptr++ = tolower(ch);
    else
      break;
  }
  while ((ch = _hcFileGetc(&f->file)) != EOF && ch != '=' && ch != '>' && !isspace(ch));

  *ptr = '\0';

  if (ch == '=')
  {
   /*
    * Read value...
    */

    ptr = value;
    end = value + sizeof(value) - 1;

    if ((ch = _hcFileGetc(&f->file)) == '\'' || ch == '\"')
    {
      char quote = ch;			/* Quote character */

      while ((ch = _hcFileGetc(&f->file)) != EOF && ch != quote)
      {
	if (ptr < end)
	  *ptr++ = ch;
	else
	  break;
      }
    }
    else if (!isspace(ch) && ch != '>' && ch != EOF)
    {
      do
      {
	if (ptr < end)
	  *ptr++ = ch;
	else
	  break;
      }
      while ((ch = _hcFileGetc(&f->file)) != EOF && ch != '>' && !isspace(ch));
    }

    *ptr = '\0';
    hcNodeAttrSetNameValue(f->html, node, name, value);
  }
  else if (ch != EOF)
  {
   /*
    * Add "name=name"...
    */

    hcNodeAttrSetNameValue(f->html, node, name, name);
  }

  return (ch);
}


/*
 * 'html_parse_comment()' - Parse a comment.
 */

static int				/* O - 1 to continue, 0 to stop */
html_parse_comment(_html_file_t *f)	/* I - HTML file info */
{
  int	ch;				/* Current character */
  char	buffer[8192],			/* String buffer */
	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */


  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = _hcFileGetc(&f->file)) != EOF)
  {
    if (ch == '>' && bufptr > (buffer + 1) && bufptr[-1] == '-' && bufptr[-2] == '-')
    {
     /*
      * End of comment...
      */

      bufptr -= 2;
      break;
    }
    else if (bufptr < bufend)
      *bufptr++ = ch;
    else if (!_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Comment too long."))
      return (0);
    else
      break;
  }

  *bufptr = '\0';

  hcNodeNewComment(f->parent, buffer);

  if (ch == EOF)
    return (_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Unexpected end-of-file."));
  else if (ch != '>')
    return (_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Comment too long."));
  else
    return (1);
}


/*
 * 'html_parse_doctype()' - Parse a DOCTYPE element.
 */

static int				/* O - 1 to continue, 0 to stop */
html_parse_doctype(_html_file_t *f)	/* I - HTML file info */
{
  int	ch;				/* Character from file */
  char	buffer[2048],			/* String buffer */
	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */


  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = _hcFileGetc(&f->file)) != EOF)
  {
    if (!isspace(ch))
      break;
  }

  while (ch != EOF && ch != '>')
  {
    if (bufptr < bufend)
      *bufptr++ = ch;
    else
      break;

    if (ch == '\'' || ch == '\"')
    {
      int quote = ch;			/* Quote character */

      while ((ch = _hcFileGetc(&f->file)) != EOF && ch != quote)
      {
        if (bufptr < bufend)
          *bufptr++ = ch;
        else
          break;
      }
    }
    else
      ch = _hcFileGetc(&f->file);
  }

  *bufptr = '\0';

  if (ch == EOF)
  {
    _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Unexpected end-of-file.");
    return (0);
  }
  else if (ch != '>')
    _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "<!DOCTYPE ...> too long.");

  return ((f->parent = hcHTMLNewRootNode(f->html, buffer)) != NULL);
}


/*
 * 'html_parse_element()' - Parse an element.
 */

static int				/* O - 1 to continue, 0 to stop */
html_parse_element(_html_file_t *f,	/* I - HTML file info */
                   int          ch)	/* I - Initial character after '<' */
{
  char		buffer[256],		/* String buffer */
		*bufptr,		/* Pointer into buffer */
		*bufend;		/* End of buffer */
  hc_element_t	element;		/* Element index */
  hc_node_t	*node;			/* New node */
  int		close_el = ch == '/';	/* Close element? */


 /*
  * Read the element name...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;
  if (!close_el)
    *bufptr++ = ch;

  while ((ch = _hcFileGetc(&f->file)) != EOF)
  {
    if (isspace(ch) || ch == '>' || ch == '/')
      break;
    else if (bufptr < bufend)
      *bufptr++ = ch;
    else if (!_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Element name too long."))
      return (0);
    else
      break;

    if ((bufptr - buffer) == 3 && !memcmp(buffer, "!--", 3))
    {
     /*
      * Comment without whitespace, pretend we got some...
      */

      ch = ' ';
      break;
    }
  }

  if (ch == EOF)
  {
    _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Unexpected end-of-file.");
    return (0);
  }

  *bufptr = '\0';

 /*
  * Convert the name to an enum...
  */

  if (isspace(ch) || ch == '>' || ch == '/')
  {
    element = _hcElementLookup(buffer);

    if (element == HC_ELEMENT_UNKNOWN && !_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Unknown element '%s'.", buffer))
      return (0);
  }
  else
  {
    _hcFileUngetc(ch, &f->file);
    element = HC_ELEMENT_UNKNOWN;
  }

 /*
  * Parse unknown, comment, and doctype elements accordingly...
  */

  if (element == HC_ELEMENT_DOCTYPE)
  {
    if (close_el)
    {
      _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Invalid </!DOCTYPE> seem.");
      return (0);
    }
    else if (f->html->root)
    {
      _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Duplicate <!DOCTYPE> seen.");
      return (0);
    }

    return (html_parse_doctype(f));
  }
  else if (!f->parent)
  {
    if (!_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Missing <!DOCTYPE html> directive."))
      return (0);

    f->parent = hcHTMLNewRootNode(f->html, "html");
  }
  else if (element == HC_ELEMENT_UNKNOWN)
  {
    if (close_el)
    {
      char	unk[257];		/* Unknown value */

      snprintf(unk, sizeof(unk), "/%s", buffer);
      return (html_parse_unknown(f, unk));
    }
    else
    {
      return (html_parse_unknown(f, buffer));
    }
  }
  else if (element == HC_ELEMENT_COMMENT)
  {
    return (html_parse_comment(f));
  }

 /*
  * Otherwise add the element (or close it) in the right place...
  */

  if (close_el)
  {
   /*
    * Close the specified element...
    */

    if (ch != '>' && !_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Invalid </%s> element.", buffer))
      return (0);

    for (node = f->parent; node; node = node->parent)
    {
      if (node->element == element)
        break;
    }

    if (node)
      f->parent = node->parent;
    else if (!_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Missing <%s> for </%s> element.", buffer, buffer))
      return (0);

    return (1);
  }

 /*
  * HTML doesn't enforce strict open/close markup semantics, so allow <p>, <li>,
  * etc. to close out like markup...
  */

  if (html_issuper(element))
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (html_istentry(node->element))
	break;
    }
  }
  else if (html_islist(element))
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (html_isblock(node->element) || html_islentry(node->element) || html_istentry(node->element) || html_issuper(node->element))
	break;
    }
  }
  else if (html_islentry(element))
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (html_islist(node->element))
	break;
    }
  }
  else if (html_isblock(element))
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (html_isblock(node->element))
      {
        node = node->parent;
	break;
      }
      else if (html_istentry(node->element) || html_islist(node->element) || html_islentry(node->element) || html_issuper(node->element))
      {
	break;
      }
    }
  }
  else if (element == HC_ELEMENT_THEAD || element == HC_ELEMENT_TBODY || element == HC_ELEMENT_TFOOT)
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (node->element == HC_ELEMENT_TABLE)
	break;
    }
  }
  else if (html_istentry(element))
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (html_istentry(node->element))
      {
        node = node->parent;
	break;
      }
      else if (node->element == HC_ELEMENT_TR)
      {
        break;
      }
      else if (node->element == HC_ELEMENT_TABLE || html_istable(node->element))
      {
        if (!_hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "No <tr> element before <%s> element.", buffer))
          return (0);

        node = hcNodeNewElement(f->parent, HC_ELEMENT_TR);
        break;
      }
    }
  }
  else
  {
   /*
    * No new parent...
    */

    node = NULL;
  }

  if (node)
    f->parent = node;

  node = hcNodeNewElement(f->parent, element);

  if (ch != '/' && !html_isleaf(element))
    f->parent = node;

  while (ch != '>' && ch != EOF)
  {
    while ((ch = _hcFileGetc(&f->file)) != EOF)
    {
      if (!isspace(ch))
        break;
    }

    if (ch != '>')
      ch = html_parse_attr(f, node, ch);
  }

  return (ch == '>');
}


/*
 * 'html_parse_unknown()' - Parse an unknown element or processing directive.
 */

static int				/* O - 1 to continue, 0 to stop */
html_parse_unknown(_html_file_t *f,	/* I - HTML file info */
                   const char   *unk)	/* I - Start of unknown markup */
{
  int	ch;				/* Character from file */
  char	buffer[2048],			/* String buffer */
	*bufptr,			/* Pointer into buffer */
	*bufend;			/* End of buffer */


  strncpy(buffer, unk, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  bufptr = buffer + strlen(buffer);
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = _hcFileGetc(&f->file)) != EOF && ch != '>')
  {
    if (bufptr < bufend)
      *bufptr++ = ch;
    else
      break;

    if (ch == '\'' || ch == '\"')
    {
      int quote = ch;			/* Quote character */

      while ((ch = _hcFileGetc(&f->file)) != EOF && ch != quote)
      {
        if (bufptr < bufend)
          *bufptr++ = ch;
        else
          break;
      }

      if (ch == EOF || ch != quote || bufptr >= bufend)
        break;

      *bufptr++ = ch;
    }
  }

  *bufptr = '\0';

  if (ch == EOF)
  {
    _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Unexpected end-of-file.");
    return (0);
  }
  else if (ch != '>')
    _hcError(f->html->error_cb, f->html->error_ctx, f->file.url, f->file.linenum, "Element too long.");

  return (_hcNodeNewUnknown(f->parent, buffer) != NULL);
}
