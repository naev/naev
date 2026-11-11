local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Pirate Ancestor"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            systems_secondary = outfit.exists("Unicorp PT-16 Core System"),
            engines = outfit.exists("Unicorp Hawk 160 Engine"),
            engines_secondary = outfit.exists("Unicorp Hawk 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("TeraCom Medusa Launcher"),
            outfit.exists("TeraCom Banshee Launcher"),
            outfit.exists("TeraCom Mace Launcher"),
            outfit.exists("TeraCom Mace Launcher"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
