#!/usr/bin/sed -f
s/ /_/g;
s/["':.()?]//g;
s/&amp;/_and_/g;
s/^s_and_k_/sk_/;
s/-\([0-9]\)/\1/g;
s/_\(i*[vi]i*\)-\([a-z]\)$/_\1\2/g;
s/\(._\)_*/\1/g;
s/_-_/_/g;
# greeks letters
s/Ω/omega/;
s/Ψ/psi/;
# sometimes suffixes/words can be concatenated
s/-in$/in/;
s/-out$/out/;
s/-up$/up/;
s/blood_lust$/bloodlust/;
s/trade_lane/tradelane/;
s/photo-voltaic/photovoltaic/;
s/^red_star_/rs_/;
s/super-fast/superfast/;
