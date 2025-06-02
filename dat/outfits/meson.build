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
}

gen_outfits = []
foreach name, data : customoutfits
  gen_outfits += [custom_target( name,
    command: [find_program( 'py' / data['command'] ), '@INPUT@', '@OUTPUT@', ],
    depend_files: 'py' / data['command'],
    input: data['input'],
    output: data['output'],
  )]
endforeach
