local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Black Diamond"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Milspec Orion 2301 Core System"),
            systems_secondary = outfit.exists("Milspec Aegis 2201 Core System"),
            engines = outfit.exists("Nexus Dart 160 Engine"),
            engines_secondary = outfit.exists("Nexus Dart 160 Engine"),
            hull = outfit.exists("S&K Skirmish Plating"),
            hull_secondary = outfit.exists("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.exists("TeraCom Banshee Launcher"),
            outfit.exists("Laser Cannon MK2"),
            outfit.exists("Laser Cannon MK2"),
            outfit.exists("Plasteel Plating"),
            outfit.exists("Reactor Class I"),
            outfit.exists("Reactor Class I"),
            outfit.exists("Agility Combat AI"),
            outfit.exists("Nebula Resistant Coating"), --Applied by the Thurion covertly when recovering the Emerald Sword for the FLF. Helps them not autodestruct immediately for the Arandon boss battle with Emerald Sword.
         }
      end
      fb.equip( p, o )
      return true
   end,
}
