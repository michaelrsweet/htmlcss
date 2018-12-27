/*
 * File handling header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_FILE_H
#  define HTMLCSS_FILE_H

/*
 * Include necessary headers...
 */

#  include "pool.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _hc_file_s hc_file_t;	/* High-level file/stream */


/*
 * Functions...
 */

extern void		hcFileDelete(hc_file_t *file);
extern int		hcFileGetc(hc_file_t *file);
extern hc_file_t	*hcFileNewBuffer(hc_pool_t *pool, const void *buffer, size_t bytes);
extern hc_file_t	*hcFileNewString(hc_pool_t *pool, const char *s);
extern hc_file_t	*hcFileNewURL(hc_pool_t *pool, const char *url, const char *baseurl);
extern size_t		hcFileRead(hc_file_t *file, void *buffer, size_t bytes);
extern void		hcFileUngetc(hc_file_t *file, int ch);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FILE_H */
