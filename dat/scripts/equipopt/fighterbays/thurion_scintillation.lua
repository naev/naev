local fb = require "equipopt.fighterbays"
local ob -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Thurion Scintillation"),
   equip = function ( p )
      if not ob then
         ob = {
            systems = outfit.exists("Milspec Thalos 2202 Core System"),
            engines = outfit.exists("Unicorp Hawk 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            outfit.exists("Convulsion Launcher"),
            outfit.exists("Convulsion Launcher"),
         }
      end
      local o = tcopy( ob )
      local nebu = select(2, system.cur():nebula())
      if nebu >= 8 then
         o["systems"] = outfit.exists("Milspec Aegis 2201 Core System")
         table.insert(o, outfit.exists("Nebula Resistant Coating"))
      else
         table.insert(o, outfit.exists("Nexus Concealment Coating"))
         table.insert(o, outfit.exists("Laser Cannon MK1"))
      end
      if nebu >= 17 then
         o["engines"] = outfit.exists("Melendez Ox Engine")
         table.insert(o, outfit.exists("Reactor Class I"))
         table.insert(o, outfit.exists("Small Shield Booster"))
      else
         table.insert(o, outfit.exists("Unicorp Scrambler"))
      end
      fb.equip( p, o )
      return true
   end,
}
