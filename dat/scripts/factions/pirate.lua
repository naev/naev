return {
   fct            = faction.get("Pirate"), -- To be replaced by clans
   rep_max        = 80-3*3-5*3, -- Player should be able to get to 80
   rep_max_var    = "_fcap_pirate",
   destroy_max    = 30,
   destroy_mod    = 0.8,
   board_max      = 50, -- In general prefer boarding / capturing over killing
   board_min      = 30, -- Don't care too much about being boarded
   capture_max    = 60,
   distress_max   = 10, -- Can enjoy you distressing others
   distress_min   = 0, -- Don't care too much about themselves
   distress_mod   = 0.1,
   scan_mod       = 0, -- Don't care about scanning
   rep_max_tags   = {
      ["pir_cap_ch01_sml"] = { val=1, max=80 },
      ["pir_cap_ch01_med"] = { val=3, max=80 },
      ["pir_cap_ch01_lrg"] = { val=5, max=80 },
   },
   text = {
      [95] = _("Clan Legend"),
      [80] = _("Clan Lord"),
      [60] = _("Clan Warrior"),
      [40] = _("Clan Plunderer"),
      [20] = _("Clan Thug"),
      [0]  = _("Common Thief"),
      [-1] = _("Normie"),
   },
   --text_friendly  = _("Benevolent"),
   --text_neutral   = _("Impartial"),
   --text_hostile   = _("Belligerent"),
   text_bribed    = _("Paid Off"),
   hit_range      = 5,
}
