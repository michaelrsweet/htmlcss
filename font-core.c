//
// Core font object functions for HTMLCSS library.
//
//     https://github.com/michaelrsweet/htmlcss
//
// Copyright © 2018-2025 by Michael R Sweet.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#include "font-private.h"
#include "file-private.h"


//
// OFF/TTF tag constants...
//

#define _HC_OFF_cmap	0x636d6170	// Character to glyph mapping
#define _HC_OFF_head	0x68656164	// Font header
#define _HC_OFF_hhea	0x68686561	// Horizontal header
#define _HC_OFF_hmtx	0x686d7478	// Horizontal metrics
#define _HC_OFF_maxp	0x6d617870	// Maximum profile
#define _HC_OFF_name	0x6e616d65	// Naming table
#define _HC_OFF_OS_2	0x4f532f32	// OS/2 and Windows specific metrics
#define _HC_OFF_post	0x706f7374	// PostScript information

#define _HC_OFF_Unicode		0	// Unicode platform ID

#define _HC_OFF_Mac		1	// Macintosh platform ID
#define _HC_OFF_Mac_Roman	0	// Macintosh Roman encoding ID
#define _HC_OFF_Mac_USEnglish	0	// Macintosh US English language ID

#define _HC_OFF_Windows		3	// Windows platform ID
#define _HC_OFF_Windows_English 9	// Windows English language ID base
#define _HC_OFF_Windows_UCS2	1	// Windows UCS-2 encoding
#define _HC_OFF_Windows_UCS4	10	// Windows UCS-4 encoding

#define _HC_OFF_Copyright	0	// Copyright naming string
#define _HC_OFF_FontFamily	1	// Font family naming string ("Arial")
#define _HC_OFF_FontSubfamily	2	// Font sub-family naming string ("Bold")
#define _HC_OFF_FontFullName	4	// Font full name ("Arial Bold")
#define _HC_OFF_FontVersion	5	// Font version number
#define _HC_OFF_PostScriptName	6	// Font PostScript name


//
// Local types...
//

typedef struct _hc_off_dir_s		// OFF/TTF directory entry
{
  unsigned	tag;			// Table identifier
  unsigned	checksum;		// Checksum of table
  unsigned	offset;			// Offset from the beginning of the file
  unsigned	length;			// Length
} _hc_off_dir_t;

typedef struct _hc_off_table_s		// OFF/TTF offset table
{
  int		num_entries;		// Number of table entries
  _hc_off_dir_t	*entries;		// Table entries
} _hc_off_table_t;

typedef struct _hc_off_name_s		// OFF/TTF name string
{
  unsigned short	platform_id,	// Platform identifier
			encoding_id,	// Encoding identifier
			language_id,	// Language identifier
			name_id,	// Name identifier
			length,		// Length of string
			offset;		// Offset from start of storage area
} _hc_off_name_t;

typedef struct _hc_off_names_s		// OFF/TTF naming table
{
  int			num_names;	// Number of names
  _hc_off_name_t	*names;		// Names
  unsigned char		*storage;	// Storage area
  unsigned		storage_size;	// Size of storage area
} _hc_off_names_t;

typedef struct _hc_off_cmap4_s		// Format 4 cmap table
{
  unsigned short	startCode,	// First character
			endCode,	// Last character
			idRangeOffset;	// Offset for range (modulo 65536)
  short			idDelta;	// Delta for range (modulo 65536)
} _hc_off_cmap4_t;

typedef struct _hc_off_cmap12_s		// Format 12 cmap table
{
  unsigned		startCharCode,	// First character
			endCharCode,	// Last character
			startGlyphID;	// Glyph index for the first character
} _hc_off_cmap12_t;

typedef struct _hc_off_cmap13_s		// Format 13 cmap table
{
  unsigned		startCharCode,	// First character
			endCharCode,	// Last character
			glyphID;	// Glyph index for all characters
} _hc_off_cmap13_t;

typedef struct _hc_off_head_s		// Font header
{
  unsigned short	unitsPerEm;	// Units for widths/coordinates
  short			xMin,		// Bounding box of all glyphs
			yMin,
			xMax,
			yMax;
  unsigned short	macStyle;	// Mac style bits
} _hc_off_head_t;

#define _HC_OFF_macStyle_Bold		0x01
#define _HC_OFF_macStyle_Italic		0x02
#define _HC_OFF_macStyle_Underline	0x04
#define _HC_OFF_macStyle_Outline	0x08
#define _HC_OFF_macStyle_Shadow		0x10
#define _HC_OFF_macStyle_Condensed	0x20
#define _HC_OFF_macStyle_Extended	0x40

typedef struct _hc_off_hhea_s		// Horizontal header
{
  short			ascender,	// Ascender
			descender;	// Descender
  int			numberOfHMetrics;
					// Number of horizontal metrics
} _hc_off_hhea_t;

typedef struct _hc_off_os_2_s		// OS/2 information
{
  unsigned short	usWeightClass,	// Font weight
			usWidthClass,	// Font weight
			fsType;		// Type bits
  short			sTypoAscender,	// Ascender
			sTypoDescender,	// Descender
			sxHeight,	// xHeight
			sCapHeight;	// CapHeight
} _hc_off_os_2_t;

typedef struct _hc_off_post_s		// PostScript information
{
  float		italicAngle;		// Italic angle
  unsigned	isFixedPitch;		// Fixed-width font?
} _hc_off_post_t;


//
// Local functions...
//

static const char *copy_name(hc_pool_t *pool, _hc_off_names_t *names, unsigned name_id);
static int	read_cmap(hc_file_t *file, _hc_off_table_t *table, int **cmap);
static bool	read_head(hc_file_t *file, _hc_off_table_t *table, _hc_off_head_t *head);
static bool	read_hhea(hc_file_t *file, _hc_off_table_t *table, _hc_off_hhea_t *hhea);
static _hc_font_metric_t *read_hmtx(hc_file_t *file, _hc_off_table_t *table, _hc_off_hhea_t *hhea);
static int	read_maxp(hc_file_t *file, _hc_off_table_t *table);
static bool	read_names(hc_file_t *file, _hc_off_table_t *table, _hc_off_names_t *names);
static bool	read_os_2(hc_file_t *file, _hc_off_table_t *table, _hc_off_os_2_t *os_2);
static bool	read_post(hc_file_t *file, _hc_off_table_t *table, _hc_off_post_t *post);
static int	read_short(hc_file_t *file);
static bool	read_table(hc_file_t *file, size_t idx, _hc_off_table_t *table, size_t *num_fonts);
static unsigned	read_ulong(hc_file_t *file);
static int	read_ushort(hc_file_t *file);
static unsigned	seek_table(hc_file_t *file, _hc_off_table_t *table, unsigned tag, unsigned offset);


//
// 'hcFontDelete()' - Free all memory used for a font family object.
//

void
hcFontDelete(hc_font_t *font)		// I - Font object
{
  int	i;				// Looping var


  if (!font)
    return;

  free(font->cmap);

  for (i = 0; i < 256; i ++)
    free(font->widths[i]);

  free(font);
}


//
// 'hcFontGetAscent()' - Get the maximum height of non-accented characters.
//

int					// O - Ascent in 1000ths
hcFontGetAscent(hc_font_t *font)	// I - Font
{
  return (font ? (int)(1000 * font->ascent / font->units) : 0);
}


//
// 'hcFontGetBounds()' - Get the bounds of all characters in a font.
//
// This function gets the bounds of all characters in a font.  The "bounds"
// argument is a pointer to a `hc_rect_t` structure that will be filled with
// the limits for characters in the font scaled to a 1000x1000 unit square.
//

hc_rect_t *				// O - Bounds or `NULL` on error
hcFontGetBounds(hc_font_t *font,	// I - Font
                hc_rect_t *bounds)	// I - Bounds buffer
{
  // Range check input...
  if (!font || !bounds)
  {
    if (bounds)
      memset(bounds, 0, sizeof(hc_rect_t));

    return (NULL);
  }

  bounds->left   = 1000.0f * font->x_min / font->units;
  bounds->right  = 1000.0f * font->x_max / font->units;
  bounds->bottom = 1000.0f * font->y_min / font->units;
  bounds->top    = 1000.0f * font->y_max / font->units;

  return (bounds);
}


//
// 'hcFontGetCapHeight()' - Get the height of capital letters.
//

int					// O - Capital letter height in 1000ths
hcFontGetCapHeight(hc_font_t *font)	// I - Font
{
  return (font ? (int)(1000 * font->cap_height / font->units) : 0);
}


//
// 'hcFontGetCMap()' - Get the Unicode to glyph mapping table.
//

const int *				// O - CMap table
hcFontGetCMap(hc_font_t *font,		// I - Font
              size_t    *num_cmap)	// O - Number of entries in table
{
  // Range check input...
  if (!font || !num_cmap)
  {
    if (num_cmap)
      *num_cmap = 0;

    return (NULL);
  }

  *num_cmap = font->num_cmap;
  return (font->cmap);
}


//
// 'hcFontGetCopyright()' - Get the copyright text for a font.
//

const char *				// O - Copyright text
hcFontGetCopyright(hc_font_t *font)	// I - Font object
{
  return (font ? font->copyright : NULL);
}


//
// 'hcFontGetDescent()' - Get the maximum depth of non-accented characters.
//

int					// O - Descent in 1000ths
hcFontGetDescent(hc_font_t *font)	// I - Font
{
  return (font ? (int)(1000 * font->descent / font->units) : 0);
}


//
// 'hcFontGetExtents()' - Get the extents of a UTF-8 string.
//
// This function computes the extents of the UTF-8 string "s" when rendered
// using the specified font "font" and size "size".  The "extents" argument is
// a pointer to a `hc_rect_t` structure that is filled with the extents of a
// simple rendering of the string with no kerning or rewriting applied.  The
// values are scaled using the specified font size.
//

hc_rect_t *				// O - Pointer to extents or `NULL` on error
hcFontGetExtents(
    hc_font_t  *font,			// I - Font
    float      size,			// I - Font size
    const char *s,			// I - String
    hc_rect_t  *extents)		// O - Extents of the string
{
  bool		first = true;		// First character?
  int		ch,			// Current character
		width = 0;		// Width
  _hc_font_metric_t *widths;		// Widths


  _HC_DEBUG("hcFontGetExtents(font=%p, size=%.2f, s=\"%s\", extents=%p)\n", (void *)font, size, s, (void *)extents);

  // Make sure extents is zeroed out...
  if (extents)
    memset(extents, 0, sizeof(hc_rect_t));

  // Range check input...
  if (!font || size <= 0.0f || !s || !extents)
    return (NULL);

  // Loop through the string...
  while (*s)
  {
    // Get the next Unicode character...
    if ((*s & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80)
    {
      // Two byte UTF-8
      ch = ((*s & 0x1f) << 6) | (s[1] & 0x3f);
      s += 2;
    }
    else if ((*s & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80)
    {
      // Three byte UTF-8
      ch = ((*s & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
      s += 3;
    }
    else if ((*s & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80)
    {
      // Four byte UTF-8
      ch = ((*s & 0x07) << 18) | ((s[1] & 0x3f) << 12) | ((s[2] & 0x3f) << 6) | (s[3] & 0x3f);
      s += 4;
    }
    else if (*s & 0x80)
    {
      // Invalid UTF-8
      _hcPoolError(font->pool, 0, "Invalid UTF-8 sequence starting with 0x%02X.", *s & 255);
      return (NULL);
    }
    else
    {
      // ASCII...
      ch = *s++;
    }

    // Find its width...
    if ((widths = font->widths[ch / 256]) != NULL)
    {
      if (first)
      {
        extents->left = -widths[ch & 255].left_bearing / font->units;
        first         = false;
      }

      width += widths[ch & 255].width;
    }
    else if ((widths = font->widths[0]) != NULL)
    {
      // Use the ".notdef" (0) glyph width...
      if (first)
      {
        extents->left = -widths[0].left_bearing / font->units;
        first         = false;
      }

      width += widths[0].width;
    }
  }

  // Calculate the bounding box for the text and return...
  _HC_DEBUG("hcFontGetExtents: width=%d\n", width);

  extents->bottom = size * font->y_min / font->units;
  extents->right  = size * width / font->units + extents->left;
  extents->top    = size * font->y_max / font->units;

  return (extents);
}


//
// 'hcFontGetFamily()' - Get the family name of a font.
//

const char *				// O - Family name
hcFontGetFamily(hc_font_t *font)	// I - Font object
{
  return (font ? font->family : NULL);
}


//
// 'hcFontGetItalicAngle()' - Get the italic angle.
//

float					// O - Angle in degrees
hcFontGetItalicAngle(hc_font_t *font)	// I - Font
{
  return (font ? font->italic_angle : 0.0f);
}


//
// 'hcFontGetMaxChar()' - Get the last character in the font.
//

int					// O - Last character in font
hcFontGetMaxChar(hc_font_t *font)	// I - Font
{
  return (font ? font->max_char : 0);
}


//
// 'hcFontGetMinChar()' - Get the first character in the font.
//

int					// O - First character in font
hcFontGetMinChar(hc_font_t *font)	// I - Font
{
  return (font ? font->min_char : 0);
}


//
// 'hcFontGetNumFonts()' - Get the number of fonts in this collection.
//

size_t
hcFontGetNumFonts(hc_font_t *font)	// I - Font object
{
  return (font ? font->num_fonts : 0);
}


//
// 'hcFontGetPostScriptName()' - Get the PostScript name of a font.
//

const char *				// O - PostScript name
hcFontGetPostScriptName(
    hc_font_t *font)			// I - Font object
{
  return (font ? font->postscript_name : NULL);
}


//
// 'hcFontGetStyle()' - Get the font style.
//

hc_font_style_t				// O - Style
hcFontGetStyle(hc_font_t *font)		// I - Font object
{
  return (font ? font->style : HC_FONT_STYLE_NORMAL);
}


//
// 'hcFontGetVersion()' - Get the version number of a font.
//

const char *				// O - Version number
hcFontGetVersion(hc_font_t *font)	// I - Font object
{
  return (font ? font->version : NULL);
}


//
// 'hcFontGetWeight()' - Get the weight of a font.
//

hc_font_weight_t			// O - Weight
hcFontGetWeight(hc_font_t *font)	// I - Font object
{
  return (font ? (hc_font_weight_t)font->weight : HC_FONT_WEIGHT_400);
}


//
// 'hcFontGetWidth()' - Get the width of a single character.
//

int					// O - Width in 1000ths
hcFontGetWidth(hc_font_t *font,		// I - Font
               int       ch)		// I - Unicode character
{
  int	bin =  ch >> 8;			// Bin in widths array


  // Range check input...
  if (!font || ch < ' ' || ch == 0x7f)
    return (0);

  if (font->widths[bin])
    return ((int)(1000.0f * font->widths[bin][ch & 255].width / font->units));
  else if (font->widths[0])		// .notdef
    return ((int)(1000.0f * font->widths[0][0].width / font->units));
  else
    return (0);
}


//
// 'hcFontGetXHeight()' - Get the height of lowercase letters.
//

int					// O - Lowercase letter height in 1000ths
hcFontGetXHeight(hc_font_t *font)	// I - Font
{
  return (font ? (int)(1000 * font->x_height / font->units) : 0);
}


//
// 'hcFontIsFixedPitch()' - Determine whether a font is fixedpitch.
//

bool					// O - `true` if fixed pitch, `false` otherwise
hcFontIsFixedPitch(hc_font_t *font)	// I - Font
{
  return (font ? font->is_fixed : false);
}


//
// 'hcFontNew()' - Create a new font object for the named font family.
//

hc_font_t *				// O - New font object
hcFontNew(hc_pool_t *pool,		// I - Memory pool
          hc_file_t *file,		// I - File
          size_t    idx)		// I - Font number in collection (0-based)
{
  hc_font_t		*font = NULL;	// New font object
  _hc_off_table_t	table;		// Offset table
  _hc_off_names_t	names;		// Names
  int			i,		// Looping var
			num_cmap,	// Number of Unicode character to glyph mappings
			*cmap = NULL,	// Unicode character to glyph mappings
			num_glyphs;	// Numnber of glyphs
  _hc_font_metric_t	*widths = NULL;	// Glyph metrics
  _hc_off_head_t	head;		// head table
  _hc_off_hhea_t	hhea;		// hhea table
  _hc_off_os_2_t	os_2;		// OS/2 table
  _hc_off_post_t	post;		// PostScript table
  size_t		num_fonts;	// Number of fonts in table


  if (read_table(file, idx, &table, &num_fonts))
    return (NULL);

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

  if (read_post(file, &table, &post))
  {
    font->italic_angle = post.italicAngle;
    font->is_fixed     = post.isFixedPitch != 0;
  }

  _HC_DEBUG("hcFontNew: family=\"%s\"\n", font->family);

  if ((num_cmap = read_cmap(file, &table, &cmap)) <= 0)
  {
    _HC_DEBUG("hcFontNew: Unable to read cmap table.\n");
    hcFontDelete(font);
    font = NULL;

    goto cleanup;
  }

  if (!read_head(file, &table, &head))
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
  {
    font->style = HC_FONT_STYLE_NORMAL;
  }

  if (!read_hhea(file, &table, &hhea))
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

  if (read_os_2(file, &table, &os_2))
  {
    // Copy key values from OS/2 table...
    static const hc_font_stretch_t stretches[] =
    {
      HC_FONT_STRETCH_ULTRA_CONDENSED,	// ultra-condensed
      HC_FONT_STRETCH_EXTRA_CONDENSED,	// extra-condensed
      HC_FONT_STRETCH_CONDENSED,	// condensed
      HC_FONT_STRETCH_SEMI_CONDENSED,	// semi-condensed
      HC_FONT_STRETCH_NORMAL,		// normal
      HC_FONT_STRETCH_SEMI_EXPANDED,	// semi-expanded
      HC_FONT_STRETCH_EXPANDED,		// expanded
      HC_FONT_STRETCH_EXTRA_EXPANDED,	// extra-expanded
      HC_FONT_STRETCH_ULTRA_EXPANDED	// ultra-expanded
    };

    if (os_2.usWidthClass >= 1 && os_2.usWidthClass <= (int)(sizeof(stretches) / sizeof(stretches[0])))
      font->stretch = stretches[os_2.usWidthClass - 1];

    font->weight     = (short)os_2.usWeightClass;
    font->cap_height = os_2.sCapHeight;
    font->x_height   = os_2.sxHeight;
  }
  else
  {
    _HC_DEBUG("hcFontNew: Unable to read OS/2 table.\n");
    font->weight = 400;
  }

  if (font->cap_height == 0)
    font->cap_height = font->ascent;

  if (font->x_height == 0)
    font->x_height = 3 * font->ascent / 5;

  // Build sparse widths table...
  font->min_char = -1;

  for (i = 0; i < num_cmap; i ++)
  {
    if (cmap[i] >= 0)
    {
      int	bin = i / 256,		// Sub-array bin
		glyph = cmap[i];	// Glyph index

      // Update min/max...
      if (font->min_char < 0)
        font->min_char = (int)i;

      font->max_char = (int)i;

      // Allocate a sub-array as needed...
      if (!font->widths[bin])
        font->widths[bin] = (_hc_font_metric_t *)calloc(256, sizeof(_hc_font_metric_t));

      // Copy the width of the specified glyph or the last one if we are past
      // the end of the table...
      if (glyph >= hhea.numberOfHMetrics)
	font->widths[bin][i & 255] = widths[hhea.numberOfHMetrics - 1];
      else
	font->widths[bin][i & 255] = widths[glyph];
    }
  }

  // Cleanup and return the font...
  cleanup:

  free(table.entries);
  free(names.names);
  free(names.storage);
  free(widths);

  return (font);
}


//
// 'copy_name()' - Copy a name string from a font.
//

static const char *			// O - Name string or `NULL`
copy_name(hc_pool_t       *pool,	// I - String pool
          _hc_off_names_t *names,	// I - Names table
          unsigned        name_id)	// I - Name identifier
{
  int			i;		// Looping var
  _hc_off_name_t	*name;		// Current name


  for (i = names->num_names, name = names->names; i > 0; i --, name ++)
  {
    if (name->name_id == name_id &&
        ((name->platform_id == _HC_OFF_Mac && name->language_id == _HC_OFF_Mac_USEnglish) ||
         (name->platform_id == _HC_OFF_Windows && (name->language_id & 0xff) == _HC_OFF_Windows_English)))
    {
      char	temp[1024],	// Temporary string buffer
		*tempptr,	// Pointer into temporary string
		*storptr;	// Pointer into storage
      int	chars,		// Length of string to copy in characters
		bpc;		// Bytes per character

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
        int ch;				// Current character

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


//
// 'read_cmap()' - Read the cmap table, getting the Unicode mapping table.
//

static int				// O - Number of cmap entries or -1 on error
read_cmap(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          int             **cmap)	// O - cmap entries
{
  unsigned	length;			// Length of cmap table
  int		i,			// Looping var
		temp,			// Temporary value
		num_cmap = 0,		// Number of cmap entries
		num_tables,		// Number of cmap tables
		platform_id,		// Platform identifier (Windows or Mac)
		encoding_id,		// Encoding identifier (varies)
		cformat;		// Formap of cmap data
  unsigned	clength,		// Length of cmap data
		coffset = 0,		// Offset to cmap data
		roman_offset = 0;	// MacRoman offset
  int		*cmapptr;		// Pointer into cmap


  // Find the cmap table...
  if (seek_table(file, table, _HC_OFF_cmap, 0) == 0)
    return (-1);

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

  // Find a Unicode table we can use...
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
    return (-1);

  if ((cformat = read_ushort(file)) < 0)
  {
    _hcFileError(file, "Unable to read cmap table format at offset %u.", coffset);
    return (-1);
  }

  _HC_DEBUG("read_cmap: cformat=%d\n", cformat);

  switch (cformat)
  {
    case 0 :
        {
          // Format 0: Byte encoding table.
          //
          // This is a simple 8-bit mapping.
          size_t	j;		// Looping var
          unsigned char bmap[256];	// Byte map buffer

	  if ((unsigned)read_ushort(file) == (unsigned)-1)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

          /* language = */ read_ushort(file);

          if (length > (256 + 6))
          {
	    _hcFileError(file, "Bad cmap table length at offset %u.", coffset);
	    return (-1);
          }

	  num_cmap = length - 6;

	  if ((*cmap = (int *)malloc(num_cmap * sizeof(int))) == NULL)
	  {
	    _hcFileError(file, "Unable to allocate cmap table.");
	    return (-1);
	  }

          if (hcFileRead(file, bmap, num_cmap) != num_cmap)
          {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
          }

	  // Copy into the actual cmap table...
	  for (j = 0; j < num_cmap; j ++)
	    *cmap[j] = bmap[j];
        }
        break;

    case 4 :
        {
          // Format 4: Segment mapping to delta values.
          //
          // This is an overly complicated linear way of encoding a sparse
          // mapping table.  And it uses 1-based indexing with modulo
          // arithmetic...
          int		ch,		// Current character
			seg,		// Current segment
			glyph,		// Current glyph
			segCount,	// Number of segments
			numGlyphIdArray,// Number of glyph IDs
			*glyphIdArray;	// Glyph IDs
          _hc_off_cmap4_t *segments,	// Segment data
			*segment;	// This segment


          // Read the table...
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

          if (!segments || !glyphIdArray)
          {
            _hcFileError(file, "Unable to allocate memory for cmap.");
            free(segments);
            free(glyphIdArray);
            return (-1);
	  }

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

          for (i = 0, segment = segments; i < segCount; i ++, segment ++)
          {
            _HC_DEBUG("read_cmap: segment[%d].startCode=%d, endCode=%d, idDelta=%d, idRangeOffset=%d\n", i, segment->startCode, segment->endCode, segment->idDelta, segment->idRangeOffset);

            if (segment->startCode > segment->endCode)
            {
	      _hcFileError(file, "Bad cmap table segment %u to %u.", segments->startCode, segment->endCode);
	      free(segments);
	      free(glyphIdArray);
	      return (-1);
            }

            // Based on the end code of the segment table, allocate space for the
            // uncompressed cmap table...
            if (segment->endCode >= num_cmap)
	      num_cmap = segment->endCode + 1;
          }

#ifdef DEBUG
          for (i = 0; i < numGlyphIdArray; i ++)
            _HC_DEBUG("read_cmap: glyphIdArray[%d]=%d\n", i, glyphIdArray[i]);
#endif /* DEBUG */

	  if (num_cmap == 0 || num_cmap > _HC_FONT_MAX_CHAR)
	  {
	    _hcFileError(file, "Invalid cmap table with %u characters.", (unsigned)num_cmap);
	    free(segments);
	    free(glyphIdArray);
	    return (-1);
	  }

	  *cmap = cmapptr = (int *)malloc(num_cmap * sizeof(int));

	  if (!*cmap)
          {
            _hcFileError(file, "Unable to allocate memory for cmap.");
            free(segments);
            free(glyphIdArray);
            return (-1);
	  }

          memset(cmapptr, -1, num_cmap * sizeof(int));

          // Now loop through the segments and assign glyph indices from the
          // array...
          for (seg = segCount, segment = segments; seg > 0; seg --, segment ++)
          {
            for (ch = segment->startCode; ch <= segment->endCode; ch ++)
            {
              if (segment->idRangeOffset)
              {
                // Use an "obscure indexing trick" (words from the spec, not
                // mine) to look up the glyph index...
                temp = (int)(segment->idRangeOffset / 2 - segCount + (ch - segment->startCode) + (segment - segments));

                _HC_DEBUG("read_cmap: ch=%d, temp=%d\n", ch, temp);
                if (temp < 0 || temp >= numGlyphIdArray)
                  glyph = -1;
		else
		  glyph = (glyphIdArray[temp] + segment->idDelta) & 65535;
              }
              else
              {
                // Just use idDelta to compute a glyph index...
                glyph = (ch + segment->idDelta) & 65535;
	      }

	      cmapptr[ch] = glyph;
            }
	  }

          // Free the segment data...
	  free(segments);
	  free(glyphIdArray);
        }
        break;

    case 12 :
	{
	  // Format 12: Segmented coverage
	  //
	  // A simple sparse linear segment mapping format.
	  unsigned	ch,		// Current character
			gidx,		// Current group
			nGroups;	// Number of groups
	  _hc_off_cmap12_t *groups,	// Groups
			*group;		// This group

	  // Read the table...
          /* reserved */ read_ushort(file);

	  if (read_ulong(file) == 0)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

	  /* language = */ read_ulong(file);
	  nGroups        = read_ulong(file);

	  _HC_DEBUG("read_cmap: nGroups=%u\n", nGroups);

	  if (nGroups > _HC_FONT_MAX_GROUPS)
	  {
	    _hcFileError(file, "Invalid cmap table with %u groups.", nGroups);
	    return (-1);
	  }

	  if ((groups = (_hc_off_cmap12_t *)calloc(nGroups, sizeof(_hc_off_cmap12_t))) == NULL)
          {
            _hcFileError(file, "Unable to allocate memory for cmap.");
            return (-1);
	  }

	  for (gidx = 0, group = groups, num_cmap = 0; gidx < nGroups; gidx ++, group ++)
	  {
	    group->startCharCode = read_ulong(file);
	    group->endCharCode   = read_ulong(file);
	    group->startGlyphID  = read_ulong(file);
	    _HC_DEBUG("read_cmap: [%u] startCharCode=%u, endCharCode=%u, startGlyphID=%u\n", gidx, group->startCharCode, group->endCharCode, group->startGlyphID);

            if (group->startCharCode > group->endCharCode || group->startCharCode >= _HC_FONT_MAX_CHAR || group->endCharCode >= _HC_FONT_MAX_CHAR)
            {
	      _hcFileError(file, "Bad cmap table segment %u to %u.", group->startCharCode, group->endCharCode);
	      free(groups);
	      return (-1);
            }

            if (group->endCharCode >= num_cmap)
              num_cmap = group->endCharCode + 1;
	  }

	  // Based on the end code of the segent table, allocate space for the
	  // uncompressed cmap table...
          _HC_DEBUG("read_cmap: num_cmap=%u\n", (unsigned)num_cmap);

	  if (num_cmap == 0 || num_cmap > _HC_FONT_MAX_CHAR)
	  {
	    _hcFileError(file, "Invalid cmap table with %u characters.", (unsigned)num_cmap);
	    free(groups);
	    return (-1);
	  }

	  *cmap = cmapptr = (int *)malloc(num_cmap * sizeof(int));

	  if (!*cmap)
          {
            _hcFileError(file, "Unable to allocate memory for cmap.");
            free(groups);
            return (-1);
	  }

	  memset(cmapptr, -1, num_cmap * sizeof(int));

	  // Now loop through the groups and assign glyph indices from the
	  // array...
	  for (gidx = 0, group = groups; gidx < nGroups; gidx ++, group ++)
	  {
            for (ch = group->startCharCode; ch <= group->endCharCode && ch < num_cmap; ch ++)
              cmapptr[ch] = (int)(group->startGlyphID + ch - group->startCharCode);
          }

	  // Free the group data...
	  free(groups);
	}
        break;

    case 13 :
	{
	  // Format 13: Many-to-one range mappings
	  //
	  // Typically used for fonts of last resort where multiple characters
	  // map to the same glyph.
	  unsigned	ch,		// Current character
			gidx,		// Current group
			nGroups;	// Number of groups
	  _hc_off_cmap13_t *groups,	// Groups
			*group;		// This group

	  // Read the table...
          /* reserved */ read_ushort(file);

	  if (read_ulong(file) == 0)
	  {
	    _hcFileError(file, "Unable to read cmap table length at offset %u.", coffset);
	    return (-1);
	  }

	  /* language = */ read_ulong(file);
	  nGroups        = read_ulong(file);

	  _HC_DEBUG("read_cmap: nGroups=%u\n", nGroups);

	  if (nGroups > _HC_FONT_MAX_GROUPS)
	  {
	    _hcFileError(file, "Invalid cmap table with %u groups.", nGroups);
	    return (-1);
	  }

	  if ((groups = (_hc_off_cmap13_t *)calloc(nGroups, sizeof(_hc_off_cmap13_t))) == NULL)
	  {
	    _hcFileError(file, "Unable to allocate memory for cmap.");
	    return (-1);
	  }

	  for (gidx = 0, group = groups, num_cmap = 0; gidx < nGroups; gidx ++, group ++)
	  {
	    group->startCharCode = read_ulong(file);
	    group->endCharCode   = read_ulong(file);
	    group->glyphID       = read_ulong(file);
	    _HC_DEBUG("read_cmap: [%u] startCharCode=%u, endCharCode=%u, glyphID=%u\n", gidx, group->startCharCode, group->endCharCode, group->glyphID);

            if (group->startCharCode > group->endCharCode || group->startCharCode >= _HC_FONT_MAX_CHAR || group->endCharCode >= _HC_FONT_MAX_CHAR)
            {
	      _hcFileError(file, "Bad cmap table segment %u to %u.", group->startCharCode, group->endCharCode);
	      free(groups);
	      return (-1);
            }

            if (group->endCharCode >= num_cmap)
              num_cmap = group->endCharCode + 1;
	  }

	  // Based on the end code of the segent table, allocate space for the
	  // uncompressed cmap table...
          _HC_DEBUG("read_cmap: num_cmap=%u\n", (unsigned)num_cmap);

	  if (num_cmap == 0 || num_cmap > _HC_FONT_MAX_CHAR)
	  {
	    _hcFileError(file, "Invalid cmap table with %u characters.", (unsigned)num_cmap);
	    free(groups);
	    return (-1);
	  }

	  *cmap = cmapptr = (int *)malloc(num_cmap * sizeof(int));

	  if (!*cmap)
	  {
	    _hcFileError(file, "Unable to allocate cmap.");
	    free(groups);
	    return (-1);
	  }

	  memset(cmapptr, -1, num_cmap * sizeof(int));

	  // Now loop through the groups and assign glyph indices from the
	  // array...
	  for (gidx = 0, group = groups; gidx < nGroups; gidx ++, group ++)
	  {
            for (ch = group->startCharCode; ch <= group->endCharCode && ch < num_cmap; ch ++)
              cmapptr[ch] = (int)group->glyphID;
          }

	  // Free the group data...
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
  {
    if (cmapptr[i] >= 0)
      _HC_DEBUG("read_cmap; cmap[%d]=%d\n", i, cmapptr[i]);
  }
#endif // DEBUG

  return (num_cmap);
}


//
// 'read_head()' - Read the head table.
//

static bool				// O - `true` on success, `false` on error
read_head(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          _hc_off_head_t  *head)	// O - head table data
{
  memset(head, 0, sizeof(_hc_off_head_t));

  if (seek_table(file, table, _HC_OFF_head, 0) == 0)
    return (-1);

  /* majorVersion       */read_ushort(file);
  /* minorVersion       */read_ushort(file);
  /* fontRevision       */read_ulong(file);
  /* checkSumAdjustment */read_ulong(file);
  /* magicNumber        */read_ulong(file);
  /* flags              */read_ushort(file);
  head->unitsPerEm      = (unsigned short)read_ushort(file);
  /* created            */read_ulong(file); read_ulong(file);
  /* modified           */read_ulong(file); read_ulong(file);
  head->xMin            = (short)read_short(file);
  head->yMin            = (short)read_short(file);
  head->xMax            = (short)read_short(file);
  head->yMax            = (short)read_short(file);
  head->macStyle        = (unsigned short)read_ushort(file);

  return (0);
}


//
// 'read_hhea()' - Read the hhea table.
//

static bool				// O - `true` on success, `false` on error
read_hhea(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          _hc_off_hhea_t  *hhea)	// O - hhea table data
{
  memset(hhea, 0, sizeof(_hc_off_hhea_t));

  if (seek_table(file, table, _HC_OFF_hhea, 0) == 0)
    return (-1);

  /* majorVersion        */read_ushort(file);
  /* minorVersion        */read_ushort(file);
  hhea->ascender         = (short)read_short(file);
  hhea->descender        = (short)read_short(file);
  /* lineGap             */read_short(file);
  /* advanceWidthMax     */read_ushort(file);
  /* minLeftSideBearing  */read_short(file);
  /* minRightSideBearing */read_short(file);
  /* mMaxExtent          */read_short(file);
  /* caretSlopeRise      */read_short(file);
  /* caretSlopeRun       */read_short(file);
  /* caretOffset         */read_short(file);
  /* (reserved)          */read_short(file);
  /* (reserved)          */read_short(file);
  /* (reserved)          */read_short(file);
  /* (reserved)          */read_short(file);
  /* metricDataFormat    */read_short(file);
  hhea->numberOfHMetrics = (unsigned short)read_ushort(file);

  return (0);
}


//
// 'read_hmtx()' - Read the horizontal metrics from the font.
//

static _hc_font_metric_t *		// O - Array of glyph metrics
read_hmtx(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          _hc_off_hhea_t  *hhea)	// O - hhea table data
{
  unsigned		length;		// Length of hmtx table
  int			i;		// Looping var
  _hc_font_metric_t	*widths;	// Glyph metrics array


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


//
// 'read_maxp()' - Read the number of glyphs in the font.
//

static int				// O - Number of glyphs or -1 on error
read_maxp(hc_file_t       *file,	// I - File
          _hc_off_table_t *table)	// I - Offset table
{
 /*
  * All we care about is the number of glyphs, so get grab that...
  */

  if (seek_table(file, table, _HC_OFF_maxp, 4) == 0)
    return (-1);
  else
    return (read_ushort(file));
}


//
// 'read_names()' - Read the name strings from a font.
//

static bool				// O - `true` on success, `false` on error
read_names(hc_file_t       *file,	// I - File
           _hc_off_table_t *table,	// I - Offset table
           _hc_off_names_t *names)	// O - Names
{
  unsigned	length;			// Length of names table
  int		i,			// Looping var
		format,			// Name table format
		offset;			// Offset to storage
  _hc_off_name_t *name;			// Current name


  memset(names, 0, sizeof(_hc_off_names_t));

  // Find the name table...
  if ((length = seek_table(file, table, _HC_OFF_name, 0)) == 0)
    return (false);

  if ((format = read_ushort(file)) < 0 || format > 1)
    return (false);

  _HC_DEBUG("read_names: format=%d\n", format);

  if ((names->num_names = read_ushort(file)) < 1)
    return (false);

  if ((names->names = (_hc_off_name_t *)calloc((size_t)names->num_names, sizeof(_hc_off_name_t))) == NULL)
    return (false);

  if ((offset = read_ushort(file)) < 0 || (unsigned)offset >= length)
    return (false);

  names->storage_size = length - (unsigned)offset;
  if ((names->storage = malloc(names->storage_size)) == NULL)
    return (false);
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
    // Skip language_id table...
    int count = read_ushort(file);	// Number of language IDs

    _HC_DEBUG("read_names: Skipping language_id table...\n");

    while (count > 0)
    {
      read_ushort(file); // length
      read_ushort(file); // offset
      count --;
    }
  }

  hcFileRead(file, names->storage, length - (unsigned)offset);

  return (true);
}


//
// 'read_os_2()' - Read the OS/2 table.
//

static bool				// O - `true` on success, `false` on error
read_os_2(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          _hc_off_os_2_t  *os_2)	// O - OS/2 table
{
  int		version;		// OS/2 table version
  unsigned char	panose[10];		// panose value


  memset(os_2, 0, sizeof(_hc_off_os_2_t));

 /*
  * Find the OS/2 table...
  */

  if (seek_table(file, table, _HC_OFF_OS_2, 0) == 0)
    return (false);

  if ((version = read_ushort(file)) < 0)
    return (false);

  _HC_DEBUG("read_names: version=%d\n", version);

  /* xAvgCharWidth */       read_short(file);
  os_2->usWeightClass     = (unsigned short)read_ushort(file);
  os_2->usWidthClass      = (unsigned short)read_ushort(file);
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
  /* panose[10] */
  if (hcFileRead(file, panose, sizeof(panose)) != sizeof(panose))
    return (false);
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
    os_2->sxHeight        = (short)read_short(file);
    os_2->sCapHeight      = (short)read_short(file);
  }

  return (true);
}


//
// 'read_post()' - Read the italicAngle value from the post table.
//

static bool				// O - `true` on success, `false` on error
read_post(hc_file_t       *file,	// I - File
          _hc_off_table_t *table,	// I - Offset table
          _hc_off_post_t  *post)	// I - PostScript table
{
  memset(post, 0, sizeof(_hc_off_post_t));

  if (seek_table(file, table, _HC_OFF_post, 0) == 0)
    return (false);

  /* version            = */read_ulong(file);
  post->italicAngle     = (int)read_ulong(file) / 65536.0f;
  /* underlinePosition  = */read_ushort(file);
  /* underlineThickness = */read_ushort(file);
  post->isFixedPitch    = read_ulong(file);

  return (true);
}


//
// 'read_short()' - Read a 16-bit signed integer.
//

static int				// O - 16-bit signed integer value or EOF
read_short(hc_file_t *file)		// i - File to read from
{
  unsigned char	buffer[2];		// Read buffer


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return (EOF);
  else if (buffer[0] & 0x80)
    return (((buffer[0] << 8) | buffer[1]) - 65536);
  else
    return ((buffer[0] << 8) | buffer[1]);
}


//
// 'read_table()' - Read an OFF/TTF offset table.
//

static bool				// O - `true` on success, `false` on error
read_table(hc_file_t       *file,	// I - File
           size_t          idx,		// I - Font number within collection
           _hc_off_table_t *table,	// O - Offset table
           size_t          *num_fonts)	// O - Number of fonts
{
  int		i;			// Looping var
  unsigned	temp;			// Temporary value
  _hc_off_dir_t	*current;		// Current table entry


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

  // sfnt version
  if ((temp = read_ulong(file)) != 0x10000 && temp != 0x4f54544f && temp != 0x74746366)
  {
    _hcFileError(file, "Invalid font file.");
    return (false);
  }

  if (temp == 0x74746366)
  {
   /*
    * Font collection, get the number of fonts and then seek to the start of
    * the offset table for the desired font...
    */

    _HC_DEBUG("read_table: Font collection\n");

    // Version
    if ((temp = read_ulong(file)) != 0x10000 && temp != 0x20000)
    {
      _hcFileError(file, "Unsupported font collection version %f.", temp / 65536.0);
      return (false);
    }

    _HC_DEBUG("read_table: Collection version=%f\n", temp / 65536.0);

    // numFonts
    if ((temp = read_ulong(file)) == 0)
    {
      _hcFileError(file, "No fonts in collection.");
      return (false);
    }

    *num_fonts = (size_t)temp;

    _HC_DEBUG("read_table: numFonts=%u\n", temp);

    if (idx >= *num_fonts)
      return (false);

    // OffsetTable
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

  // numTables
  if ((table->num_entries = read_ushort(file)) <= 0)
  {
    _hcFileError(file, "Unable to read font tables.");
    return (false);
  }

  _HC_DEBUG("read_table: num_entries=%u\n", (unsigned)table->num_entries);

  // searchRange
  if (read_ushort(file) < 0)
  {
    _hcFileError(file, "Unable to read font tables.");
    return (false);
  }

  // entrySelector
  if (read_ushort(file) < 0)
  {
    _hcFileError(file, "Unable to read font tables.");
    return (false);
  }

  // rangeShift
  if (read_ushort(file) < 0)
  {
    _hcFileError(file, "Unable to read font tables.");
    return (false);
  }

 /*
  * Read the table entries...
  */

  if ((table->entries = calloc((size_t)table->num_entries, sizeof(_hc_off_dir_t))) == NULL)
  {
    _hcFileError(file, "Unable to allocate memory for font tables.");
    return (false);
  }

  for (i = table->num_entries, current = table->entries; i > 0; i --, current ++)
  {
    current->tag      = read_ulong(file);
    current->checksum = read_ulong(file);
    current->offset   = read_ulong(file);
    current->length   = read_ulong(file);

    _HC_DEBUG("read_table: [%d] tag='%c%c%c%c' checksum=%u offset=%u length=%u\n", i, (current->tag >> 24) & 255, (current->tag >> 16) & 255, (current->tag >> 8) & 255, current->tag & 255, current->checksum, current->offset, current->length);
  }

  return (true);
}


//
// 'read_ulong()' - Read a 32-bit unsigned integer.
//

static unsigned				// O - 32-bit unsigned integer value or EOF
read_ulong(hc_file_t *file)		// I - File to read from
{
  unsigned char	buffer[4];		// Read buffer


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return ((unsigned)EOF);
  else
    return ((unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]));
}


//
// 'read_ushort()' - Read a 16-bit unsigned integer.
//

static int				// O - 16-bit unsigned integer value or EOF
read_ushort(hc_file_t *file)		// i - File to read from
{
  unsigned char	buffer[2];		// Read buffer


  if (hcFileRead(file, buffer, sizeof(buffer)) != sizeof(buffer))
    return (EOF);
  else
    return ((buffer[0] << 8) | buffer[1]);
}


//
// 'seek_table()' - Seek to a specific table in a font.
//

static unsigned				// O - Length of table or 0 if not found
seek_table(hc_file_t       *file,	// I - File
           _hc_off_table_t *table,	// I - Font table
           unsigned        tag,		// I - Tag to find
           unsigned        offset)	// I - Additional offset
{
  int		i;			// Looping var
  _hc_off_dir_t	*current;		// Current entry


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
