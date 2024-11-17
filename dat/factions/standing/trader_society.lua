-- Trader faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Traders Society"),
   rep_max        = 30,
   rep_max_var    = "_fcap_trader",
   destroy_max    = 10,
   distress_mod   = 0.1, -- They lose from distressing
   board_mod      = 0.5, -- Dislike being stolen from
}
