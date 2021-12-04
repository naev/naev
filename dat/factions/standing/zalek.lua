-- Za'lek faction standing script
local sbase = require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Za'lek"),
   cap_kill       = 15,
   delta_distress = {-0.5, 0},  -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_init  = 30,
   cap_misn_var   = "_fcap_zalek",
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
}
