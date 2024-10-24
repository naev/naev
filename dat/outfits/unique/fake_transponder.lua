--[[
   Fake transponder outfit

   Gives the player a way to not be killed on sight by many factions.
   Basically it replaces the standing with the factions the outfit is illegal
   to with the default standing for those factions, until the player is
   scanned. When they are scanned, their faction is restored until they perform
   a full cool-down.
--]]
-- This outfit applies to all factions it is illegal with!
-- Creates this table once when loading data.
local factions
function onload( o )
   factions = o:illegality()
end

local function reset( p, po )
   -- Only works on the player
   if p ~= player.pilot() then return end

   for k,f in ipairs(factions) do
      local ds = f:reputationDefault() -- default standing
      local os = f:reputationOverride()
      if not os then
         f:setReputationOverride( ds )
      end
   end
   po:state("on")
   mem.isactive = true
end

local function disable( p, po, domsg )
   -- Only works on the player
   if p ~= player.pilot() then return end

   -- Ignore if not active
   if not mem.isactive then return end

   -- Clear overrides
   for k,f in ipairs(factions) do
      local ds = f:reputationDefault() -- default standing
      local os = f:reputationOverride()
      if os==ds then
         f:setReputationOverride()
      end
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
