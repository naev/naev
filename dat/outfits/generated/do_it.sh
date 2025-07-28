#!/usr/bin/bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
FRM="$DIR"/../
DST="$FRM"sets

mkdir -p "$DST"

"$DIR/"corsair_engine.py "$FRM"'core_engine/medium/nexus_arrow_700_engine.xml' "$DST"/corsair_engine.xml
"$DIR/"corsair_hull.py "$FRM"'core_hull/medium/nexus_ghost_weave.xml' "$DST"/corsair_hull.xml
"$DIR/"corsair_systems.py "$FRM"'core_system/medium/unicorp_pt200_core_system.xml' "$DST"/corsair_systems.xml
"$DIR/"neutralizer.py "$FRM"'weapons/heavy_ion_cannon.xml' "$DST"/neutralizer.xml
"$DIR/"reaver.py "$FRM"'weapons/heavy_ripper_cannon.xml' "$DST"/reaver.xml
