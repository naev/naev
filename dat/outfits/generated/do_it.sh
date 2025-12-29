#!/usr/bin/env bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
FRM="$DIR"/../

"$DIR/"corsair_engine.py "$FRM"'core_engine/medium/nexus_arrow_700_engine.xml' "$DIR"/corsair_engine.xml
"$DIR/"corsair_hull.py "$FRM"'core_hull/medium/nexus_ghost_weave.xml' "$DIR"/corsair_hull.xml
"$DIR/"corsair_systems.py "$FRM"'core_system/medium/unicorp_pt200_core_system.xml' "$DIR"/corsair_systems.xml
"$DIR/"neutralizer.py "$FRM"'weapons/heavy_ion_cannon.xml' "$DIR"/neutralizer.xml
"$DIR/"reaver.py "$FRM"'weapons/heavy_ripper_cannon.xml' "$DIR"/reaver.xml
