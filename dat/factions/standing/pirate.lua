-- Pirate faction standing script
local spir = require "factions.standing.lib.pirate"
spir.init{
   fct            = faction.get("Pirate"),
   text = {
      [20] = _("Respected"),
      [0]  = _("Common Thief"),
      [-1] = _("Normie"),
   },
}

function hit( current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
