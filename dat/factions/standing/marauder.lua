-- Marauder faction standing script
require "factions.standing.lib.pirate"

standing = sbase.newStanding{
   fct            = faction.get("Marauder"),
   friendly_at    = 101, -- Can't get friendly
   text = {
      [0]  = _("Ignored"),
      [-1] = _("Potential Victim"),
   },
}

function standing.hit( _self, current, _amount, _source, _secondary )
   return current -- Doesn't change through hits
end
