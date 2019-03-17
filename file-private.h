/*
 * Private file handling header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_FILE_PRIVATE_H
#  define HTMLCSS_FILE_PRIVATE_H

/*
 * Include necessary headers...
 */

#  include "file.h"
#  include "pool-private.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

struct _hc_file_s			/* High-level file/stream */
{
  hc_pool_t		*pool;		/* Memory pool */
  const char		*url;		/* URL or filename */
  FILE			*fp;		/* File pointer */
  const _hc_uchar_t	*buffer,	/* String buffer */
			*bufptr,	/* Pointer into buffer */
			*bufend;	/* End of buffer */
  int			linenum;	/* Current line number */
};


/*
 * Functions...
 */

extern int	_hcFileError(hc_file_t *file, const char *message, ...) _HC_FORMAT_ARGS(2,3);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_FILE_PRIVATE_H */
