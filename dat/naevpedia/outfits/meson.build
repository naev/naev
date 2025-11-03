root = meson.project_source_root()

outfits_dir = meson.project_source_root() / 'dat/outfits/'
naevpedia_outfits_src = run_command([find_program(files(meson.project_source_root() / 'utils/find_xml.sh')),
   outfits_dir], check: true)

naevpedia_outfits_sources = []
foreach ns: naevpedia_outfits_src.stdout().strip().split('\n')
   naevpedia_outfits_sources += [outfits_dir / ns]
endforeach

outfits_gen = [find_program(files('outfits.py')), '@INPUT@', '-o', '@OUTPUT@']
naevpedia_outfits = []
foreach ns: files(naevpedia_outfits_sources)
   naevpedia_outfits += custom_target(ns.full_path().replace('/', '_'),
      command: outfits_gen,
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/outfits',
   )
endforeach

genlist = []
foreach gen: gen_outfits
   genlist += gen.to_list()
endforeach
foreach ns: genlist
   naevpedia_outfits += custom_target( ns.full_path().replace('/','_'),
      command: outfits_gen,
      depends: [gen_outfits, gen_bio_outfits], # a bit brute force to depend on gen_outfit stage, but works
      input: ns,
      output: '@BASENAME@.md',
      install: true,
      install_dir: ndata_path / 'dat/naevpedia/outfits',
   )
endforeach
