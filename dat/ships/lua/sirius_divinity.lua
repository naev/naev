local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

local DT    = 1
local RANGE = 3000

local super_init = init
function init( p )
   super_init( p )
   mem.t = 0
end

function update( p, dt )
   mem.t = mem.t - dt
   if mem.t > 0 then
      return
   end
   mem.t = mem.t + DT

   local f = flow.get( p )
   local mod = math.max( f*0.08-20, 0 )
   if mod > 0 then
      local rmod = p:shipstat("ew_detect",true)
      for k,v in ipairs(p:getAllies( RANGE*rmod )) do
         if v:leader()==p and v:memory().carried then
            v:effectAdd( "Psychic Divinity", nil, mod )
         end
      end
   end
end
