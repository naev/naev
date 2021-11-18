-- Static standing script: used for temporary/dynamic factions.
require "factions.standing.lib.base"

standing = sbase.newStanding{}

function standing.hit( _self, current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
