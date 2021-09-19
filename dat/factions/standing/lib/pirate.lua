local pir = require 'common.pirate'
require "factions.standing.lib.base"

_fcap_kill     = 30 -- Kill cap
_fdelta_distress = {-2, 0.25} -- Maximum change constraints
_fdelta_kill     = {-5, 1} -- Maximum change constraints
_fcap_misn     = 30 -- Starting mission cap, gets overwritten
_fcap_misn_var = "_fcap_pirate"
--_fthis         = faction.get("Pirate") -- Should be set seperately

-- Secondary hit modifiers.
_fmod_distress_enemy  = 1 -- Distress of the faction's enemies
_fmod_distress_friend = 0 -- Distress of the faction's allies
_fmod_kill_enemy      = 1 -- Kills of the faction's enemies
_fmod_kill_friend     = 0 -- Kills of the faction's allies
_fmod_misn_enemy      = 0.3 -- Missions done for the faction's enemies
_fmod_misn_friend     = 0.3 -- Missions done for the faction's allies

_fstanding_friendly = 40
_fstanding_neutral = 0

_ftext_standing = {}
_ftext_standing[95] = _("Clan Legend")
_ftext_standing[80] = _("Clan Lord")
_ftext_standing[60] = _("Clan Warrior")
_ftext_standing[40] = _("Clan Plunderer")
_ftext_standing[20] = _("Clan Thug")
_ftext_standing[0]  = _("Common Thief")
_ftext_standing[-1] = _("Normie")

_ftext_friendly = _("Friendly")
_ftext_neutral  = _("Neutral")
_ftext_hostile  = _("Hostile")
_ftext_bribed   = _("Paid Off")

local faction_pirate = faction.get("Pirate")
local faction_marauder = faction.get("Marauder")

function faction_hit( current, amount, source, secondary )
   local standing = math.max( -50, default_hit( current, amount, source, secondary ) )

   -- Get the maximum player standing with any pirate clan
   local maxval = standing
   for k,v in ipairs(pir.factions_clans) do
      if v ~= _fthis then
         local vs = v:playerStanding() -- Only get first parameter
         maxval = math.max( maxval, vs )
      end
   end

   -- Pirates and marauders are fixed offsets
   if pir.isPirateShip( player.pilot() ) then
      faction_pirate:setPlayerStanding(   maxval - 10 )
      faction_marauder:setPlayerStanding( maxval - 20 )
   else
      faction_pirate:setPlayerStanding(   maxval - 20 )
      faction_marauder:setPlayerStanding( maxval - 40 )
   end

   -- Set current faction standing
   return standing
end
