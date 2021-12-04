-- Soromid faction standing script
local sbase = require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Soromid"),
   cap_kill       = 15,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def  = 30,
   cap_misn_var   = "_fcap_soromid",
}
