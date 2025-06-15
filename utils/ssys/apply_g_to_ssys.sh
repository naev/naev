#!/usr/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

"$SCRIPT_DIR"/apply_g.sh | "$SCRIPT_DIR"/ssys_graph.py -w
