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
# New model starts selected, so have to deselect
for obj in bpy.data.objects:
    obj.select_set(False)
# Select only body, ignore engines and other things
# TODO support for collision shapes
selected = False
scene = bpy.data.scenes[0]
for s in bpy.data.scenes:
    if s.name=="base":
        scene = s
for i in scene.objects:
    i.select_set(True)
    selected = True
if not selected:
    print(f"{gltfpath}: Failed to find any objects to select!")
    sys.exit(-1)

# Export to STL
bpy.ops.export_mesh.stl( "EXEC_DEFAULT",  use_selection=True, filepath=stlpath )
