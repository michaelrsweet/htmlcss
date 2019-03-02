//#define DEBUG 1
/*
 * Core font object functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/hc
 *
 * Copyright © 2018-2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#include "common-private.h"
#include "font-private.h"


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

#define _HC_OFF_Mac		1	/* Macintosh platform ID */
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


/*
 * Local functions...
 */

static const char *copy_name(hc_pool_t *pool, _hc_off_names_t *names, unsigned name_id);
static int	read_names(hc_file_t *file, _hc_off_table_t *table, _hc_off_names_t *names);
static int	read_table(hc_file_t *file, _hc_off_table_t *table);
static unsigned	read_ulong(hc_file_t *file);
static int	read_ushort(hc_file_t *file);
static unsigned	seek_table(hc_file_t *file, _hc_off_table_t *table, unsigned tag);


/*
 * 'hcFontDelete()' - Free all memory used for a font family object.
 */

void
hcFontDelete(hc_font_t *font)		/* I - Font object */
{
  (void)font;
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
 * 'hcFontGetPostScriptName()' - Get the PostScript name of a font.
 */

const char *				/* O - PostScript name */
hcFontGetPostScriptName(
    hc_font_t *font)			/* I - Font object */
{
  return (font ? font->postscript_name : NULL);
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
 * 'hcFontNew()' - Create a new font object for the named font family.
 */

hc_font_t *				/* O - New font object */
hcFontNew(hc_pool_t *pool,		/* I - Memory pool */
          hc_file_t *file)		/* I - File */
{
  hc_font_t		*font = NULL;	/* New font object */
  _hc_off_table_t	table;		/* Offset table */
  _hc_off_names_t	names;		/* Names */
  unsigned		length;		/* Length of table */


  if (read_table(file, &table))
  {
    fputs("hcFontNew: Unable to read font table.\n", stderr);
    return (NULL);
  }

  _HC_DEBUG("hcFontNew: num_entries=%d\n", table.num_entries);

  if (read_names(file, &table, &names))
  {
    free(table.entries);

    if (names.names)
      free(names.names);
    if (names.storage)
      free(names.storage);

    return (NULL);
  }

  _HC_DEBUG("hcFontNew: num_names=%d\n", names.num_names);

  if ((font = (hc_font_t *)calloc(1, sizeof(hc_font_t))) == NULL)
  {
    free(table.entries);

    if (names.names)
      free(names.names);
    if (names.storage)
      free(names.storage);

    return (NULL);
  }

  font->pool            = pool;
  font->copyright       = copy_name(pool, &names, _HC_OFF_Copyright);
  font->family          = copy_name(pool, &names, _HC_OFF_FontFamily);
  font->postscript_name = copy_name(pool, &names, _HC_OFF_PostScriptName);
  font->version         = copy_name(pool, &names, _HC_OFF_FontVersion);

  _HC_DEBUG("hcFontNew: family=\"%s\"\n", font->family);

  if ((length = seek_table(file, &table, _HC_OFF_cmap)) > 0)
    _HC_DEBUG("hcFontNew: cmap table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED cmap table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_head)) > 0)
    _HC_DEBUG("hcFontNew: head table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED head table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_hhea)) > 0)
    _HC_DEBUG("hcFontNew: hhea table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED hhea table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_hmtx)) > 0)
    _HC_DEBUG("hcFontNew: hmtx table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED hmtx table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_maxp)) > 0)
    _HC_DEBUG("hcFontNew: maxp table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED maxp table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_name)) > 0)
    _HC_DEBUG("hcFontNew: name table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED name table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_OS_2)) > 0)
    _HC_DEBUG("hcFontNew: OS/2 table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED OS/2 table is missing.\n");

  if ((length = seek_table(file, &table, _HC_OFF_post)) > 0)
    _HC_DEBUG("hcFontNew: post table is %u bytes long.\n", length);
  else
    _HC_DEBUG("hcFontNew: REQUIRED post table is missing.\n");

  free(table.entries);

  if (names.names)
    free(names.names);
  if (names.storage)
    free(names.storage);

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
        storptr = (char *)names->storage + name->offset + 1;
        chars   = name->length / 2;
        bpc     = 2;
      }
      else if (name->platform_id == _HC_OFF_Windows && name->encoding_id == _HC_OFF_Windows_UCS4)
      {
        storptr = (char *)names->storage + name->offset + 3;
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
        if (tempptr < (temp + sizeof(temp) - 1))
          *tempptr++ = *storptr;
	else
	  break;

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

  if ((length = seek_table(file, table, _HC_OFF_name)) == 0)
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
 * 'read_table()' - Read an OFF/TTF offset table.
 */

static int				/* O - 0 on success, EOF on failure */
read_table(hc_file_t       *file,	/* I - File */
           _hc_off_table_t *table)	/* O - Offset table */
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

  /* sfnt version */
  if ((temp = read_ulong(file)) != 0x10000 && temp != 0x4f54544f)
    return (-1);

  /* numTables */
  if ((table->num_entries = read_ushort(file)) <= 0)
    return (-1);

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
           unsigned        tag)		/* I - Tag to find */
{
  int		i;			/* Looping var */
  _hc_off_dir_t	*current;		/* Current entry */


  for (i = table->num_entries, current = table->entries; i  > 0; i --, current ++)
  {
    if (current->tag == tag)
    {
      if (hcFileSeek(file, current->offset) == current->offset)
        return (current->length);
      else
        return (0);
    }
  }

  return (0);
}
