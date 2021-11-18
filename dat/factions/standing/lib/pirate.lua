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

_ftext_standing = {
   [95] = _("Clan Legend"),
   [80] = _("Clan Lord"),
   [60] = _("Clan Warrior"),
   [40] = _("Clan Plunderer"),
   [20] = _("Clan Thug"),
   [0]  = _("Common Thief"),
   [-1] = _("Normie"),
}

local default_standing_broad = faction_standing_broad
local text_bribed = _("Paid Off")
function faction_standing_broad( standing, bribed, override )
   if bribed then
      return text_bribed
   else
      return default_standing_broad( standing, bribed, override )
   end
end

local default_hit = faction_hit
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

   -- Update pirate and marauder standings
   pir.updateStandings( maxval )

   -- Set current faction standing
   return standing
end
