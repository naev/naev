local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Sirius Fidelity"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Unicorp PT-16 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("Ion Cannon"),
            outfit.get("Razor Artillery S1"),
            outfit.get("Razor Artillery S1"),
            outfit.get("Seeking Chakra"),
            outfit.get("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
