subdir('bioship')

gen_outfits = [
  custom_target(
    'neutralizer',
    command: [find_program('py/neutralizer.py'), '@INPUT@', '@OUTPUT@', ],
    depend_files: 'py/neutralizer.py',
    input: 'weapons/heavy_ion_cannon.xml',
    output: 'neutralizer.xml',
  ),
  custom_target(
    'reaver_cannon',
    command: [find_program('py/reaver.py'), '@INPUT@', '@OUTPUT@', ],
    depend_files: 'py/reaver.py',
    input: 'weapons/heavy_ripper_cannon.xml',
    output: 'reaver_cannon.xml',
  ),
]
