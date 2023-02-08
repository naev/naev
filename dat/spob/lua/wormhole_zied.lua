local wormhole = require "spob.lua.lib.wormhole"

function init( p )
   return wormhole.init( p, "Wormhole Rei" )
end

function load ()
   -- Gets a random target every time it loads
   if not mem.shader then
      local candidates = {}
      for k,v in ipairs(spob.getAll()) do
         if mem.spob~=v and  v:tags().wormhole then
            table.insert( candidates, v )
         end
      end
      mem.target = candidates[ rnd.rnd(1,#candidates) ]:nameRaw()
   end
   return wormhole.load()
end

unload   = wormhole.unload
update   = wormhole.update
render   = wormhole.render
can_land = wormhole.can_land
land     = wormhole.land
