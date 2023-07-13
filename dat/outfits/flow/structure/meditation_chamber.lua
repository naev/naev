notactive = true -- Doesnt' become active

local flow = require "ships.lua.lib.flow"
local fmt = require "format"

function descextra( _p, o )
   return fmt.f(_("Provides {flow} maximum flow capacity. Requires a flow amplifier."),
      { flow=flow.list_base[o:nameRaw()] })
end
