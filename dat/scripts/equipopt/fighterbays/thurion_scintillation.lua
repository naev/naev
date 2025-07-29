local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Thurion Scintillation"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Milspec Thalos 2202 Core System"),
            engines = outfit.get("Unicorp Hawk 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("Convulsion Launcher"),
            outfit.get("Convulsion Launcher"),
            outfit.get("Laser Cannon MK1"),
            outfit.get("Nexus Concealment Coating"),
            outfit.get("Unicorp Scrambler"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
