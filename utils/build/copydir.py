#!/usr/bin/env python3
import os
import sys
import shutil
outdir = sys.argv[1]
os.makedirs( outdir, exist_ok=True )
for f in sys.argv[2:]:
    shutil.copy( f, os.path.join(outdir,os.path.basename(f)) )
