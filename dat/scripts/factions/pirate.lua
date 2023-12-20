return {
   fct            = faction.get("Pirate"), -- To be replaced by clans
   cap_kill       = 50,
   delta_distress = {-2, 0.25},    -- Maximum change constraints
   --delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def   = 80-3*3-5*3, -- Player should be able to get to 80
   cap_misn_var   = "_fcap_pirate",
   cap_tags       = {
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
   mod_distress_enemy = 1, -- Distress of the faction's enemies
   mod_distress_friend= 0, -- Distress of the faction's allies
   mod_kill_enemy     = 1, -- Kills of the faction's enemies
   mod_kill_friend    = 0, -- Kills of the faction's allies
   --text_friendly  = _("Benevolent"),
   --text_neutral   = _("Impartial"),
   --text_hostile   = _("Belligerent"),
   text_bribed    = _("Paid Off"),
}
