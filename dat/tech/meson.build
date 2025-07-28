
#c = run_command([find_program('all_ships_dep.sh'), meson.project_source_root() / 'dat/ships/' ], check: false)
#ship_sources = c.stdout().strip().split('\n')
#custom_target( 'all_ships',
#   command: [find_program( 'gen_all_ships_tech.sh' ), '@OUTPUT', '@INPUT@'],
#   install: true,
#   install_dir: ndata_path / 'dat/tech',
#   input: ship_sources,
#   output: 'all_ships.xml',
#)

custom_target( 'all_outfit',
   command: [find_program( 'gen_all_outfits_tech.sh' ), '@OUTPUT@', '@INPUT@'] + gen_outfits,
   install: true,
   install_dir: ndata_path / 'dat/tech',
   depends: [gen_outfits] ,
   input: outfit_sources,
   output: 'all_outfits.xml',
)
