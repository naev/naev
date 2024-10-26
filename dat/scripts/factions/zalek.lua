return {
   fct            = faction.get("Za'lek"),
   rep_max        = 70-13, --30, -- TODO lower as missions get added
   rep_max_var    = "_fcap_zalek",
   secondary_default = 0.3, -- Care less about others
   destroy_max    = 15,
   rep_max_tags   = {
      -- TODO return cap_ch01 to 50 when possible
      ["zlk_cap_ch01_sml"] = { val=1, max=70 },
      ["zlk_cap_ch01_med"] = { val=3, max=70 },
      ["zlk_cap_ch01_lrg"] = { val=5, max=70 },
      -- ch2z should be 70
      ["zlk_cap_ch2z_sml"] = { val=1, max=70 },
      ["zlk_cap_ch2z_med"] = { val=3, max=70 },
      ["zlk_cap_ch2z_lrg"] = { val=5, max=70 },
   },
   -- Based on academic hierarchy
   text = {
      [100] = _("Professor Emeritus"),
      [90]  = _("Professor"),
      [80]  = _("Associate Professor"),
      [70]  = _("Assistant Professor"),
      [50]  = _("Senior Fellow"),
      [30]  = _("Fellow"),
      [10]  = _("Independent Researcher"),
      [0]   = _("Learner"),
      [-1]  = _("Scorned"),
      [-30] = _("Outlaw"),
      [-50] = _("Enemy"),
   },
   text_friendly  = _("Benevolent"),
   text_neutral   = _("Impartial"),
   text_hostile   = _("Belligerent"),
   text_bribed    = _("Funded"),
   hit_range      = 3,
}
