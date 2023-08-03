-- Trader faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Traders Society"),
   cap_kill       = 0,
   delta_distress = {-1.5, 0},  -- Maximum change constraints
   delta_kill     = {-7, 2},    -- Maximum change constraints
   cap_misn_def  = 30,
   cap_misn_var   = "_fcap_trader",
}
