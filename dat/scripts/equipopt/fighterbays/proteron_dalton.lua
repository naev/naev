local fb = require "equipopt.fighterbays"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Proteron Dalton"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            -- Cores
            systems = outfit.exists("Unicorp PT-16 Core System"),
            engines = outfit.exists("Unicorp Hawk 160 Engine"),
            hull = outfit.exists("S&K Skirmish Plating"),
            -- Other outfits
            outfit.exists("Light Pulse Cannon"),
            outfit.exists("Light Pulse Cannon"),
            outfit.exists("Reactor Class I"),
         }
      end
      fb.equip( p, o )
      return true
   end,
}
