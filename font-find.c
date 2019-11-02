/*
 * Font search/cache functions for HTMLCSS library.
 *
 *     https://github.com/michaelrsweet/htmlcss
 *
 * Copyright © 2019 by Michael R Sweet.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/*
 * Include necessary headers...
 */

#include "font-private.h"
#include "pool-private.h"

#include <dirent.h>
#include <sys/stat.h>


/*
 * Structure...
 */

struct _hc_font_info_t			/* Font cache information */
{
  const char		*font_url;	/* Font filename/URL */
  size_t		font_index;	/* Font number in collection */
  hc_font_t		*font;		/* Loaded font */
  const char		*font_family;
  hc_font_stretch_t	font_stretch;
  hc_font_style_t	font_style;
  hc_font_variant_t	font_variant;
  hc_font_weight_t	font_weight;
};


/*
 * Local functions...
 */

static void	hc_add_font(hc_pool_t *pool, hc_font_t *font, const char *url, int delete_it);
static int	hc_compare_info(_hc_font_info_t *a, _hc_font_info_t *b);
static void	hc_get_cname(char *cname, size_t cnamesize);
static void	hc_load_all_fonts(hc_pool_t *pool);
static void	hc_load_fonts(hc_pool_t *pool, const char *d);
static void	hc_save_all_fonts(hc_pool_t *pool, const char *cname);
static void	hc_sort_fonts(hc_pool_t *pool);


/*
 * 'hcFontAddCached()' - Add a font to a memory pool cache.
 */

void
hcFontAddCached(hc_pool_t  *pool,	/* I - Memory pool for cache */
                hc_font_t  *font,	/* I - Font to add */
                const char *url)	/* I - URL for font */
{
  if (!pool || !font)
    return;

  hc_add_font(pool, font, url, 0);
  hc_sort_fonts(pool);
}


/*
 * 'hcFontFind()' - Find a font...
 */

hc_font_t *				/* O - Matching font or `NULL` */
hcFontFindCached(
    hc_pool_t         *pool,		/* I - Memory pool for cache */
    const char        *family,		/* I - Family name */
    hc_font_stretch_t stretch,		/* I - Stretching of font */
    hc_font_style_t   style,		/* I - Style of font */
    hc_font_variant_t variant,		/* I - Variant of font */
    hc_font_weight_t  weight)		/* I - Weight of font */
{
  size_t		i;		/* Looping var */
  _hc_font_info_t	*info,		/* Current font info */
			*best_info;	/* Best match */
  int			result,		/* Result of compare */
			score,		/* Current score */
			best_score;	/* Best score */


  (void)stretch;
  (void)variant;

  if (!pool || !family)
    return (NULL);

 /*
  * Map generic font famlilies to real fonts...
  *
  * TODO: Provide config options for generic font families...
  */

  if (!strcasecmp(family, "cursive"))
    family = "Zapfino";
  else if (!strcasecmp(family, "fantasy"))
    family = "Comic Sans MS";
  else if (!strcasecmp(family, "monospace"))
#if __APPLE__
    family = "Menlo";
#else
    family = "Courier New";
#endif /* __APPLE_ */
  else if (!strcasecmp(family, "sans-serif"))
#if _WIN32
    family = "Arial";
#else
    family = "Helvetica";
#endif /* _WIN32 */
  else if (!strcasecmp(family, "serif"))
    family = "Times New Roman";

  if (weight == HC_FONT_WEIGHT_NORMAL)
    weight = HC_FONT_WEIGHT_400;
  else if (weight == HC_FONT_WEIGHT_BOLD)
    weight = HC_FONT_WEIGHT_700;
  else if (weight == HC_FONT_WEIGHT_BOLDER)
    weight = HC_FONT_WEIGHT_900;
  else if (weight == HC_FONT_WEIGHT_LIGHTER)
    weight = HC_FONT_WEIGHT_100;

  if (!pool->fonts_loaded)
    hc_load_all_fonts(pool);

  for (i = pool->font_index[tolower(*family & 255)], info = pool->fonts + i, best_info = NULL, best_score = 999999; i < pool->num_fonts; i ++, info ++)
  {
    if ((result = strcasecmp(family, info->font_family)) != 0)
      continue;
    else if (result > 0)
      break;

    if (info->font_weight > weight)
      score = (int)(info->font_weight - weight);
    else
      score = (int)(weight - info->font_weight);

    if ((info->font_style != HC_FONT_STYLE_NORMAL) != (style != HC_FONT_STYLE_NORMAL))
      score ++;

    if (score < best_score)
    {
      best_score = score;
      best_info  = info;

      if (score == 0)
        break;
    }
  }

  if (best_info)
  {
    if (!best_info->font)
    {
      hc_file_t *file = hcFileNewURL(pool, best_info->font_url, NULL);

      best_info->font = hcFontNew(pool, file, best_info->font_index);

      hcFileDelete(file);
    }

    return (best_info->font);
  }
  else
    return (NULL);
}


/*
 * 'hcFontGetCached()' - Get a cached font from a pool.
 */

hc_font_t *				/* O - Font */
hcFontGetCached(hc_pool_t *pool,	/* I - Memory pool */
                size_t    idx)		/* I - Font number (0-based) */
{
  _hc_font_info_t	*info;		/* Font information */


  if (!pool)
    return (NULL);

  if (!pool->fonts_loaded)
    hc_load_all_fonts(pool);

  if (idx >= pool->num_fonts)
    return (NULL);

  info = pool->fonts + idx;

  if (!info->font)
  {
    hc_file_t	*file = hcFileNewURL(pool, info->font_url, NULL);
					/* Font file */

    info->font = hcFontNew(pool, file, info->font_index);

    hcFileDelete(file);
  }

  return (info->font);
}


/*
 * 'hcFontGetCachedCount()' - Return the number of cached fonts.
 */

size_t					/* O - Number of cached fonts */
hcFontGetCachedCount(hc_pool_t *pool)	/* I - Memory pool */
{
  if (!pool)
    return (0);

  if (!pool->fonts_loaded)
    hc_load_all_fonts(pool);

  return (pool->num_fonts);
}


/*
 * '_hcPoolDeleteFonts()' - Free any cached fonts.
 */

void
_hcPoolDeleteFonts(hc_pool_t *pool)	/* I - Memory pool */
{
  if (pool->fonts)
  {
    size_t		i;		/* Looping var */
    _hc_font_info_t	*font;		/* Current font */

    for (i = pool->num_fonts, font = pool->fonts; i > 0; i --, font ++)
      hcFontDelete(font->font);
  }

  free(pool->fonts);

  pool->fonts       = NULL;
  pool->alloc_fonts = 0;
  pool->num_fonts   = 0;
}


/*
 * 'hc_add_font()' - Add a font to the cache, optionally deleting it.
 */

static void
hc_add_font(hc_pool_t  *pool,		/* I - Memory pool */
            hc_font_t  *font,		/* I - Font */
            const char *url,		/* I - Filename/URL */
            int        delete_it)	/* I - Delete font after adding? */
{
  _hc_font_info_t	*info;		/* Font information */


  if (!font->family)
    return;

  if (pool->num_fonts >= pool->alloc_fonts)
  {
    if ((info = realloc(pool->fonts, (pool->alloc_fonts + 16) * sizeof(_hc_font_info_t))) == NULL)
      return;

    pool->fonts       = info;
    pool->alloc_fonts += 16;
  }

  info = pool->fonts + pool->num_fonts;
  pool->num_fonts ++;

  memset(info, 0, sizeof(_hc_font_info_t));

  info->font_url      = hcPoolGetString(pool, url);
  info->font_index    = font->idx;
  info->font_family   = font->family;
//      info->font_stretch  = font->font_stretch;
  info->font_style    = font->style;
//      info->font_variant  = font->font_variant;
  info->font_weight   = (hc_font_weight_t)font->weight;

  _HC_DEBUG("hc_add_font: %s \"%s\" S%d W%d\n", url, info->font_family, info->font_style, info->font_weight);

  if (delete_it)
    hcFontDelete(font);
  else
    info->font = font;
}


/*
 * 'hc_compare_info()' - Compare the information for two fonts.
 */

static int				/* O - Result of comparison */
hc_compare_info(_hc_font_info_t *a,	/* I - First font */
                _hc_font_info_t *b)	/* I - Second font */
{
  int	ret = strcmp(a->font_family, b->font_family);

  if (!ret)
    ret = (int)a->font_style - (int)b->font_style;
  if (!ret)
    ret = (int)a->font_weight - (int)b->font_weight;

  return (ret);
}


/*
 * 'hc_get_cname()' - Get the name of the font cache file.
 */

static void
hc_get_cname(char   *cname,		/* I - Cache filename */
             size_t cnamesize)		/* I - Size of buffer */
{
  const char	*home = getenv("HOME");	/* Home directory */


#ifdef __APPLE__
  if (home)
  {
    snprintf(cname, cnamesize, "%s/Library/Preferences/org.msweet.htmlcss.txt", home);
  }
  else
  {
    strncpy(cname, "/private/tmp/org.msweet.htmlcss.txt", cnamesize - 1);
    cname[cnamesize - 1] = '\0';
  }

#elif _WIN32
  if (home)
  {
    snprintf(cname, cnamesize, "%s/.htmlcss.txt", home);
  }
  else
  {
    strncpy(cname, "C:/WINDOWS/TEMP/.htmlcss.txt", cnamesize - 1);
    cname[cnamesize - 1] = '\0';
  }

#else
  if (home)
  {
    snprintf(cname, cnamesize, "%s/.htmlcss.txt", home);
  }
  else
  {
    strncpy(cname, "/tmp/.htmlcss.txt", cnamesize - 1);
    cname[cnamesize - 1] = '\0';
  }
#endif /* __APPLE__ */
}


/*
 * 'hc_load_all_fonts()' - Load all fonts available to the user.
 */

static void
hc_load_all_fonts(hc_pool_t *pool)	/* I - Memory pool */
{
  int		i,			/* Looping var */
		num_dirs = 0;		/* Number of directories */
  const char	*dirs[5];		/* Directories */
  char		dname[1024];		/* Directory filename */
  struct stat	dinfo;			/* Directory information */
  const char	*home = getenv("HOME");	/* Home directory */
  char		cname[1024];		/* Cache filename */
  struct stat	cinfo;			/* Cache file information */
  int		rescan = 0;		/* Rescan fonts? */


 /*
  * Build a list of font directories...
  */

#ifdef __APPLE__
  dirs[num_dirs ++] = "/System/Library/Fonts";
  dirs[num_dirs ++] = "/Library/Fonts";

  if (home)
  {
    snprintf(dname, sizeof(dname), "%s/Library/Fonts", home);
    dirs[num_dirs ++] = dname;
  }

#elif _WIN32
  dirs[num_dirs ++] = "C:/Windows/Fonts"; /* TODO: fix me */

#else
  dirs[num_dirs ++] = "/usr/X11R6/lib/X11/fonts";
  dirs[num_dirs ++] = "/usr/share/fonts";
  dirs[num_dirs ++] = "/usr/local/share/fonts";

  if (home)
  {
    snprintf(dname, sizeof(dname), "%s/.fonts", home);
    dirs[num_dirs ++] = dname;
  }
#endif /* __APPLE__ */

 /*
  * See if we need to re-scan the font directories...
  */

  hc_get_cname(cname, sizeof(cname));
  if (stat(cname, &cinfo))
  {
    rescan = 1;
  }
  else
  {
    for (i = 0; i < num_dirs; i ++)
    {
      if (!stat(dirs[i], &dinfo) && dinfo.st_mtime > cinfo.st_mtime)
      {
        rescan = 1;
        break;
      }
    }
  }

 /*
  * Load the list of system fonts...
  */

  if (rescan)
  {
   /*
    * Scan for fonts...
    */

    for (i = 0; i < num_dirs; i ++)
      hc_load_fonts(pool, dirs[i]);

   /*
    * Save the cache...
    */

    hc_save_all_fonts(pool, cname);
  }
  else
  {
   /*
    * Load the cache...
    */

    /* TODO: Implement cache load */
  }

  hc_sort_fonts(pool);

  pool->fonts_loaded = 1;
}


/*
 * 'hc_load_fonts()' - Load fonts in a directory...
 */

static void
hc_load_fonts(hc_pool_t  *pool,		/* I - Memory pool */
              const char *d)		/* I - Directory to load */
{
  DIR		*dir;			/* Directory pointer */
  struct dirent	*dent;			/* Directory entry */
  char		filename[1024],		/* Filename */
		*ext;			/* Extension */
  hc_file_t	*file;			/* File */
  hc_font_t	*font;			/* Font */


  if ((dir = opendir(d)) == NULL)
    return;

  while ((dent = readdir(dir)) != NULL)
  {
    if (dent->d_name[0] == '.')
      continue;

    if ((ext = strrchr(dent->d_name, '.')) == NULL)
      continue;

    if (strcmp(ext, ".otc") && strcmp(ext, ".otf") && strcmp(ext, ".ttc") && strcmp(ext, ".ttf"))
      continue;

    snprintf(filename, sizeof(filename), "%s/%s", d, dent->d_name);
    file = hcFileNewURL(pool, filename, NULL);

    if ((font = hcFontNew(pool, file, 0)) != NULL)
    {
      if (!font->family || font->family[0] == '.')
      {
       /*
        * Ignore fonts starting with a "." since (on macOS at least) these are
        * hidden system fonts...
        */

        hcFontDelete(font);
      }
      else
      {
        size_t	num_fonts = hcFontGetNumFonts(font);
					/* Number of fonts */

        hc_add_font(pool, font, filename, 1);

	if (num_fonts > 1)
	{
	  size_t	i;		/* Looping var */

	  for (i = 1; i < num_fonts; i ++)
	  {
	    hcFileSeek(file, 0);

	    if ((font = hcFontNew(pool, file, i)) != NULL)
	    {
	      if (font->family && font->family[0] != '.')
		hc_add_font(pool, font, filename, 1);
	      else
	        hcFontDelete(font);
	    }
	  }
	}
      }
    }

    hcFileDelete(file);
  }

  closedir(dir);
}


/*
 * 'hc_save_all_fonts()' - Save all fonts to the cache...
 */

static void
hc_save_all_fonts(hc_pool_t  *pool,	/* I - Memory pool */
		  const char *cname)	/* I - Cache filename */
{
  /* TODO: Implement cache save */
  (void)pool;
  (void)cname;
}


/*
 * 'hc_sort_fonts()' - Sort and index the fonts in the cache.
 */

static void
hc_sort_fonts(hc_pool_t *pool)		/* I - Memory pool */
{
  size_t		i;		/* Looping var */
  _hc_font_info_t	*info;		/* Current font info */


 /*
  * First sort the fonts...
  */

  if (pool->num_fonts > 1)
    qsort(pool->fonts, pool->num_fonts, sizeof(_hc_font_info_t), (_hc_compare_func_t)hc_compare_info);

 /*
  * Then index the fonts based on the initial character...
  */

  for (i = 0; i < 256; i ++)
    pool->font_index[i] = pool->num_fonts;

  for (i = 0, info = pool->fonts; i < pool->num_fonts; i ++, info ++)
  {
    int ch = tolower(info->font_family[0] & 255);
					/* Initial character of font, lowercase */

    if (i < pool->font_index[ch])
      pool->font_index[ch] = i;
  }
}
