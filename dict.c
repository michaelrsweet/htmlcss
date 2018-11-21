/*
 * HTML attribute functions for HTMLCSS library.
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

#include "common-private.h"
#include "dict.h"


/*
 * Private types...
 */

typedef struct _hc_pair_s		/* Key/value pair */
{
  const char	*key;			/* Key */
  const char	*value;			/* Value */
} _hc_pair_t;

struct _hc_dict_s			/* Dictionary */
{
  hc_pool_t	*pool;			/* Memory pool */
  size_t	num_pairs;		/* Number of pairs */
  size_t	alloc_pairs;		/* Allocated pairs */
  _hc_pair_t	*pairs;			/* Key/value pairs */
};


/*
 * Local functions...
 */

static int	compare_pairs(_hc_pair_t *a, _hc_pair_t *b);


/*
 * 'hcDictCopy()' - Make a copy of a dictionary.
 */

hc_dict_t *				/* O - New dictionary */
hcDictCopy(hc_dict_t *dict)		/* I - Dictionary to copy */
{
  hc_dict_t	*newdict;		/* New dictionary */


  if (!dict)
    return (NULL);

  if ((newdict = calloc(1, sizeof(hc_dict_t))) == NULL)
    return (NULL);

  newdict->pool        = dict->pool;
  newdict->num_pairs   = dict->num_pairs;
  newdict->alloc_pairs = dict->num_pairs;

  if ((newdict->pairs = calloc(newdict->num_pairs, sizeof(_hc_pair_t))) == NULL)
  {
    free(newdict);
    return (NULL);
  }

  memcpy(newdict->pairs, dict->pairs, newdict->num_pairs * sizeof(_hc_pair_t));

  return (newdict);
}


/*
 * 'hcDictGetCount()' - Return the number of key/value pairs in a dictionary.
 */

size_t					/* O - Number of key/value pairs */
hcDictGetCount(hc_dict_t *dict)		/* I - Dictionary */
{
  return (dict ? dict->num_pairs : 0);
}


/*
 * 'hcDictDelete()' - Delete a dictionary.
 */

void
hcDictDelete(hc_dict_t *dict)		/* I - Dictionary */
{
  if (dict)
  {
    if (dict->pairs)
      free(dict->pairs);

    free(dict);
  }
}


/*
 * 'hcdictGetKeyValue()' - Get the value for a key in a dictionary.
 */

const char *				/* O - Value or `NULL` if not found. */
hcdictGetKeyValue(hc_dict_t  *dict,	/* I - Dictionary */
                  const char *key)	/* I - Key string */
{
  _hc_pair_t	temp,			/* Temporary search key */
		*ptr;			/* Pointer to match */


  if (!dict || dict->num_pairs == 0)
    return (NULL);

  temp.key = key;

  if ((ptr = (_hc_pair_t *)bsearch(&temp, dict->pairs, dict->num_pairs, sizeof(_hc_pair_t), (int (*)(const void *, const void *))compare_pairs)) != NULL)
    return (ptr->value);
  else
    return (NULL);
}


/*
 * 'hcDictGetIndexKeyValue()' - Return the key and value for the specified pair.
 */

const char *				/* O - Value or `NULL` if `idx` is invalid. */
hcDictGetIndexKeyValue(
    hc_dict_t  *dict,			/* I - Dictionary */
    size_t     idx,			/* I - Index (0-based) */
    const char **key)			/* O - Key or `NULL` if `idx` is invalid. */
{
  if (!dict || idx >= dict->num_pairs || !key)
    return (NULL);

  *key = dict->pairs[idx].key;

  return (dict->pairs[idx].value);
}


/*
 * 'hcDictRemoveKey()' - Remove a key/value pair from a dictionary.
 */

void
hcDictRemoveKey(hc_dict_t  *dict,	/* I - Dictionary */
                const char *key)	/* I - Key string */
{
  _hc_pair_t	temp,			/* Temporary search key */
		*ptr;			/* Pointer to match */
  size_t	idx;			/* Index into dictionary */


  if (!dict || dict->num_pairs == 0)
    return;

  temp.key = key;

  if ((ptr = (_hc_pair_t *)bsearch(&temp, dict->pairs, dict->num_pairs, sizeof(_hc_pair_t), (int (*)(const void *, const void *))compare_pairs)) != NULL)
  {
    dict->num_pairs --;

    idx = ptr - dict->pairs;

    if (idx < dict->num_pairs)
      memmove(ptr, ptr + 1, (dict->num_pairs - idx) * sizeof(_hc_pair_t));
  }
}


/*
 * 'hcDictSetKeyValue()' - Set a key/value pair in a dictionary.
 */

void
hcDictSetKeyValue(hc_dict_t  *dict,	/* I - Dictionary */
	          const char *key,	/* I - Key string */
	          const char *value)	/* I - Value string */
{
  _hc_pair_t	temp,			/* Search key */
		*ptr = NULL;		/* New key/value pair */


  if (!dict)
  {
    return;
  }
  else if (dict->num_pairs == 1 && !strcmp(dict->pairs[0].key, key))
  {
    ptr = dict->pairs;
  }
  else if (dict->num_pairs > 1)
  {
    temp.key = key;

    ptr = (_hc_pair_t *)bsearch(&temp, dict->pairs, dict->num_pairs, sizeof(_hc_pair_t), (int (*)(const void *, const void *))compare_pairs);
  }

  if (ptr)
  {
    ptr->value = hcPoolGetString(dict->pool, value);
    return;
  }

  if (dict->num_pairs >= dict->alloc_pairs)
  {
    if ((ptr = realloc(dict->pairs, (dict->alloc_pairs + 4) * sizeof(_hc_pair_t))) == NULL)
      return;

    dict->alloc_pairs += 4;
    dict->pairs       = ptr;
  }

  ptr = dict->pairs + dict->num_pairs;
  dict->num_pairs ++;

  ptr->key   = hcPoolGetString(dict->pool, key);
  ptr->value = hcPoolGetString(dict->pool, value);

  qsort(dict->pairs, dict->num_pairs, sizeof(_hc_pair_t), (int (*)(const void *, const void *))compare_pairs);
}


/*
 * 'hcDictNew()' - Create a new dictionary.
 */

hc_dict_t *				/* O - New dictionary */
hcDictNew(hc_pool_t *pool)		/* I - Memory pool */
{
  hc_dict_t	*dict;			/* New dictionary */


  if ((dict = (hc_dict_t *)calloc(1, sizeof(hc_dict_t))) != NULL)
    dict->pool = pool;

  return (dict);
}


/*
 * 'compare_pairs()' - Compare two key/value pairs.
 */

static int				/* O - Result of comparison */
compare_pairs(_hc_pair_t *a,		/* I - First pair */
              _hc_pair_t *b)		/* I - Second pair */
{
#ifdef _WIN32
  return (_stricmp(a->key, b->key));
#else
  return (strcasecmp(a->key, b->key));
#endif /* _WIN32 */
}
