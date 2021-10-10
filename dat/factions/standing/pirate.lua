require "factions.standing.lib.pirate"

_fthis         = faction.get("Pirate")

_ftext_standing = {}
_ftext_standing[20] = _("Respected")
_ftext_standing[0]  = _("Common Thief")
_ftext_standing[-1] = _("Normie")

function faction_hit( current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
