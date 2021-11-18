-- Collective faction standing script
require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Collective"),
   cap_kill       = 30,
   delta_distress = {0, 0},     -- Maximum change constraints
   delta_kill     = {0, 0},     -- Maximum change constraints
   cap_misn_var   = "_fcap_collective",
}
