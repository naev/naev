local flow = require "ships.lua.lib.flow"
require "ships.lua.sirius"

local super_init = init
function init( p )
   super_init( p )
end

function update( p, _dt )
   local f = flow.get( p )
   local mod = f*0.04-10
   p:shippropSet{
      fwd_damage = mod,
      tur_damage = mod,
      launch_damage = mod,
   }
end
