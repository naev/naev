-- Static standing script: used for temporary/dynamic factions.
local sbase = require "factions.standing.lib.base"
sbase.init()
function hit( current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
