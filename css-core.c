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
  return (NULL);
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
