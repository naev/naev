local fb = require "equipopt.fighterbays"
return {
   priority = 10,
   ship = ship.get("Thurion Perspicacity"),
   equip = function ( p )
      local o = {
         systems = outfit.get("Milspec Thalos 2202 Core System"),
         engines = outfit.get("Nexus Dart 160 Engine"),
         hull = outfit.get("Unicorp D-2 Light Plating"),
         outfit.get("Laser Cannon MK2"),
         outfit.get("Unicorp Scrambler"),
         outfit.get("Reactor Class I"),
      }
      local nebu = select(2, system.cur():nebula())
      if nebu >= 9 then
         table.insert(o, outfit.get("Nebula Resistant Coating"))
      else
         table.insert(o, outfit.get("Nexus Concealment Coating"))
      end
      if nebu >= 16 then
         table.insert(o, outfit.get("Small Shield Booster"))
      else
         table.insert(o, outfit.get("Flicker Drive"))
      end
      fb.equip( p, o )
      return true
   end,
}
