/*
 * File handling functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2021 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#include "file-private.h"


/*
 * 'hcFileDelete()' - Close a file and free any memory associated with it.
 */

void
hcFileDelete(hc_file_t *file)		/* I - File */
{
  if (!file)
    return;

  if (file->fp)
    gzclose(file->fp);

  free(file);
}


/*
 * '_hcFileError()' - Report an error while reading the specified file.
 */

int					/* O - 1 to continue, 0 to stop */
_hcFileError(hc_file_t  *file,		/* I - File */
             const char *message,	/* I - Printf-style message */
             ...)			/* I - Additional arguments as needed */
{
  int		ret;			/* Return value */
  char		temp[1024];		/* Temporary message buffer */
  va_list	ap;			/* Pointer to arguments */


  if (file->url && file->linenum)
    snprintf(temp, sizeof(temp), "%s:%d: %s", file->url, file->linenum, message);
  else if (file->url)
    snprintf(temp, sizeof(temp), "%s: %s", file->url, message);
  else if (file->linenum)
    snprintf(temp, sizeof(temp), "%d: %s", file->linenum, message);
  else
    snprintf(temp, sizeof(temp), "%s", message);

  va_start(ap, message);
  ret = _hcPoolErrorv(file->pool, file->linenum, temp, ap);
  va_end(ap);

  return (ret);
}


/*
 * 'hcFileGetc()' - Get a character from a file.
 */

int					/* O - Character or `EOF` */
hcFileGetc(hc_file_t *file)		/* I - File */
{
  int	ch;				/* Current character */


  if (file->bufptr)
  {
    if (file->bufptr < file->bufend)
      ch = (int)*(file->bufptr)++;
    else
      ch = EOF;
  }
  else
    ch = gzgetc(file->fp);

  if (ch == '\n')
    file->linenum ++;

  return (ch);
}


/*
 * 'hcFileNewBuffer()' - Create a new file buffer stream.
 */

hc_file_t *				/* O - File */
hcFileNewBuffer(hc_pool_t  *pool,	/* I - Memory pool */
                const void *buffer,	/* I - Buffer */
                size_t     bytes)	/* I - Size of buffer */
{
  hc_file_t	*file;			/* File */


  if ((file = calloc(1, sizeof(hc_file_t))) != NULL)
  {
    file->pool    = pool;
    file->buffer  = buffer;
    file->bufptr  = file->buffer;
    file->bufend  = file->buffer + bytes;
    file->linenum = 1;
  }

  return (file);
}


/*
 * 'hcFileNewString()' - Create a new file string stream.
 */

hc_file_t *				/* O - File */
hcFileNewString(hc_pool_t  *pool,	/* I - Memory pool */
                const char *s)		/* I - String */
{
  return (hcFileNewBuffer(pool, s, strlen(s)));
}


/*
 * 'hcFileNewURL()' - Create a new file URL stream.
 */

hc_file_t *				/* O - File */
hcFileNewURL(hc_pool_t  *pool,		/* I - Memory pool */
             const char *url,		/* I - URL or filename */
             const char *baseurl)	/* I - Base URL or `NULL` */
{
  hc_file_t	*file;			/* File */
  const char	*filename;		/* Local file */


  _HC_DEBUG("hcFileNewURL(pool=%p, url=\"%s\", baseurl=\"%s\")\n", (void *)pool, url, baseurl);

  if ((filename = hcPoolGetURL(pool, url, baseurl)) == NULL)
    return (NULL);

  _HC_DEBUG("hcFileNewURL: filename=\"%s\"\n", filename);

  if ((file = calloc(1, sizeof(hc_file_t))) != NULL)
  {
    file->pool    = pool;
    file->url     = filename;
    file->fp      = gzopen(filename, "rb");
    file->linenum = 1;

    if (!file->fp)
    {
      perror(filename);
      free(file);
      file = NULL;
    }
  }

  return (file);
}


/*
 * 'hcFileRead()' - Read bytes from a file.
 */

size_t					/* O - Number of bytes read */
hcFileRead(hc_file_t *file,		/* I - File */
           void      *buffer,		/* I - Buffer */
           size_t    bytes)		/* I - Number of bytes to read */
{
  ssize_t	rbytes;			/* Number of bytes read */


  if (!file || !buffer || bytes == 0)
    return (0);

  if (file->bufptr)
  {
    if ((size_t)(file->bufend - file->bufptr) < bytes)
      bytes = (size_t)(file->bufend - file->bufptr);

    if (bytes > 0)
    {
      memcpy(buffer, file->bufptr, bytes);
      file->bufptr += bytes;
    }

    return (bytes);
  }
  else if ((rbytes = gzread(file->fp, buffer, (unsigned)bytes)) < 0)
    return (0);
  else
    return ((size_t)rbytes);
}


/*
 * 'hcFileSeek()' - Randomly access data within a file.
 */

size_t					/* O - New file offset or 0 on error */
hcFileSeek(hc_file_t *file,		/* I - File */
           size_t    offset)		/* I - Offset within file */
{
  ssize_t	soffset;		/* Seek offset */


  if (!file)
    return (0);

  if (file->bufptr)
  {
    if (offset > (size_t)(file->bufend - file->buffer))
      offset = (size_t)(file->bufend - file->buffer);

    file->bufptr = file->buffer + offset;

    return (offset);
  }

  if ((soffset = gzseek(file->fp, (long)offset, SEEK_SET)) < 0)
    return (0);
  else
    return ((size_t)soffset);
}


/*
 * 'hcFileUngetc()' - Return a character to a file.
 */

void
hcFileUngetc(hc_file_t *f,		/* I - File */
             int       ch)		/* I - Character */
{
  if (f->bufptr && f->bufptr > f->buffer)
    f->bufptr --;
  else if (f->fp)
    gzungetc(ch, f->fp);

  if (ch == '\n')
    f->linenum --;
}
