local sbase = require "factions.standing.lib.base"
sbase.init({
   fct         = faction.get("Independent"),
   rep_max     = 100,
   hit_range   = 0,
   destroy_max = 50,
   distress_mod = 0.1,
})
