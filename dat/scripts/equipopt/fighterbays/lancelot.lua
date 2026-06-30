local fb = require "equipopt.fighterbays"
local o, ogorgon -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Lancelot"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Unicorp PT-16 Core System"),
            systems_secondary = outfit.get("Unicorp PT-16 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            engines_secondary = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("TeraCom Fury Launcher"),
            outfit.get("Laser Cannon MK1"),
            outfit.get("Laser Cannon MK1"),
            outfit.get("Ion Cannon"),
            outfit.get("Unicorp Light Afterburner"),
            outfit.get("Reactor Class I"),
         }
         ogorgon = {
            -- Cores
            systems = outfit.get("Unicorp PT-16 Core System"),
            systems_secondary = outfit.get("Unicorp PT-16 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            engines_secondary = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("Unicorp D-2 Light Plating"),
            hull_secondary = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("TeraCom Medusa Launcher"),
            outfit.get("Ion Cannon"),
            outfit.get("Ion Cannon"),
            outfit.get("Ion Cannon"),
            outfit.get("Unicorp Light Afterburner"),
            outfit.get("Reactor Class I"),
            outfit.get("Reactor Class I"),
         }
      end
      if p:dockoutfit() == outfit.get("Gorgon Lancelot Bay") then
         fb.equip( p, ogorgon )
      else
         fb.equip( p, o )
      end
      return true
   end,
}
