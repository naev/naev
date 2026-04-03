#!/bin/bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DAT=$(realpath --relative-to="$PWD" "${DIR}/../../")

sed -e 's/\([0-9]*[1-9][0-9]*\.[0-9]\{6\}\)[0-9]*/\1/g' \
    -e 's/\([0-9]\+\.[0-9]*[1-9]\)0*\([^0-9]\)/\1\2/g' \
    -e 's/\([0-9]\+\)\.0*\([^0-9]\)/\1\2/g' \
    -i "$DAT"/ssys/*.xml "$DAT"/spob/*.xml "$DAT"/map_decorator/*.xml
