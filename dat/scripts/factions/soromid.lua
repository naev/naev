return {
   fct            = faction.get("Soromid"),
   rep_max        = 70, --30, -- TODO decrease as missions get added
   rep_max_var    = "_fcap_soromid",
   -- As their ships are living, capturing is frowned upon
   capture_max    = -100,
   capture_mod    = 2,
   scan_mod       = 0, -- Don't care about scanning
   cap_tags       = {
      -- TODO return cap_ch01 to 50 when possible
      ["srm_cap_ch01_sml"] = { val=1, max=70 },
      ["srm_cap_ch01_med"] = { val=3, max=70 },
      ["srm_cap_ch01_lrg"] = { val=5, max=70 },
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
