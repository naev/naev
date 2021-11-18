-- Pirate faction standing script
require "factions.standing.lib.pirate"

_fthis         = faction.get("Pirate")

_ftext_standing = {
   [20] = _("Respected"),
   [0]  = _("Common Thief"),
   [-1] = _("Normie"),
}

function faction_hit( current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
