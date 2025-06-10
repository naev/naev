subdir('bioship')

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
  'corsair_systems' : {
     'command': 'corsair_systems.py',
     'input'  : 'core_system/medium/unicorp_pt200_core_system.xml',
     'output' : 'corsair_systems.xml',
   },
}

gen_outfits = []
foreach name, data : customoutfits
  gen_outfits += [custom_target( name,
    command: [find_program( 'py' / data['command'] ), '@INPUT@', '@OUTPUT@', ],
    install: true,
    install_dir: ndata_path / 'dat/outfits',
    depend_files: 'py' / data['command'],
    input: data['input'],
    output: data['output'],
  )]
endforeach
