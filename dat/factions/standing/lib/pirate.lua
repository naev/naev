local pir = require 'common.pirate'
local sbase = require "factions.standing.lib.base"

local spir = {}
friendly_at = 40 -- Lower default than sbase

function spir.init( args )
   args.cap_kill           = args.cap_kill            or 30          -- Kill cap
   args.delta_distress     = args.delta_distress      or {-2, 0.25}  -- Maximum change constraints
   args.cap_misn_var       = args.cap_misn_var        or "_fcap_pirate"

   -- Secondary hit modifiers.
   args.mod_distress_enemy = args.mod_distress_enemy  or 1           -- Distress of the faction's enemies
   args.mod_distress_friend= args.mod_distress_friend or 0           -- Distress of the faction's allies
   args.mod_kill_enemy     = args.mod_kill_enemy      or 1           -- Kills of the faction's enemies
   args.mod_kill_friend    = args.mod_kill_friend     or 0           -- Kills of the faction's allies

   args.text = args.text or {
      [95] = _("Clan Legend"),
      [80] = _("Clan Lord"),
      [60] = _("Clan Warrior"),
      [40] = _("Clan Plunderer"),
      [20] = _("Clan Thug"),
      [0]  = _("Common Thief"),
      [-1] = _("Normie"),
   }
   args.text_bribed  = _("Paid Off")
   return sbase.init( args )
end

-- Override hit function
local oldhit = hit
function hit( current, amount, source, secondary )
   local value = math.max( -50, oldhit( current, amount, source, secondary ) )

   -- Get the maximum player value with any pirate clan
   local maxval = value
   for k,v in ipairs(pir.factions_clans) do
      if v ~= sbase.fct then
         local vs = v:playerStanding() -- Only get first parameter
         maxval = math.max( maxval, vs )
      end
   end

   -- Update pirate and marauder standings
   pir.updateStandings( maxval )

   -- Set current faction standing
   return value
end

return spir
