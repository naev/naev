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
   cp "${ARTWORK_PATH}$1_comm.png" "$2_comm.png"
}

copy_over "seaxbane" "drone"
copy_over "llama" "llama"
copy_over "lancelot0" "lancelot"
copy_over "lancelot1" "lancelot_empire"
copy_over "mule" "mule"
copy_over "pacifier0" "pacifier"
copy_over "pacifier1" "pacifier_empire"
copy_over "gawain0" "gawain"
copy_over "goddard0" "goddard"
copy_over "goddard1" "goddard_dvaered"
copy_over "hawking2" "hawking"
copy_over "hawking3" "hawking_empire"
copy_over "hyena1" "hyena"
copy_over "admonisher0" "admonisher"
copy_over "admonisher2" "admonisher_pirate"
copy_over "admonisher4" "admonisher_empire"
copy_over "schroedinger0" "schroedinger"
copy_over "vendetta0" "vendetta"
copy_over "vendetta1" "vendetta_pirate"
copy_over "vendetta2" "vendetta_dvaered"
copy_over "ancestor0" "ancestor"
copy_over "ancestor1" "ancestor_pirate"
copy_over "ancestor2" "ancestor_dvaered"
