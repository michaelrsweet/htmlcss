/*
 * HTML load functions for HTMLCSS library.
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

#include "html-private.h"
#include <ctype.h>


/*
 * Local macros...
 */

#define html_isblock(x)	((x) == HTML_ELEMENT_ADDRESS || (x) == HTML_ELEMENT_P || (x) == HTML_ELEMENT_PRE || ((x) >= HTML_ELEMENT_H1 && (x) <= HTML_ELEMENT_H6) || (x) == HTML_ELEMENT_HR || (x) == HTML_ELEMENT_TABLE)
#define html_isleaf(x)	((x) == HTML_ELEMENT_AREA || (x) == HTML_ELEMENT_BASE || (x) == HTML_ELEMENT_BR || (x) == HTML_ELEMENT_COL || (x) == HTML_ELEMENT_EMBED || (x) == HTML_ELEMENT_HR ||(x) == HTML_ELEMENT_IMG || (x) == HTML_ELEMENT_INPUT || (x) == HTML_ELEMENT_ISINDEX || (x) == HTML_ELEMENT_LINK || (x) == HTML_ELEMENT_META || (x) == HTML_ELEMENT_PARAM || (x) == HTML_ELEMENT_SOURCE || (x) == HTML_ELEMENT_SPACER || (x) == HTML_ELEMENT_TRACK || (x) == HTML_ELEMENT_WBR)
#define html_islist(x)	((x) == HTML_ELEMENT_DL || (x) == HTML_ELEMENT_OL || (x) == HTML_ELEMENT_UL || (x) == HTML_ELEMENT_DIR || (x) == HTML_ELEMENT_MENU)
#define html_islentry(x)	((x) == HTML_ELEMENT_LI || (x) == HTML_ELEMENT_DD || (x) == HTML_ELEMENT_DT)
#define html_issuper(x)	((x) == HTML_ELEMENT_CENTER || (x) == HTML_ELEMENT_DIV || (x) == HTML_ELEMENT_BLOCKQUOTE)
#define html_istable(x)	((x) == HTML_ELEMENT_TBODY || (x) == HTML_ELEMENT_THEAD || (x) == HTML_ELEMENT_TFOOT || (x) == HTML_ELEMENT_TR)
#define html_istentry(x)	((x) == HTML_ELEMENT_TD || (x) == HTML_ELEMENT_TH)


/*
 * Local types...
 */

typedef struct _html_file_s
{
  html_t	*html;			/* HTML document */
  const char	*url;			/* URL or filename */
  FILE		*fp;			/* File pointer */
  int		linenum;		/* Current line number */
  html_node_t	*parent;		/* Parent node */
} _html_file_t;


/*
 * Local functions...
 */

static int	html_compare_elements(const char **a, const char **b);
static int	html_parse_attr(_html_file_t *f, html_node_t *node, int ch);
static int	html_parse_comment(_html_file_t *f);
static int	html_parse_doctype(_html_file_t *f);
static int	html_parse_element(_html_file_t *f, int ch);
static int	html_parse_unknown(_html_file_t *f, const char *unk);


/*
 * 'htmlLoad()' - Load a HTML file into a document.
 */

int					/* O - 1 on success, 0 on failure */
htmlLoad(html_t     *html,		/* I - HTML document */
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

  f.html    = html;
  f.url     = url;
  f.fp      = fp;
  f.linenum = 1;
  f.parent  = NULL;

  if (!fp)
  {
    if (!(html->url_cb)(url, buffer, sizeof(buffer), html->url_ctx))
    {
      _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }

    if ((f.fp = fopen(buffer, "rb")) == NULL)
    {
      _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, 0, "Unable to open: %s", strerror(errno));
      return (0);
    }
  }
  else if (!url)
  {
    if (fp == stdin)
      f.url = "<stdin>";
    else
      f.url = "<unknown>";
  }

 /*
  * Parse file...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = getc(f.fp)) != EOF)
  {
    if (ch == '<')
    {
     /*
      * Read a HTML element...
      */

      ch = getc(f.fp);

      if (isspace(ch) || ch == '=' || ch == '<')
      {
       /*
        * Sigh...  "<" followed by anything but an element name is invalid HTML,
        * but many pages are still broken.  Log it and abort if the error
        * callback says to...
	*/

        if (!_htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, f.linenum, "Unquoted '<'."))
        {
          status = 0;
          break;
        }

	if (ch == '\n')
	  f.linenum ++;

        if (bufptr >= (bufend - 1))
        {
	 /*
	  * Add text string...
	  */

	  if (f.parent)
	  {
	    *bufptr = '\0';
	    htmlNewString(f.parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = 0;
	    _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, f.linenum, "Text without leading element or directive.");
	    break;
	  }
        }

        *bufptr++ = '<';

	if (ch == '<')
	  ungetc(ch, f.fp);
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
	    htmlNewString(f.parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = 0;
	    _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, f.linenum, "Text without leading element or directive.");
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

      if (ch == '\n')
	f.linenum ++;

      if (ch == '\n' || bufptr >= bufend)
      {
	if (f.parent)
	{
	  *bufptr = '\0';
	  htmlNewString(f.parent, buffer);
	  bufptr = buffer;
	}
	else
	{
	  status = 0;
	  _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, f.linenum, "Text without leading element or directive.");
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
      htmlNewString(f.parent, buffer);
    }
    else
    {
      status = 0;
      _htmlcssError(f.html->error_cb, f.html->error_ctx, f.url, f.linenum, "Text without leading element or directive.");
    }
  }

 /*
  * Close file as needed and return...
  */

  if (f.fp != fp)
    fclose(f.fp);

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
                html_node_t  *node,	/* I - HTML element node */
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
  while ((ch = getc(f->fp)) != EOF && ch != '=' && ch != '>' && !isspace(ch));

  *ptr = '\0';

  if (ch == '=')
  {
   /*
    * Read value...
    */

    ptr = value;
    end = value + sizeof(value) - 1;

    if ((ch = getc(f->fp)) == '\'' || ch == '\"')
    {
      char quote = ch;			/* Quote character */

      while ((ch = getc(f->fp)) != EOF && ch != quote)
      {
        if (ch == '\n')
          f->linenum ++;

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
      while ((ch = getc(f->fp)) != EOF && ch != '>' && !isspace(ch));
    }
    else if (ch == '\n')
      f->linenum ++;

    *ptr = '\0';
    htmlNewAttr(node, name, value);
  }
  else if (ch != EOF)
  {
   /*
    * Add "name=name"...
    */

    if (ch == '\n')
      f->linenum ++;

    htmlNewAttr(node, name, name);
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

  while ((ch = getc(f->fp)) != EOF)
  {
    if (ch == '\n')
      f->linenum ++;

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
    else if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Comment too long."))
      return (0);
    else
      break;
  }

  *bufptr = '\0';

  htmlNewComment(f->parent, buffer);

  if (ch == EOF)
    return (_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Unexpected end-of-file."));
  else if (ch != '>')
    return (_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Comment too long."));
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

  while ((ch = getc(f->fp)) != EOF)
  {
    if (!isspace(ch))
      break;
    else if (ch == '\n')
      f->linenum ++;
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

      while ((ch = getc(f->fp)) != EOF && ch != quote)
      {
        if (ch == '\n')
          f->linenum ++;

        if (bufptr < bufend)
          *bufptr++ = ch;
        else
          break;
      }
    }
    else if ((ch = getc(f->fp)) == '\n')
      f->linenum ++;
  }

  *bufptr = '\0';

  if (ch == EOF)
  {
    _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Unexpected end-of-file.");
    return (0);
  }
  else if (ch != '>')
    _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "<!DOCTYPE ...> too long.");

  return ((f->parent = htmlNewRoot(f->html, buffer)) != NULL);
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
  const char	**match;		/* Matching element */
  html_element_t element;		/* Element index */
  html_node_t	*node;			/* New node */
  int		close_el = ch == '/';	/* Close element? */


 /*
  * Read the element name...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;
  if (!close_el)
    *bufptr++ = ch;

  while ((ch = getc(f->fp)) != EOF)
  {
    if (isspace(ch) || ch == '>' || ch == '/')
      break;
    else if (bufptr < bufend)
      *bufptr++ = ch;
    else if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Element name too long."))
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
    _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Unexpected end-of-file.");
    return (0);
  }
  else if (ch == '\n')
    f->linenum ++;

  *bufptr = '\0';

 /*
  * Convert the name to an enum...
  */

  if (isspace(ch) || ch == '>' || ch == '/')
  {
    bufptr = buffer;
    match  = bsearch(&bufptr, htmlElements, sizeof(htmlElements) / sizeof(htmlElements[0]), sizeof(htmlElements[0]), (int (*)(const void *, const void *))html_compare_elements);

    if (match)
      element = (html_element_t)(match - htmlElements);
    else if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Unknown element '%s'.", buffer))
      return (0);
    else
      element = HTML_ELEMENT_UNKNOWN;
  }
  else
  {
    ungetc(ch, f->fp);
    element = HTML_ELEMENT_UNKNOWN;
  }

 /*
  * Parse unknown, comment, and doctype elements accordingly...
  */

  if (element == HTML_ELEMENT_DOCTYPE)
  {
    if (close_el)
    {
      _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Invalid </!DOCTYPE> seem.");
      return (0);
    }
    else if (f->html->root)
    {
      _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Duplicate <!DOCTYPE> seen.");
      return (0);
    }

    return (html_parse_doctype(f));
  }
  else if (!f->parent)
  {
    if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Missing <!DOCTYPE html> directive."))
      return (0);

    f->parent = htmlNewRoot(f->html, "html");
  }
  else if (element == HTML_ELEMENT_UNKNOWN)
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
  else if (element == HTML_ELEMENT_COMMENT)
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

    if (ch != '>' && !_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Invalid </%s> element.", buffer))
      return (0);

    for (node = f->parent; node; node = node->parent)
    {
      if (node->element == element)
        break;
    }

    if (node)
      f->parent = node->parent;
    else if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Missing <%s> for </%s> element.", buffer, buffer))
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
  else if (element == HTML_ELEMENT_THEAD || element == HTML_ELEMENT_TBODY || element == HTML_ELEMENT_TFOOT)
  {
    for (node = f->parent; node; node = node->parent)
    {
      if (node->element == HTML_ELEMENT_TABLE)
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
      else if (node->element == HTML_ELEMENT_TR)
      {
        break;
      }
      else if (node->element == HTML_ELEMENT_TABLE || html_istable(node->element))
      {
        if (!_htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "No <tr> element before <%s> element.", buffer))
          return (0);

        node = htmlNewElement(f->parent, HTML_ELEMENT_TR);
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

  node = htmlNewElement(f->parent, element);

  if (ch != '/' && !html_isleaf(element))
    f->parent = node;

  while (ch != '>' && ch != EOF)
  {
    while ((ch = getc(f->fp)) != EOF)
    {
      if (isspace(ch))
      {
        if (ch == '\n')
          f->linenum ++;
      }
      else
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

  while ((ch = getc(f->fp)) != EOF && ch != '>')
  {
    if (ch == '\n')
      f->linenum ++;

    if (bufptr < bufend)
      *bufptr++ = ch;
    else
      break;

    if (ch == '\'' || ch == '\"')
    {
      int quote = ch;			/* Quote character */

      while ((ch = getc(f->fp)) != EOF && ch != quote)
      {
        if (ch == '\n')
          f->linenum ++;

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
    _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Unexpected end-of-file.");
    return (0);
  }
  else if (ch != '>')
    _htmlcssError(f->html->error_cb, f->html->error_ctx, f->url, f->linenum, "Element too long.");

  return (_htmlNewUnknown(f->parent, buffer) != NULL);
}
