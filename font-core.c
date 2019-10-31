/*
 * Core font object functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2018-2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#include "font-private.h"
#include "file-private.h"


/*
 * OFF/TTF tag constants...
 */

#define _HC_OFF_cmap	0x636d6170	/* Character to glyph mapping */
#define _HC_OFF_head	0x68656164	/* Font header */
#define _HC_OFF_hhea	0x68686561	/* Horizontal header */
#define _HC_OFF_hmtx	0x686d7478	/* Horizontal metrics */
#define _HC_OFF_maxp	0x6d617870	/* Maximum profile */
#define _HC_OFF_name	0x6e616d65	/* Naming table */
#define _HC_OFF_OS_2	0x4f532f32	/* OS/2 and Windows specific metrics */
#define _HC_OFF_post	0x706f7374	/* PostScript information */

#define _HC_OFF_Unicode		0	/* Unicode platform ID */

#define _HC_OFF_Mac		1	/* Macintosh platform ID */
#define _HC_OFF_Mac_Roman	0	/* Macintosh Roman encoding ID */
#define _HC_OFF_Mac_USEnglish	0	/* Macintosh US English language ID */

#define _HC_OFF_Windows		3	/* Windows platform ID */
#define _HC_OFF_Windows_English 9	/* Windows English language ID base */
#define _HC_OFF_Windows_UCS2	1	/* Windows UCS-2 encoding */
#define _HC_OFF_Windows_UCS4	10	/* Windows UCS-4 encoding */

#define _HC_OFF_Copyright	0	/* Copyright naming string */
#define _HC_OFF_FontFamily	1	/* Font family naming string ("Arial") */
#define _HC_OFF_FontSubfamily	2	/* Font sub-family naming string ("Bold") */
#define _HC_OFF_FontFullName	4	/* Font full name ("Arial Bold") */
#define _HC_OFF_FontVersion	5	/* Font version number */
#define _HC_OFF_PostScriptName	6	/* Font PostScript name */


/*
 * Local types...
 */

typedef struct _hc_off_dir_s		/* OFF/TTF directory entry */
{
  unsigned	tag;			/* Table identifier */
  unsigned	checksum;		/* Checksum of table */
  unsigned	offset;			/* Offset from the beginning of the file */
  unsigned	length;			/* Length */
} _hc_off_dir_t;

typedef struct _hc_off_table_s		/* OFF/TTF offset table */
{
  int		num_entries;		/* Number of table entries */
  _hc_off_dir_t	*entries;		/* Table entries */
} _hc_off_table_t;

typedef struct _hc_off_name_s		/* OFF/TTF name string */
{
  unsigned short	platform_id,	/* Platform identifier */
			encoding_id,	/* Encoding identifier */
			language_id,	/* Language identifier */
			name_id,	/* Name identifier */
			length,		/* Length of string */
			offset;		/* Offset from start of storage area */
} _hc_off_name_t;

typedef struct _hc_off_names_s		/* OFF/TTF naming table */
{
  int			num_names;	/* Number of names */
  _hc_off_name_t	*names;		/* Names */
  unsigned char		*storage;	/* Storage area */
  unsigned		storage_size;	/* Size of storage area */
} _hc_off_names_t;

typedef struct _hc_off_cmap4_s		/* Format 4 cmap table */
{
  unsigned short	startCode,	/* First character */
			endCode,	/* Last character */
			idRangeOffset;	/* Offset for range (modulo 65536) */
  short			idDelta;	/* Delta for range (modulo 65536) */
} _hc_off_cmap4_t;

typedef struct _hc_off_cmap12_s		/* Format 12 cmap table */
{
  unsigned		startCharCode,	/* First character */
			endCharCode,	/* Last character */
			startGlyphID;	/* Glyph index for the first character */
} _hc_off_cmap12_t;

typedef struct _hc_off_cmap13_s		/* Format 13 cmap table */
{
  unsigned		startCharCode,	/* First character */
			endCharCode,	/* Last character */
			glyphID;	/* Glyph index for all characters */
} _hc_off_cmap13_t;

typedef struct _hc_off_head_s		/* Font header */
{
  unsigned short	unitsPerEm;	/* Units for widths/coordinates */
  short			xMin,		/* Bounding box of all glyphs */
			yMin,
			xMax,
			yMax;
  unsigned short	macStyle;	/* Mac style bits */
} _hc_off_head_t;

#define _HC_OFF_macStyle_Bold		0x01
#define _HC_OFF_macStyle_Italic		0x02
#define _HC_OFF_macStyle_Underline	0x04
#define _HC_OFF_macStyle_Outline	0x08
#define _HC_OFF_macStyle_Shadow		0x10
#define _HC_OFF_macStyle_Condensed	0x20
#define _HC_OFF_macStyle_Extended	0x40

typedef struct _hc_off_hhea_s		/* Horizontal header */
{
  short			ascender,	/* Ascender */
			descender;	/* Descender */
  int			numberOfHMetrics;
					/* Number of horizontal metrics */
} _hc_off_hhea_t;

typedef struct _hc_off_os_2_s		/* OS/2 information */
{
  unsigned short	usWeightClass,	/* Font weight */
			fsType;		/* Type bits */
  short			sTypoAscender,	/* Ascender */
			sTypoDescender,	/* Descender */
			sCapHeight;	/* CapHeight */
} _hc_off_os_2_t;

#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x
#define _HC_OFF_fsType_	0x


/*
 * Local functions...
 */

static const char *copy_name(hc_pool_t *pool, _hc_off_names_t *names, unsigned name_id);
static int	read_cmap(hc_file_t *file, _hc_off_table_t *table, int **cmap);
static int	read_head(hc_file_t *file, _hc_off_table_t *table, _hc_off_head_t *head);
static int	read_hhea(hc_file_t *file, _hc_off_table_t *table, _hc_off_hhea_t *hhea);
static _hc_font_metric_t *read_hmtx(hc_file_t *file, _hc_off_table_t *table, _hc_off_hhea_t *hhea);
static int	read_maxp(hc_file_t *file, _hc_off_table_t *table);
static int	read_names(hc_file_t *file, _hc_off_table_t *table, _hc_off_names_t *names);
static int	read_os_2(hc_file_t *file, _hc_off_table_t *table, _hc_off_os_2_t *os_2);
static float	read_post(hc_file_t *file, _hc_off_table_t *table);
static int	read_short(hc_file_t *file);
static int	read_table(hc_file_t *file, size_t idx, _hc_off_table_t *table, size_t *num_fonts);
static unsigned	read_ulong(hc_file_t *file);
static int	read_ushort(hc_file_t *file);
static unsigned	seek_table(hc_file_t *file, _hc_off_table_t *table, unsigned tag, unsigned offset);


/*
 * 'hcFontDelete()' - Free all memory used for a font family object.
 */

void
hcFontDelete(hc_font_t *font)		/* I - Font object */
{
  int	i;				/* Looping var */


  if (!font)
    return;

  for (i = 0; i < 256; i ++)
    free(font->widths[i]);

  free(font);
}


/*
 * 'hcFontGetCopyright()' - Get the copyright text for a font.
 */

const char *				/* O - Copyright text */
hcFontGetCopyright(hc_font_t *font)	/* I - Font object */
{
  return (font ? font->copyright : NULL);
}


/*
 * 'hcFontGetFamily()' - Get the family name of a font.
 */

const char *				/* O - Family name */
hcFontGetFamily(hc_font_t *font)	/* I - Font object */
{
  return (font ? font->family : NULL);
}


/*
 * 'hcFontGetNumFonts()' - Get the number of fonts in this collection.
 */

size_t
hcFontGetNumFonts(hc_font_t *font)	/* I - Font object */
{
  return (font ? font->num_fonts : 0);
}


/*
 * 'hcFontGetPostScriptName()' - Get the PostScript name of a font.
 */

const char *				/* O - PostScript name */
hcFontGetPostScriptName(
    hc_font_t *font)			/* I - Font object */
{
  return (font ? font->postscript_name : NULL);
}


/*
 * 'hcFontGetStyle()' - Get the font style.
 */

hc_font_style_t				/* O - Style */
hcFontGetStyle(hc_font_t *font)		/* I - Font object */
{
  return (font ? font->style : HC_FONT_STYLE_NORMAL);
}


/*
 * 'hcFontGetVersion()' - Get the version number of a font.
 */

const char *				/* O - Version number */
hcFontGetVersion(hc_font_t *font)	/* I - Font object */
{
  return (font ? font->version : NULL);
}


/*
 * 'hcFontGetWeight()' - Get the weight of a font.
 */

hc_font_weight_t			/* O - Weight */
hcFontGetWeight(hc_font_t *font)	/* I - Font object */
{
  return (font ? (hc_font_weight_t)font->weight : HC_FONT_WEIGHT_400);
}


/*
 * 'hcFontNew()' - Create a new font object for the named font family.
 */

hc_font_t *				/* O - New font object */
hcFontNew(hc_pool_t *pool,		/* I - Memory pool */
          hc_file_t *file,		/* I - File */
          size_t    idx)		/* I - Font number in collection (0-based) */
{
  hc_font_t		*font = NULL;	/* New font object */
  _hc_off_table_t	table;		/* Offset table */
  _hc_off_names_t	names;		/* Names */
  int			i,		/* Looping var */
			num_cmap,	/* Number of Unicode character to glyph mappings */
			*cmap = NULL,	/* Unicode character to glyph mappings */
			num_glyphs;	/* Numnber of glyphs */
  _hc_font_metric_t	*widths = NULL;	/* Glyph metrics */
  _hc_off_head_t	head;		/* head table */
  _hc_off_hhea_t	hhea;		/* hhea table */
  _hc_off_os_2_t	os_2;		/* OS/2 table */
  size_t		num_fonts;	/* Number of fonts in table */


  if (read_table(file, idx, &table, &num_fonts))
  {
    _hcFileError(file, "Unable to read font table.");
    return (NULL);
  }

  _HC_DEBUG("hcFontNew: num_entries=%d\n", table.num_entries);

  if (read_names(file, &table, &names))
  {
    _hcFileError(file, "Unable to read names from font.");
    goto cleanup;
  }

  _HC_DEBUG("hcFontNew: num_names=%d\n", names.num_names);

  if ((font = (hc_font_t *)calloc(1, sizeof(hc_font_t))) == NULL)
  {
    goto cleanup;
  }

  font->pool            = pool;
  font->idx             = idx;
  font->num_fonts       = num_fonts;
  font->copyright       = copy_name(pool, &names, _HC_OFF_Copyright);
  font->family          = copy_name(pool, &names, _HC_OFF_FontFamily);
  font->postscript_name = copy_name(pool, &names, _HC_OFF_PostScriptName);
  font->version         = copy_name(pool, &names, _HC_OFF_FontVersion);
  font->italic_angle    = read_post(file, &table);

  _HC_DEBUG("hcFontNew: family=\"%s\"\n", font->family);

  if ((num_cmap = read_cmap(file, &table, &cmap)) <= 0)
  {
    _HC_DEBUG("hcFontNew: Unable to read cmap table.\n");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  if (read_head(file, &table, &head) < 0)
  {
    _hcFileError(file, "Unable to read head table from font.");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  font->units = (float)head.unitsPerEm;
  font->x_max = head.xMax;
  font->x_min = head.xMin;
  font->y_max = head.yMax;
  font->y_min = head.yMin;

  if (head.macStyle & _HC_OFF_macStyle_Italic)
  {
    if (font->postscript_name && strstr(font->postscript_name, "Oblique"))
      font->style = HC_FONT_STYLE_OBLIQUE;
    else
      font->style = HC_FONT_STYLE_ITALIC;
  }
  else
    font->style = HC_FONT_STYLE_NORMAL;

  if (read_hhea(file, &table, &hhea) < 0)
  {
    _hcFileError(file, "Unable to read hhea table from font.");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  font->ascent  = hhea.ascender;
  font->descent = hhea.descender;

  if ((num_glyphs = read_maxp(file, &table)) < 0)
  {
    _hcFileError(file, "Unable to read maxp table from font.");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  _HC_DEBUG("hcFontNew: num_glyphs=%d\n", num_glyphs);

  if (hhea.numberOfHMetrics > 0)
  {
    if ((widths = read_hmtx(file, &table, &hhea)) == NULL)
    {
      _hcFileError(file, "Unable to read hmtx table from font.");
      hcFontDelete(font);
      font = NULL;

      goto cleanup;
    }
  }
  else
  {
    _hcFileError(file, "Number of horizontal metrics is 0.");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  if (read_os_2(file, &table, &os_2) >= 0)
  {
    font->weight     = (short)os_2.usWeightClass;
    font->cap_height = os_2.sCapHeight;
  }
  else
  {
    _HC_DEBUG("hcFontNew: Unable to read OS/2 table.\n");
    font->weight     = 400;
    font->cap_height = font->ascent;
  }

  font->italic_angle = read_post(file, &table);

 /*
  * Build sparse widths table...
  */

  for (i = 0; i < num_cmap; i ++)
  {
    if (cmap[i] >= 0)
    {
      int	bin = i / 256,		/* Sub-array bin */
		glyph = cmap[i];	/* Glyph index */


      if (!font->widths[bin])
        font->widths[bin] = (_hc_font_metric_t *)calloc(256, sizeof(_hc_font_metric_t));

      if (glyph >= hhea.numberOfHMetrics)
	font->widths[bin][i & 255] = widths[hhea.numberOfHMetrics - 1];
      else
	font->widths[bin][i & 255] = widths[glyph];
    }
  }

 /*
  * Cleanup and return the font...
  */

  cleanup:

  free(table.entries);
  free(names.names);
  free(names.storage);
  free(widths);

  return (font);
}


/*
 * 'copy_name()' - Copy a name string from a font.
 */

static const char *			/* O - Name string or `NULL` */
copy_name(hc_pool_t       *pool,	/* I - String pool */
          _hc_off_names_t *names,	/* I - Names table */
          unsigned        name_id)	/* I - Name identifier */
{
  int			i;		/* Looping var */
  _hc_off_name_t	*name;		/* Current name */


  for (i = names->num_names, name = names->names; i > 0; i --, name ++)
  {
    if (name->name_id == name_id &&
        ((name->platform_id == _HC_OFF_Mac && name->language_id == _HC_OFF_Mac_USEnglish) ||
         (name->platform_id == _HC_OFF_Windows && (name->language_id & 0xff) == _HC_OFF_Windows_English)))
    {
      char	temp[1024],	/* Temporary string buffer */
		*tempptr,	/* Pointer into temporary string */
		*storptr;	/* Pointer into storage */
      int	chars,		/* Length of string to copy in characters */
		bpc;		/* Bytes per character */

      if ((name->offset + name->length) > names->storage_size)
      {
        _HC_DEBUG("copy_name: offset(%d)+length(%d) > storage_size(%d)\n", name->offset, name->length, names->storage_size);
        continue;
      }

      if (name->platform_id == _HC_OFF_Windows && name->encoding_id == _HC_OFF_Windows_UCS2)
      {
        storptr = (char *)names->storage + name->offset;
        chars   = name->length / 2;
        bpc     = 2;
      }
      else if (name->platform_id == _HC_OFF_Windows && name->encoding_id == _HC_OFF_Windows_UCS4)
      {
        storptr = (char *)names->storage + name->offset;
        chars   = name->length / 4;
        bpc     = 4;
      }
      else
      {
        storptr = (char *)names->storage + name->offset;
        chars   = name->length;
        bpc     = 1;
      }

      for (tempptr = temp; chars > 0; storptr += bpc, chars --)
      {
        int ch;				/* Current character */

       /*
        * Convert to Unicode...
        */

        if (bpc == 1)
          ch = *storptr;
	else if (bpc == 2)
	  ch = ((storptr[0] & 255) << 8) | (storptr[1] & 255);
	else
	  ch = ((storptr[0] & 255) << 24) | ((storptr[1] & 255) << 16) | ((storptr[2] & 255) << 8) | (storptr[3] & 255);

       /*
        * Convert to UTF-8...
        */

        if (ch < 0x80)
        {
         /*
          * 1-byte ASCII...
          */

	  if (tempptr < (temp + sizeof(temp) - 1))
	    *tempptr++ = (char)ch;
	  else
	    break;
	}
	else if (ch < 0x400)
	{
	 /*
	  * 2-byte UTF-8...
	  */

	  if (tempptr < (temp + sizeof(temp) - 2))
	  {
	    *tempptr++ = (char)(0xc0 | (ch >> 6));
	    *tempptr++ = (char)(0x80 | (ch & 0x3f));
	  }
	  else
	    break;
	}
	else if (ch < 0x10000)
	{
	 /*
	  * 3-byte UTF-8...
	  */

	  if (tempptr < (temp + sizeof(temp) - 3))
	  {
	    *tempptr++ = (char)(0xe0 | (ch >> 12));
	    *tempptr++ = (char)(0x80 | ((ch >> 6) & 0x3f));
	    *tempptr++ = (char)(0x80 | (ch & 0x3f));
	  }
	  else
	    break;
	}
	else
	{
	 /*
	  * 4-byte UTF-8...
	  */

	  if (tempptr < (temp + sizeof(temp) - 4))
	  {
	    *tempptr++ = (char)(0xf0 | (ch >> 18));
	    *tempptr++ = (char)(0x80 | ((ch >> 12) & 0x3f));
	    *tempptr++ = (char)(0x80 | ((ch >> 6) & 0x3f));
	    *tempptr++ = (char)(0x80 | (ch & 0x3f));
	  }
	  else
	    break;
	}
      }

      *tempptr = '\0';

      _HC_DEBUG("copy_name: name_id(%d) = \"%s\"\n", name_id, temp);

      return (hcPoolGetString(pool, temp));
    }
  }

  _HC_DEBUG("copy_name: No English name string for %d.\n", name_id);
  for (i = names->num_names, name = names->names; i > 0; i --, name ++)
    if (name->name_id == name_id)
      _HC_DEBUG("copy_name: Found name_id=%d, platform_id=%d, language_id=%d(0x%04x)\n", name_id, name->platform_id, name->language_id, name->language_id);

  return (NULL);
}


/*
 * 'read_cmap()' - Read the cmap table, getting the Unicode mapping table.
 */

static int				/* O - Number of cmap entries or -1 on error */
read_cmap(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table,	/* I - Offset table */
          int             **cmap)	/* O - cmap entries */
{
  unsigned	length;			/* Length of cmap table */
  int		i,			/* Looping var */
		temp,			/* Temporary value */
		num_tables,		/* Number of cmap tables */
		num_cmap = 0,		/* Number of cmap entries */
		platform_id,		/* Platform identifier (Windows or Mac) */
		encoding_id,		/* Encoding identifier (varies) */
		cformat;		/* Formap of cmap data */
  unsigned	clength,		/* Length of cmap data */
		coffset = 0,		/* Offset to cmap data */
		roman_offset = 0;	/* MacRoman offset */
  int		*cmapptr;		/* Pointer into cmap */
#if 0
  const int	*unimap = NULL;		/* Unicode character map, if any */
  static const int romanmap[256] =	/* MacRoman to Unicode map */
  {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1,
        0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
    0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3,
        0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
    0x2020, 0xB0, 0xA2, 0xA3, 0xA7, 0x2022, 0xB6, 0xDF,
        0xAE, 0xA9, 0x2122, 0xB4, 0xA8, 0x2260, 0xC6, 0xD8,
    0x221E, 0xB1, 0x2264, 0x2265, 0xA5, 0xB5, 0x2202, 0x2211,
        0x220F, 0x3C0, 0x222B, 0xAA, 0xBA, 0x3A9, 0xE6, 0xF8,
    0xBF, 0xA1, 0xAC, 0x221A, 0x192, 0x2248, 0x2206, 0xAB,
        0xBB, 0x2026, 0xA0, 0xC0, 0xC3, 0xD5, 0x152, 0x153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0xF7, 0x25CA,
        0xFF, 0x178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0xB7, 0x201A, 0x201E, 0x2030, 0xC2, 0xCA, 0xC1,
        0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xD3, 0xD4,
    0xF8FF, 0xD2, 0xDA, 0xDB, 0xD9, 0x131, 0x2C6, 0x2DC,
        0xAF, 0x2D8, 0x2D9, 0x2DA, 0xB8, 0x2DD, 0x2DB, 0x2C7
  };
#endif /* 0 */


  *cmap = NULL;

 /*
  * Find the cmap table...
  */

  if ((length = seek_table(file, table, _HC_OFF_cmap, 0)) == 0)
  {
    _hcFileError(file, "Missing cmap table.");
    return (-1);
  }

  if ((temp = read_ushort(file)) != 0)
  {
    _hcFileError(file, "Unknown cmap version %d.", temp);
    return (-1);
  }

  if ((num_tables = read_ushort(file)) < 1)
  {
    _hcFileError(file, "No cmap tables to read.");
    return (-1);
  }

  _HC_DEBUG("read_cmap: num_tables=%d\n", num_tables);

  for (i = 0; i < num_tables; i ++)
  {
    platform_id = read_ushort(file);
    encoding_id = read_ushort(file);
    coffset     = read_ulong(file);

    _HC_DEBUG("read_cmap: table[%d].platform_id=%d, encoding_id=%d, coffset=%u\n", i, platform_id, encoding_id, coffset);

    if (platform_id == _HC_OFF_Unicode || (platform_id == _HC_OFF_Windows && encoding_id == _HC_OFF_Windows_UCS2))
      break;

    if (platform_id == _HC_OFF_Mac && encoding_id == _HC_OFF_Mac_Roman)
      roman_offset = coffset;
  }

  if (i >= num_tables)
  {
    if (roman_offset)
    {
      _HC_DEBUG("read_cmap: Using MacRoman cmap table.\n");
      coffset = roman_offset;
    }
    else
    {
      _hcFileError(file, "No usable cmap table.");
      return (-1);
    }
  }

  if ((length = seek_table(file, table, _HC_OFF_cmap, coffset)) == 0)
  {
    _hcFileError(file, "Unable to read cmap table at offset %u.", coffset);
    return (-1);
  }

  if ((cformat = read_ushort(file)) < 0)
  {
    _hcFileError(file, "Unable to read cmap table format at offset %u.", coffset);
    return (-1);
  }

  _HC_DEBUG("read_cmap: cformat=%d\n", cformat);

  switch (cformat)
  {
    case 4 :
        {
         /*
          * Format 4: Segment mapping to delta values.
          *
          * This is an overly complicated linear way of encoding a sparse
          * mapping table.  And it uses 1-based indexing with modulo
          * arithmetic...
          */

          int		ch,		/* Current character */
			seg,		/* Current segment */
			glyph,		/* Current glyph */
			segCount,	/* Number of segments */
			numGlyphIdArray,/* Number of glyph IDs */
			*glyphIdArray;	/* Glyph IDs */
          _hc_off_cmap4_t *segments,	/* Segment data */
			*segment;	/* This segment */


         /*
          * Read the table...
          */

	  if ((clength = (unsigned)read_ushort(file)) == (unsigned)-1)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

	  _HC_DEBUG("read_cmap: clength=%u\n", clength);

          /* language = */       read_ushort(file);
          segCount             = read_ushort(file) / 2;
	  /* searchRange = */    read_ushort(file);
	  /* entrySelectoed = */ read_ushort(file);
	  /* rangeShift = */     read_ushort(file);

          _HC_DEBUG("read_cmap: segCount=%d\n", segCount);

          if (segCount < 2)
          {
	    _hcFileError(file, "Bad cmap table.");
	    return (-1);
          }

          numGlyphIdArray = ((int)clength - 8 * segCount - 16) / 2;
          segments        = (_hc_off_cmap4_t *)calloc((size_t)segCount, sizeof(_hc_off_cmap4_t));
          glyphIdArray    = (int *)calloc((size_t)numGlyphIdArray, sizeof(int));

          _HC_DEBUG("read_cmap: numGlyphIdArray=%d\n", numGlyphIdArray);

          for (i = 0; i < segCount; i ++)
            segments[i].endCode = (unsigned short)read_ushort(file);

	  /* reservedPad = */ read_ushort(file);

          for (i = 0; i < segCount; i ++)
            segments[i].startCode = (unsigned short)read_ushort(file);

          for (i = 0; i < segCount; i ++)
            segments[i].idDelta = (short)read_short(file);

          for (i = 0; i < segCount; i ++)
            segments[i].idRangeOffset = (unsigned short)read_ushort(file);

          for (i = 0; i < numGlyphIdArray; i ++)
            glyphIdArray[i] = read_ushort(file);

#ifdef DEBUG
          for (i = 0; i < segCount; i ++)
            _HC_DEBUG("read_cmap: segment[%d].startCode=%d, endCode=%d, idDelta=%d, idRangeOffset=%d\n", i, segments[i].startCode, segments[i].endCode, segments[i].idDelta, segments[i].idRangeOffset);

          for (i = 0; i < numGlyphIdArray; i ++)
            _HC_DEBUG("read_cmap: glyphIdArray[%d]=%d\n", i, glyphIdArray[i]);
#endif /* DEBUG */

         /*
          * Based on the end code of the segent table, allocate space for the
          * uncompressed cmap table...
          */

          segCount --;			/* Last segment is not used (sigh) */

	  num_cmap = segments[segCount - 1].endCode + 1;
	  cmapptr  = *cmap = (int *)malloc((size_t)num_cmap * sizeof(int));

          memset(cmapptr, -1, (size_t)num_cmap * sizeof(int));

         /*
          * Now loop through the segments and assign glyph indices from the
          * array...
          */

          for (seg = segCount, segment = segments; seg > 0; seg --, segment ++)
          {
            for (ch = segment->startCode; ch <= segment->endCode; ch ++)
            {
              if (segment->idRangeOffset)
              {
               /*
                * Use an "obscure indexing trick" (words from the spec, not
                * mine) to look up the glyph index...
                */

                int temp = segment->idRangeOffset / 2 + ch - segment->startCode + seg - segCount - 1;

                if (temp < 0 || temp >= numGlyphIdArray || !glyphIdArray[temp])
                  glyph = -1;
		else
		  glyph = ((glyphIdArray[temp] + segment->idDelta) & 65535) - 1;
              }
              else
              {
               /*
                * Just use idDelta to compute a glyph index...
                */

                glyph = ((ch + segment->idDelta) & 65535) - 1;
	      }

	      cmapptr[ch] = glyph;
            }
	  }

         /*
          * Free the segment data...
          */

	  free(segments);
	  free(glyphIdArray);
        }
        break;

    case 12 :
	{
	 /*
	  * Format 12: Segmented coverage
	  *
	  * A simple sparse linear segment mapping format.
	  */

	  unsigned	ch,		/* Current character */
			gidx,		/* Current group */
			nGroups;	/* Number of groups */
	  _hc_off_cmap12_t *groups,	/* Groups */
			*group;		/* This group */


	 /*
	  * Read the table...
	  */

          /* reserved */ read_ushort(file);

	  if ((clength = read_ulong(file)) == 0)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

	  /* language = */ read_ulong(file);
	  nGroups        = read_ulong(file);

	  _HC_DEBUG("read_cmap: nGroups=%u\n", nGroups);

	  groups = (_hc_off_cmap12_t *)calloc(nGroups, sizeof(_hc_off_cmap12_t));

	  for (gidx = 0, group = groups, num_cmap = 0; gidx < nGroups; gidx ++, group ++)
	  {
	    group->startCharCode = read_ulong(file);
	    group->endCharCode   = read_ulong(file);
	    group->startGlyphID  = read_ulong(file);
	    _HC_DEBUG("read_cmap: [%u] startCharCode=%u, endCharCode=%u, startGlyphID=%u\n", gidx, group->startCharCode, group->endCharCode, group->startGlyphID);

            if (group->endCharCode >= (unsigned)num_cmap)
              num_cmap = (int)group->endCharCode + 1;
	  }

	 /*
	  * Based on the end code of the segent table, allocate space for the
	  * uncompressed cmap table...
	  */

          _HC_DEBUG("read_cmap: num_cmap=%u\n", (unsigned)num_cmap);
	  cmapptr = *cmap = (int *)malloc((size_t)num_cmap * sizeof(int));

	  memset(cmapptr, -1, (size_t)num_cmap * sizeof(int));

	 /*
	  * Now loop through the groups and assign glyph indices from the
	  * array...
	  */

	  for (gidx = 0, group = groups; gidx < nGroups; gidx ++, group ++)
	  {
            for (ch = group->startCharCode; ch <= group->endCharCode && ch < _HC_FONT_MAX_CHAR; ch ++)
              cmapptr[ch] = (int)(group->startGlyphID + ch - group->startCharCode);
          }

	 /*
	  * Free the group data...
	  */

	  free(groups);
	}
        break;

    case 13 :
	{
	 /*
	  * Format 13: Many-to-one range mappings
	  *
	  * Typically used for fonts of last resort where multiple characters
	  * map to the same glyph.
	  */

	  unsigned	ch,		/* Current character */
			gidx,		/* Current group */
			nGroups;	/* Number of groups */
	  _hc_off_cmap13_t *groups,	/* Groups */
			*group;		/* This group */


	 /*
	  * Read the table...
	  */

          /* reserved */ read_ushort(file);

	  if ((clength = read_ulong(file)) == 0)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

	  /* language = */ read_ulong(file);
	  nGroups        = read_ulong(file);

	  _HC_DEBUG("read_cmap: nGroups=%u\n", nGroups);

	  groups = (_hc_off_cmap13_t *)calloc(nGroups, sizeof(_hc_off_cmap13_t));

	  for (gidx = 0, group = groups, num_cmap = 0; gidx < nGroups; gidx ++, group ++)
	  {
	    group->startCharCode = read_ulong(file);
	    group->endCharCode   = read_ulong(file);
	    group->glyphID       = read_ulong(file);
	    _HC_DEBUG("read_cmap: [%u] startCharCode=%u, endCharCode=%u, glyphID=%u\n", gidx, group->startCharCode, group->endCharCode, group->glyphID);

            if (group->endCharCode >= (unsigned)num_cmap)
              num_cmap = (int)group->endCharCode + 1;
	  }

	 /*
	  * Based on the end code of the segent table, allocate space for the
	  * uncompressed cmap table...
	  */

          _HC_DEBUG("read_cmap: num_cmap=%u\n", (unsigned)num_cmap);
	  cmapptr = *cmap = (int *)malloc((size_t)num_cmap * sizeof(int));

	  memset(cmapptr, -1, (size_t)num_cmap * sizeof(int));

	 /*
	  * Now loop through the groups and assign glyph indices from the
	  * array...
	  */

	  for (gidx = 0, group = groups; gidx < nGroups; gidx ++, group ++)
	  {
            for (ch = group->startCharCode; ch <= group->endCharCode && ch < _HC_FONT_MAX_CHAR; ch ++)
              cmapptr[ch] = (int)group->glyphID;
          }

	 /*
	  * Free the group data...
	  */

	  free(groups);
	}
        break;

    default :
        _hcFileError(file, "Format %d cmap tables are not yet supported.", cformat);
        return (-1);
  }

#ifdef DEBUG
  cmapptr = *cmap;
  for (i = 0; i < num_cmap && i < 127; i ++)
    if (cmapptr[i] >= 0)
      _HC_DEBUG("read_cmap; cmap[%d]=%d\n", i, cmapptr[i]);
#endif /* DEBUG */

  return (num_cmap);
}


/*
 * 'read_head()' - Read the head table.
 */

static int				/* O - 0 on success, -1 on error */
read_head(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table,	/* I - Offset table */
          _hc_off_head_t  *head)	/* O - head table data */
{
  unsigned	length;			/* Length of head table */


  memset(head, 0, sizeof(_hc_off_head_t));

  if ((length = seek_table(file, table, _HC_OFF_head, 0)) == 0)
    return (-1);

  /* majorVersion */       read_ushort(file);
  /* minorVersion */       read_ushort(file);
  /* fontRevision */       read_ulong(file);
  /* checkSumAdjustment */ read_ulong(file);
  /* magicNumber */        read_ulong(file);
  /* flags */              read_ushort(file);
  head->unitsPerEm       = (unsigned short)read_ushort(file);
  /* created */            read_ulong(file); read_ulong(file);
  /* modified */           read_ulong(file); read_ulong(file);
  head->xMin             = (short)read_short(file);
  head->yMin             = (short)read_short(file);
  head->xMax             = (short)read_short(file);
  head->yMax             = (short)read_short(file);
  head->macStyle         = (unsigned short)read_ushort(file);

  return (0);
}


/*
 * 'read_hhea()' - Read the hhea table.
 */

static int				/* O - 0 on success, -1 on error */
read_hhea(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table,	/* I - Offset table */
          _hc_off_hhea_t  *hhea)	/* O - hhea table data */
{
  unsigned	length;			/* Length of hhea table */


  memset(hhea, 0, sizeof(_hc_off_hhea_t));

  if ((length = seek_table(file, table, _HC_OFF_hhea, 0)) == 0)
    return (-1);

  /* majorVersion */        read_ushort(file);
  /* minorVersion */        read_ushort(file);
  hhea->ascender          = (short)read_short(file);
  hhea->descender         = (short)read_short(file);
  /* lineGap */             read_short(file);
  /* advanceWidthMax */     read_ushort(file);
  /* minLeftSideBearing */  read_short(file);
  /* minRightSideBearing */ read_short(file);
  /* mMaxExtent */          read_short(file);
  /* caretSlopeRise */      read_short(file);
  /* caretSlopeRun */       read_short(file);
  /* caretOffset */         read_short(file);
  /* (reserved) */          read_short(file);
  /* (reserved) */          read_short(file);
  /* (reserved) */          read_short(file);
  /* (reserved) */          read_short(file);
  /* metricDataFormat */    read_short(file);
  hhea->numberOfHMetrics  = (unsigned short)read_ushort(file);

  return (0);
}


/*
 * 'read_hmtx()' - Read the horizontal metrics from the font.
 */

static _hc_font_metric_t *		/* O - Array of glyph metrics */
read_hmtx(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table,	/* I - Offset table */
          _hc_off_hhea_t  *hhea)	/* O - hhea table data */
{
  unsigned		length;		/* Length of hmtx table */
  int			i;		/* Looping var */
  _hc_font_metric_t	*widths;	/* Glyph metrics array */


  if ((length = seek_table(file, table, _HC_OFF_hmtx, 0)) == 0)
    return (NULL);

  if (length < (unsigned)(4 * hhea->numberOfHMetrics))
  {
    _hcFileError(file, "Length of hhea table is only %u, expected at least %d.", length, 4 * hhea->numberOfHMetrics);
    return (NULL);
  }

  if ((widths = (_hc_font_metric_t *)calloc((size_t)hhea->numberOfHMetrics, sizeof(_hc_font_metric_t))) == NULL)
    return (NULL);

  for (i = 0; i < hhea->numberOfHMetrics; i ++)
  {
    widths[i].width        = (short)read_ushort(file);
    widths[i].left_bearing = (short)read_short(file);
  }

  return (widths);
}


/*
 * 'read_maxp()' - Read the number of glyphs in the font.
 */

static int				/* O - Number of glyphs or -1 on error */
read_maxp(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table)	/* I - Offset table */
{
 /*
  * All we care about is the number of glyphs, so get grab that...
  */

  if (seek_table(file, table, _HC_OFF_maxp, 4) == 0)
    return (-1);
  else
    return (read_ushort(file));
}


/*
 * 'read_names()' - Read the name strings from a font.
 */

static int				/* O - 0 on success, -1 on error */
read_names(hc_file_t       *file,	/* I - File */
           _hc_off_table_t *table,	/* I - Offset table */
           _hc_off_names_t *names)	/* O - Names */
{
  unsigned	length;			/* Length of names table */
  int		i,			/* Looping var */
		format,			/* Name table format */
		offset;			/* Offset to storage */
  _hc_off_name_t *name;			/* Current name */


  memset(names, 0, sizeof(_hc_off_names_t));

 /*
  * Find the name table...
  */

  if ((length = seek_table(file, table, _HC_OFF_name, 0)) == 0)
    return (-1);

  if ((format = read_ushort(file)) < 0 || format > 1)
    return (-1);

  _HC_DEBUG("read_names: format=%d\n", format);

  if ((names->num_names = read_ushort(file)) < 1)
    return (-1);

  if ((names->names = (_hc_off_name_t *)calloc((size_t)names->num_names, sizeof(_hc_off_name_t))) == NULL)
    return (-1);

  if ((offset = read_ushort(file)) < 0 || (unsigned)offset >= length)
    return (-1);

  names->storage_size = length - (unsigned)offset;
  if ((names->storage = malloc(names->storage_size)) == NULL)
    return (-1);
  memset(names->storage, 'A', names->storage_size);

  for (i = names->num_names, name = names->names; i > 0; i --, name ++)
  {
    name->platform_id = (unsigned short)read_ushort(file);
    name->encoding_id = (unsigned short)read_ushort(file);
    name->language_id = (unsigned short)read_ushort(file);
    name->name_id     = (unsigned short)read_ushort(file);
    name->length      = (unsigned short)read_ushort(file);
    name->offset      = (unsigned short)read_ushort(file);

    _HC_DEBUG("name->platform_id=%d, encoding_id=%d, language_id=%d(0x%04x), name_id=%d, length=%d, offset=%d\n", name->platform_id, name->encoding_id, name->language_id, name->language_id, name->name_id, name->length, name->offset);
  }

  if (format == 1)
  {
   /*
    * Skip language_id table...
    */

    int count = read_ushort(file);	/* Number of language IDs */

    _HC_DEBUG("read_names: Skipping language_id table...\n");

    while (count > 0)
    {
      read_ushort(file); /* length */
      read_ushort(file); /* offset */
      count --;
    }
  }

  hcFileRead(file, names->storage, length - (unsigned)offset);

  return (0);
}


/*
 * 'read_os_2()' - Read the OS/2 table.
 */

static int				/* O - 0 on success, -1 on error */
read_os_2(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table,	/* I - Offset table */
          _hc_off_os_2_t  *os_2)	/* O - OS/2 table */
{
  unsigned	length;			/* Length of OS/2 table */
  int		version;		/* OS/2 table version */
  unsigned char	panose[10];		/* panose value */


  memset(os_2, 0, sizeof(_hc_off_os_2_t));

 /*
  * Find the OS/2 table...
  */

  if ((length = seek_table(file, table, _HC_OFF_OS_2, 0)) == 0)
    return (-1);

  if ((version = read_ushort(file)) < 0)
    return (-1);

  _HC_DEBUG("read_names: version=%d\n", version);

  /* xAvgCharWidth */       read_short(file);
  os_2->usWeightClass     = (unsigned short)read_ushort(file);
  /* usWidthClass */        read_ushort(file);
  os_2->fsType            = (unsigned short)read_ushort(file);
  /* ySubscriptXSize */     read_short(file);
  /* ySubscriptYSize */     read_short(file);
  /* ySubscriptXOffset */   read_short(file);
  /* ySubscriptYOffset */   read_short(file);
  /* ySuperscriptXSize */   read_short(file);
  /* ySuperscriptYSize */   read_short(file);
  /* ySuperscriptXOffset */ read_short(file);
  /* ySuperscriptYOffset */ read_short(file);
  /* yStrikeoutSize */      read_short(file);
  /* yStrikeoutOffset */    read_short(file);
  /* sFamilyClass */        read_short(file);
  /* panose[10] */          hcFileRead(file, panose, sizeof(panose));
  /* ulUnicodeRange1 */     read_ulong(file);
  /* ulUnicodeRange2 */     read_ulong(file);
  /* ulUnicodeRange3 */     read_ulong(file);
  /* ulUnicodeRange4 */     read_ulong(file);
  /* achVendID */           read_ulong(file); read_ulong(file);
                            read_ulong(file); read_ulong(file);
  /* fsSelection */         read_ushort(file);
  /* usFirstCharIndex */    read_ushort(file);
  /* usLastCharIndex */     read_ushort(file);
  os_2->sTypoAscender     = (short)read_short(file);
  os_2->sTypoDescender    = (short)read_short(file);
  /* sTypoLineGap */        read_short(file);
  /* usWinAscent */         read_ushort(file);
  /* usWinDescent */        read_ushort(file);

  if (version >= 4)
  {
    /* ulCodePageRange1 */  read_ulong(file);
    /* ulCodePageRange2 */  read_ulong(file);
    /* sxHeight */          read_short(file);
    os_2->sCapHeight      = (short)read_short(file);
  }

  return (0);
}


/*
 * 'read_post()' - Read the italicAngle value from the post table.
 */

static float				/* O - italicAngle value or 0.0 */
read_post(hc_file_t       *file,	/* I - File */
          _hc_off_table_t *table)	/* I - Offset table */
{
  unsigned	length;			/* Length of post table */


  if ((length = seek_table(file, table, _HC_OFF_post, 0)) == 0)
    return (0.0f);

  /* version */ read_ulong(file);

  return ((int)read_ulong(file) / 65536.0f);
}


/*
 * 'read_short()' - Read a 16-bit signed integer.
 */

static int				/* O - 16-bit signed integer value or EOF */
read_short(hc_file_t *file)		/* i - File to read from */
{
  unsigned char	buffer[2];		/* Read buffer */


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return (EOF);
  else if (buffer[0] & 0x80)
    return (((buffer[0] << 8) | buffer[1]) - 65536);
  else
    return ((buffer[0] << 8) | buffer[1]);
}


/*
 * 'read_table()' - Read an OFF/TTF offset table.
 */

static int				/* O - 0 on success, EOF on failure */
read_table(hc_file_t       *file,	/* I - File */
           size_t          idx,		/* I - Font number within collection */
           _hc_off_table_t *table,	/* O - Offset table */
           size_t          *num_fonts)	/* O - Number of fonts */
{
  int		i;			/* Looping var */
  unsigned	temp;			/* Temporary value */
  _hc_off_dir_t	*current;		/* Current table entry */


 /*
  * Read the table header:
  *
  *     Fixed  sfnt version (should be 0x10000 for version 1.0)
  *     USHORT numTables
  *     USHORT searchRange
  *     USHORT entrySelector
  *     USHORT rangeShift
  */

  memset(table, 0, sizeof(_hc_off_table_t));

  *num_fonts = 0;

  /* sfnt version */
  if ((temp = read_ulong(file)) != 0x10000 && temp != 0x4f54544f && temp != 0x74746366)
    return (-1);

  if (temp == 0x74746366)
  {
   /*
    * Font collection, get the number of fonts and then seek to the start of
    * the offset table for the desired font...
    */

    _HC_DEBUG("read_table: Font collection\n");

    /* Version */
    if ((temp = read_ulong(file)) != 0x10000 && temp != 0x20000)
      return (-1);

    _HC_DEBUG("read_table: Collection version=%d.0\n", temp / 65536);

    /* numFonts */
    if ((temp = read_ulong(file)) == 0)
      return (-1);

    *num_fonts = (size_t)temp;

    _HC_DEBUG("read_table: numFonts=%u\n", temp);

    if (idx >= *num_fonts)
      return (-1);

    /* OffsetTable */
    temp = read_ulong(file);
    while (idx > 0)
    {
      temp = read_ulong(file);
      idx --;
    }

    _HC_DEBUG("read_table: Offset for font %u is %u.\n", (unsigned)idx, temp);

    hcFileSeek(file, temp + 4);
  }
  else
    *num_fonts = 1;

  /* numTables */
  if ((table->num_entries = read_ushort(file)) <= 0)
    return (-1);

  _HC_DEBUG("read_table: num_entries=%u\n", (unsigned)table->num_entries);

  /* searchRange */
  if (read_ushort(file) < 0)
    return (-1);

  /* entrySelector */
  if (read_ushort(file) < 0)
    return (-1);

  /* rangeShift */
  if (read_ushort(file) < 0)
    return (-1);

 /*
  * Read the table entries...
  */

  if ((table->entries = calloc((size_t)table->num_entries, sizeof(_hc_off_dir_t))) == NULL)
    return (-1);

  for (i = table->num_entries, current = table->entries; i > 0; i --, current ++)
  {
    current->tag      = read_ulong(file);
    current->checksum = read_ulong(file);
    current->offset   = read_ulong(file);
    current->length   = read_ulong(file);

    _HC_DEBUG("read_table: [%d] tag='%c%c%c%c' checksum=%u offset=%u length=%u\n", i, (current->tag >> 24) & 255, (current->tag >> 16) & 255, (current->tag >> 8) & 255, current->tag & 255, current->checksum, current->offset, current->length);
  }

  return (0);
}


/*
 * 'read_ulong()' - Read a 32-bit unsigned integer.
 */

static unsigned				/* O - 32-bit unsigned integer value or EOF */
read_ulong(hc_file_t *file)		/* I - File to read from */
{
  unsigned char	buffer[4];		/* Read buffer */


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return ((unsigned)EOF);
  else
    return ((unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]));
}


/*
 * 'read_ushort()' - Read a 16-bit unsigned integer.
 */

static int				/* O - 16-bit unsigned integer value or EOF */
read_ushort(hc_file_t *file)		/* i - File to read from */
{
  unsigned char	buffer[2];		/* Read buffer */


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return (EOF);
  else
    return ((buffer[0] << 8) | buffer[1]);
}


/*
 * 'seek_table()' - Seek to a specific table in a font.
 */

static unsigned				/* O - Length of table or 0 if not found */
seek_table(hc_file_t       *file,	/* I - File */
           _hc_off_table_t *table,	/* I - Font table */
           unsigned        tag,		/* I - Tag to find */
           unsigned        offset)	/* I - Additional offset */
{
  int		i;			/* Looping var */
  _hc_off_dir_t	*current;		/* Current entry */


  for (i = table->num_entries, current = table->entries; i  > 0; i --, current ++)
  {
    if (current->tag == tag)
    {
      if (hcFileSeek(file, current->offset + offset) == (current->offset + offset))
        return (current->length - offset);
      else
        return (0);
    }
  }

  return (0);
}
