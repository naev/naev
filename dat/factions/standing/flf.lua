-- FLF faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("FLF"),
   destroy_max    = 5,
   rep_max        = 5,
   rep_max_var    = "_fcap_flf",
   hit_range      = 5,
}
friendly_at    = 30
