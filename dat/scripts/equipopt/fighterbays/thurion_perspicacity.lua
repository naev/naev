local fb = require "equipopt.fighterbays"
local ob -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Thurion Perspicacity"),
   equip = function ( p )
      if not ob then
         ob = {
            systems = outfit.exists("Milspec Aegis 2201 Core System"),
            engines = outfit.exists("Nexus Dart 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            outfit.exists("Light Muon Disruptor"),
            outfit.exists("Unicorp Scrambler"),
            outfit.exists("Sensor Array"),
            outfit.exists("Reactor Class I"),
         }
      end
      local o = tcopy( ob )
      local nebu = select(2, system.cur():nebula())
      if nebu >= 13 then
         table.insert(o, outfit.exists("Nebula Resistant Coating"))
      else
         table.insert(o, outfit.exists("Nexus Concealment Coating"))
      end
      if nebu >= 25 then
         table.insert(o, outfit.exists("Small Shield Booster"))
      else
         table.insert(o, outfit.exists("Flicker Drive"))
      end
      fb.equip( p, o )
      return true
   end,
}
