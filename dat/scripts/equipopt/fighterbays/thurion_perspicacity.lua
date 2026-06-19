local fb = require "equipopt.fighterbays"
local ob -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Thurion Perspicacity"),
   equip = function ( p )
      if not ob then
         ob = {
            systems = outfit.get("Milspec Aegis 2201 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            outfit.get("Electron Burst Cannon"),
            outfit.get("Unicorp Scrambler"),
            outfit.get("Sensor Array"),
            outfit.get("Reactor Class I"),
         }
      end
      local o = tcopy( ob )
      local nebu = select(2, system.cur():nebula())
      if nebu >= 13 then
         table.insert(o, outfit.get("Nebula Resistant Coating"))
      else
         table.insert(o, outfit.get("Nexus Concealment Coating"))
      end
      if nebu >= 25 then
         table.insert(o, outfit.get("Small Shield Booster"))
      else
         table.insert(o, outfit.get("Flicker Drive"))
      end
      fb.equip( p, o )
      return true
   end,
}
