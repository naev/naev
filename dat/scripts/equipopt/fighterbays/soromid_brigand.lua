local fb = require "equipopt.fighterbays"
local bioship = require "bioship"
local o -- Lazy load the outfits
return {
   priority = 10,
   ship = ship.get("Soromid Brigand"),
   equip = function ( p )
      -- Lazy loading
      if not o then
         o = {
            outfit.get("Plasma Blaster MK1"),
         }
      end
      bioship.simulate( p, bioship.maxstage( p ), {
         "bite1",
         "bite2",
         "bite3",
         "stealth1",
         "plasma1",
      } )
      fb.equip( p, o )
      return true
   end,
}
