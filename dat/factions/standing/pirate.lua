

include "dat/factions/standing/skel.lua"


_fcap_kill     = 30 -- Kill cap
_fdelta_distress = {-2, 0.25} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn     = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var = "_fcap_pirate"
_fthis         = faction.get("Pirate")

-- Secondary hit modifiers.
_fmod_distress_enemy  = 1 -- Distress of the faction's enemies
_fmod_distress_friend = 0 -- Distress of the faction's allies
_fmod_kill_enemy      = 1 -- Kills of the faction's enemies
_fmod_kill_friend     = 0 -- Kills of the faction's allies
_fmod_misn_enemy      = 0.3 -- Missions done for the faction's enemies
_fmod_misn_friend     = 0.3 -- Missions done for the faction's allies

_fstanding_friendly = 20
_fstanding_neutral = 0


_ftext_standing = {}
_ftext_standing[95] = _("Legend")
_ftext_standing[20] = _("Clansman")
_ftext_standing[0]  = _("Partner")
_ftext_standing[-1] = _("Normie")

_ftext_friendly = _("Friendly")
_ftext_neutral  = _("Neutral")
_ftext_hostile  = _("Hostile")
_ftext_bribed   = _("Paid Off")


function faction_hit( current, amount, source, secondary )
   return math.max( -20, default_hit( current, amount, source, secondary ) )
end
