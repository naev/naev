#!/usr/bin/sed -f
/\<carnis_minor\>/ s/$/ X/
/\<carnis_major\>.*X/ s/ X$/ 0.2/
s/ X$//
/\<gliese\>/ s/$/ X/
/\<gliese_minor\>.*X/ s/ X$/ 0.5/
s/ X$//
/\<kruger\>/ s/$/ X/
/\<krugers_pocket\>.*X/ s/ X$/ 0.1/
s/ X$//
