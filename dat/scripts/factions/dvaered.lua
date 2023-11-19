return {
   fct            = faction.get("Dvaered"),
   cap_kill       = 25,
   delta_distress = {-0.5, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1.5},    -- Maximum change constraints
   cap_misn_def   = 70-10, --40, -- TODO decrease as campaigns get added
   cap_misn_var   = "_fcap_dvaered",
   cap_tags       = {
      ["dva_cap_ch01_sml"] = { val=1, max=50 },
      ["dva_cap_ch01_med"] = { val=3, max=50 },
      ["dva_cap_ch01_lrg"] = { val=5, max=50 },
   },
   -- Based on feudal hierarchy
   text = {
      [100] = _("Lord"),
      [90]  = _("Liege Lord"),
      [70]  = _("Esquire"),
      [50]  = _("Yeoman"),
      [20]  = _("Bordar"),
      [0]   = _("Vagabond"),
      [-1]  = _("Offender"),
      [-30] = _("Enemy"),
   },
   text_friendly  = _("Amicable"),
   text_neutral   = _("Indifferent"),
   text_hostile   = _("Warlike"),
   text_bribed    = _("Bribed"), -- Default
}
