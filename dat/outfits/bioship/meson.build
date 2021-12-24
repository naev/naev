gen_bioship = find_program('generate.py')
bio_cmd = [gen_bioship, '@INPUT@', '-o', '@OUTPUT@']
bio_dir = ndata_path / 'dat/outfits/bioship'

# Define a target per outfit family. Today, a single "generate.py" script is used. (TODO: Per Discord, we may use more scripts?)
# The target name (first arg to custom_target()) is arbitrary except for having to be unique.
# The input/output lists are duplicated, at least for now, in the generate.py blocks like
#    BioOutfit( "foo.xml.template", {...} ).run( [N_(nice_name_of_output_1), ... ] )
# We're discussing ideas about how to reduce this duplication. For now, the build rules and generate.py blocks MUST MATCH.

bio_outfits = [
   custom_target('stinger_organ', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/weapon.xml.template'),
      output: [
        'stinger_organ_i.xml',
        'stinger_organ_ii.xml',
        'stinger_organ_iii.xml',
      ],
   ),
   custom_target('perleve_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
        'perleve_cerebrum_i.xml',
        'perleve_cerebrum_ii.xml',
      ],
   ),
   custom_target('perlevis_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'perlevis_gene_drive_i.xml',
         'perlevis_gene_drive_ii.xml',
      ],
   ),
   custom_target('perlevis_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'perlevis_cortex_i.xml',
         'perlevis_cortex_ii.xml',
      ],
   ),
   custom_target('laevum_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
         'laevum_cerebrum_i.xml',
         'laevum_cerebrum_ii.xml',
      ],
   ),
   custom_target('laevis_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'laevis_gene_drive_i.xml',
         'laevis_gene_drive_ii.xml',
      ],
   ),
   custom_target('laevis_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'laevis_cortex_i.xml',
         'laevis_cortex_ii.xml',
      ],
   ),
   custom_target('talon_organ', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/weapon.xml.template'),
      output: [
        'talon_organ_i.xml',
        'talon_organ_ii.xml',
        'talon_organ_iii.xml',
        'talon_organ_iv.xml',
      ],
   ),
   custom_target('mediocre_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
         'mediocre_cerebrum_i.xml',
         'mediocre_cerebrum_ii.xml',
      ],
   ),
   custom_target('mediocris_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'mediocris_gene_drive_i.xml',
         'mediocris_gene_drive_ii.xml',
         'mediocris_gene_drive_iii.xml',
      ],
   ),
   custom_target('mediocris_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'mediocris_cortex_i.xml',
         'mediocris_cortex_ii.xml',
      ],
   ),
   custom_target('largum_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
         'largum_cerebrum_i.xml',
         'largum_cerebrum_ii.xml',
      ],
   ),
   custom_target('largus_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'largus_gene_drive_i.xml',
         'largus_gene_drive_ii.xml',
      ],
   ),
   custom_target('largus_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'largus_cortex_i.xml',
         'largus_cortex_ii.xml',
         'largus_cortex_iii.xml',
      ],
   ),
   custom_target('ponderosum_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
         'ponderosum_cerebrum_i.xml',
         'ponderosum_cerebrum_ii.xml',
         'ponderosum_cerebrum_iii.xml',
      ],
   ),
   custom_target('ponderosus_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'ponderosus_gene_drive_i.xml',
         'ponderosus_gene_drive_ii.xml',
         'ponderosus_gene_drive_iii.xml',
      ],
   ),
   custom_target('ponderosus_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'ponderosus_cortex_i.xml',
         'ponderosus_cortex_ii.xml',
         'ponderosus_cortex_iii.xml',
      ],
   ),
   custom_target('immane_cerebrum', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cerebrum.xml.template'),
      output: [
         'immane_cerebrum_i.xml',
         'immane_cerebrum_ii.xml',
         'immane_cerebrum_iii.xml',
      ],
   ),
   custom_target('immanis_gene_drive', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/gene_drive.xml.template'),
      output: [
         'immanis_gene_drive_i.xml',
         'immanis_gene_drive_ii.xml',
         'immanis_gene_drive_iii.xml',
      ],
   ),
   custom_target('immanis_cortex', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/cortex.xml.template'),
      output: [
         'immanis_cortex_i.xml',
         'immanis_cortex_ii.xml',
         'immanis_cortex_iii.xml',
      ],
   ),
   custom_target('tentacle_organ', command: bio_cmd, install: true, install_dir: bio_dir,
      input: files('templates/weapon.xml.template'),
      output: [
        'tentacle_organ_i.xml',
        'tentacle_organ_ii.xml',
        'tentacle_organ_iii.xml',
        'tentacle_organ_iv.xml'
      ],
   ),
]
