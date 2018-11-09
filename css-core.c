/*
 * CSS import functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#  include "css-private.h"


/*
 * Local functions...
 */

static int	compare_strings(char **a, char **b);


/*
 * 'cssDelete()' - Free memory associated with a stylesheet.
 */

void
cssDelete(css_t *css)			/* I - Stylesheet */
{
  if (!css)
    return;

  free(css);
}


/*
 * 'cssNew()' - Allocate a new stylesheet.
 */

css_t *					/* O - Stylesheet */
cssNew(void)
{
  css_t	*css = (css_t *)calloc(1, sizeof(css_t));
					/* Stylesheet */


  if (css)
  {
    css->error_cb = _htmlcssDefaultErrorCB;
    css->url_cb   = _htmlcssDefaultURLCB;
  }

  return (css);
}


/*
 * 'cssSetMedia()' - Set the base media settings.
 */

int					/* O - 1 on success, 0 on failure */
cssSetMedia(css_t      *css,		/* I - Stylesheet */
            const char *media,		/* I - Media name ("print', etc.) */
            float      width,		/* I - Device width */
            float      height)		/* I - Device height */
{
  (void)css;
  (void)media;
  (void)width;
  (void)height;

  return (0);
}


/*
 * '_cssString()' - Find or copy a string.
 *
 * This function makes a copy of the passed string that will be freed when the
 * corresponding stylesheet is also freed.  It allows for more efficient memory
 * usage when tracking the various CSS property names and values.
 */

char *					/* O - New string pointer */
_cssString(css_t      *css,		/* I - Stylesheet */
           const char *s)		/* I - String to find/copy */
{
  char	**temp;				/* Temporary string pointer */


  if (css->num_strings > 0)
  {
    if ((temp = bsearch(&s, css->strings, css->num_strings, sizeof(char *), (int (*)(const void *, const void *))compare_strings)) != NULL)
      return (*temp);
  }

  if (css->num_strings >= css->alloc_strings)
  {
    if ((temp = realloc(css->strings, (css->alloc_strings + 32) * sizeof(char *))) == NULL)
      return (NULL);

    css->alloc_strings += 32;
    css->strings       = temp;
  }

  temp  = css->strings + css->num_strings;
  *temp = strdup(s);
  css->num_strings ++;

  if (css->num_strings > 1)
    qsort(css->strings, css->num_strings, sizeof(char *), (int (*)(const void *, const void *))compare_strings);

  return (*temp);
}


/*
 * 'compare_strings()' - Compare two strings...
 */

static int				/* O - Result of comparison */
compare_strings(char **a,		/* I - First string */
                char **b)		/* I - Second string */
{
#ifdef _WIN32
  return (_stricmp(*a, *b));
#else
  return (strcasecmp(*a, *b));
#endif /* _WIN32 */
}
