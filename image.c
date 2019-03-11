/*
 * Image handling functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#include "image.h"
#include "common-private.h"


/*
 * Types...
 */

struct _hc_image_s
{
  hc_pool_t	*pool;			/* Memory pool */
  int		width,			/* Width in pixels */
		height;			/* Height in pixels */
};


/*
 * 'hcImageDelete()' - Delete an image object.
 */

void
hcImageDelete(hc_image_t *image)	/* I - Image object */
{
  free(image);
}


/*
 * 'hcImageGetHeight()' - Get the height of an image.
 */

int					/* O - Height in pixels */
hcImageGetHeight(hc_image_t *image)	/* I - Image object */
{
  return (image ? image->height : 0);
}


/*
 * 'hcImageGetWidth()' - Get the width of an image.
 */

int					/* O - Width in pixels */
hcImageGetWidth(hc_image_t *image)	/* I - Image object */
{
  return (image ? image->width : 0);
}


/*
 * 'hcImageNew()' - Create a new image object.
 */

hc_image_t *				/* O - Image object */
hcImageNew(hc_pool_t *pool,		/* I - Memory pool */
           hc_file_t *file)		/* I - File */
{
  hc_image_t	*image;			/* Image object */


  if (!pool || !file)
    return (NULL);

  if ((image = (hc_image_t *)calloc(1, sizeof(hc_image_t))) == NULL)
    return (NULL);

  return (image);
}
