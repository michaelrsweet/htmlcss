/*
 * Dictionary header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_DICT_H
#  define HTMLCSS_DICT_H

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

typedef struct _htmlcss_dict_s htmlcss_dict_t;
					/* Key/value string dictionary */


/*
 * Functions...
 */

extern size_t		htmlcssDictCount(htmlcss_dict_t *dict);
extern void		htmlcssDictDelete(htmlcss_dict_t *dict);
extern const char	*htmlcssDictGet(htmlcss_dict_t *dict, const char *key);
extern const char	*htmlcssDictIndex(htmlcss_dict_t *dict, size_t idx, const char **key);
extern void		htmlcssDictRemove(htmlcss_dict_t *dict, const char *key);
extern void		htmlcssDictSet(htmlcss_dict_t *dict, const char *key, const char *value);
extern htmlcss_dict_t	*htmlcssNewDict(htmlcss_pool_t *pool);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_DICT_H */
