

c = run_command(find_program('all_ships_dep.sh'), check: true)
ship_sources = c.stdout().strip().split('\n')
custom_target( 'all_ships',
   command: [find_program( 'gen_all_ships_tech.sh' ), '@INPUT@'],
   install: true,
   install_dir: ndata_path / 'dat/tech',
   input: ship_sources,
   output: 'all_ships.xml',
)

c = run_command(find_program('all_outfits_dep.sh'), check: true)
outfits_sources = c.stdout().strip().split('\n')
gen_files = ['neutralizer.xml', 'reaver_cannon.xml', 'corsair_systems.xml', 'corsair_hull_plating.xml', 'corsair_engine.xml']

generated = []
foreach nam : gen_files
   generated += [meson.project_build_root() / 'dat/outfits/generated' / nam]
endforeach

custom_target( 'all_outfit',
   command: [find_program( 'gen_all_outfits_tech.sh' ), '@INPUT@'] + generated,
   install: true,
   install_dir: ndata_path / 'dat/tech',
   depends: [gen_outfits] ,
   input: outfits_sources,
   output: 'all_outfit.xml',
)
