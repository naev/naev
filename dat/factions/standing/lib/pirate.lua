local class = require "class"
local pir = require 'common.pirate'
local sbase = require "factions.standing.lib.base"

local spir = {}

spir.PirateStanding = class.inheritsFrom( sbase.Standing )
function spir.newPirateStanding( args )
   return spir.PirateStanding.new():init( args )
end

function spir.PirateStanding:init(args)
   args.cap_kill           = args.cap_kill            or 30          -- Kill cap
   args.delta_distress     = args.delta_distress      or {-2, 0.25}  -- Maximum change constraints
   self.cap_misn_var       = args.cap_misn_var        or "_fcap_pirate"

   -- Secondary hit modifiers.
   args.mod_distress_enemy = args.mod_distress_enemy  or 1           -- Distress of the faction's enemies
   args.mod_distress_friend= args.mod_distress_friend or 0           -- Distress of the faction's allies
   args.mod_kill_enemy     = args.mod_kill_enemy      or 1           -- Kills of the faction's enemies
   args.mod_kill_friend    = args.mod_kill_friend     or 0           -- Kills of the faction's allies

   args.friendly_at       = args.friendly_at          or 40          -- Standing value threshold between neutral and friendly.

   args.text = args.text or {
      [95] = _("Clan Legend"),
      [80] = _("Clan Lord"),
      [60] = _("Clan Warrior"),
      [40] = _("Clan Plunderer"),
      [20] = _("Clan Thug"),
      [0]  = _("Common Thief"),
      [-1] = _("Normie"),
   }
   return sbase.Standing.init( self, args )
end

local text_bribed = _("Paid Off")

function spir.PirateStanding:text_broad( value, bribed, override )
   if bribed then
      return text_bribed
   else
      return sbase.Standing.text_broad( self, value, bribed, override )
   end
end

function spir.PirateStanding:hit( current, amount, source, secondary )
   local value = math.max( -50, sbase.Standing.hit( self, current, amount, source, secondary ) )

   -- Get the maximum player value with any pirate clan
   local maxval = value
   for k,v in ipairs(pir.factions_clans) do
      if v ~= self.fct then
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
