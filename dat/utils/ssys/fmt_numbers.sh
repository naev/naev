#!/bin/bash

DAT=$(realpath --relative-to="$PWD" "${DIR}/../../")

sed -e 's/\([0-9]*[1-9][0-9]*\.[0-9]\{6\}\)[0-9]*/\1/g' \
    -e 's/\([0-9]\+\.[0-9]*[1-9]\)0*\([^0-9]\)/\1\2/g' \
    -e 's/\([0-9]\+\)\.0*\([^0-9]\)/\1\2/g' \
    -i ssys/*.xml spob/*.xml map_decorator/*.xml
