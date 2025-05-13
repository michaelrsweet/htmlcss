//
// HTML load functions for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

//
// Include necessary headers...
//

#include "html-private.h"
#include "file-private.h"
#include <ctype.h>


//
// Local macros...
//

#define html_isblock(x)	((x) == HC_ELEMENT_ADDRESS || (x) == HC_ELEMENT_P || (x) == HC_ELEMENT_PRE || ((x) >= HC_ELEMENT_H1 && (x) <= HC_ELEMENT_H6) || (x) == HC_ELEMENT_HR || (x) == HC_ELEMENT_TABLE)
#define html_isleaf(x)	((x) == HC_ELEMENT_AREA || (x) == HC_ELEMENT_BASE || (x) == HC_ELEMENT_BR || (x) == HC_ELEMENT_COL || (x) == HC_ELEMENT_EMBED || (x) == HC_ELEMENT_HR ||(x) == HC_ELEMENT_IMG || (x) == HC_ELEMENT_INPUT || (x) == HC_ELEMENT_ISINDEX || (x) == HC_ELEMENT_LINK || (x) == HC_ELEMENT_META || (x) == HC_ELEMENT_PARAM || (x) == HC_ELEMENT_SOURCE || (x) == HC_ELEMENT_SPACER || (x) == HC_ELEMENT_TRACK || (x) == HC_ELEMENT_WBR)
#define html_islist(x)	((x) == HC_ELEMENT_DL || (x) == HC_ELEMENT_OL || (x) == HC_ELEMENT_UL || (x) == HC_ELEMENT_DIR || (x) == HC_ELEMENT_MENU)
#define html_islentry(x)	((x) == HC_ELEMENT_LI || (x) == HC_ELEMENT_DD || (x) == HC_ELEMENT_DT)
#define html_issuper(x)	((x) == HC_ELEMENT_CENTER || (x) == HC_ELEMENT_DIV || (x) == HC_ELEMENT_BLOCKQUOTE)
#define html_istable(x)	((x) == HC_ELEMENT_TBODY || (x) == HC_ELEMENT_THEAD || (x) == HC_ELEMENT_TFOOT || (x) == HC_ELEMENT_TR)
#define html_istentry(x)	((x) == HC_ELEMENT_TD || (x) == HC_ELEMENT_TH)


//
// Local functions...
//

static int	html_parse_attr(hc_file_t *file, int ch, hc_node_t *node);
static bool	html_parse_comment(hc_file_t *file, hc_node_t **parent);
static bool	html_parse_doctype(hc_file_t *file, hc_html_t *html, hc_node_t **parent);
static bool	html_parse_element(hc_file_t *file, int ch, hc_html_t *html, hc_node_t **parent);
static bool	html_parse_unknown(hc_file_t *file, hc_node_t **parent, const char *unk);


//
// 'hcHTMLImport()' - Load a HTML file into a document.
//

bool					// O - `true` on success, `false` on error
hcHTMLImport(hc_html_t *html,		// I - HTML document
	     hc_file_t *file)		// I - File to import
{
  hc_node_t	*parent = NULL;		// Parent node
  bool		status = true;		// Load status
  int		ch;			// Current character
  char		buffer[8192],		// Temporary buffer
		*bufptr,		// Pointer into buffer
		*bufend;		// End of buffer


 /*
  * Range check input...
  */

  if (!html || html->root || !file)
    return (false);

//  f.parent       = NULL;

 /*
  * Parse file...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = hcFileGetc(file)) != EOF)
  {
    if (ch == '<')
    {
     /*
      * Read a HTML element...
      */

      ch = hcFileGetc(file);

      if (isspace(ch) || ch == '=' || ch == '<')
      {
       /*
        * Sigh...  "<" followed by anything but an element name is invalid HTML,
        * but many pages are still broken.  Log it and abort if the error
        * callback says to...
	*/

        if (!_hcFileError(file, "Unquoted '<'."))
        {
          status = false;
          break;
        }

        if (bufptr >= (bufend - 1))
        {
	 /*
	  * Add text string...
	  */

	  if (parent)
	  {
	    *bufptr = '\0';
	    hcNodeNewString(parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = false;
	    _hcFileError(file, "Text without leading element or directive.");
	    break;
	  }
        }

        *bufptr++ = '<';

	if (ch == '<')
	  hcFileUngetc(file, ch);
        else
          *bufptr++ = (char)ch;
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

	  if (parent)
	  {
	    *bufptr = '\0';
	    hcNodeNewString(parent, buffer);
	    bufptr = buffer;
	  }
	  else
	  {
	    status = false;
	    _hcFileError(file, "Text without leading element or directive.");
	    break;
	  }
	}

	if (!(status = html_parse_element(file, ch, html, &parent)))
	  break;
      }
    }
    else
    {
      if (bufptr < bufend)
        *bufptr++ = (char)ch;

      if (ch == '\n' || bufptr >= bufend)
      {
	if (parent)
	{
	  *bufptr = '\0';
	  hcNodeNewString(parent, buffer);
	  bufptr = buffer;
	}
	else
	{
	  status = false;
	  _hcFileError(file, "Text without leading element or directive.");
	}
      }
    }

  }

  if (bufptr > buffer)
  {
   /*
    * Add trailing text string...
    */

    if (parent)
    {
      *bufptr = '\0';
      hcNodeNewString(parent, buffer);
    }
    else
    {
      status = false;
      _hcFileError(file, "Text without leading element or directive.");
    }
  }

  return (status);
}


//
// 'html_parse_attr()' - Parse an attribute.
//

static int				// O - Character or `EOF` to stop
html_parse_attr(hc_file_t *file,	// I - File to read from
                int       ch,		// I - Initial character
                hc_node_t *node)	// I - HTML element node
{
  char	name[256],			// Name string
	value[2048],			// Value string
	*ptr,				// Pointer into string
	*end;				// End of string


 /*
  * Read name...
  */

  ptr = name;
  end = name + sizeof(name) - 1;

  do
  {
    if (ptr < end)
      *ptr++ = (char)tolower(ch);
    else
      break;
  }
  while ((ch = hcFileGetc(file)) != EOF && ch != '=' && ch != '>' && !isspace(ch));

  *ptr = '\0';

  if (ch == '=')
  {
   /*
    * Read value...
    */

    ptr = value;
    end = value + sizeof(value) - 1;

    if ((ch = hcFileGetc(file)) == '\'' || ch == '\"')
    {
      int quote = ch;			// Quote character

      while ((ch = hcFileGetc(file)) != EOF && ch != quote)
      {
	if (ptr < end)
	  *ptr++ = (char)ch;
	else
	  break;
      }
    }
    else if (!isspace(ch) && ch != '>' && ch != EOF)
    {
      do
      {
	if (ptr < end)
	  *ptr++ = (char)ch;
	else
	  break;
      }
      while ((ch = hcFileGetc(file)) != EOF && ch != '>' && !isspace(ch));
    }

    *ptr = '\0';
    hcNodeAttrSetNameValue(node, name, value);
  }
  else if (ch != EOF)
  {
   /*
    * Add "name=name"...
    */

    hcNodeAttrSetNameValue(node, name, name);
  }

  return (ch);
}


//
// 'html_parse_comment()' - Parse a comment.
//

static bool				// O  - `true` to continue, `false` to stop
html_parse_comment(hc_file_t *file,	// I  - File to read from
                   hc_node_t **parent)	// IO - Parent node
{
  int	ch;				// Current character
  char	buffer[8192],			// String buffer
	*bufptr,			// Pointer into buffer
	*bufend;			// End of buffer


  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = hcFileGetc(file)) != EOF)
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
      *bufptr++ = (char)ch;
    else if (!_hcFileError(file, "Comment too long."))
      return (false);
    else
      break;
  }

  *bufptr = '\0';

  hcNodeNewComment(*parent, buffer);

  if (ch == EOF)
    return (_hcFileError(file, "Unexpected end-of-file."));
  else if (ch != '>')
    return (_hcFileError(file, "Comment too long."));
  else
    return (true);
}


//
// 'html_parse_doctype()' - Parse a DOCTYPE element.
//

static bool				// O  - `true` to continue, `false` to stop
html_parse_doctype(hc_file_t *file,	// I  - File to read from
                   hc_html_t *html,	// I  - HTML document
		   hc_node_t **parent)	// IO - Parent node
{
  int	ch;				// Character from file
  char	buffer[2048],			// String buffer
	*bufptr,			// Pointer into buffer
	*bufend;			// End of buffer


  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = hcFileGetc(file)) != EOF)
  {
    if (!isspace(ch))
      break;
  }

  while (ch != EOF && ch != '>')
  {
    if (bufptr < bufend)
      *bufptr++ = (char)ch;
    else
      break;

    if (ch == '\'' || ch == '\"')
    {
      int quote = ch;			// Quote character

      while ((ch = hcFileGetc(file)) != EOF && ch != quote)
      {
        if (bufptr < bufend)
          *bufptr++ = (char)ch;
        else
          break;
      }
    }
    else
      ch = hcFileGetc(file);
  }

  *bufptr = '\0';

  if (ch == EOF)
  {
    _hcFileError(file, "Unexpected end-of-file.");
    return (false);
  }
  else if (ch != '>')
    _hcFileError(file, "<!DOCTYPE ...> too long.");

  *parent = hcHTMLNewRootNode(html, buffer);

  return (*parent != NULL);
}


//
// 'html_parse_element()' - Parse an element.
//

static bool				// O  - `true` to continue, `false` to stop
html_parse_element(hc_file_t *file,	// I  - File to read from
                   int       ch,	// I  - Initial character after '<'
                   hc_html_t *html,	// I  - HTML document
                   hc_node_t **parent)	// IO - Parent node
{
  char		buffer[256],		// String buffer
		*bufptr,		// Pointer into buffer
		*bufend;		// End of buffer
  hc_element_t	element;		// Element index
  hc_node_t	*node;			// New node
  bool		close_el = ch == '/';	// Close element?


 /*
  * Read the element name...
  */

  bufptr = buffer;
  bufend = buffer + sizeof(buffer) - 1;
  if (!close_el)
    *bufptr++ = (char)ch;

  while ((ch = hcFileGetc(file)) != EOF)
  {
    if (isspace(ch) || ch == '>' || ch == '/')
      break;
    else if (bufptr < bufend)
      *bufptr++ = (char)ch;
    else if (!_hcFileError(file, "Element name too long."))
      return (false);
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
    _hcFileError(file, "Unexpected end-of-file.");
    return (false);
  }

  *bufptr = '\0';

 /*
  * Convert the name to an enum...
  */

  if (isspace(ch) || ch == '>' || ch == '/')
  {
    element = hcElementValue(buffer);

    if (element == HC_ELEMENT_UNKNOWN && !_hcFileError(file, "Unknown element '%s'.", buffer))
      return (false);
  }
  else
  {
    hcFileUngetc(file, ch);
    element = HC_ELEMENT_UNKNOWN;
  }

 /*
  * Parse unknown, comment, and doctype elements accordingly...
  */

  if (element == HC_ELEMENT_DOCTYPE)
  {
    if (close_el)
    {
      _hcFileError(file, "Invalid </!DOCTYPE> seem.");
      return (false);
    }
    else if (html->root)
    {
      _hcFileError(file, "Duplicate <!DOCTYPE> seen.");
      return (false);
    }

    return (html_parse_doctype(file, html, parent));
  }
  else if (!*parent)
  {
    if (!_hcFileError(file, "Missing <!DOCTYPE html> directive."))
      return (false);

    *parent = hcHTMLNewRootNode(html, "html");
  }
  else if (element == HC_ELEMENT_UNKNOWN)
  {
    if (close_el)
    {
      char	unk[257];		// Unknown value

      snprintf(unk, sizeof(unk), "/%s", buffer);
      return (html_parse_unknown(file, parent, unk));
    }
    else
    {
      return (html_parse_unknown(file, parent, buffer));
    }
  }
  else if (element == HC_ELEMENT_COMMENT)
  {
    return (html_parse_comment(file, parent));
  }

 /*
  * Otherwise add the element (or close it) in the right place...
  */

  if (close_el)
  {
   /*
    * Close the specified element...
    */

    if (ch != '>' && !_hcFileError(file, "Invalid </%s> element.", buffer))
      return (false);

    for (node = *parent; node; node = node->parent)
    {
      if (node->element == element)
        break;
    }

    if (node)
      *parent = node->parent;
    else if (!_hcFileError(file, "Missing <%s> for </%s> element.", buffer, buffer))
      return (false);

    return (true);
  }

 /*
  * HTML doesn't enforce strict open/close markup semantics, so allow <p>, <li>,
  * etc. to close out like markup...
  */

  if (html_issuper(element))
  {
    for (node = *parent; node; node = node->parent)
    {
      if (html_istentry(node->element))
	break;
    }
  }
  else if (html_islist(element))
  {
    for (node = *parent; node; node = node->parent)
    {
      if (html_isblock(node->element) || html_islentry(node->element) || html_istentry(node->element) || html_issuper(node->element))
	break;
    }
  }
  else if (html_islentry(element))
  {
    for (node = *parent; node; node = node->parent)
    {
      if (html_islist(node->element))
	break;
    }
  }
  else if (html_isblock(element))
  {
    for (node = *parent; node; node = node->parent)
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
    for (node = *parent; node; node = node->parent)
    {
      if (node->element == HC_ELEMENT_TABLE)
	break;
    }
  }
  else if (html_istentry(element))
  {
    for (node = *parent; node; node = node->parent)
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
        if (!_hcFileError(file, "No <tr> element before <%s> element.", buffer))
          return (false);

        node = hcNodeNewElement(*parent, HC_ELEMENT_TR);
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
    *parent = node;

  node = hcNodeNewElement(*parent, element);

  if (ch != '/' && !html_isleaf(element))
    *parent = node;

  while (ch != '>' && ch != EOF)
  {
    while ((ch = hcFileGetc(file)) != EOF)
    {
      if (!isspace(ch))
        break;
    }

    if (ch != '>')
      ch = html_parse_attr(file, ch, node);
  }

  return (ch == '>');
}


//
// 'html_parse_unknown()' - Parse an unknown element or processing directive.
//

static bool				// O  - `true` to continue, `false` to stop
html_parse_unknown(hc_file_t  *file,	// I  - File to read from
                   hc_node_t  **parent,	// IO - Parent node
                   const char *unk)	// I  - Start of unknown markup
{
  int	ch;				// Character from file
  char	buffer[2048],			// String buffer
	*bufptr,			// Pointer into buffer
	*bufend;			// End of buffer


  strncpy(buffer, unk, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  bufptr = buffer + strlen(buffer);
  bufend = buffer + sizeof(buffer) - 1;

  while ((ch = hcFileGetc(file)) != EOF && ch != '>')
  {
    if (bufptr < bufend)
      *bufptr++ = (char)ch;
    else
      break;

    if (ch == '\'' || ch == '\"')
    {
      int quote = ch;			// Quote character

      while ((ch = hcFileGetc(file)) != EOF && ch != quote)
      {
        if (bufptr < bufend)
          *bufptr++ = (char)ch;
        else
          break;
      }

      if (ch == EOF || ch != quote || bufptr >= bufend)
        break;

      *bufptr++ = (char)ch;
    }
  }

  *bufptr = '\0';

  if (ch == EOF)
  {
    _hcFileError(file, "Unexpected end-of-file.");
    return (false);
  }
  else if (ch != '>')
    _hcFileError(file, "Element too long.");

  return (_hcNodeNewUnknown(*parent, buffer) != NULL);
}
