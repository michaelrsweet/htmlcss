//
// Image handling functions for HTMLCSS library.
//
// Note: The purpose of these functions is currently just to discover the
// dimensions and format of an image file and not to decode or transform the
// contents of an image.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2019-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#include "image.h"
#include "common-private.h"


//
// Types...
//

typedef enum _hc_res_e
{
  _HC_RES_NONE,				// No units
  _HC_RES_PER_INCH,			// Pixels per inch
  _HC_RES_PER_CM			// Pixels per cm
} _hc_res_t;

struct _hc_image_s
{
  hc_pool_t	*pool;			// Memory pool
  const char	*format;		// MIME media type
  int		width,			// Width in pixels
		height;			// Height in pixels
  int		xres,			// Width resolution
		yres;			// Height resolution
  _hc_res_t	units;			// Resolution units
};


//
// Local functions...
//


//
// 'hcImageDelete()' - Delete an image object.
//

void
hcImageDelete(hc_image_t *image)	// I - Image object
{
  free(image);
}


//
// 'hcImageGetFormat()' - Get the MIME media type for the image.
//

const char *				// O - MIME media type
hcImageGetFormat(hc_image_t *image)	// I - Image object
{
  return (image ? image->format : NULL);
}


//
// 'hcImageGetHeight()' - Get the height of an image.
//

int					// O - Height in pixels
hcImageGetHeight(hc_image_t *image)	// I - Image object
{
  return (image ? image->height : 0);
}


//
// 'hcImageGetSize()' - Get the natural size of an image.
//

hc_size_t				// O - CSS dimensions
hcImageGetSize(hc_image_t *image)	// I - Image object
{
  hc_size_t	size;			// CSS dimensions


  if (image)
  {
    if (image->xres > 0 && image->yres > 0 && image->units != _HC_RES_NONE)
    {
      if (image->units == _HC_RES_PER_INCH)
      {
       /*
        * Convert from PPI to points...
        */

        size.width  = 72.0f * image->width / image->xres;
        size.height = 72.0f * image->height / image->yres;
      }
      else
      {
       /*
        * Convert from PPCM to points...
        */

        size.width  = 72.0f / 2.54f * image->width / image->xres;
        size.height = 72.0f / 2.54f * image->height / image->yres;
      }
    }
    else
    {
     /*
      * Default resolution is 100 PPI, CSS units are points (72 points per inch).
      */

      size.width  = 0.72f * image->width;
      size.height = 0.72f * image->height;
    }
  }
  else
  {
    size.width  = 0.0f;
    size.height = 0.0f;
  }

  return (size);
}


//
// 'hcImageGetWidth()' - Get the width of an image.
//

int					// O - Width in pixels
hcImageGetWidth(hc_image_t *image)	// I - Image object
{
  return (image ? image->width : 0);
}


//
// 'hcImageNew()' - Create a new image object.
//

hc_image_t *				// O - Image object
hcImageNew(hc_pool_t *pool,		// I - Memory pool
           hc_file_t *file)		// I - File
{
  hc_image_t	*image;			// Image object
  unsigned char	buffer[2048];		// Buffer
  size_t	bytes;			// Bytes in buffer


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

    image->format = "image/png";
    image->width  = (buffer[16] << 24) | (buffer[17] << 16) | (buffer[18] << 8) | buffer[19];
    image->height = (buffer[20] << 24) | (buffer[21] << 16) | (buffer[22] << 8) | buffer[23];
  }
  else if (bytes > 12 && (!memcmp(buffer, "GIF87a", 6) || !memcmp(buffer, "GIF89a", 6)))
  {
   /*
    * GIF image...
    */

    image->format = "image/gif";
    image->width  = (buffer[7] << 8) | buffer[6];
    image->height = (buffer[9] << 8) | buffer[8];
  }
  else if (bytes > 3 && !memcmp(buffer, "\377\330\377", 3))
  {
   /*
    * JPEG image...
    */

    unsigned char	*bufptr = buffer + 2,
					// Pointer into buffer
			*bufend = buffer + bytes;
					// End of buffer
    size_t		length;		// Length of marker

    image->format = "image/jpeg";

   /*
    * Scan the file for a SOFn marker, then we can get the dimensions...
    */

    while (bufptr < bufend)
    {
      if (*bufptr == 0xff)
      {
        bufptr ++;

        if (bufptr >= bufend)
        {
         /*
          * If we are at the end of the current buffer, re-fill and continue...
          */

          if ((bytes = hcFileRead(file, buffer, sizeof(buffer))) == 0)
            break;

          bufptr = buffer;
          bufend = buffer + bytes;
        }

        if (*bufptr == 0xff)
          continue;

        if ((bufptr + 16) >= bufend)
        {
         /*
          * Read more of the marker...
          */

          bytes = (size_t)(bufend - bufptr);

          memmove(buffer, bufptr, bytes);
          bufptr = buffer;
          bufend = buffer + bytes;

          if ((bytes = hcFileRead(file, bufend, sizeof(buffer) - bytes)) == 0)
            break;

          bufend += bytes;
	}

        length = (size_t)((bufptr[1] << 8) | bufptr[2]);

        _HC_DEBUG("hcImageNew: JPEG X'FF%02X' (length %u)\n", *bufptr, (unsigned)length);

        if (*bufptr == 0xe0 && length >= 16 && !memcmp(bufptr + 3, "JFIF", 5))
        {
         /*
          * APP0 marker for JFIF...
          */

          if (bufptr[10] == 1)
          {
            image->units = _HC_RES_PER_INCH;
            image->xres  = (bufptr[11] << 8) | bufptr[12];
            image->yres  = (bufptr[13] << 8) | bufptr[14];
	  }
	  else if (bufptr[10] == 2)
          {
            image->units = _HC_RES_PER_CM;
            image->xres  = (bufptr[11] << 8) | bufptr[12];
            image->yres  = (bufptr[13] << 8) | bufptr[14];
	  }
        }
        else if ((*bufptr >= 0xc0 && *bufptr <= 0xc3) ||
		 (*bufptr >= 0xc5 && *bufptr <= 0xc7) ||
		 (*bufptr >= 0xc9 && *bufptr <= 0xcb) ||
		 (*bufptr >= 0xcd && *bufptr <= 0xcf))
        {
	 /*
	  * SOFn marker, look for dimensions...
	  */

          image->width  = (bufptr[6] << 8) | bufptr[7];
          image->height = (bufptr[4] << 8) | bufptr[5];
	  break;
        }

       /*
        * Skip past this marker...
        */

        bufptr ++;
        bytes = (size_t)(bufend - bufptr);

        while (length >= bytes)
        {
          length -= bytes;

          if ((bytes = hcFileRead(file, buffer, sizeof(buffer))) == 0)
            break;

          bufptr = buffer;
          bufend = buffer + bytes;
        }

        if (length > bytes)
          break;

        bufptr += length;
      }
    }

    if (image->width == 0 || image->height == 0)
    {
      free(image);
      return (NULL);
    }
  }
  else
  {
    free(image);
    return (NULL);
  }

  return (image);
}
