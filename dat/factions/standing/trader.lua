-- Trader faction standing script
require "factions.standing.lib.base"

standing = sbase.newStanding{
   fct            = faction.get("Traders Guild"),
   cap_kill       = 0,
   delta_distress = {-1.5, 0},  -- Maximum change constraints
   delta_kill     = {-7, 2},    -- Maximum change constraints
   cap_misn_init  = 30,
   cap_misn_var   = "_fcap_trader",
}
