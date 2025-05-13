//
// Memory pool functions for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#include "pool-private.h"


//
// Local functions...
//

static int	compare_strings(char **a, char **b);


//
// 'hcPoolDelete()' - Free the memory used by a pool.
//

void
hcPoolDelete(hc_pool_t *pool)	// I - Memory pool
{
  if (pool)
  {
    if (pool->num_fonts > 0)
      _hcPoolDeleteFonts(pool);

    if (pool->num_strings > 0)
    {
      size_t	i;			// Looping var
      char	**temp;			// String pointer

      for (i = pool->num_strings, temp = pool->strings; i > 0; i --, temp ++)
        free(*temp);

      free(pool->strings);
    }

    free(pool->last_error);
    free(pool);
  }
}


//
// '_hcPoolError()' - Display an error message.
//

bool					// O - `true` to continue, `false` to stop
_hcPoolError(
    hc_pool_t  *pool,			// I - Memory pool
    int        linenum,			// I - Line number in file or 0
    const char *message,		// I - Printf-style message string
    ...)				// I - Additional arguments as needed
{
  bool		ret;			// Return value
  va_list	ap;			// Pointer to additional arguments


  va_start(ap, message);
  ret = _hcPoolErrorv(pool, linenum, message, ap);
  va_end(ap);

  return (ret);
}


//
// '_hcPoolErrorv()' - Display an error message.
//

bool					// O - `true` to continue, `false` to stop
_hcPoolErrorv(
    hc_pool_t  *pool,			// I - Memory pool
    int        linenum,			// I - Line number in file or 0
    const char *message,		// I - Printf-style message string
    va_list    ap)			// I - Pointer to additional arguments
{
  char		buffer[8192];		// Message buffer


  vsnprintf(buffer, sizeof(buffer), message, ap);

  free(pool->last_error);
  pool->last_error = strdup(buffer);

  return ((pool->error_cb)(pool->error_ctx, buffer, linenum));
}


//
// 'hcPoolGetLastError()' - Return the last error message recorded.
//

const char *				// O - Last error message or `NULL`
hcPoolGetLastError(hc_pool_t *pool)	// I - Memory pool
{
  return (pool ? pool->last_error : NULL);
}


//
// 'hcPoolGetString()' - Find or copy a string.
//
// This function finds or makes a copy of the passed string that will be freed
// when the corresponding memory pool is deleted.  Since the memory pool only
// maintains a single copy of any string, copied strings are immutable.
//

const char *				// O - New string pointer
hcPoolGetString(
    hc_pool_t  *pool,			// I - Memory pool
    const char *s)			// I - String to find/copy
{
  char	*news,				// New string
	**temp;				// Temporary string pointer


  if (!pool || !s)
    return (NULL);
  else if (!*s)
    return ("");

  if (pool->num_strings == 1 && !strcmp(pool->strings[0], s))
  {
    _HC_DEBUG("hcPoolGetString: Existing string '%s' (%p) found.\n", pool->strings[0], (void *)pool->strings[0]);
    return (pool->strings[0]);
  }
  else if (pool->num_strings > 1)
  {
    if ((temp = bsearch(&s, pool->strings, pool->num_strings, sizeof(char *), (_hc_compare_func_t)compare_strings)) != NULL)
    {
      _HC_DEBUG("hcPoolGetString: Existing string '%s' (%p) found.\n", *temp, (void *)*temp);
      return (*temp);
    }
  }

  if (pool->num_strings >= pool->alloc_strings)
  {
    if ((temp = realloc(pool->strings, (pool->alloc_strings + 32) * sizeof(char *))) == NULL)
      return (NULL);

    pool->alloc_strings += 32;
    pool->strings       = temp;
  }

  temp  = pool->strings + pool->num_strings;
  *temp = news = strdup(s);
  pool->num_strings ++;

  if (pool->num_strings > 1)
    qsort(pool->strings, pool->num_strings, sizeof(char *), (_hc_compare_func_t)compare_strings);

  _HC_DEBUG("hcPoolGetString: New string '%s' (%p), pool now contains %d strings.\n", news, (void *)news, (int)pool->num_strings);

  return (news);
}


//
// 'hcPoolGetURL()' - Get a file corresponding to a URL.
//

const char *				// O - Filename or `NULL` on error
hcPoolGetURL(hc_pool_t  *pool,		// I - Memory pool
             const char *url,		// I - URL
             const char *baseurl)	// I - Base URL, if any
{
  const char	*mapped;		// Mapped file
  char		*ptr,			// Pointer into URL
		temp[1024],		// Temporary path
		newurl[1024];		// New URL


  if (*url == '/')
  {
    if (!baseurl)
      return (hcPoolGetString(pool, url));
    else if (!strncmp(baseurl, "http://", 7))
    {
      strncpy(temp, baseurl, sizeof(temp) - 1);
      temp[sizeof(temp) - 1] = '\0';
      if ((ptr = strchr(temp + 7, '/')) != NULL)
        *ptr = '\0';

      snprintf(newurl, sizeof(newurl), "%s%s", temp, url);
      url = newurl;
    }
    else if (!strncmp(baseurl, "https://", 8))
    {
      strncpy(temp, baseurl, sizeof(temp) - 1);
      temp[sizeof(temp) - 1] = '\0';
      if ((ptr = strchr(temp + 8, '/')) != NULL)
        *ptr = '\0';

      snprintf(newurl, sizeof(newurl), "%s%s", temp, url);
      url = newurl;
    }
    else
      return (hcPoolGetString(pool, url));
  }
  else if (strncmp(url, "http://", 7) && strncmp(url, "https://", 8))
  {
    if (!baseurl)
    {
      getcwd(temp, sizeof(temp));
      snprintf(newurl, sizeof(newurl), "%s/%s", temp, url);

      return (hcPoolGetString(pool, newurl));
    }
    else
    {
      strncpy(temp, baseurl, sizeof(temp) - 1);
      temp[sizeof(temp) - 1] = '\0';

      if ((ptr = strrchr(temp, '/')) != NULL)
        *ptr = '\0';

      snprintf(newurl, sizeof(newurl), "%s/%s", temp, url);

      if (newurl[0] == '/')
	return (hcPoolGetString(pool, newurl));

      url = newurl;
    }
  }

  if ((mapped = (pool->url_cb)(pool->url_ctx, url, temp, sizeof(temp))) != NULL)
  {
    if (!pool->urls)
      pool->urls = hcDictNew(pool);

    hcDictSetKeyValue(pool->urls, url, temp);
    mapped = hcPoolGetString(pool, temp);
  }

  return (mapped);
}


//
// 'hcPoolNew()' - Create a new memory pool.
//

hc_pool_t *			// O - New memory pool
hcPoolNew(void)
{
  hc_pool_t *pool = (hc_pool_t *)calloc(1, sizeof(hc_pool_t));
				// New memory pool

  if (pool)
  {
    if ((pool->loc = localeconv()) != NULL)
    {
      if (!pool->loc->decimal_point || !strcmp(pool->loc->decimal_point, "."))
	pool->loc = NULL;
      else
	pool->loc_declen = strlen(pool->loc->decimal_point);
    }

    pool->error_cb = _hcDefaultErrorCB;
    pool->url_cb   = _hcDefaultURLCB;
  }

  return (pool);
}


//
// 'hcPoolSetErrorCallback()' - Set the error reporting callback.
//
// The default error callback writes the message to `stderr`.
//
// The error callback returns 1 to continue processing or 0 to stop immediately.
//

void
hcPoolSetErrorCallback(
    hc_pool_t     *pool,		// I - Memory pool
    hc_error_cb_t cb,			// I - Error callback or `NULL` for the default
    void          *ctx)			// I - Context pointer for callback
{
  if (!pool)
    return;

  pool->error_cb  = cb ? cb : _hcDefaultErrorCB;
  pool->error_ctx = ctx;
}


//
// 'hcPoolSetURLCallback()' - Set the URL callback.
//
// The default URL callback supports local files (only).
//
// The URL callback returns a local pathname (copied to the specified buffer)
// or `NULL` if the URL cannot be loaded/found.
//

void
hcPoolSetURLCallback(
    hc_pool_t   *pool,			// I - Memory pool
    hc_url_cb_t cb,			// I - URL callback or `NULL` for the default
    void        *ctx)			// I - Context pointer for callback
{
  if (!pool)
    return;

  pool->url_cb  = cb ? cb : _hcDefaultURLCB;
  pool->url_ctx = ctx;
}


//
// 'compare_strings()' - Compare two strings...
//

static int				// O - Result of comparison
compare_strings(char **a,		// I - First string
                char **b)		// I - Second string
{
  return (strcmp(*a, *b));
}
