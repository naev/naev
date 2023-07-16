notactive = true -- Doesnt' become active

local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function descextra( _p, o )
   return fmt.f("#y".._("Provides {regen} flow regeneration per second up to 50% maximum capacity. Requires a flow amplifier.").."#0",
      { regen=flow.list_regen[o:nameRaw()] })
end
