#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DST=${SCRIPT_DIR}'/../../dat/outfits/core_engine'

echo $DST
${SCRIPT_DIR}/mvx2xmllua.py  alone < ${DST}/large/krain_remige_engine.mvx > ${DST}/large/krain_remige_engine.xml
${SCRIPT_DIR}/mvx2xmllua.py twins < ${DST}/medium/krain_patagium_twin_engine.mvx > ${DST}/medium/krain_patagium_twin_engine.xml
for i in `find ${DST} | grep mvx | grep -v krain`;
   do ${SCRIPT_DIR}/mvx2xmllua.py < $i > ${i%mvx}xml
done
