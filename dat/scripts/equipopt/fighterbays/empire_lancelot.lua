local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Empire Lancelot"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            systems_secondary = outfit.exists("Milspec Orion 2301 Core System"),
            engines = outfit.exists("Nexus Dart 160 Engine"),
            engines_secondary = outfit.exists("Nexus Dart 160 Engine"),
            hull = outfit.exists("S&K Skirmish Plating"),
            hull_secondary = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("Heavy Ripper Cannon"),
            outfit.exists("Laser Cannon MK1"),
            outfit.exists("Laser Cannon MK1"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Unicorp Light Afterburner"),
            outfit.exists("Photo-Voltaic Nanobot Coating"),
            outfit.exists("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
