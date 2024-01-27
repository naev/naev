# A small file to automatically export from Blender to STL
# This script should be called from shell via :
# blender --background --python blend_to_stl.py -- infile.gltf outfile.stl
import bpy
import os

import sys
argv = sys.argv
argv = argv[argv.index("--") + 1:]  # get all args after "--"
gltfpath = argv[0]
stlpath = argv[1]

# Extract name of file
filename = os.path.basename( bpy.data.filepath )

# Delete all
for obj in bpy.data.objects:
    obj.select_set(True)
bpy.ops.object.delete()

# Import GLTF
bpy.ops.import_scene.gltf( 'EXEC_DEFAULT', filepath=gltfpath )
for s in bpy.data.scenes:
    if s.name != "body":
        continue
    for obj in s.objects:
        obj.select_set(True)

# Export to STL
bpy.ops.export_mesh.stl( "EXEC_DEFAULT",  use_selection=True, filepath=stlpath )
