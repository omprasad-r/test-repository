The thumbgen module creates thumbnails of web pages using a modified version of
pythumbnail, available at http://www.gruppo4.com/~tobia/pythumbnail.shtml

The modifications were introduced to help create thumbnails that have no
distortion by keeping the aspect ratio consistent between the browser window
from which the screenshot is taken and the final thumbnail size.  Also I changed
the image format from jpeg to png.  Perhaps it would make sense to offer this
as an exposed option, but for our immediate purposes png makes more sense.

Installation of this module requires that the xvfb (virtual x framebuffer) and
PyGTK available at http://www.pygtk.org/ be installed.  For Ubuntu this requires
the installations of packages called 'xvfb' and 'python-gnome2-extras'.
