/*
 * HTML core functions for HTMLCSS library.
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
#include <stdarg.h>


/*
 * HTML element strings...
 */

const char * const	hcElements[HC_ELEMENT_MAX] =
{
  "", /* "*" */
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
 * 'hcHTMLDelete()' - Free the memory used by a HTML document.
 */

void
hcHTMLDelete(hc_html_t *html)		/* I - HTML document */
{
  if (html)
  {
    hcNodeDelete(html, html->root);
    free(html);
  }
}


/*
 * 'hcHTMLGetCSS()' - Get the stylesheet for a HTML document.
 */

hc_css_t *					/* O - Stylesheet */
hcHTMLGetCSS(hc_html_t *html)		/* I - HTML document */
{
  return (html ? html->css : NULL);
}


/*
 * 'hcNodeNew()' - Create a new HTML document.
 */

hc_html_t *				/* O - HTML document */
hcHTMLNew(hc_pool_t *pool,		/* I - Memory pool */
          hc_css_t  *css)		/* I - Base stylesheet */
{
  hc_html_t *html;			/* New HTML document */


  if ((html = (hc_html_t *)calloc(1, sizeof(hc_html_t))) != NULL)
  {
    html->pool     = pool;
    html->css      = css;
    html->error_cb = _hcDefaultErrorCB;
    html->url_cb   = _hcDefaultURLCB;
  }

  return (html);
}
