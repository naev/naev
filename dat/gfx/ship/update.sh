#!/usr/bin/env bash

#
#   Variables
#
ARTWORK_PATH="../../../../naev-artwork/"


#
#   Copy them all over
#
ARTWORK_PATH=$ARTWORK_PATH"3d/final/"

function copy_over {
   cp "${ARTWORK_PATH}$1.png" "$2.png"
   cp "${ARTWORK_PATH}$1_engine.png" "$2_engine.png"
   cp "${ARTWORK_PATH}$1_comm.png" "$2_comm.png"
}

copy_over "drone" "drone/drone"
copy_over "llama" "llama/llama"
copy_over "lancelot" "lancelot/lancelot"
copy_over "lancelot_empire" "lancelot/lancelot_empire"
copy_over "mule" "mule/mule"
copy_over "pacifier" "pacifier/pacifier"
copy_over "pacifier_empire" "pacifier/pacifier_empire"
copy_over "gawain" "gawain/gawain"
copy_over "goddard" "goddard/goddard"
copy_over "goddard_dvaered" "goddard/goddard_dvaered"
copy_over "hawking" "hawking/hawking"
copy_over "hawking_empire" "hawking/hawking_empire"
copy_over "hyena" "hyena/hyena"
copy_over "admonisher" "admonisher/admonisher"
copy_over "admonisher_pirate" "admonisher/admonisher_pirate"
copy_over "admonisher_empire" "admonisher/admonisher_empire"
copy_over "schroedinger" "schroedinger/schroedinger"
copy_over "vendetta" "vendetta/vendetta"
copy_over "vendetta_pirate" "vendetta/vendetta_pirate"
copy_over "vendetta_dvaered" "vendetta/vendetta_dvaered"
copy_over "ancestor" "ancestor/ancestor"
copy_over "ancestor_pirate" "ancestor/ancestor_pirate"
copy_over "ancestor_dvaered" "ancestor/ancestor_dvaered"
copy_over "koala" "koala/koala"
copy_over "kestrel" "kestrel/kestrel"
copy_over "kestrel_pirate" "kestrel/kestrel_pirate"
copy_over "archimedes" "archimedes/archimedes"
copy_over "derivative" "derivative/derivative"
copy_over "kahan" "kahan/kahan"
copy_over "vigilance" "vigilance/vigilance"
copy_over "vigilance_dvaered" "vigilance/vigilance_dvaered"
copy_over "watson" "watson/watson"
