local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

update_dt = 1 -- Update once per second
local RANGE = 3000

function update( p )
   local f = flow.get( p, mem )
   local mod = math.max( (f-250)*0.08, 0 )
   if mod > 0 then
      local rmod = p:shipstat("ew_detect",true)
      for k,v in ipairs(p:getAllies( RANGE*rmod )) do
         if v:leader()==p and v:memory().carried then
            v:effectAdd( "Psychic Divinity", nil, mod )
         end
      end
   end
end
