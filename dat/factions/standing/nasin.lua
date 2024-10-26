-- Nasin faction standing script
local sbase = require "factions.standing.lib.base"
sbase.init{
   fct            = faction.get("Nasin"),
   destroy_max    = 20,
   rep_max        = 100,
   rep_max_var    = "_fcap_nasin",
   secondary_default = 0.3,
   hit_range      = 0,
}
