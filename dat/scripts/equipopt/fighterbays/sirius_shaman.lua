local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Sirius Shaman"),
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
            outfit.get("TeraCom Fury Launcher"),
            outfit.get("TeraCom Fury Launcher"),
            outfit.get("Razor Artillery S1"),
            outfit.get("Razor Artillery S1"),
            outfit.get("Seeking Chakra"),
            --outfit.get("Small Flow Resonator"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
