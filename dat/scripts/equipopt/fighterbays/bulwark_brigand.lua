local fb = require "equipopt.fighterbays"
local bioship = require "bioship"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Bulwark Brigand"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            outfit.get("Plasma Blaster MK1"),
         }
      end
      bioship.simulate( p, bioship.maxstage( p ), {
         "plasma1",
         "plasma2",
         "plasma3",
         "health1",
         "health2",
      } )
      fb.equip( p, o )
      return true
   end,
}
