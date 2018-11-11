/*
 * Common header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_COMMON_H
#  define HTMLCSS_COMMON_H

/*
 * Include necessary headers...
 */

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <errno.h>

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef int (*hc_error_cb_t)(const char *message, int linenum, void *ctx);
typedef char *(*hc_url_cb_t)(const char *url, char *buffer, size_t bufsize, void *ctx);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_COMMON_H */
