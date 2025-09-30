
com_ships = run_command([find_program('all_ships_dep.sh'), meson.project_source_root() / 'dat/ships/' ], check: true)
ship_sources = com_ships.stdout().strip().split('\n')
custom_target( 'all_ships',
   command: [find_program( 'gen_all_ships_tech.sh' ), '@OUTPUT@', '@INPUT@'],
   install: true,
   install_dir: ndata_path / 'dat/tech',
   input: ship_sources,
   output: 'all_ships.xml',
)


com_outfits = run_command([find_program('all_outfits_dep.sh'), meson.project_source_root() / 'dat/outfits/' ], check: true)
outfit_sources = com_outfits.stdout().strip().split('\n')

custom_target( 'all_outfits',
   command: [find_program( 'gen_all_outfits_tech.sh' ), '@OUTPUT@', '@INPUT@'],
   install: true,
   install_dir: ndata_path / 'dat/tech',
   depends: gen_outfits ,
   input: outfit_sources + gen_outfits,
   output: 'all_outfits.xml',
)
