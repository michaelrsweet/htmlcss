HTMLCSS - A lightweight HTML and CSS Parser in C
================================================

Initial Goals:

- Implement using only C99 features
- HTML 5 compliant HTML markup parser
- CSS 3 compliant CSS parser
- OFC/OFF/TTC/TTF compliant font file parser
- Efficient memory usage
- Calculation of CSS properties for a given node in a HTML document
- Extraction of HTML "runs"
  - CSS properties + text strings (array of strings with leading space when
    there is leading space, strings separate at break points)
  - Include before/after content from CSS

Reach Goals:

- XHTML support (pretty straight-forward)
- WOFF2 compliant font file parser (but that needs a Brotli decompressor
  implementation, and the available options are not pretty)
- SVG/PNG/JPG/GIF image file parsers (probably using external libraries)

Ultimately the end result should be usable for things like HTMLDOC, embedding
support for printing of HTML, etc.

