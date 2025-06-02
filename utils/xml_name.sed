#!/usr/bin/sed -f
# main part: matches func uniedit_nameFilter@dev_uniedit.c
s/ /_/g;
s/["':.()?]//g;
s/&amp;/_and_/g;
s/-\([0-9]\)/\1/g;
s/_\(i*[vi]i*\)-\([a-z]\)$/_\1\2/;
s/_-/-/g;
s/-_/-/g;
s/\(._\)_*/\1/g;
# Bonuses (only in this file)
# let us shorten these
s/^red_star_/rs_/;
s/^s_and_k_/sk_/;
# sometimes suffixes/words can be concatenated
s/-in$/in/;
s/-out$/out/;
s/-up$/up/;
s/blood_lust/bloodlust/;
s/trade_lane/tradelane/;
s/photo-voltaic/photovoltaic/;
s/super-fast/superfast/;
# greeks letters
s/Ω/omega/;
s/Ψ/psi/;
