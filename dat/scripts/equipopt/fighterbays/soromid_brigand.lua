local fb = require "equipopt.fighterbays"
local bioship = require "bioship"
local o -- Lazy load the outfits
local NAVIUMNAL = outfit.get("Naviumnal Brigand Bay")
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
      if fb.spawnerOutfit(p) == NAVIUMNAL then
         bioship.simulate( p, bioship.maxstage( p ), {
            "plasma1",
            "plasma2",
            "plasma3",
            "health1",
            "health2",
         } )
      else
         bioship.simulate( p, bioship.maxstage( p ), {
            "bite1",
            "bite2",
            "bite3",
            "stealth1",
            "plasma1",
         } )
      end
      fb.equip( p, o )
      return true
   end,
}
