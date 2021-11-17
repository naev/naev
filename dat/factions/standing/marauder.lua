require "factions.standing.lib.pirate"

_fthis         = faction.get("Marauder")

_fstanding_friendly = 101 -- Can't get friendly

_ftext_standing = {
   [0]  = _("Ignored"),
   [-1] = _("Potential Victim"),
}

function faction_hit( current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
