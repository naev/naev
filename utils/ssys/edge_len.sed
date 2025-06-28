#!/usr/bin/sed -f
/\<carnis_minor\>/ s/$/ X/
/\<carnis_major\>.*X/ s/ X$/ 0.5/
/\<gliese\>/ s/$/ X/
/\<gliese_minor\>.*X/ s/ X$/ 0.5/
/\<kruger\>/ s/$/ X/
/\<krugers_pocket\>.*X/ s/ X$/ 0.5/
s/ X$//
