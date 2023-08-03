-- Nasin faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Nasin"),
   cap_kill       = 20,
   delta_distress = {-1, 0},    -- Maximum change constraints
   delta_kill     = {-5, 1},    -- Maximum change constraints
   cap_misn_def  = 100,
   cap_misn_var   = "_fcap_nasin",
   --cap_mod_sec  = 0.3, -- Modulation from secondary (TODO: hasn't worked since 2012. Bring it back?)
}
