-- Static standing script: used for temporary/dynamic factions.
local sbase = require "factions.standing.lib.base"
sbase.init()
function hit( _sys, _mod, _source, _secondary )
   return 0 -- Doesn't change through hits
end
