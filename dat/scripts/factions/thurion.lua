return {
   fct            = faction.get("Thurion"),
   rep_max        = 70, -- TODO lower as campaigns get added
   rep_max_var    = "_fcap_thurion",
   destroy_max    = 10,
   -- Based on academic hierarchy
   text = {
      [90]  = _("Tangible Avatar"),
      [70]  = _("Digital Ally"),
      [50]  = _("Computer Collaborator"),
      [20]  = _("Silicon Sympathizer"),
      [0]   = _("Non-Uploaded"),
      [-1]  = _("Antagonist"),
      [-30] = _("Adversary"),
      [-50] = _("Analogue Rubbish"),
   },
   text_friendly  = _("Eleemosynary"),
   text_neutral   = _("Phlegmatic"),
   text_hostile   = _("Bellicose"),
   text_bribed    = _("Pacified"),
   hit_range      = 5,
   rep_max_tags   = {},
}
