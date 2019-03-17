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

typedef struct _hc_dict_s hc_dict_t;
					/* Key/value string dictionary */


/*
 * Functions...
 */

extern hc_dict_t	*hcDictCopy(const hc_dict_t *dict);
extern void		hcDictDelete(hc_dict_t *dict);
extern size_t		hcDictGetCount(const hc_dict_t *dict);
extern const char	*hcDictGetIndexKeyValue(const hc_dict_t *dict, size_t idx, const char **key);
extern const char	*hcDictGetKeyValue(const hc_dict_t *dict, const char *key);
extern hc_dict_t	*hcDictNew(hc_pool_t *pool);
extern void		hcDictRemoveKey(hc_dict_t *dict, const char *key);
extern void		hcDictSetKeyValue(hc_dict_t *dict, const char *key, const char *value);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_DICT_H */
