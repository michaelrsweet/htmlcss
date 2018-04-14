/*
 * HTML header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_HTML_H
#  define HTMLCSS_HTML_H

/*
 * Include necessary headers...
 */

#  include "common.h"


#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef enum				/* HTML element enum */
{
  HTML_ELEMENT_WILDCARD = -3,		/* Wildcard (*) */
  HTML_ELEMENT_UNKNOWN = -2,		/* Unknown element/directive */
  HTML_ELEMENT_STRING = -1,		/* String */
  HTML_ELEMENT_COMMENT,			/* !-- */
  HTML_ELEMENT_DOCTYPE,			/* !DOCTYPE */
  HTML_ELEMENT_A,
  HTML_ELEMENT_ABBR,
  HTML_ELEMENT_ACRONYM,
  HTML_ELEMENT_ADDRESS,
  HTML_ELEMENT_APPLET,
  HTML_ELEMENT_AREA,
  HTML_ELEMENT_ARTICLE,
  HTML_ELEMENT_ASIDE,
  HTML_ELEMENT_AUDIO,
  HTML_ELEMENT_B,
  HTML_ELEMENT_BASE,
  HTML_ELEMENT_BASEFONT,
  HTML_ELEMENT_BDI,
  HTML_ELEMENT_BDO,
  HTML_ELEMENT_BIG,
  HTML_ELEMENT_BLINK,
  HTML_ELEMENT_BLOCKQUOTE,
  HTML_ELEMENT_BODY,
  HTML_ELEMENT_BR,
  HTML_ELEMENT_BUTTON,
  HTML_ELEMENT_CANVAS,
  HTML_ELEMENT_CAPTION,
  HTML_ELEMENT_CENTER,
  HTML_ELEMENT_CITE,
  HTML_ELEMENT_CODE,
  HTML_ELEMENT_COL,
  HTML_ELEMENT_COLGROUP,
  HTML_ELEMENT_DATA,
  HTML_ELEMENT_DATALIST,
  HTML_ELEMENT_DD,
  HTML_ELEMENT_DEL,
  HTML_ELEMENT_DETAILS,
  HTML_ELEMENT_DFN,
  HTML_ELEMENT_DIALOG,
  HTML_ELEMENT_DIR,
  HTML_ELEMENT_DIV,
  HTML_ELEMENT_DL,
  HTML_ELEMENT_DT,
  HTML_ELEMENT_EM,
  HTML_ELEMENT_EMBED,
  HTML_ELEMENT_FIELDSET,
  HTML_ELEMENT_FIGCAPTION,
  HTML_ELEMENT_FIGURE,
  HTML_ELEMENT_FONT,
  HTML_ELEMENT_FOOTER,
  HTML_ELEMENT_FORM,
  HTML_ELEMENT_FRAME,
  HTML_ELEMENT_FRAMESET,
  HTML_ELEMENT_H1,
  HTML_ELEMENT_H2,
  HTML_ELEMENT_H3,
  HTML_ELEMENT_H4,
  HTML_ELEMENT_H5,
  HTML_ELEMENT_H6,
  HTML_ELEMENT_HEAD,
  HTML_ELEMENT_HEADER,
  HTML_ELEMENT_HR,
  HTML_ELEMENT_HTML,
  HTML_ELEMENT_I,
  HTML_ELEMENT_IFRAME,
  HTML_ELEMENT_IMG,
  HTML_ELEMENT_INPUT,
  HTML_ELEMENT_INS,
  HTML_ELEMENT_ISINDEX,
  HTML_ELEMENT_KBD,
  HTML_ELEMENT_LABEL,
  HTML_ELEMENT_LEGEND,
  HTML_ELEMENT_LI,
  HTML_ELEMENT_LINK,
  HTML_ELEMENT_MAIN,
  HTML_ELEMENT_MAP,
  HTML_ELEMENT_MARK,
  HTML_ELEMENT_MENU,
  HTML_ELEMENT_META,
  HTML_ELEMENT_METER,
  HTML_ELEMENT_MULTICOL,
  HTML_ELEMENT_NAV,
  HTML_ELEMENT_NOBR,
  HTML_ELEMENT_NOFRAMES,
  HTML_ELEMENT_NOSCRIPT,
  HTML_ELEMENT_OBJECT,
  HTML_ELEMENT_OL,
  HTML_ELEMENT_OPTGROUP,
  HTML_ELEMENT_OPTION,
  HTML_ELEMENT_OUTPUT,
  HTML_ELEMENT_P,
  HTML_ELEMENT_PARAM,
  HTML_ELEMENT_PICTURE,
  HTML_ELEMENT_PRE,
  HTML_ELEMENT_PROGRESS,
  HTML_ELEMENT_Q,
  HTML_ELEMENT_RB,
  HTML_ELEMENT_RP,
  HTML_ELEMENT_RT,
  HTML_ELEMENT_RTC,
  HTML_ELEMENT_RUBY,
  HTML_ELEMENT_S,
  HTML_ELEMENT_SAMP,
  HTML_ELEMENT_SCRIPT,
  HTML_ELEMENT_SECTION,
  HTML_ELEMENT_SELECT,
  HTML_ELEMENT_SMALL,
  HTML_ELEMENT_SOURCE,
  HTML_ELEMENT_SPACER,
  HTML_ELEMENT_SPAN,
  HTML_ELEMENT_STRIKE,
  HTML_ELEMENT_STRONG,
  HTML_ELEMENT_STYLE,
  HTML_ELEMENT_SUB,
  HTML_ELEMENT_SUMMARY,
  HTML_ELEMENT_SUP,
  HTML_ELEMENT_TABLE,
  HTML_ELEMENT_TBODY,
  HTML_ELEMENT_TD,
  HTML_ELEMENT_TEMPLATE,
  HTML_ELEMENT_TEXTAREA,
  HTML_ELEMENT_TFOOT,
  HTML_ELEMENT_TH,
  HTML_ELEMENT_THEAD,
  HTML_ELEMENT_TIME,
  HTML_ELEMENT_TITLE,
  HTML_ELEMENT_TR,
  HTML_ELEMENT_TRACK,
  HTML_ELEMENT_TT,
  HTML_ELEMENT_U,
  HTML_ELEMENT_UL,
  HTML_ELEMENT_VAR,
  HTML_ELEMENT_VIDEO,
  HTML_ELEMENT_WBR,
  HTML_ELEMENT_MAX
} html_element_t;

typedef struct _css_s css_t;		/* CSS data */

typedef struct _html_node_s html_node_t;/* HTML node */

typedef struct _html_s html_t;		/* HTML document */


/*
 * Globals...
 */

extern const char * const	htmlElements[HTML_ELEMENT_MAX];


/*
 * Functions...
 */

extern void		htmlDelete(html_t *html);
extern void		htmlDeleteAttr(html_node_t *node, const char *name);
extern void		htmlDeleteNode(html_t *html, html_node_t *node);
extern html_node_t	*htmlFindNode(html_t *html, html_node_t *current, html_element_t element, const char *id);
extern const char	*htmlGetAttr(html_node_t *node, const char *name);
extern int		htmlGetAttrCount(html_node_t *node);
extern const char	*htmlGetAttrIndex(html_node_t *node, int idx, const char **name);
extern const char	*htmlGetComment(html_node_t *node);
extern css_t		*htmlGetCSS(html_t *html);
extern const char	*htmlGetDOCTYPE(html_t *html);
extern html_element_t	htmlGetElement(html_node_t *node);
extern html_node_t	*htmlGetFirstChildNode(html_node_t *node);
extern html_node_t	*htmlGetLastChildNode(html_node_t *node);
extern html_node_t	*htmlGetNextSiblingNode(html_node_t *node);
extern html_node_t	*htmlGetParentNode(html_node_t *node);
extern html_node_t	*htmlGetPrevSiblingNode(html_node_t *node);
extern html_node_t	*htmlGetRootNode(html_t *html);
extern const char	*htmlGetString(html_node_t *node);
extern int		htmlLoad(html_t *html, const char *filename, FILE *fp);
extern html_t		*htmlNew(css_t *css);
extern void		htmlNewAttr(html_node_t *node, const char *name, const char *value);
extern html_node_t	*htmlNewComment(html_node_t *parent, const char *c);
extern html_node_t	*htmlNewElement(html_node_t *parent, html_element_t element);
extern html_node_t	*htmlNewRoot(html_t *html, const char *doctype);
extern html_node_t	*htmlNewString(html_node_t *parent, const char *s);
extern void		htmlSetErrorCallback(html_t *html, htmlcss_error_cb_t cb, void *ctx);
extern void		htmlSetURLCallback(html_t *html, htmlcss_url_cb_t cb, void *ctx);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_HTML_H */
