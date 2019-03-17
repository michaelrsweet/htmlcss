/*
 * Common functions for HTMLCSS library.
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

#include "common-private.h"


/*
 * '_hcDefaultErrorCB()' - Default error callback.
 */

int					/* O - 1 to continue, 0 to stop */
_hcDefaultErrorCB(
    void       *ctx,			/* I - Context pointer (unused) */
    const char *message,		/* I - Message string */
    int        linenum)			/* I - Line number (unused) */
{
  (void)ctx;
  (void)linenum;

  fputs(message, stderr);
  putc('\n', stderr);

  return (1);
}


/*
 * '_hcDefaultURLCB()' - Default URL callback.
 */

char *					/* O - Local path to URL or `NULL` */
_hcDefaultURLCB(
    void       *ctx,			/* I - Context pointer (unused) */
    const char *url,			/* I - URL or filename */
    char       *buffer,			/* I - Filename buffer */
    size_t     bufsize)			/* I - Size of filename buffer */
{
  (void)ctx;

  if (!access(url, R_OK))
  {
   /*
    * Local file we can read...
    */

    strncpy(buffer, url, bufsize - 1);
    buffer[bufsize - 1] = '\0';
  }
  else if (!strncmp(url, "file:///", 8))
  {
    char	*bufptr,		/* Pointer into buffer */
		*bufend;		/* End of buffer */

    for (url += 7, bufptr = buffer, bufend = buffer + bufsize - 1; *url; url ++)
    {
      int ch = *url;			/* Current character */

      if (ch == '%' && isxdigit(url[1] & 255) && isxdigit(url[2] & 255))
      {
       /*
        * Percent-escaped character in URL...
        */

	if (isdigit(url[1]))
	  ch = (url[1] - '0') << 4;
	else
	  ch = (tolower(url[1]) - 'a' + 10) << 4;

	if (isdigit(url[2]))
	  ch |= (url[2] - '0');
	else
	  ch |= (tolower(url[2]) - 'a' + 10);

	url += 2;
      }

      if (bufptr < bufend)
      {
        *bufptr++ = (char)ch;
      }
      else
      {
        errno   = E2BIG;
        *buffer = '\0';
        return (NULL);
      }
    }

    *bufptr = '\0';

    if (access(buffer, R_OK))
    {
     /*
      * File not readable/found...
      */

      *buffer = '\0';
      return (NULL);
    }
  }
  else
  {
    errno   = EINVAL;
    *buffer = '\0';
    return (NULL);
  }

  return (buffer);
}
