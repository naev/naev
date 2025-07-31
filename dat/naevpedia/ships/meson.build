root = meson.project_source_root()

ships_dir = meson.project_source_root() / 'dat/ships/'
naevpedia_ships_src = run_command([find_program(meson.project_source_root() / 'utils/find_xml.sh'),
   ships_dir], check: true)

naevpedia_ships_sources = []
foreach ns: naevpedia_ships_src.stdout().strip().split('\n')
   naevpedia_ships_sources += [ships_dir / ns]
endforeach

ships_gen = [find_program('ships.py'), '@INPUT@', '-o', '@OUTPUT@']
naevpedia_ships = []
foreach ns: files(naevpedia_ships_sources)
   naevpedia_ships += custom_target(ns.full_path().replace('/', '_'),
      command: ships_gen,
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/ships',
    )
endforeach
