/*
 * Common functions for HTMLCSS library.
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
#include <stdarg.h>


/*
 * '_hcError()' - Display an error message.
 */

int					/* O - 1 to continue, 0 to stop */
_hcError(
    hc_error_cb_t error_cb,	/* I - Error callback */
    void               *ctx,		/* I - Context pointer */
    const char         *url,		/* I - Filename/URL or `NULL` */
    int                linenum,		/* I - Line number in file or 0 */
    const char         *message,	/* I - Printf-style message string */
    ...)				/* I - Additional arguments as needed */
{
  char		temp[1024],		/* Temporary format string buffer */
		buffer[8192];		/* Message buffer */
  va_list	ap;			/* Pointer to additional arguments */


  if (url && linenum)
  {
    snprintf(temp, sizeof(temp), "%s:%d: %s", url, linenum, message);
    message = temp;
  }
  else if (url)
  {
    snprintf(temp, sizeof(temp), "%s: %s", url, message);
    message = temp;
  }

  va_start(ap, message);
  vsnprintf(buffer, sizeof(buffer), message, ap);
  va_end(ap);

  return ((error_cb)(buffer, linenum, ctx));
}


/*
 * '_hcDefaultErrorCB()' - Default error callback.
 */

int					/* O - 1 to continue, 0 to stop */
_hcDefaultErrorCB(
    const char *message,		/* I - Message string */
    int        linenum,			/* I - Line number (unused) */
    void       *ctx)			/* I - Context pointer (unused) */
{
  (void)linenum;
  (void)ctx;

  fputs(message, stderr);
  putc('\n', stderr);

  return (1);
}


/*
 * '_hcDefaultURLCB()' - Default URL callback.
 */

char *					/* O - Local path to URL or `NULL` */
_hcDefaultURLCB(
    const char *url,			/* I - URL or filename */
    char       *buffer,			/* I - Filename buffer */
    size_t     bufsize,			/* I - Size of filename buffer */
    void       *ctx)			/* I - Context pointer (unused) */
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
      char ch = *url;			/* Current character */

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
        *bufptr++ = ch;
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
