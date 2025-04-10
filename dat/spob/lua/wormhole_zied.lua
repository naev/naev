local wormhole = require "spob.lua.lib.wormhole"
wormhole.setup( function ()
   local candidates = {}
   for k,v in ipairs(spob.getAll()) do
      if mem.spob~=v and  v:tags().wormhole then
         table.insert( candidates, v )
      end
   end
   return candidates[ rnd.rnd(1,#candidates) ]
end )
