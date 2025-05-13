//
// HTML header file for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#ifndef HTMLCSS_HTML_H
#  define HTMLCSS_HTML_H
#  include "css.h"
#  ifdef __cplusplus
extern "C" {
#  endif // __cplusplus


//
// Types...
//

typedef enum				// HTML element enum
{
  HC_ELEMENT_UNKNOWN = -2,		// Unknown element/directive
  HC_ELEMENT_STRING = -1,		// String
  HC_ELEMENT_WILDCARD,			// Wildcard (*)
  HC_ELEMENT_COMMENT,			// !--
  HC_ELEMENT_DOCTYPE,			// !DOCTYPE
  HC_ELEMENT_A,
  HC_ELEMENT_ABBR,
  HC_ELEMENT_ACRONYM,
  HC_ELEMENT_ADDRESS,
  HC_ELEMENT_APPLET,
  HC_ELEMENT_AREA,
  HC_ELEMENT_ARTICLE,
  HC_ELEMENT_ASIDE,
  HC_ELEMENT_AUDIO,
  HC_ELEMENT_B,
  HC_ELEMENT_BASE,
  HC_ELEMENT_BASEFONT,
  HC_ELEMENT_BDI,
  HC_ELEMENT_BDO,
  HC_ELEMENT_BIG,
  HC_ELEMENT_BLINK,
  HC_ELEMENT_BLOCKQUOTE,
  HC_ELEMENT_BODY,
  HC_ELEMENT_BR,
  HC_ELEMENT_BUTTON,
  HC_ELEMENT_CANVAS,
  HC_ELEMENT_CAPTION,
  HC_ELEMENT_CENTER,
  HC_ELEMENT_CITE,
  HC_ELEMENT_CODE,
  HC_ELEMENT_COL,
  HC_ELEMENT_COLGROUP,
  HC_ELEMENT_DATA,
  HC_ELEMENT_DATALIST,
  HC_ELEMENT_DD,
  HC_ELEMENT_DEL,
  HC_ELEMENT_DETAILS,
  HC_ELEMENT_DFN,
  HC_ELEMENT_DIALOG,
  HC_ELEMENT_DIR,
  HC_ELEMENT_DIV,
  HC_ELEMENT_DL,
  HC_ELEMENT_DT,
  HC_ELEMENT_EM,
  HC_ELEMENT_EMBED,
  HC_ELEMENT_FIELDSET,
  HC_ELEMENT_FIGCAPTION,
  HC_ELEMENT_FIGURE,
  HC_ELEMENT_FONT,
  HC_ELEMENT_FOOTER,
  HC_ELEMENT_FORM,
  HC_ELEMENT_FRAME,
  HC_ELEMENT_FRAMESET,
  HC_ELEMENT_H1,
  HC_ELEMENT_H2,
  HC_ELEMENT_H3,
  HC_ELEMENT_H4,
  HC_ELEMENT_H5,
  HC_ELEMENT_H6,
  HC_ELEMENT_HEAD,
  HC_ELEMENT_HEADER,
  HC_ELEMENT_HR,
  HC_ELEMENT_HTML,
  HC_ELEMENT_I,
  HC_ELEMENT_IFRAME,
  HC_ELEMENT_IMG,
  HC_ELEMENT_INPUT,
  HC_ELEMENT_INS,
  HC_ELEMENT_ISINDEX,
  HC_ELEMENT_KBD,
  HC_ELEMENT_LABEL,
  HC_ELEMENT_LEGEND,
  HC_ELEMENT_LI,
  HC_ELEMENT_LINK,
  HC_ELEMENT_MAIN,
  HC_ELEMENT_MAP,
  HC_ELEMENT_MARK,
  HC_ELEMENT_MENU,
  HC_ELEMENT_META,
  HC_ELEMENT_METER,
  HC_ELEMENT_MULTICOL,
  HC_ELEMENT_NAV,
  HC_ELEMENT_NOBR,
  HC_ELEMENT_NOFRAMES,
  HC_ELEMENT_NOSCRIPT,
  HC_ELEMENT_OBJECT,
  HC_ELEMENT_OL,
  HC_ELEMENT_OPTGROUP,
  HC_ELEMENT_OPTION,
  HC_ELEMENT_OUTPUT,
  HC_ELEMENT_P,
  HC_ELEMENT_PARAM,
  HC_ELEMENT_PICTURE,
  HC_ELEMENT_PRE,
  HC_ELEMENT_PROGRESS,
  HC_ELEMENT_Q,
  HC_ELEMENT_RB,
  HC_ELEMENT_RP,
  HC_ELEMENT_RT,
  HC_ELEMENT_RTC,
  HC_ELEMENT_RUBY,
  HC_ELEMENT_S,
  HC_ELEMENT_SAMP,
  HC_ELEMENT_SCRIPT,
  HC_ELEMENT_SECTION,
  HC_ELEMENT_SELECT,
  HC_ELEMENT_SMALL,
  HC_ELEMENT_SOURCE,
  HC_ELEMENT_SPACER,
  HC_ELEMENT_SPAN,
  HC_ELEMENT_STRIKE,
  HC_ELEMENT_STRONG,
  HC_ELEMENT_STYLE,
  HC_ELEMENT_SUB,
  HC_ELEMENT_SUMMARY,
  HC_ELEMENT_SUP,
  HC_ELEMENT_TABLE,
  HC_ELEMENT_TBODY,
  HC_ELEMENT_TD,
  HC_ELEMENT_TEMPLATE,
  HC_ELEMENT_TEXTAREA,
  HC_ELEMENT_TFOOT,
  HC_ELEMENT_TH,
  HC_ELEMENT_THEAD,
  HC_ELEMENT_TIME,
  HC_ELEMENT_TITLE,
  HC_ELEMENT_TR,
  HC_ELEMENT_TRACK,
  HC_ELEMENT_TT,
  HC_ELEMENT_U,
  HC_ELEMENT_UL,
  HC_ELEMENT_VAR,
  HC_ELEMENT_VIDEO,
  HC_ELEMENT_WBR,
  HC_ELEMENT_MAX
} hc_element_t;

typedef struct _hc_node_s hc_node_t;	// HTML node

typedef struct _hc_html_s hc_html_t;	// HTML document


//
// Functions...
//

extern const char	*hcElementString(hc_element_t e) _HC_PUBLIC;
extern hc_element_t	hcElementValue(const char *s) _HC_PUBLIC;

extern void		hcHTMLDelete(hc_html_t *html) _HC_PUBLIC;
extern hc_node_t	*hcHTMLFindNode(hc_html_t *html, hc_node_t *current, hc_element_t element, const char *id) _HC_PUBLIC;
extern hc_css_t		*hcHTMLGetCSS(hc_html_t *html) _HC_PUBLIC;
extern const char	*hcHTMLGetDOCTYPE(hc_html_t *html) _HC_PUBLIC;
extern hc_node_t	*hcHTMLGetRootNode(hc_html_t *html) _HC_PUBLIC;
extern bool		hcHTMLImport(hc_html_t *html, hc_file_t *file) _HC_PUBLIC;
extern hc_html_t	*hcHTMLNew(hc_pool_t *pool, hc_css_t *css) _HC_PUBLIC;
extern hc_node_t	*hcHTMLNewRootNode(hc_html_t *html, const char *doctype) _HC_PUBLIC;
extern void		hcHTMLSetErrorCallback(hc_html_t *html, hc_error_cb_t cb, void *cbdata) _HC_PUBLIC;
extern void		hcHTMLSetURLCallback(hc_html_t *html, hc_url_cb_t cb, void *cbdata) _HC_PUBLIC;

extern size_t		hcNodeAttrGetCount(hc_node_t *node) _HC_PUBLIC;
extern const char	*hcNodeAttrGetIndexNameValue(hc_node_t *node, size_t idx, const char **name) _HC_PUBLIC;
extern const char	*hcNodeAttrGetNameValue(hc_node_t *node, const char *name) _HC_PUBLIC;
extern void		hcNodeAttrRemove(hc_node_t *node, const char *name) _HC_PUBLIC;
extern void		hcNodeAttrSetNameValue(hc_node_t *node, const char *name, const char *value) _HC_PUBLIC;

extern bool		hcNodeComputeCSSBox(hc_node_t *node, hc_compute_t compute, hc_box_t *box) _HC_PUBLIC;
extern char		*hcNodeComputeCSSContent(hc_node_t *node, hc_compute_t compute) _HC_PUBLIC;
extern hc_display_t	hcNodeComputeCSSDisplay(hc_node_t *node, hc_compute_t compute) _HC_PUBLIC;
extern bool		hcNodeComputeCSSMedia(hc_node_t *node, hc_compute_t compute, hc_media_t *media) _HC_PUBLIC;
extern const hc_dict_t	*hcNodeComputeCSSProperties(hc_node_t *node, hc_compute_t compute) _HC_PUBLIC;
extern bool		hcNodeComputeCSSTable(hc_node_t *node, hc_compute_t compute, hc_table_t *table) _HC_PUBLIC;
extern bool		hcNodeComputeCSSText(hc_node_t *node, hc_compute_t compute, hc_text_t *text) _HC_PUBLIC;

extern void		hcNodeDelete(hc_html_t *html, hc_node_t *node) _HC_PUBLIC;
extern const char	*hcNodeGetComment(hc_node_t *node) _HC_PUBLIC;
extern hc_element_t	hcNodeGetElement(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeGetFirstChildNode(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeGetLastChildNode(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeGetNextSiblingNode(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeGetParentNode(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeGetPrevSiblingNode(hc_node_t *node) _HC_PUBLIC;
extern const char	*hcNodeGetString(hc_node_t *node) _HC_PUBLIC;
extern hc_node_t	*hcNodeNewComment(hc_node_t *parent, const char *c) _HC_PUBLIC;
extern hc_node_t	*hcNodeNewElement(hc_node_t *parent, hc_element_t element) _HC_PUBLIC;
extern hc_node_t	*hcNodeNewString(hc_node_t *parent, const char *s) _HC_PUBLIC;


#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // !HTMLCSS_HTML_H
