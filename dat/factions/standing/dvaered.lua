-- Dvaered faction standing script
require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Dvaered"),
   cap_kill       = 25,
   delta_distress = {-0.5, 0},  -- Maximum change constraints
   delta_kill     = {-5, 1.5},  -- Maximum change constraints
   cap_misn_init  = 40,
   cap_misn_var   = "_fcap_dvaered",
}
