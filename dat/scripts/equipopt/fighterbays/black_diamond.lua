local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Black Diamond"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Milspec Orion 2301 Core System"),
            systems_secondary = outfit.get("Milspec Aegis 2201 Core System"),
            engines = outfit.get("Nexus Dart 160 Engine"),
            engines_secondary = outfit.get("Nexus Dart 160 Engine"),
            hull = outfit.get("S&K Skirmish Plating"),
            hull_secondary = outfit.get("Unicorp D-2 Light Plating"),
            -- Other outfits
            outfit.get("TeraCom Banshee Launcher"),
            outfit.get("Laser Cannon MK2"),
            outfit.get("Laser Cannon MK2"),
            outfit.get("Plasteel Plating"),
            outfit.get("Reactor Class I"),
            outfit.get("Reactor Class I"),
            outfit.get("Agility Combat AI"),
            outfit.get("Nebula Resistant Coating"), --Applied by the Thurion covertly when recovering the Emerald Sword for the FLF. Helps them not autodestruct immediately for the Arandon boss battle with Emerald Sword.
         }
      end
      fb.equip( p, o )
      return true
   end,
}
