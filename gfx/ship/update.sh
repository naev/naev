#!/usr/bin/env bash

#
#   Variables
#
ARTWORK_PATH="../../../naev-artwork/"


#
#   Copy them all over
#
ARTWORK_PATH=$ARTWORK_PATH"3d/final/"

function copy_over {
   cp "${ARTWORK_PATH}$1.png" "$2.png"
   cp "${ARTWORK_PATH}$1_engine.png" "$2_engine.png"
   cp "${ARTWORK_PATH}$1_comm.png" "$2_comm.png"
}

copy_over "seaxbane" "drone/drone"
copy_over "llama" "llama/llama"
copy_over "lancelot0" "lancelot/lancelot"
copy_over "lancelot1" "lancelot/lancelot_empire"
copy_over "mule" "mule/mule"
copy_over "pacifier0" "pacifier/pacifier"
copy_over "pacifier1" "pacifier/pacifier_empire"
copy_over "gawain0" "gawain/gawain"
copy_over "goddard0" "goddard/goddard"
copy_over "goddard1" "goddard/goddard_dvaered"
copy_over "hawking2" "hawking/hawking"
copy_over "hawking3" "hawking/hawking_empire"
copy_over "hyena1" "hyena/hyena"
copy_over "admonisher0" "admonisher/admonisher"
copy_over "admonisher2" "admonisher/admonisher_pirate"
copy_over "admonisher4" "admonisher/admonisher_empire"
copy_over "schroedinger0" "schroedinger/schroedinger"
copy_over "vendetta0" "vendetta/vendetta"
copy_over "vendetta1" "vendetta/vendetta_pirate"
copy_over "vendetta2" "vendetta/vendetta_dvaered"
copy_over "ancestor0" "ancestor/ancestor"
copy_over "ancestor1" "ancestor/ancestor_pirate"
copy_over "ancestor2" "ancestor/ancestor_dvaered"
copy_over "koala" "koala/koala"
copy_over "kestrel0" "kestrel/kestrel"
copy_over "kestrel1" "kestrel/kestrel_pirate"
