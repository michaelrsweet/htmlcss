/*
 * Private font header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_FONT_PRIVATE_H
#  define HTMLCSS_FONT_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "font.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

struct _hc_font_s
{
  hc_pool_t	*pool;
  const char	*family;
};


/*
 * Functions...
 */



#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FONT_PRIVATE_H */
