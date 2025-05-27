local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Thurion Perspicacity"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Milspec Thalos 2202 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("Laser Cannon MK2"),
            outfit.get("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
