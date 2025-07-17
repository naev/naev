#!/usr/bin/sed -f
/\<carnis_minor\>/ s/$/ X/
/\<carnis_major\>.*X/ s/ X$/ 0.3/
s/ X$//
/\<gliese\>/ s/$/ X/
/\<gliese_minor\>.*X/ s/ X$/ 0.4/
s/ X$//
/\<kruger\>/ s/$/ X/
/\<krugers_pocket\>.*X/ s/ X$/ 0.3/
s/ X$//
/\<bastion\>/ s/$/ X/
/\<taiomi\>.*X/ s/ X$/ 0.4/
/\<elixir\>.*X/ s/ X$/ 0.4/
/\<halo\>.*X/ s/ X$/ 0.8/
/\<fury\>.*X/ s/ X$/ 0.8/
/\<gamel\>.*X/ s/ X$/ 0.8/
s/ X$//
/\<titus\>/ s/$/ X/
/\<elixir\>.*X/ s/ X$/ 0.8/
s/ X$//
s/^\(.*\)\<\(hidden\)\> \(..*\)/\1\3 \2/
