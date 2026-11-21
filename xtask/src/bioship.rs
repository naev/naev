pub struct Family {
   /// Without the `.xml.template` suffix.
   pub template: &'static str,
   /// generate.py writes nothing unless these match its names for the family, in order.
   pub outputs: &'static [&'static str],
   /// The outfits under dat/outfits the family is derived from.
   pub sources: &'static [&'static str],
}

pub const FAMILIES: &[Family] = &[
   Family {
      template: "gene_drive_tricon",
      outputs: &["perlevis_gene_drive_i.xml", "perlevis_gene_drive_ii.xml"],
      sources: &["core_engine/small/tricon_zephyr_engine.xml"],
   },
   Family {
      template: "gene_drive_tricon",
      outputs: &["laeviter_gene_drive_i.xml", "laeviter_gene_drive_ii.xml"],
      sources: &["core_engine/small/tricon_zephyr_engine.xml"],
   },
   Family {
      template: "gene_drive_melendez",
      outputs: &["laevis_gene_drive_i.xml", "laevis_gene_drive_ii.xml"],
      sources: &["core_engine/small/melendez_ox_engine.xml"],
   },
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "mediocris_gene_drive_i.xml",
         "mediocris_gene_drive_ii.xml",
         "mediocris_gene_drive_iii.xml",
      ],
      sources: &["core_engine/medium/tricon_cyclone_engine.xml"],
   },
   Family {
      template: "gene_drive_tricon",
      outputs: &["largus_gene_drive_i.xml", "largus_gene_drive_ii.xml"],
      sources: &["core_engine/medium/tricon_cyclone_engine.xml"],
   },
   Family {
      template: "gene_drive_melendez",
      outputs: &["supernus_gene_drive_i.xml", "supernus_gene_drive_ii.xml"],
      sources: &[
         "core_engine/medium/melendez_buffalo_engine.xml",
         "core_engine/medium/tricon_cyclone_engine.xml",
      ],
   },
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "ponderosus_gene_drive_i.xml",
         "ponderosus_gene_drive_ii.xml",
         "ponderosus_gene_drive_iii.xml",
      ],
      sources: &["core_engine/large/tricon_typhoon_engine.xml"],
   },
   Family {
      template: "gene_drive_melendez",
      outputs: &[
         "grandis_gene_drive_i.xml",
         "grandis_gene_drive_ii.xml",
         "grandis_gene_drive_iii.xml",
      ],
      sources: &["core_engine/large/melendez_mammoth_engine.xml"],
   },
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "magnus_gene_drive_i.xml",
         "magnus_gene_drive_ii.xml",
         "magnus_gene_drive_iii.xml",
      ],
      sources: &["core_engine/large/tricon_typhoon_engine.xml"],
   },
   Family {
      template: "gene_drive",
      outputs: &[
         "immanis_gene_drive_i.xml",
         "immanis_gene_drive_ii.xml",
         "immanis_gene_drive_iii.xml",
      ],
      sources: &["core_engine/large/nexus_bolt_3000_engine.xml"],
   },
   Family {
      template: "cortex",
      outputs: &["perlevis_cortex_i.xml", "perlevis_cortex_ii.xml"],
      sources: &[
         "core_hull/small/unicorp_d2_light_plating.xml",
         "core_hull/small/sk_skirmish_plating.xml",
      ],
   },
   Family {
      template: "cortex",
      outputs: &["laevis_cortex_i.xml", "laevis_cortex_ii.xml"],
      sources: &[
         "core_hull/small/unicorp_d2_light_plating.xml",
         "core_hull/small/sk_skirmish_plating.xml",
      ],
   },
   Family {
      template: "cortex",
      outputs: &["mediocris_cortex_i.xml", "mediocris_cortex_ii.xml"],
      sources: &[
         "core_hull/medium/unicorp_d23_medium_plating.xml",
         "core_hull/medium/sk_battle_plating.xml",
      ],
   },
   Family {
      template: "cortex",
      outputs: &[
         "largus_cortex_i.xml",
         "largus_cortex_ii.xml",
         "largus_cortex_iii.xml",
      ],
      sources: &[
         "core_hull/medium/unicorp_d23_medium_plating.xml",
         "core_hull/medium/sk_battle_plating.xml",
      ],
   },
   Family {
      template: "cortex",
      outputs: &[
         "ponderosus_cortex_i.xml",
         "ponderosus_cortex_ii.xml",
         "ponderosus_cortex_iii.xml",
         "ponderosus_cortex_iv.xml",
      ],
      sources: &[
         "core_hull/large/unicorp_d58_heavy_plating.xml",
         "core_hull/large/sk_war_plating.xml",
      ],
   },
   Family {
      template: "cortex",
      outputs: &[
         "immanis_cortex_i.xml",
         "immanis_cortex_ii.xml",
         "immanis_cortex_iii.xml",
      ],
      sources: &[
         "core_hull/large/unicorp_d58_heavy_plating.xml",
         "core_hull/large/sk_war_plating.xml",
      ],
   },
   Family {
      template: "cerebrum",
      outputs: &["perleve_cerebrum_i.xml", "perleve_cerebrum_ii.xml"],
      sources: &["core_system/small/milspec_orion_2301_core_system.xml"],
   },
   Family {
      template: "cerebrum",
      outputs: &["laevum_cerebrum_i.xml", "laevum_cerebrum_ii.xml"],
      sources: &["core_system/small/milspec_orion_2301_core_system.xml"],
   },
   Family {
      template: "cerebrum",
      outputs: &["mediocre_cerebrum_i.xml", "mediocre_cerebrum_ii.xml"],
      sources: &["core_system/medium/milspec_orion_4801_core_system.xml"],
   },
   Family {
      template: "cerebrum",
      outputs: &["largum_cerebrum_i.xml", "largum_cerebrum_ii.xml"],
      sources: &["core_system/medium/milspec_orion_4801_core_system.xml"],
   },
   Family {
      template: "cerebrum",
      outputs: &[
         "ponderosum_cerebrum_i.xml",
         "ponderosum_cerebrum_ii.xml",
         "ponderosum_cerebrum_iii.xml",
      ],
      sources: &["core_system/large/milspec_orion_8601_core_system.xml"],
   },
   Family {
      template: "cerebrum",
      outputs: &[
         "immane_cerebrum_i.xml",
         "immane_cerebrum_ii.xml",
         "immane_cerebrum_iii.xml",
      ],
      sources: &["core_system/large/milspec_orion_8601_core_system.xml"],
   },
   Family {
      template: "weapon",
      outputs: &[
         "stinger_organ_i.xml",
         "stinger_organ_ii.xml",
         "stinger_organ_iii.xml",
      ],
      sources: &[],
   },
   Family {
      template: "weapon",
      outputs: &[
         "talon_organ_i.xml",
         "talon_organ_ii.xml",
         "talon_organ_iii.xml",
         "talon_organ_iv.xml",
      ],
      sources: &[],
   },
   Family {
      template: "weapon",
      outputs: &[
         "tentacle_organ_i.xml",
         "tentacle_organ_ii.xml",
         "tentacle_organ_iii.xml",
         "tentacle_organ_iv.xml",
      ],
      sources: &[],
   },
];
