/*
 * Memory pool functions for HTMLCSS library.
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
#include "pool.h"


/*
 * Local types...
 */

struct _hc_pool_s			/* Memory pool */
{
  size_t	num_strings;		/* Number of strings in pool */
  size_t	alloc_strings;		/* Allocated size of strings array */
  char		**strings;		/* Strings array */
};


/*
 * Local functions...
 */

static int	compare_strings(char **a, char **b);


/*
 * 'hcPoolNew()' - Create a new memory pool.
 */

hc_pool_t *			/* O - New memory pool */
hcPoolNew(void)
{
  return ((hc_pool_t *)calloc(1, sizeof(hc_pool_t)));
}


/*
 * 'hcPoolGetString()' - Find or copy a string.
 *
 * This function finds or makes a copy of the passed string that will be freed
 * when the corresponding memory pool is deleted.  Since the memory pool only
 * maintains a single copy of any string, copied strings are immutable.
 */

const char *				/* O - New string pointer */
hcPoolGetString(
    hc_pool_t  *pool,			/* I - Memory pool */
    const char *s)			/* I - String to find/copy */
{
  char	**temp;				/* Temporary string pointer */


  if (!pool || !s)
    return (NULL);

  if (pool->num_strings == 1 && !strcmp(pool->strings[0], s))
    return (pool->strings[0]);
  else if (pool->num_strings > 1)
  {
    if ((temp = bsearch(&s, pool->strings, pool->num_strings, sizeof(char *), (int (*)(const void *, const void *))compare_strings)) != NULL)
      return (*temp);
  }

  if (pool->num_strings >= pool->alloc_strings)
  {
    if ((temp = realloc(pool->strings, (pool->alloc_strings + 32) * sizeof(char *))) == NULL)
      return (NULL);

    pool->alloc_strings += 32;
    pool->strings       = temp;
  }

  temp  = pool->strings + pool->num_strings;
  *temp = strdup(s);
  pool->num_strings ++;

  if (pool->num_strings > 1)
    qsort(pool->strings, pool->num_strings, sizeof(char *), (int (*)(const void *, const void *))compare_strings);

  return (*temp);
}


/*
 * 'hcPoolDelete()' - Free the memory used by a pool.
 */

void
hcPoolDelete(hc_pool_t *pool)	/* I - Memory pool */
{
  if (pool)
  {
    if (pool->num_strings > 0)
    {
      size_t	i;			/* Looping var */
      char	**temp;			/* String pointer */

      for (i = pool->num_strings, temp = pool->strings; i > 0; i --, temp ++)
        free(*temp);

      free(pool->strings);
    }

    free(pool);
  }
}


/*
 * 'compare_strings()' - Compare two strings...
 */

static int				/* O - Result of comparison */
compare_strings(char **a,		/* I - First string */
                char **b)		/* I - Second string */
{
  return (strcmp(*a, *b));
}
