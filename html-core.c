/*
 * HTML core functions for HTMLCSS library.
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
#include <stdarg.h>


/*
 * HTML element strings...
 */

const char * const	htmlElements[HTML_ELEMENT_MAX] =
{
  "" /* "*" */
  "!--",
  "!DOCTYPE",
  "a",
  "abbr",
  "acronym",
  "address",
  "applet",
  "area",
  "article",
  "aside",
  "audio",
  "b",
  "base",
  "basefont",
  "bdi",
  "bdo",
  "big",
  "blink",
  "blockquote",
  "body",
  "br",
  "button",
  "canvas",
  "caption",
  "center",
  "cite",
  "code",
  "col",
  "colgroup",
  "data",
  "datalist",
  "dd",
  "del",
  "details",
  "dfn",
  "dialog",
  "dir",
  "div",
  "dl",
  "dt",
  "em",
  "embed",
  "fieldset",
  "figcaption",
  "figure",
  "font",
  "footer",
  "form",
  "frame",
  "frameset",
  "h1",
  "h2",
  "h3",
  "h4",
  "h5",
  "h6",
  "head",
  "header",
  "hr",
  "html",
  "i",
  "iframe",
  "img",
  "input",
  "ins",
  "isindex",
  "kbd",
  "label",
  "legend",
  "li",
  "link",
  "main",
  "map",
  "mark",
  "menu",
  "meta",
  "meter",
  "multicol",
  "nav",
  "nobr",
  "noframes",
  "noscript",
  "object",
  "ol",
  "optgroup",
  "option",
  "output",
  "p",
  "param",
  "picture",
  "pre",
  "progress",
  "q",
  "rb",
  "rp",
  "rt",
  "rtc",
  "ruby",
  "s",
  "samp",
  "script",
  "section",
  "select",
  "small",
  "source",
  "spacer",
  "span",
  "strike",
  "strong",
  "style",
  "sub",
  "summary",
  "sup",
  "table",
  "tbody",
  "td",
  "template",
  "textarea",
  "tfoot",
  "th",
  "thead",
  "time",
  "title",
  "tr",
  "track",
  "tt",
  "u",
  "ul",
  "var",
  "video",
  "wbr"
};


/*
 * 'htmlDelete()' - Free the memory used by a HTML document.
 */

void
htmlDelete(html_t *html)		/* I - HTML document */
{
  if (html)
  {
    htmlDeleteNode(html, html->root);
    free(html);
  }
}


/*
 * 'htmlGetCSS()' - Get the stylesheet for a HTML document.
 */

css_t *					/* O - Stylesheet */
htmlGetCSS(html_t *html)		/* I - HTML document */
{
  return (html ? html->css : NULL);
}


/*
 * 'htmlNew()' - Create a new HTML document.
 */

html_t *				/* O - HTML document */
htmlNew(htmlcss_pool_t *pool,		/* I - Memory pool */
        css_t          *css)		/* I - Base stylesheet */
{
  html_t *html;				/* New HTML document */


  if ((html = (html_t *)calloc(1, sizeof(html_t))) != NULL)
  {
    html->pool     = pool;
    html->css      = css;
    html->error_cb = _htmlcssDefaultErrorCB;
    html->url_cb   = _htmlcssDefaultURLCB;
  }

  return (html);
}


/*
 * 'htmlSetErrorCallback()' - Set the error reporting callback.
 *
 * The default error callback writes the message to `stderr`.
 *
 * The error callback returns 1 to continue processing or 0 to stop immediately.
 */

void
htmlSetErrorCallback(
    html_t             *html,		/* I - HTML document */
    htmlcss_error_cb_t cb,		/* I - Error callback or `NULL` for the default */
    void               *ctx)		/* I - Context pointer for callback */
{
  if (!html)
    return;

  html->error_cb  = cb ? cb : _htmlcssDefaultErrorCB;
  html->error_ctx = ctx;
}


/*
 * 'htmlSetURLCallback()' - Set the URL callback.
 *
 * The default URL callback supports local files (only).
 *
 * The URL callback returns a local pathname (copied to the specified buffer)
 * or `NULL` if the URL cannot be loaded/found.
 */

void
htmlSetURLCallback(
    html_t           *html,		/* I - HTML document */
    htmlcss_url_cb_t cb,		/* I - URL callback or `NULL` for the default */
    void             *ctx)		/* I - Context pointer for callback */
{
  if (!html)
    return;

  html->url_cb  = cb ? cb : _htmlcssDefaultURLCB;
  html->url_ctx = ctx;
}
