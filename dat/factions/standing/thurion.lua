-- Thurion faction standing script
require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Thurion"),
   cap_kill       = 10,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_init  = 30,
   cap_misn_var   = "_fcap_thurion",
}
