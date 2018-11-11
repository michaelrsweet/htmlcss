/*
 * Dictionary header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLHC_DICT_H
#  define HTMLHC_DICT_H

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

typedef struct _hc_dict_s hc_dict_t;
					/* Key/value string dictionary */


/*
 * Functions...
 */

extern size_t		hcDictCount(hc_dict_t *dict);
extern void		hcDictDelete(hc_dict_t *dict);
extern const char	*hcDictGet(hc_dict_t *dict, const char *key);
extern const char	*hcDictIndex(hc_dict_t *dict, size_t idx, const char **key);
extern void		hcDictRemove(hc_dict_t *dict, const char *key);
extern void		hcDictSet(hc_dict_t *dict, const char *key, const char *value);
extern hc_dict_t	*hcNewDict(hc_pool_t *pool);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLHC_DICT_H */
