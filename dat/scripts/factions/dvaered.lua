return {
   fct            = faction.get("Dvaered"),
   rep_max        = 70-10, --40, -- TODO decrease as campaigns get added
   rep_max_var    = "_fcap_dvaered",
   destroy_max    = 40,
   disable_max    = 0,
   capture_max    = 0,
   board_max      = 0,
   cap_tags       = {
      -- TODO return cap_ch01 to 50 when possible
      ["dva_cap_ch01_sml"] = { val=1, max=70 },
      ["dva_cap_ch01_med"] = { val=3, max=70 },
      ["dva_cap_ch01_lrg"] = { val=5, max=70 },
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
