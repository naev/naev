-- Pirate faction standing script
local spir = require "factions.standing.lib.pirate"

standing = spir.newPirateStanding{
   fct            = faction.get("Pirate"),
   text = {
      [20] = _("Respected"),
      [0]  = _("Common Thief"),
      [-1] = _("Normie"),
   },
}

function standing.hit( _self, current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
