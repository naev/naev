subdir('bioship')

gen_outfits = [
  custom_target(
    'neutralizer',
    command: [find_program('py/neutralizer.py'), '@INPUT@', '@OUTPUT@', ],
    input: 'weapons/heavy_ion_cannon.xml',
    output: 'neutralizer.xml',
  )
]
