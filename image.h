/*
 * Image handling header for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

#ifndef HTMLCSS_IMAGE_H
#  define HTMLCSS_IMAGE_H

/*
 * Include necessary headers...
 */

#  include "file.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */


/*
 * Types...
 */

typedef struct _hc_image_s hc_image_t;	/* Image object */


/*
 * Functions...
 */

extern void		hcImageDelete(hc_image_t *image);
extern int		hcImageGetHeight(hc_image_t *image);
extern int		hcImageGetWidth(hc_image_t *image);
extern hc_image_t	*hcImageNew(hc_pool_t *pool, hc_file_t *file);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* !HTMLCSS_IMAGE_H */
