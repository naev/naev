local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Pirate Hyena"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            engines = outfit.exists("Unicorp Hawk 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("Gauss Gun"),
            outfit.exists("Laser Cannon MK1"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
