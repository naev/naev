return {
   fct            = faction.get("Soromid"),
   cap_kill       = 15,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 70, --30, -- TODO decrease as missions get added
   cap_misn_var   = "_fcap_soromid",
   cap_tags       = {
      ["srm_cap_ch01_sml"] = { val=1, max=50 },
      ["srm_cap_ch01_med"] = { val=3, max=50 },
      ["srm_cap_ch01_lrg"] = { val=5, max=50 },
   },
   -- Based on tribal hierarchy
   text = {
      [100] = _("Elder"),
      [70]  = _("Warrior"),
      [50]  = _("Hunter"),
      [30]  = _("Initiate"),
      [0]   = _("Neutral"),
      [-1]  = _("Despised"),
      [-30] = _("Adversary"),
      [-50] = _("Natural Enemy"),
   },
   text_friendly  = _("Chivalrous"),
   text_neutral   = _("Indifferent"),
   text_hostile   = _("Violent"),
}
