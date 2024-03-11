--[[
   Fake transponder outfit

   Gives the player a way to not be killed on sight by many factions.
   Basically it replaces the standing with the factions the outfit is illegal
   to with the default standing for those factions, until the player is
   scanned. When they are scanned, their faction is restored until they perform
   a full cool-down.
--]]
local fmt = require "format"

-- This outfit applies to all factions it is illegal with!
-- Creates this table once when loading data.
local factions
function onload( o )
   factions = o:illegality()
end

local fpre = "faket_" -- Variable prefix
local function fget( f )
   return var.peek( fpre..f:nameRaw() )
end
local function fset( f, v )
   return var.push( fpre..f:nameRaw(), v )
end
local function fclear( f )
   var.pop( fpre..f:nameRaw() )
end

local function reset( p, po )
   -- Only works on the player
   if p ~= player.pilot() then return end

   for k,f in ipairs(factions) do
      local os = fget( f ) -- original standing
      local cs = f:playerStanding() -- current standing
      local ds = f:playerStandingDefault() -- default standing
      -- See how to modify saved value
      if not os then
         -- If not set, just set
         fset( f, cs )
      else
         -- Otherwise, we use negative hits until the player drops to default (in case of positive)
         local offset = cs-ds
         if offset < 0 then
            fset( f, math.min( os+offset, ds ) ) -- offset will be negative
         end
      end
      -- Reset current standing
      f:setPlayerStanding( ds )
   end
   po:state("on")
   mem.isactive = true
end

local function disable( p, po, domsg )
   -- Only works on the player
   if p ~= player.pilot() then return end

   -- Ignore if not active
   if not mem.isactive then return end

   for k,f in ipairs(factions) do
      local fval = fget( f )
      if f ~= nil then
         f:setPlayerStanding( fval )
      else
         warn(fmt.f(_("Faction '{fname}' standing not found in fake transponder variables!"),{fname=f}))
      end
      fclear( f )
   end
   po:state("off")
   if domsg then
      player.msg("#r".._("Your fake transponder has been discovered and is useless until you change systems, land, or cooldown!").."#0")
   end
   mem.isactive = false
end

function onadd( p, po )
   reset( p, po )
end

function onremove( p, po )
   disable( p, po )
end

function init( p, po )
   reset( p, po )
end

function onscanned( p, po, scanner )
   -- Only uncovered by people who care
   local fct = scanner:faction()
   if inlist( factions, fct ) then
      disable( p, po, true )
   end
end

function cooldown( p, po, done, opt )
   if not done or not opt then return end
   reset( p, po )
end
