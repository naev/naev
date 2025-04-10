local wormhole = require "spob.lua.lib.wormhole"
wormhole.setup( function ()
   local candidates = {}
   for k,v in ipairs(spob.getAll()) do
      if mem.spob~=v and  v:tags().wormhole then
         table.insert( candidates, v )
      end
   end
   return candidates[ rnd.rnd(1,#candidates) ]
end, {
   col_inner   = {0.8, 0.2, 1.0},
   col_outter  = {0.8, 0.0, 1.0},
} )
