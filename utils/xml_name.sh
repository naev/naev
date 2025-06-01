#!/usr/bin/bash

sed -e "s/ /_/g" -e "s/["\'":.()?]//g" |
sed -e 's/&amp;/_and_/g' -e 's/^s_and_k_/sk_/' |
sed -e "s/-\([0-9]\)/\1/g" |
sed 's/_\(i*[vi]i*\)-\([a-z]\)$/_\1\2/g' |
sed -e "s/\(._\)_*/\1/g" -e "s/_-_/_/" |
# greeks letters
sed -e 's/Ω/omega/' -e 's/Ψ/psi/' |
# sometimes suffixes/words can be concatenated
sed -e "s/-in$/in/" -e "s/-out$/out/" -e "s/-up$/up/" -e "s/_lust$/lust/" |
sed -e "s/trade_lane/tradelane/" -e "s/photo-voltaic/photovoltaic/g" |
sed -e "s/^red_star_/rs_/" -e "s/super-fast/superfast/"
