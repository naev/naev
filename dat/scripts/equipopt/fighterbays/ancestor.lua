local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Ancestor"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Unicorp PT-16 Core System"),
            systems_secondary = outfit.get("Unicorp PT-16 Core System"),
            engines = outfit.get("Unicorp Hawk 160 Engine"),
            engines_secondary = outfit.get("Unicorp Hawk 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("TeraCom Vengeance Launcher"),
			outfit.get("TeraCom Banshee Launcher"),
            outfit.get("TeraCom Mace Launcher"),
            outfit.get("TeraCom Mace Launcher"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
