local fb = require "equipopt.fighterbays"
return {
   priority = 10,
   ship = ship.get("Thurion Scintillation"),
   equip = function ( p )
      local o = {
         systems = outfit.get("Milspec Thalos 2202 Core System"),
         engines = outfit.get("Unicorp Hawk 160 Engine"),
         hull = outfit.get("Unicorp D-2 Light Plating"),
         outfit.get("Convulsion Launcher"),
         outfit.get("Convulsion Launcher"),
      }
      local nebu = select(2, system.cur():nebula())
      if nebu >= 7 then
         o["systems"] = outfit.get("Milspec Aegis 2201 Core System")
         table.insert(o, outfit.get("Nebula Resistant Coating"))
      else
         table.insert(o, outfit.get("Nexus Concealment Coating"))
         table.insert(o, outfit.get("Laser Cannon MK1"))
      end
      if nebu >= 14 then
         o["engines"] = outfit.get("Melendez Ox Engine")
         table.insert(o, outfit.get("Reactor Class I"))
         table.insert(o, outfit.get("Small Shield Booster"))
      else
         table.insert(o, outfit.get("Unicorp Scrambler"))
      end
      fb.equip( p, o )
      return true
   end,
}
