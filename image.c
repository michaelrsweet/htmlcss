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
  const char	*format;		/* MIME media type */
  int		width,			/* Width in pixels */
		height;			/* Height in pixels */
};


/*
 * Local functions...
 */


/*
 * 'hcImageDelete()' - Delete an image object.
 */

void
hcImageDelete(hc_image_t *image)	/* I - Image object */
{
  free(image);
}


/*
 * 'hcImageGetFormat()' - Get the MIME media type for the image.
 */

const char *				/* O - MIME media type */
hcImageGetFormat(hc_image_t *image)	/* I - Image object */
{
  return (image ? image->format : NULL);
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
  unsigned char	buffer[2048];		/* Buffer */
  size_t	bytes;			/* Bytes in buffer */


  _HC_DEBUG("hcImageNew(pool=%p, file=%p)\n", (void *)pool, (void *)file);

  if (!pool || !file)
    return (NULL);

  if ((image = (hc_image_t *)calloc(1, sizeof(hc_image_t))) == NULL)
    return (NULL);

  image->pool = pool;

  bytes = hcFileRead(file, buffer, sizeof(buffer));

  _HC_DEBUG("hcImageNew: Read %d bytes from file.\n", (int)bytes);
  _HC_DEBUG("hcImageNew: \\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\\%03o\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);

  if (bytes > 27 && !memcmp(buffer, "\211PNG\015\012\032\012\000\000\000\015IHDR", 16))
  {
   /*
    * PNG image...
    */

    image->format = hcPoolGetString(pool, "image/png");
    image->width  = (buffer[16] << 24) | (buffer[17] << 16) | (buffer[18] << 8) | buffer[19];
    image->height = (buffer[20] << 24) | (buffer[21] << 16) | (buffer[22] << 8) | buffer[23];
  }
  else if (bytes > 12 && (!memcmp(buffer, "GIF87a", 6) || !memcmp(buffer, "GIF89a", 6)))
  {
   /*
    * GIF image...
    */

    image->format = hcPoolGetString(pool, "image/gif");
    image->width  = (buffer[7] << 8) | buffer[6];
    image->height = (buffer[9] << 8) | buffer[8];
  }
  else
  {
    free(image);
    return (NULL);
  }

  return (image);
}
