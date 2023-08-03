-- Collective faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Collective"),
   cap_kill       = 30,
   delta_distress = {0, 0},     -- Maximum change constraints
   delta_kill     = {0, 0},     -- Maximum change constraints
   cap_misn_var   = "_fcap_collective",
}
