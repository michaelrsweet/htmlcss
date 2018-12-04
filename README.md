HTMLCSS - A lightweight HTML and CSS Parser in C
================================================

Goals:

- Implement using only C99 features.
- HTML 5 compliant HTML markup parser.
- CSS 3 compliant CSS parser.
- OTF/TTF compliant font file parser.
- Efficient memory usage.
- Calculation of CSS properties for a given node in a HTML document.
- Extraction of HTML "runs"
  - CSS properties + text strings (array of strings with leading space when
    there is leading space, strings separate at break points)
  - Include before/after content from CSS
