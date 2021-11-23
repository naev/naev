-- FLF faction standing script
local sbase = require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("FLF"),
   cap_kill       = 5,
   delta_distress = {-0.5, 0},  -- Maximum change constraints
   delta_kill     = {-5, 1.5},  -- Maximum change constraints
   cap_misn_init  = 5,
   cap_misn_var   = "_fcap_flf",
   friendly_at    = 30,
}
