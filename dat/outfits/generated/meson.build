# Outfits that are built from references
customoutfits = {
   'neutralizer' : {
      'command': 'neutralizer.py',
      'input'  : 'weapons/heavy_ion_cannon.xml',
      'output' : 'neutralizer.xml',
   },
   'reaver' : {
      'command': 'reaver.py',
      'input'  : 'weapons/heavy_ripper_cannon.xml',
      'output' : 'reaver_cannon.xml',
   },
   # Corsair Set
   'corsair_systems' : {
      'command': 'corsair_systems.py',
      'input'  : 'core_system/medium/unicorp_pt200_core_system.xml',
      'output' : 'corsair_systems.xml',
   },
   'corsair_hull' : {
      'command': 'corsair_hull.py',
      'input'  : 'core_hull/medium/nexus_ghost_weave.xml',
      'output' : 'corsair_hull_plating.xml',
   },
   'corsair_engine' : {
      'command': 'corsair_engine.py',
      'input'  : 'core_engine/medium/nexus_arrow_700_engine.xml',
      'output' : 'corsair_engine.xml',
   },
   # Junker Set
   'Junker Hull' : {
      'command': 'junker_hull.py',
      'input'  : 'core_hull/small/sk_small_cargo_hull.xml',
      'output' : 'junker_plates.xml',
   },
}

# Consider some dependencies to try to propagate changes
py_outfits_extra_deps = files('helper.py')
py_outfits_extra_deps += meson.project_source_root() / 'dat/utils/outfits/outfit.py'
py_outfits_extra_deps += meson.project_source_root() / 'utils/naev_content.py'

gen_outfits = []
foreach name, data : customoutfits
   gen_outfits += [custom_target( name,
      command: [find_program( data['command'] ), '@INPUT@', '@OUTPUT@', ],
      install: true,
      install_dir: ndata_path / 'dat/outfits/generated',
      depend_files: [data['command']] + py_outfits_extra_deps,
      input: meson.project_source_root() / 'dat/outfits' / data['input'],
      output: data['output'],
   )]
endforeach
