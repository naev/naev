local hypergate = require "spob.lua.lib.hypergate"

function load( p )
   return hypergate.load( p )
end

unload   = hypergate.unload
update   = hypergate.update
render   = hypergate.render
can_land = hypergate.can_land
land     = hypergate.land
