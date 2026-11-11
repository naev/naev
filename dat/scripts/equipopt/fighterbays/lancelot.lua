local fb = require "equipopt.fighterbays"
local o, ogorgon -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Lancelot"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            systems_secondary = outfit.exists("Unicorp PT-16 Core System"),
            engines = outfit.exists("Nexus Dart 160 Engine"),
            engines_secondary = outfit.exists("Nexus Dart 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("TeraCom Fury Launcher"),
            outfit.exists("Laser Cannon MK1"),
            outfit.exists("Laser Cannon MK1"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Unicorp Light Afterburner"),
            outfit.exists("Reactor Class I"),
         }
         ogorgon = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            systems_secondary = outfit.exists("Unicorp PT-16 Core System"),
            engines = outfit.exists("Nexus Dart 160 Engine"),
            engines_secondary = outfit.exists("Nexus Dart 160 Engine"),
            hull = outfit.exists("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("TeraCom Medusa Launcher"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Ion Cannon"),
            outfit.exists("Unicorp Light Afterburner"),
            outfit.exists("Reactor Class I"),
            outfit.exists("Reactor Class I"),
         }
      end
      if p:dockoutfit() == outfit.exists("Gorgon Lancelot Bay") then
         fb.equip( p, ogorgon )
      else
         fb.equip( p, o )
      end
      return true
   end,
}
