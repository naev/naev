# Define a target per outfit family. Today, a single "generate.py" script is used. (TODO: Per Discord, we may use more scripts?)
# The target name (first arg to custom_target()) is arbitrary except for having to be unique.
# The input/output lists are duplicated, at least for now, in the generate.py blocks like
#    BioOutfit( "foo.xml.template", {...} ).run( [N_(nice_name_of_output_1), ... ] )
# We're discussing ideas about how to reduce this duplication. For now, the build rules and generate.py blocks MUST MATCH.

biooutfits = {
   'perlevis_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/small/tricon_zephyr_engine.xml' ],
      'output' : [
         'perlevis_gene_drive_i.xml',
         'perlevis_gene_drive_ii.xml',
      ],
   },
   'laeviter_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/small/tricon_zephyr_engine.xml' ],
      'output' : [
         'laeviter_gene_drive_i.xml',
         'laeviter_gene_drive_ii.xml',
      ],
   },
   'laevis_gene_drive' : {
      'template': 'gene_drive_melendez',
      'depend_files' : [ 'core_engine/small/melendez_ox_engine.xml' ],
      'output' : [
         'laevis_gene_drive_i.xml',
         'laevis_gene_drive_ii.xml',
      ],
   },
   'mediocris_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/medium/tricon_cyclone_engine.xml' ],
      'output' : [
         'mediocris_gene_drive_i.xml',
         'mediocris_gene_drive_ii.xml',
         'mediocris_gene_drive_iii.xml',
      ],
   },
   'largus_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/medium/tricon_cyclone_engine.xml' ],
      'output' : [
         'largus_gene_drive_i.xml',
         'largus_gene_drive_ii.xml',
      ],
   },
   'ponderosus_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/large/tricon_typhoon_engine.xml' ],
      'output' : [
         'ponderosus_gene_drive_i.xml',
         'ponderosus_gene_drive_ii.xml',
         'ponderosus_gene_drive_iii.xml',
      ],
   },
   'grandis_gene_drive' : {
      'template': 'gene_drive_melendez',
      'depend_files' : [ 'core_engine/large/melendez_mammoth_engine.xml' ],
      'output' : [
         'grandis_gene_drive_i.xml',
         'grandis_gene_drive_ii.xml',
         'grandis_gene_drive_iii.xml',
      ],
   },
   'magnus_gene_drive' : {
      'template': 'gene_drive_tricon',
      'depend_files' : [ 'core_engine/large/tricon_typhoon_engine.xml' ],
      'output' : [
         'magnus_gene_drive_i.xml',
         'magnus_gene_drive_ii.xml',
         'magnus_gene_drive_iii.xml',
      ],
   },
   'immanis_gene_drive' : {
      'template': 'gene_drive',
      'depend_files' : [ 'core_engine/large/nexus_bolt_3000_engine.xml' ],
      'output' : [
         'immanis_gene_drive_i.xml',
         'immanis_gene_drive_ii.xml',
         'immanis_gene_drive_iii.xml',
      ],
   },
   'perlevis_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/small/unicorp_d2_light_plating.xml',
         'core_hull/small/sk_skirmish_plating.xml',
      ],
      'output' : [
         'perlevis_cortex_i.xml',
         'perlevis_cortex_ii.xml',
      ],
   },
   'laevis_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/small/unicorp_d2_light_plating.xml',
         'core_hull/small/sk_skirmish_plating.xml',
      ],
      'output' : [
         'laevis_cortex_i.xml',
         'laevis_cortex_ii.xml',
      ],
   },
   'mediocris_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/medium/unicorp_d23_medium_plating.xml',
         'core_hull/medium/sk_battle_plating.xml',
      ],
      'output' : [
         'mediocris_cortex_i.xml',
         'mediocris_cortex_ii.xml',
      ],
   },
   'largus_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/medium/unicorp_d23_medium_plating.xml',
         'core_hull/medium/sk_battle_plating.xml',
      ],
      'output' : [
         'largus_cortex_i.xml',
         'largus_cortex_ii.xml',
         'largus_cortex_iii.xml',
      ],
   },
   'ponderosus_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/large/unicorp_d58_heavy_plating.xml',
         'core_hull/large/sk_war_plating.xml',
      ],
      'output' : [
         'ponderosus_cortex_i.xml',
         'ponderosus_cortex_ii.xml',
         'ponderosus_cortex_iii.xml',
         'ponderosus_cortex_iv.xml',
      ],
   },
   'immanis_cortex' : {
      'template': 'cortex',
      'depend_files' : [
         'core_hull/large/unicorp_d58_heavy_plating.xml',
         'core_hull/large/sk_war_plating.xml',
      ],
      'output' : [
         'immanis_cortex_i.xml',
         'immanis_cortex_ii.xml',
         'immanis_cortex_iii.xml',
      ],
   },
   'perleve_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/small/milspec_orion_2301_core_system.xml' ],
      'output' : [
        'perleve_cerebrum_i.xml',
        'perleve_cerebrum_ii.xml',
      ],
   },
   'laevum_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/small/milspec_orion_2301_core_system.xml' ],
      'output' : [
         'laevum_cerebrum_i.xml',
         'laevum_cerebrum_ii.xml',
      ],
   },
   'mediocre_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/medium/milspec_orion_4801_core_system.xml' ],
      'output' : [
         'mediocre_cerebrum_i.xml',
         'mediocre_cerebrum_ii.xml',
      ],
   },
   'largum_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/medium/milspec_orion_4801_core_system.xml' ],
      'output' : [
         'largum_cerebrum_i.xml',
         'largum_cerebrum_ii.xml',
      ],
   },
   'ponderosum_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/large/milspec_orion_8601_core_system.xml' ],
      'output' : [
         'ponderosum_cerebrum_i.xml',
         'ponderosum_cerebrum_ii.xml',
         'ponderosum_cerebrum_iii.xml',
      ],
   },
   'immane_cerebrum' : {
      'template': 'cerebrum',
      'depend_files' : [ 'core_system/large/milspec_orion_8601_core_system.xml' ],
      'output' : [
         'immane_cerebrum_i.xml',
         'immane_cerebrum_ii.xml',
         'immane_cerebrum_iii.xml',
      ],
   },
   'stinger_organ' : {
      'template': 'weapon',
      'output' : [
        'stinger_organ_i.xml',
        'stinger_organ_ii.xml',
        'stinger_organ_iii.xml',
      ],
   },
   'talon_organ' : {
      'template': 'weapon',
      'output' : [
        'talon_organ_i.xml',
        'talon_organ_ii.xml',
        'talon_organ_iii.xml',
        'talon_organ_iv.xml',
      ],
   },
   'tentacle_organ' : {
      'template': 'weapon',
      'output' : [
        'tentacle_organ_i.xml',
        'tentacle_organ_ii.xml',
        'tentacle_organ_iii.xml',
        'tentacle_organ_iv.xml'
      ],
   },
}

gen_bioship = find_program('generate.py')
bio_cmd = [gen_bioship, '@INPUT@', '-o', '@OUTPUT@']
bio_dir = ndata_path / 'dat/outfits/bioship'

bio_outfits = []
foreach name, data : biooutfits
   depend_files = []
   if 'depend_files' in data
      foreach df : data['depend_files']
         depend_files += files( '../' + df )
      endforeach
   endif

   bio_outfits += [custom_target( name, command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/' + data['template'] + '.xml.template')
      depend_files: depend_files,
      output: data['output'],
   )]
endforeach
