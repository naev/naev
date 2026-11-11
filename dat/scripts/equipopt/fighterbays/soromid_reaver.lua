local fb = require "equipopt.fighterbays"
local bioship = require "bioship"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.exists("Soromid Reaver"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            outfit.exists("Plasma Blaster MK2"),
            outfit.exists("Reactor Class I"),
            outfit.exists("Plasteel Plating"),
         }
      end
      bioship.simulate( p, bioship.maxstage( p ), {
         "plasma1",
         "plasma2",
         "plasma3",
         "health1",
         "health2",
         "health3",
      } )
      fb.equip( p, o )
      return true
   end,
}
