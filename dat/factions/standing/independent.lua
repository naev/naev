local sbase = require "factions.standing.lib.base"
sbase.init({
   fct         = faction.get("Independent"),
   rep_max     = 50,
   hit_range   = 0,
   destroy_max = 50,
   distress_mod = 0.1,
   text = {
      [50]  = _("Local Hero"),
      [40]  = _("Role Model"),
      [30]  = _("Upstanding Citizen"),
      [20]  = _("Good Egg"),
      [10]  = _("Decent Individual"),
      [0]   = _("Clean"),
      [-1]  = _("Offender"),
      [-10]  = _("Criminal"),
      [-30]  = _("Felon"),
      [-50]  = _("Public Enemy"),
   },
   mod_func = function ( min, max, mod, sys, _source, _secondary, _primary_fct )
      -- Require an independent spob to "remember"
      local do_hit = false
      for k,p in ipairs(sys:spobs()) do
         if p:faction()==sbase.fct then
            do_hit = true
            break
         end
      end
      if not do_hit then
         -- Set to default just in case
         sys:setReputation( sbase.fct, sbase.fct:reputationDefault() )
         return min, max, 0
      end
      return min, max, mod
   end,
})
