-- Static standing script: used for temporary/dynamic factions.
local sbase = require "factions.standing.lib.base"
sbase.init()
function hit()
   return 0 -- Doesn't change through hits
end
