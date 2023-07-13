local flow = require "ships.lua.lib.flow"

notactive = false

function init( p )
   flow.recalculate( p )
end

function update( p, _po, dt )
   flow.update( p, dt )
end

function onhit( p, _po, armour, shield )
   flow.onhit( p, armour, shield )
end
