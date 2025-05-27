local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Proteron Dalton"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.get("Unicorp PT-16 Core System"),
            engines = outfit.get("Unicorp Hawk 160 Engine"),
            hull = outfit.get("S&K Skirmish Plating"),
            -- Other outfits
            -- They just use Gauss Guns for now
            outfit.get("Gauss Gun"),
            outfit.get("Gauss Gun"),
            outfit.get("Gauss Gun"),
            outfit.get("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
