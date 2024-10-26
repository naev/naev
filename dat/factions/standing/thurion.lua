-- Thurion faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Thurion"),
   rep_max        = 30,
   rep_max_var    = "_fcap_thurion",
   destroy_max    = 10,
   hit_range      = 5,
}
