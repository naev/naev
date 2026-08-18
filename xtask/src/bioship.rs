//! The bioship outfit families, mirroring the table the old
//! `dat/outfits/bioship/meson.build` carried.
//!
//! The output names are also spelled out inside `generate.py`, and the two
//! have to agree. Reading them from one place is worth doing eventually.

/// A family of outfits generated from one template.
pub struct Family {
   /// Template to expand, without the `.xml.template` suffix.
   pub template: &'static str,
   /// Files the family produces.
   pub outputs: &'static [&'static str],
}

pub const FAMILIES: &[Family] = &[
   // perlevis_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &["perlevis_gene_drive_i.xml", "perlevis_gene_drive_ii.xml"],
   },
   // laeviter_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &["laeviter_gene_drive_i.xml", "laeviter_gene_drive_ii.xml"],
   },
   // laevis_gene_drive
   Family {
      template: "gene_drive_melendez",
      outputs: &["laevis_gene_drive_i.xml", "laevis_gene_drive_ii.xml"],
   },
   // mediocris_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "mediocris_gene_drive_i.xml",
         "mediocris_gene_drive_ii.xml",
         "mediocris_gene_drive_iii.xml",
      ],
   },
   // largus_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &["largus_gene_drive_i.xml", "largus_gene_drive_ii.xml"],
   },
   // supernus_gene_drive
   Family {
      template: "gene_drive_melendez",
      outputs: &["supernus_gene_drive_i.xml", "supernus_gene_drive_ii.xml"],
   },
   // ponderosus_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "ponderosus_gene_drive_i.xml",
         "ponderosus_gene_drive_ii.xml",
         "ponderosus_gene_drive_iii.xml",
      ],
   },
   // grandis_gene_drive
   Family {
      template: "gene_drive_melendez",
      outputs: &[
         "grandis_gene_drive_i.xml",
         "grandis_gene_drive_ii.xml",
         "grandis_gene_drive_iii.xml",
      ],
   },
   // magnus_gene_drive
   Family {
      template: "gene_drive_tricon",
      outputs: &[
         "magnus_gene_drive_i.xml",
         "magnus_gene_drive_ii.xml",
         "magnus_gene_drive_iii.xml",
      ],
   },
   // immanis_gene_drive
   Family {
      template: "gene_drive",
      outputs: &[
         "immanis_gene_drive_i.xml",
         "immanis_gene_drive_ii.xml",
         "immanis_gene_drive_iii.xml",
      ],
   },
   // perlevis_cortex
   Family {
      template: "cortex",
      outputs: &["perlevis_cortex_i.xml", "perlevis_cortex_ii.xml"],
   },
   // laevis_cortex
   Family {
      template: "cortex",
      outputs: &["laevis_cortex_i.xml", "laevis_cortex_ii.xml"],
   },
   // mediocris_cortex
   Family {
      template: "cortex",
      outputs: &["mediocris_cortex_i.xml", "mediocris_cortex_ii.xml"],
   },
   // largus_cortex
   Family {
      template: "cortex",
      outputs: &[
         "largus_cortex_i.xml",
         "largus_cortex_ii.xml",
         "largus_cortex_iii.xml",
      ],
   },
   // ponderosus_cortex
   Family {
      template: "cortex",
      outputs: &[
         "ponderosus_cortex_i.xml",
         "ponderosus_cortex_ii.xml",
         "ponderosus_cortex_iii.xml",
         "ponderosus_cortex_iv.xml",
      ],
   },
   // immanis_cortex
   Family {
      template: "cortex",
      outputs: &[
         "immanis_cortex_i.xml",
         "immanis_cortex_ii.xml",
         "immanis_cortex_iii.xml",
      ],
   },
   // perleve_cerebrum
   Family {
      template: "cerebrum",
      outputs: &["perleve_cerebrum_i.xml", "perleve_cerebrum_ii.xml"],
   },
   // laevum_cerebrum
   Family {
      template: "cerebrum",
      outputs: &["laevum_cerebrum_i.xml", "laevum_cerebrum_ii.xml"],
   },
   // mediocre_cerebrum
   Family {
      template: "cerebrum",
      outputs: &["mediocre_cerebrum_i.xml", "mediocre_cerebrum_ii.xml"],
   },
   // largum_cerebrum
   Family {
      template: "cerebrum",
      outputs: &["largum_cerebrum_i.xml", "largum_cerebrum_ii.xml"],
   },
   // ponderosum_cerebrum
   Family {
      template: "cerebrum",
      outputs: &[
         "ponderosum_cerebrum_i.xml",
         "ponderosum_cerebrum_ii.xml",
         "ponderosum_cerebrum_iii.xml",
      ],
   },
   // immane_cerebrum
   Family {
      template: "cerebrum",
      outputs: &[
         "immane_cerebrum_i.xml",
         "immane_cerebrum_ii.xml",
         "immane_cerebrum_iii.xml",
      ],
   },
   // stinger_organ
   Family {
      template: "weapon",
      outputs: &[
         "stinger_organ_i.xml",
         "stinger_organ_ii.xml",
         "stinger_organ_iii.xml",
      ],
   },
   // talon_organ
   Family {
      template: "weapon",
      outputs: &[
         "talon_organ_i.xml",
         "talon_organ_ii.xml",
         "talon_organ_iii.xml",
         "talon_organ_iv.xml",
      ],
   },
   // tentacle_organ
   Family {
      template: "weapon",
      outputs: &[
         "tentacle_organ_i.xml",
         "tentacle_organ_ii.xml",
         "tentacle_organ_iii.xml",
         "tentacle_organ_iv.xml",
      ],
   },
];
