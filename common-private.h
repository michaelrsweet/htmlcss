/*
 * Common private header file for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLHC_COMMON_PRIVATE_H
#  define HTMLHC_COMMON_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "common.h"

#  ifdef WIN32
#    include <io.h>
#    include <direct.h>

/*
 * Microsoft renames the POSIX functions to _name, and introduces
 * a broken compatibility layer using the original names.  As a result,
 * random crashes can occur when, for example, strdup() allocates memory
 * from a different heap than used by malloc() and free().
 *
 * To avoid moronic problems like this, we #define the POSIX function
 * names to the corresponding non-standard Microsoft names.
 */

#    define access	_access
#    define close	_close
#    define fileno	_fileno
#    define lseek	_lseek
#    define mkdir(d,p)	_mkdir(d)
#    define open	_open
#    define read	_read
#    define rmdir	_rmdir
#    define snprintf	_snprintf
#    define strdup	_strdup
#    define unlink	_unlink
#    define vsnprintf	_vsnprintf
#    define write	_write

/*
 * Map various parameters for POSIX...
 */

#    define F_OK	00
#    define W_OK	02
#    define R_OK	04
#    define O_RDONLY	_O_RDONLY
#    define O_WRONLY	_O_WRONLY
#    define O_CREAT	_O_CREAT
#    define O_TRUNC	_O_TRUNC

#  else
#    include <unistd.h>
#  endif /* WIN32 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _hc_file_s
{
  const char	*url;			/* URL or filename */
  FILE		*fp;			/* File pointer */
  const char	*s,			/* String */
		*sptr;			/* Pointer into string */
  int		linenum;		/* Current line number */
} _hc_file_t;


/*
 * Functions...
 */

extern int	_hcDefaultErrorCB(const char *message, int linenum, void *ctx);
extern char	*_hcDefaultURLCB(const char *url, char *buffer, size_t bufsize, void *ctx);
extern int	_hcError(hc_error_cb_t error_cb, void *ctx, const char *url, int linenum, const char *message, ...);

static inline int _hcFileGetc(_hc_file_t *f)
{
  int	ch;				/* Current character */

  if (f->sptr && *(f->sptr))
    ch = *(f->sptr)++;
  else if (f->sptr)
    ch = EOF;
  else
    ch = getc(f->fp);

  if (ch == '\n')
    f->linenum ++;

  return (ch);
}
static inline void _hcFileUngetc(int ch, _hc_file_t *f)
{
  if (f->sptr && f->sptr > f->s)
    (f->sptr) --;
  else if (f->fp)
    ungetc(ch, f->fp);

  if (ch == '\n')
    f->linenum --;
}


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLHC_COMMON_PRIVATE_H */
